# iobench
C++ I/O library test.

For AWS linux server:
install liburing through https://github.com/axboe/liburing.
install spdk through https://github.com/spdk/spdk.

Build
```
mkdir build
cd build
cmake ..
make
```


Generate data on disk:

```
mkdir data
./build/gendata ./data/file_1MB.bin 1048576
./build/gendata ./data/file_512MB.bin 536870912
stat ./data/file_1MB.bin
stat ./data/file_512MB.bin
```

Test pread:
```
./build/pread ./data/file_1MB.bin 1
./build/pread ./data/file_512MB.bin 1
```


Test aio:


Test io_uring:


Test spdk:

