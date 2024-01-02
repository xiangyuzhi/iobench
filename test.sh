#!/bin/bash
cd build
cmake ..
make
cd ..

# Sync lib
echo "Test pread library"
for lib in "pread";
do
  for thd in 1 4 16 32;
  do
    for block in 4 16 64;
    do
      s="./build/${lib} /data/file_1GB.bin ${thd} ${block}"
      echo ${s}
      $s
      echo "------------------------------------------"
    done
  done
done

# Async lib
echo "Test aio library"
for lib in "aio";
do
  for thd in 1 4 16 32;
  do
    s="./build/${lib} /data/file_1GB.bin ${thd}"
    echo ${s}
    $s
    echo "------------------------------------------"
  done
done

echo "Test io_uring library"
for lib in "iouring";
do
  for thd in 1 2 4 8 16 32 64 128;
  do
    s="./build/${lib} /data/file_1GB.bin ${thd}"
    echo ${s}
    $s
    echo "------------------------------------------"
  done
done


