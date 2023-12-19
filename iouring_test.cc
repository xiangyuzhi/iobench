#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <liburing.h>
#include <unistd.h>
#include <vector>

#define FILE_SIZE (1024 * 1024 * 100) // 100 MB

void io_uring_error(int ret, const char *msg) {
  std::cerr << msg << ": " << strerror(-ret) << std::endl;
  exit(1);
}

void io_uring_write_test(const std::string &filename, std::size_t fileSize) {
  io_uring ring;
  io_uring_queue_init(16, &ring, 0);

  int file =
      open(filename.c_str(), O_CREAT | O_WRONLY | O_DIRECT | O_TRUNC, 0666);
  if (file < 0) {
    std::cerr << "Error opening file for writing: " << strerror(errno)
              << std::endl;
    exit(1);
  }

  char *data = static_cast<char *>(aligned_alloc(512, fileSize));
  if (!data) {
    std::cerr << "Error allocating memory for data" << std::endl;
    close(file);
    exit(1);
  }

  std::vector<io_uring_sqe *> sqes;

  // Populate the submission queue with write operations
  for (off_t offset = 0; offset < fileSize; offset += 4096) {
    io_uring_sqe *sqe = io_uring_get_sqe(&ring);

    if (!sqe) {
      io_uring_submit(&ring);
      sqe = io_uring_get_sqe(&ring);
    }

    io_uring_prep_write(sqe, file, data + offset, 4096, offset);
    sqes.push_back(sqe);
  }

  // Submit the operations
  io_uring_submit(&ring);

  // Wait for completion
  for (auto *sqe : sqes) {
    io_uring_cqe *cqe;
    io_uring_wait_cqe(&ring, &cqe);

    if (cqe->res < 0) {
      io_uring_error(cqe->res, "Write error");
    }

    io_uring_cqe_seen(&ring, cqe);
  }

  free(data);
  close(file);
  io_uring_queue_exit(&ring);
}

int main() {
  const std::string filename = "testfile";
  io_uring_write_test(filename, FILE_SIZE);

  std::cout << "Write test completed." << std::endl;

  return 0;
}
