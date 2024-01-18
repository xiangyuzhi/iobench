#!/bin/bash

# fio -ioengine=libaio -bs=128k -direct=1 -thread -numjobs=2 -rw=read -filename=/data/file_1GB.bin -name="BS 16KB read test" -iodepth=512 -runtime=20

libarray=()
rwarray=()
thdarray=(1 4 8 16 32)
blockarray=(4 16 64 128 512)

# aio & iouring
for lib in "libaio" "io_uring";
do
  for rw in "read" "randread";
  do
    for block in 4 16 32 64 128 512;
    do
      for thd in 1 2 4 8 16 32;
      do
        bs="${block}k"
        (fio -ioengine=$lib -bs=$bs -direct=1 -thread -numjobs=$thd -rw=$rw -filename=/data/file_1GB.bin -name="[[BS-${block}KB-${rw}-test]]" -iodepth=512 -runtime=20 | grep "READ: bw=" | awk 'match($0, /[0-9]+(\.[0-9]+)?MiB\/s/) {bw=substr($0, RSTART, RLENGTH-5)} END {print "'$lib'"",", "'$rw'"",", "'$thd'"",", "'$block'"",", bw}') >> ../output.txt
      done
    done
  done
done

# lib, mode, thread_num, block_size, iops, bandwidth