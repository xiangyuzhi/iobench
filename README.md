# iobench
C++ I/O library test.


build
```
mkdir build
cd build
cmake ..
make
```

data size:
1048576 1MB
536870912 512MB

generate data on disk:

```
mkdir data
./build/gendata ./data/output_file.bin 1048576
./build/gendata ./data/output_file2.bin 536870912
stat ./data/output_file.bin
stat ./data/output_file2.bin
```

Test pread:
```
./build/pread ./data/output_file.bin 1
./build/pread ./data/output_file2.bin 1
```


