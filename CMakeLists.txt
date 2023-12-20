cmake_minimum_required(VERSION 3.18)
project(iotest)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_FLAGS "-g -O3 -fopenmp -shared-libgcc -luring")


add_executable(gendata generate_data.cc)

add_executable(pread pread_test.cc)

find_library(RT_LIB rt)
if (NOT RT_LIB)
    message(FATAL_ERROR "librt not found")
endif()

add_executable(aio aio_test.cc)
target_link_libraries(aio PRIVATE ${RT_LIB})


add_executable(iouring iouring_test.cc)
target_link_libraries(iouring uring)