## DwaRF-LSM: Leveraging Disaggregated Memory for Fast LSM-Based Storage Engine in Disaggregated Infrastructure

Log-structured merge-tree-based key-value stores (LSM-KVS) are widely used as write-optimized persistent storage engines for data-intensive applications. In recent years, disaggregated infrastructure has become the trend in data centers to achieve better resource utilization and management, and existing studies are focusing on optimizing LSM-KVS for disaggregated storage. However, the limited and imbalanced CPU, memory, and I/O resources among compute nodes still exist and cause explicit performance issues, especially in the write path.
In this paper, we propose leveraging the shared disaggregated memory (DM) to address the aforementioned issues of the LSM-based storage engine, called DwaRF-LSM. First, we propose a novel DM-optimized memtable and dynamically offload them to DM to achieve write buffer extension with the fast transmission. Second, to address the performance challenges of accessing and searching memtables at DM, DwaRF-LSM proposes adaptive Read Delegation at DM using local CPUs. Third, DwaRF-LSM proposes Shard-Level Remote Flush, leveraging the sharing capability of memtables at DM to enable flexible and fine-grained flush execution on idle compute nodes, which effectively enhances write performance and improves cluster load balancing. We implemented DwaRF-LSM based on RocksDB and evaluated it in various tests with different workloads. Our evaluation shows that DwaRF-LSM effectively utilizes DM and outperforms Disaggregating-RocksDB (developed by Meta) by up to 136%.

---



## RocksDB: A Persistent Key-Value Store for Flash and RAM Storage

[![CircleCI Status](https://circleci.com/gh/facebook/rocksdb.svg?style=svg)](https://circleci.com/gh/facebook/rocksdb)
[![Appveyor Build status](https://ci.appveyor.com/api/projects/status/fbgfu0so3afcno78/branch/main?svg=true)](https://ci.appveyor.com/project/Facebook/rocksdb/branch/main)
[![PPC64le Build Status](http://140-211-168-68-openstack.osuosl.org:8080/buildStatus/icon?job=rocksdb&style=plastic)](http://140-211-168-68-openstack.osuosl.org:8080/job/rocksdb)

RocksDB is developed and maintained by Facebook Database Engineering Team.
It is built on earlier work on [LevelDB](https://github.com/google/leveldb) by Sanjay Ghemawat (sanjay@google.com)
and Jeff Dean (jeff@google.com)

This code is a library that forms the core building block for a fast
key-value server, especially suited for storing data on flash drives.
It has a Log-Structured-Merge-Database (LSM) design with flexible tradeoffs
between Write-Amplification-Factor (WAF), Read-Amplification-Factor (RAF)
and Space-Amplification-Factor (SAF). It has multi-threaded compactions,
making it especially suitable for storing multiple terabytes of data in a
single database.

Start with example usage here: https://github.com/facebook/rocksdb/tree/main/examples

See the [github wiki](https://github.com/facebook/rocksdb/wiki) for more explanation.

The public interface is in `include/`.  Callers should not include or
rely on the details of any other header files in this package.  Those
internal APIs may be changed without warning.

Questions and discussions are welcome on the [RocksDB Developers Public](https://www.facebook.com/groups/rocksdb.dev/) Facebook group and [email list](https://groups.google.com/g/rocksdb) on Google Groups.

## License

RocksDB is dual-licensed under both the GPLv2 (found in the COPYING file in the root directory) and Apache 2.0 License (found in the LICENSE.Apache file in the root directory).  You may select, at your option, one of the above-listed licenses.
