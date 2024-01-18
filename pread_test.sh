#!/bin/bash
cd build
cmake ..
make
cd ..

# Sync lib
echo "Test pread library"
for rw in 0 1;
do
  for block in 4 16 32 64 128 512;
  do
    for thd in 1 2 4 8 16 32;
    do
      s="./build/pread /data/file_1GB.bin ${rw} ${thd} ${block}"
      echo ${s}
      echo "------------------------------------------"
      if [ $rw -eq 0 ]
      then 
        a="read"
      else 
        a="randread"
      fi
      ($s | awk 'match($0, /[0-9]+(\.[0-9]+)?MB\/s/) {bw=substr($0, RSTART, RLENGTH-4)} END {print "pread,", "'$a'"",", "'$thd'"",", "'$block'"",", bw}') >> ../output.txt
      
    done
  done
done

# Async lib
# echo "Test aio library"
# for lib in "aio";
# do
#   for thd in 1 4 16 32;
#   do
#     s="./build/${lib} /data/file_1GB.bin ${thd}"
#     echo ${s}
#     $s
#     echo "------------------------------------------"
#   done
# done

# echo "Test io_uring library"
# for lib in "iouring";
# do
#   for thd in 1 2 4 8 16 32 64 128;
#   do
#     s="./build/${lib} /data/file_1GB.bin ${thd}"
#     echo ${s}
#     $s
#     echo "------------------------------------------"
#   done
# done


