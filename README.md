# iobench
C++ I/O library test.

Test Disk IO Libraries:
- pread
- AIO
- io_uring
- spdk

## Report
[C++ Library Benchmark Report](https://quip-amazon.com/Nz0GABxvGipG/Methods-to-Saturate-the-Full-Bandwidth-of-NVMe-SSD)

## For AWS linux server

Install [liburing](https://github.com/axboe/liburing).

Install [spdk](https://github.com/spdk/spdk).

Follow the [user guide](https://docs.aws.amazon.com/zh_cn/AWSEC2/latest/UserGuide/ebs-using-volumes.html) to config your machine and mount device.

Remember to check your nvme SSD device: 
```bash
sudo lsblk -f
```

## Build
```bash
mkdir build
cd build
cmake ..
make
```


## Generate data on disk
```bash
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
```bash
sudo apt install fio
fio -ioengine=libaio -bs=16k -direct=1 -thread -numjobs=1 -rw=read -filename=/data/file_1GB.bin -name="BS 16KB read test" -iodepth=512 -runtime=20
fio -ioengine=libaio -bs=64k -direct=1 -thread -numjobs=1 -rw=read -filename=/data/file_1GB.bin -name="BS 64KB read test" -iodepth=512 -runtime=20
fio -ioengine=libaio -bs=64k -direct=1 -thread -numjobs=8 -rw=read -filename=/data/file_1GB.bin -name="BS 64KB read test" -iodepth=512 -runtime=20
```
The bandwidth should between 2000-3500 MB/s.

## Usage
```
$ ./<lib> <file> <thread_num> <block size>
```

Or run the script:
```bash
bash ./test.sh
```


### Test pread:
```bash
./pread /data/file_1GB.bin 1 16
./pread /data/file_1GB.bin 8 16
./pread /data/file_1GB.bin 8 64
```

Expect output:
```
5570568009356738390, 2536.537ms, 0.394GB/s
5570568009356738390, 789.062ms, 1.267GB/s
5570568009356738390, 350.578ms, 2.852GB/s
5570568009356738390, 314.861ms, 3.176GB/s
```

### Test aio:
```bash
./aio /data/file_1GB.bin 1
./aio /data/file_1GB.bin 8
```

Expect output:
```
5570568009356738390, 554.996ms, 1.802GB/s
5570568009356738390, 582.408ms, 1.717GB/s
```


### Test io_uring:
```bash
./iouring /data/file_1GB.bin 1
./iouring /data/file_1GB.bin 4
./iouring /data/file_1GB.bin 16
```

Expect output:
```
5570568009356738390, 1154.126ms, 0.866GB/s
5570568009356738390, 833.122ms, 1.200GB/s
5570568009356738390, 774.006ms, 1.292GB/s
```


### Test spdk:


