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
cd build
./gendata ../data/file_128MB.bin 128
./gendata ../data/file_512MB.bin 512
./gendata ../data/file_1GB.bin 1024
stat ../data/file_128MB.bin
stat ../data/file_512MB.bin
stat ../data/file_1GB.bin
```

Test pread:
```
./pread ../data/file_128MB.bin 1
./pread ../data/file_512MB.bin 1
```


Test aio:
```
./aio ../data/file_128MB.bin 1
./aio ../data/file_128MB.bin 4
```

Expect output:
```
8373570978799532707, 693.795523ms, 0.180168GB/s
8373570978799532707, 366.275577ms, 0.341273GB/s
```


Test io_uring:


Test spdk:

