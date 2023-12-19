# iobench
C++ I/O library test.


build
```
mkdir build
cd build
cmake ..
make
```


generate data on disk:

```
mkdir data
./build/gendata ./data/file_1MB.bin 1048576
./build/gendata ./data/file_512MB.bin 536870912
stat ./data/output_file.bin
stat ./data/output_file2.bin
```

Test pread:
```
./build/pread ./data/file_1MB.bin 1
./build/pread ./data/file_512MB.bin 1
```


