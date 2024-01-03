#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

uint64_t time_diff(struct timespec *start, struct timespec *end) {
  return (end->tv_sec - start->tv_sec) * 1000000000 +
         (end->tv_nsec - start->tv_nsec);
}

void test_pread_seq(const std::string &file_path, int thread_num,
                    uint32_t block_size) {
  std::ifstream is(file_path, std::ifstream::binary | std::ifstream::ate);
  std::size_t file_size = is.tellg();
  is.close();

  int fd = open(file_path.c_str(), O_RDONLY | O_DIRECT);

  size_t reminder = file_size % block_size;
  size_t buf_size =
      reminder == 0 ? file_size : file_size - reminder + block_size;
  char *buf = (char *)aligned_alloc(block_size, buf_size);
  auto num_blocks = static_cast<int>((buf_size + block_size - 1) / block_size);
  // std::cout << block_size / 1024 << "KB " << std::endl;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

#pragma omp parallel for num_threads(thread_num)
  for (int i = 0; i < num_blocks; ++i) {
    size_t read_size =
        i == (num_blocks - 1) ? buf_size - i * block_size : block_size;
    if (pread(fd, buf + i * block_size, read_size, i * block_size) == -1) {
      printf("error\n");
    }
  }

  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  auto duration = time_diff(&start, &end) / 1000000.0;
  uint64_t bandwidth = (buf_size / (1024 * 1024)) / (duration / 1000);
  uint64_t *array = (uint64_t *)buf;
  uint64_t sum = 0;
  for (int i = 0; i < buf_size / sizeof(uint64_t); ++i) {
    sum ^= array[i];
  }
  printf("%lu, %.3fms, %luMB/s\n", sum, duration, bandwidth);
  free(buf);
  close(fd);
}

void test_pread_randread(const std::string &file_path, int thread_num,
                         uint32_t block_size) {
  std::ifstream is(file_path, std::ifstream::binary | std::ifstream::ate);
  std::size_t file_size = is.tellg();
  is.close();

  int fd = open(file_path.c_str(), O_RDONLY | O_DIRECT);

  int num_read = 2000;
  size_t buf_size = block_size * num_read;
  char *buf = (char *)aligned_alloc(block_size, buf_size);
  // std::cout << block_size / 1024 << "KB " << std::endl;
  auto num_blocks = file_size / block_size;
  int offset[num_read];
  for (int i = 0; i < num_read; i++) {
    offset[i] = rand() % num_blocks;
  }

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

#pragma omp parallel for num_threads(thread_num)
  for (int i = 0; i < num_read; ++i) {
    if (pread(fd, buf + i * block_size, block_size, offset[i] * block_size) ==
        -1) {
      printf("error\n");
    }
  }

  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  auto duration = time_diff(&start, &end) / 1000000.0;
  uint64_t bandwidth = (buf_size / (1024 * 1024)) / (duration / 1000);
  uint64_t *array = (uint64_t *)buf;
  uint64_t sum = 0;
  for (int i = 0; i < buf_size / sizeof(uint64_t); ++i) {
    sum ^= array[i];
  }
  printf("%lu, %.3fms, %luMB/s\n", sum, duration, bandwidth);
  free(buf);
  close(fd);
}

int main(int argc, char *argv[]) {

  if (argc != 5) {
    std::cerr << "Usage: " << argv[0]
              << "<filename> <is_random>  <thread_num> <block_size KB>"
              << std::endl;
  }
  const std::string filename = argv[1];
  int rand = atoi(argv[2]);
  int t = atoi(argv[3]);
  int block = atoi(argv[4]) * 1024;
  if (rand) {
    test_pread_randread(filename, t, block);
  } else {
    test_pread_seq(filename, t, block);
  }
}