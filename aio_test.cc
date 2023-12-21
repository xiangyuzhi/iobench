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

  struct aiocb rd[thread_num];
  char *buf = (char *)aligned_alloc(BUFFER_SIZE, file_size);

  off_t per_thd_len = file_size / thread_num;
  for (int i = 0; i < thread_num; i++) {
    bzero(&rd[i], sizeof(struct aiocb));
    rd[i].aio_buf = malloc(BUFFER_SIZE + 1);
    rd[i].aio_fildes = fd;
    rd[i].aio_nbytes = BUFFER_SIZE;
    rd[i].aio_offset = i * per_thd_len;
  }
  off_t cnt_read[thread_num];

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

#pragma omp parallel for num_threads(thread_num)
  for (int i = 0; i < thread_num; i++) {
    cnt_read[i] = 0;
    do {
      auto ret = aio_read(&rd[i]);

      if (ret < 0) {
        perror("aio_read");
        exit(1);
      }

      while (aio_error(&rd[i]) == EINPROGRESS)
        ;
      ret = aio_return(&rd[i]);
      memcpy(buf + i * per_thd_len + cnt_read[i], (char *)rd[i].aio_buf, ret);
      rd[i].aio_offset += ret;
      cnt_read[i] += ret;

    } while (cnt_read[i] < per_thd_len);
    printf("thread %d read %ld MB\n", i, cnt_read[i] / 1024 / 1024);
  }
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  auto duration = time_diff(&start, &end) / 1000000.0;
  auto bandwidth = (file_size / (1024.0 * 1024.0 * 1024.0)) / (duration / 1000);
  uint64_t *array = (uint64_t *)buf;
  uint64_t sum = 0;
  for (int i = 0; i < file_size / sizeof(uint64_t); ++i) {
    sum += array[i];
  }
  printf("%lu, %fms, %fGB/s\n", sum, duration, bandwidth);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << "<filename> <thread_num>" << std::endl;
  }

  const std::string filename = argv[1];
  int t = atoi(argv[2]);

  aio(filename, t);
}
