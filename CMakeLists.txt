cmake_minimum_required(VERSION 3.18)
project(iotest)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(gendata generate_data.cc)

add_executable(pread pread_test.cc)




