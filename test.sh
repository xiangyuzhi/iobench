#!/bin/bash

# Sync lib
# for lib in "pread";
# do
#   for data in "./data/file_128MB.bin" "./data/file_512MB.bin" "./data/file_1GB.bin";
#   do 
#     for thd in 1 2 4 8;
#     do
#       for block in 4 8 16 32 64;
#       do
#         s="./build/${lib} ${data} ${thd} ${block}"
#         echo ${s}
#         $s
#      done
#     done
#   done
# done

# Async lib

for lib in "aio";
do
  for data in "./data/file_128MB.bin" "./data/file_512MB.bin" "./data/file_1GB.bin";
  do
    for thd in 1 2 4 8;
    do
      s="./build/${lib} ${data} ${thd}"
      echo ${s}
      $s
    done
  done
done

for lib in "iouring";
do
  for data in "./data/file_128MB.bin" "./data/file_512MB.bin" "./data/file_1GB.bin";
  do
    for thd in 1 2 4 8 16 32 64 128;
    do
      s="./build/${lib} ${data} ${thd}"
      echo ${s}
      $s
    done
  done
done


fio -ioengine=libaio -bs=4k -direct=1 -thread -numjobs=8 -rw=read -filename=/home/ubuntu/dgl/iobench/data/file_1GB.bin -name="BS 4KB read test" -iodepth=512 -runtime=60


fio -ioengine=libaio -bs=16k -direct=1 -thread -numjobs=8 -rw=read -filename=/home/ubuntu/dgl/iobench/data/file_1GB.bin -name="BS 16KB read test" -iodepth=512


fio -ioengine=libaio -bs=64k -direct=1 -thread -numjobs=8 -rw=read -filename=/home/ubuntu/dgl/iobench/data/file_1GB.bin -name="BS 16KB read test" -iodepth=512 -runtime=20

fio -ioengine=libaio -bs=128k -direct=1 -thread -numjobs=8 -rw=read -filename=/home/ubuntu/dgl/iobench/data/file_1GB.bin -name="BS 16KB read test" -iodepth=512 -runtime=20

