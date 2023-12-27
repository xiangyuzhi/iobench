#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#define BUFFER_SIZE 4096

uint64_t time_diff(struct timespec *start, struct timespec *end) {
  return (end->tv_sec - start->tv_sec) * 1000000000 +
         (end->tv_nsec - start->tv_nsec);
}

struct IoContext {
  int fd;
  char *buffer;
  struct io_uring ring;
};

void io_uring(const std::string &file_path, int thread_num) {
  int fd = open(file_path.c_str(), O_RDONLY);
  if (fd == -1) {
    perror("open");
    return;
  }

  struct IoContext ioContext;
  ioContext.fd = fd;
  char *buffer = (char *)malloc(BUFFER_SIZE);

  if (io_uring_queue_init(thread_num, &ioContext.ring, 0) < 0) {
    perror("io_uring_queue_init");
    close(fd);
    free(buffer);
    return;
  }

  off_t file_size = lseek(fd, 0, SEEK_END);
  off_t offset = 0;
  char *buf = (char *)aligned_alloc(BUFFER_SIZE, file_size);

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  while (offset < file_size) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ioContext.ring);
    if (!sqe) {
      perror("io_uring_get_sqe");
      break;
    }

    // Set up read operation
    io_uring_prep_read(sqe, fd, buf + offset, BUFFER_SIZE, offset);
    io_uring_sqe_set_data(sqe, (void *)(buf + offset));

    // Set up callback
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);
    io_uring_sqe_set_data(sqe, (void *)(buf + offset));

    // Submit the read operation
    io_uring_submit(&ioContext.ring);

    offset += BUFFER_SIZE;
  }

  // Wait for completions
  for (off_t i = 0; i < file_size / BUFFER_SIZE; ++i) {
    struct io_uring_cqe *cqe;
    if (io_uring_wait_cqe(&ioContext.ring, &cqe) < 0) {
      perror("io_uring_wait_cqe");
      break;
    }

    // Handle completion
    io_uring_cqe_seen(&ioContext.ring, cqe);
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
  // Clean up resources
  close(fd);
  io_uring_queue_exit(&ioContext.ring);
  free(buffer);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << "<filename> <thread_num>" << std::endl;
  }

  const std::string filename = argv[1];
  int t = atoi(argv[2]);

  io_uring(filename, t);

  return 0;
}