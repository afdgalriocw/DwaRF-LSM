//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

// Arena is an implementation of Allocator class. For a request of small size,
// it allocates a block with pre-defined block size. For a request of big
// size, it uses malloc to directly get the requested size.

#pragma once

#include <cassert>
#include <cstddef>
#include <deque>

#include "memory/allocator.h"
#include "port/mmap.h"
#include "rocksdb/env.h"
#include "rocksdb/remote_flush_service.h"

namespace ROCKSDB_NAMESPACE {

class Arena : public BasicArena {
 public:
  void PackLocal(TransferService* node) const override;
  static void* UnPackLocal(TransferService* node);
  Status SendToRemote() const override;
  void get_remote_page_info(uint64_t* info) const override;

 public:
  // No copying allowed
  Arena(const Arena&) = delete;
  void operator=(const Arena&) = delete;

  const char* name() const override { return "Arena"; }

  static constexpr size_t kInlineSize = 0;
  static constexpr size_t kMinBlockSize = 4096;
  static constexpr size_t kMaxBlockSize = 2u << 30;

  static constexpr unsigned kAlignUnit = alignof(std::max_align_t);
  static_assert((kAlignUnit & (kAlignUnit - 1)) == 0,
                "Pointer size should be power of 2");

  // huge_page_size: if 0, don't use huge page TLB. If > 0 (should set to the
  // supported hugepage size of the system), block allocation will try huge
  // page TLB first. If allocation fails, will fall back to normal case.
  explicit Arena(size_t block_size = kMinBlockSize,
                 AllocTracker* tracker = nullptr, size_t huge_page_size = 0,
                 RDMAClient* client = nullptr,
                 RDMANode::rdma_connection* conn = nullptr);
  ~Arena() override;

  char* Allocate(size_t bytes) override;

  // huge_page_size: if >0, will try to allocate from huage page TLB.
  // The argument will be the size of the page size for huge page TLB. Bytes
  // will be rounded up to multiple of the page size to allocate through mmap
  // anonymous option with huge page on. The extra  space allocated will be
  // wasted. If allocation fails, will fall back to normal case. To enable it,
  // need to reserve huge pages for it to be allocated, like:
  //     sysctl -w vm.nr_hugepages=20
  // See linux doc Documentation/vm/hugetlbpage.txt for details.
  // huge page allocation can fail. In this case it will fail back to
  // normal cases. The messages will be logged to logger. So when calling with
  // huge_page_tlb_size > 0, we highly recommend a logger is passed in.
  // Otherwise, the error message will be printed out to stderr directly.
  char* AllocateAligned(size_t bytes, size_t huge_page_size = 0,
                        Logger* logger = nullptr) override;

  // Returns an estimate of the total memory usage of data allocated
  // by the arena (exclude the space allocated but not yet used for future
  // allocations).
  size_t ApproximateMemoryUsage() const override {
    return blocks_memory_ + blocks_.size() * sizeof(char*) -
           alloc_bytes_remaining_;
  }

  size_t MemoryAllocatedBytes() const override { return blocks_memory_; }

  size_t AllocatedAndUnused() const override { return alloc_bytes_remaining_; }

  // If an allocation is too big, we'll allocate an irregular block with the
  // same size of that allocation.
  size_t IrregularBlockNum() const override { return irregular_block_num; }

  size_t BlockSize() const override { return kBlockSize; }

  size_t RawDataUsage() const override {
    return kBlockSize - (aligned_alloc_ptr_ - mem_begin_) -
           (mem_begin_ + kBlockSize - unaligned_alloc_ptr_);
  }

  bool IsInInlineBlock() const override {
    return blocks_.empty() && huge_blocks_.empty();
  }

  const void* MemBegin() const override {
    return reinterpret_cast<const void*>(mem_begin_);
  }

  // check and adjust the block_size so that the return value is
  //  1. in the range of [kMinBlockSize, kMaxBlockSize].
  //  2. the multiple of align unit.
  static size_t OptimizeBlockSize(size_t block_size);

 private:
  alignas(std::max_align_t) char inline_block_[kInlineSize];
  // Number of bytes allocated in one block
  const size_t kBlockSize;
  // Allocated memory blocks
  std::deque<std::unique_ptr<char[]>> blocks_;
  // Huge page allocations
  std::deque<MemMapping> huge_blocks_;
  size_t irregular_block_num = 0;

  // Stats for current active block.
  // For each block, we allocate aligned memory chucks from one end and
  // allocate unaligned memory chucks from the other end. Otherwise the
  // memory waste for alignment will be higher if we allocate both types of
  // memory from one direction.
  char* unaligned_alloc_ptr_ = nullptr;
  char* aligned_alloc_ptr_ = nullptr;
  const char* mem_begin_;
  // How many bytes left in currently active block?
  size_t alloc_bytes_remaining_ = 0;

  size_t hugetlb_size_ = 0;

  char* AllocateFromHugePage(size_t bytes);
  char* AllocateFallback(size_t bytes, bool aligned);
  char* AllocateNewBlock(size_t block_bytes);

  // Bytes of memory in blocks allocated so far
  size_t blocks_memory_ = 0;
  // Non-owned
  AllocTracker* tracker_;
  RDMAClient* client_ = nullptr;
  RDMANode::rdma_connection* conn_ = nullptr;
  std::pair<int64_t, int64_t> remote_reg_mem = {0, 0};
};

inline char* Arena::Allocate(size_t bytes) {
  // The semantics of what to return are a bit messy if we allow
  // 0-byte allocations, so we disallow them here (we don't need
  // them for our internal use).
  assert(bytes > 0);
  if (bytes <= alloc_bytes_remaining_) {
    unaligned_alloc_ptr_ -= bytes;
    alloc_bytes_remaining_ -= bytes;
    return unaligned_alloc_ptr_;
  }
  return AllocateFallback(bytes, false /* unaligned */);
}

}  // namespace ROCKSDB_NAMESPACE
