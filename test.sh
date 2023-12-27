#!/bin/bash

# Sync lib
# for lib in "pread";
# do
#   for thd in 1 2 4 8;
#   do
#     for data in "./data/file_128MB.bin" "./data/file_512MB.bin" "./data/file_1GB.bin"
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
for lib in "aio" "iouring";
do
  for thd in 1 2 4 8;
  do
    for data in "./data/file_128MB.bin" "./data/file_512MB.bin" "./data/file_1GB.bin"
    do
      s="./build/${lib} ${data} ${thd}"
      echo ${s}
      $s
    done
  done
done




