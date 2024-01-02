# iobench
C++ I/O library test.

Test Disk IO Libraries:
- pread
- AIO
- io_uring
- spdk


For AWS linux server:

Install liburing through https://github.com/axboe/liburing.

Install spdk through https://github.com/spdk/spdk.

Follow the [user guide](https://docs.aws.amazon.com/zh_cn/AWSEC2/latest/UserGuide/ebs-using-volumes.html) to config your machine and mount your nvme device.

Remember to check your nvme SSD device: sudo lsblk -f

Build
```
mkdir build
cd build
cmake ..
make
```


Generate data on disk:

```
mkdir /data
cd build
./gendata /data/file_128MB.bin 128
./gendata /data/file_512MB.bin 512
./gendata /data/file_1GB.bin 1024
stat /data/file_128MB.bin
stat /data/file_512MB.bin
stat /data/file_1GB.bin
```

Use fio to test and verify your SSD bandwidth:
```
sudo apt install fio
fio -ioengine=libaio -bs=64k -direct=1 -thread -numjobs=1 -rw=read -filename=/data/file_1GB.bin -name="BS 64KB read test" -iodepth=512 -runtime=20
fio -ioengine=libaio -bs=64k -direct=1 -thread -numjobs=8 -rw=read -filename=/data/file_1GB.bin -name="BS 128KB read test" -iodepth=512 -runtime=20
```
The bandwidth should be 2000-3500 MB/s.

Usage:
```
Usage: $ ./pread <file> <thread_num>
```

Test pread:
```
./pread /data/file_128MB.bin 1 16
./pread /data/file_128MB.bin 4 16
./pread /data/file_128MB.bin 8 16
```

Expect output:
```
148511787354331535, 103.839903ms, 1.203776GB/s
148511787354331535, 23.065304ms, 5.419395GB/s
148511787354331535, 15.298945ms, 8.170498GB/s
```

Test aio:
```
./aio /data/file_128MB.bin 1
./aio /data/file_128MB.bin 4
```

Expect output:
```
148511787354331535, 65.604999ms, 1.905343GB/s
148511787354331535, 67.069362ms, 1.863742GB/s
```


Test io_uring:
```
./iouring /data/file_128MB.bin 1
./iouring /data/file_128MB.bin 4
```

Expect output:
```
148511787354331535, 115.360530ms, 1.083560GB/s
148511787354331535, 106.383809ms, 1.174991GB/s
```


Test spdk:
