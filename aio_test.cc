#include <aio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

uint64_t time_diff(struct timespec *start, struct timespec *end) {
  return (end->tv_sec - start->tv_sec) * 1000000000 +
         (end->tv_nsec - start->tv_nsec);
}

void aio(const std::string &file_path, int thread_num) {

  std::ifstream is(file_path, std::ifstream::binary | std::ifstream::ate);
  std::size_t file_size = is.tellg();
  is.close();

  int fd = open(file_path.c_str(), O_RDONLY);

  if (fd < 0) {
    perror("file error\n");
  }

  char *buf = (char *)aligned_alloc(BUFFER_SIZE, file_size);

  // size_t block_size = BUFFER_SIZE * thread_num;
  // auto num_blocks = static_cast<int>((file_size + block_size - 1) /
  // block_size); std::cout << block_size / 1024 << "KB " << block_size / (1024
  // * 1024) << "MB"
  //           << std::endl;

  off_t per_thd_len = file_size / thread_num;
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  // #pragma omp parallel for num_threads(thread_num)
  //   for (int i = 0; i < num_blocks; ++i) {
  //     size_t read_size =
  //         i == (num_blocks - 1) ? file_size - i * block_size : block_size;
  //     struct aiocb rd;
  //     bzero(&rd, sizeof(struct aiocb));
  //     rd.aio_buf = buf + i * block_size;
  //     rd.aio_fildes = fd;
  //     rd.aio_nbytes = block_size;
  //     rd.aio_offset = i * block_size;
  //     off_t cnt_read = 0;
  //     auto ret = aio_read(&rd);

  //     if (ret < 0) {
  //       perror("aio_read");
  //       exit(1);
  //     }

  //     while (aio_error(&rd) == EINPROGRESS)
  //       ;
  //     ret = aio_return(&rd);
  //   }

#pragma omp parallel for num_threads(thread_num)
  for (int i = 0; i < thread_num; i++) {
    struct aiocb rd;
    bzero(&rd, sizeof(struct aiocb));
    rd.aio_buf = buf + i * per_thd_len;
    rd.aio_fildes = fd;
    rd.aio_nbytes = per_thd_len;
    rd.aio_offset = i * per_thd_len;
    off_t cnt_read = 0;
    do {
      auto ret = aio_read(&rd);

      if (ret < 0) {
        perror("aio_read");
        exit(1);
      }

      while (aio_error(&rd) == EINPROGRESS)
        ;
      ret = aio_return(&rd);
      rd.aio_offset += ret;
      cnt_read += ret;

    } while (cnt_read < per_thd_len);
    printf("thread %d read %ld MB\n", i, cnt_read / 1024 / 1024);
  }
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  auto duration = time_diff(&start, &end) / 1000000.0;
  auto bandwidth = (file_size / (1024.0 * 1024.0 * 1024.0)) / (duration / 1000);
  uint64_t *array = (uint64_t *)buf;
  uint64_t sum = 0;
  for (int i = 0; i < file_size / sizeof(uint64_t); ++i) {
    sum ^= array[i];
  }
  printf("%lu, %.3fms, %.3fGB/s\n", sum, duration, bandwidth);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << "<filename> <thread_num>" << std::endl;
  }

  const std::string filename = argv[1];
  int t = atoi(argv[2]);

  aio(filename, t);
}
