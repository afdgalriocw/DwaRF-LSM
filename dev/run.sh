#!/bin/bash
PROJECT_ROOT="$(pwd)/../"
cd $PROJECT_ROOT/dev
rm -rf trace
rm -rf ./db/*
cd $PROJECT_ROOT
mkdir build
cd $PROJECT_ROOT/build
if [[ "$1" == "1" ]]; then
    make clean
    rm -rf ./*
    rm -rf .cmake
fi
cmake -DCMAKE_BUILD_TYPE=Release ..
make rdma_server -j $(nproc)
make remote_flush_worker -j $(nproc)
make db_bench -j $(nproc)

cd $PROJECT_ROOT/dev

cp ../build/rdma_server ./
cp ../build/remote_flush_worker ./
cp ../build/db_bench ./

rm Log-*

chown -R $(whoami) ../
