#include <aio.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#define FILE_PATH "testfile.txt"
#define BUFFER_SIZE 4096
#define NUM_REQUESTS 10

void aio_completion_handler(sigval sigval) {
  struct aiocb *aiocbp = static_cast<struct aiocb *>(sigval.sival_ptr);

  if (aio_error(aiocbp) == 0) {
    // AIO operation completed successfully
    ssize_t bytesRead = aio_return(aiocbp);
    std::cout << "Read " << bytesRead << " bytes from file." << std::endl;
  } else {
    // AIO operation failed
    std::cerr << "Error reading from file: " << strerror(aio_error(aiocbp))
              << std::endl;
  }

  //   free(aiocbp->aio_buf);
  free(aiocbp);
}

int main() {
  int fileDescriptor = open(FILE_PATH, O_RDONLY);
  if (fileDescriptor == -1) {
    std::cerr << "Error opening file: " << strerror(errno) << std::endl;
    return 1;
  }

  for (int i = 0; i < NUM_REQUESTS; ++i) {
    // Allocate buffer for AIO read
    char *buffer = static_cast<char *>(malloc(BUFFER_SIZE));

    // Create aiocb structure
    struct aiocb *aiocbp =
        static_cast<struct aiocb *>(malloc(sizeof(struct aiocb)));
    memset(aiocbp, 0, sizeof(struct aiocb));

    aiocbp->aio_fildes = fileDescriptor;
    aiocbp->aio_buf = buffer;
    aiocbp->aio_nbytes = BUFFER_SIZE;
    aiocbp->aio_offset = i * BUFFER_SIZE;

    // Set up signal handler
    aiocbp->aio_sigevent.sigev_notify = SIGEV_THREAD;
    aiocbp->aio_sigevent.sigev_notify_function = aio_completion_handler;
    aiocbp->aio_sigevent.sigev_value.sival_ptr = aiocbp;

    // Start asynchronous read
    if (aio_read(aiocbp) == -1) {
      std::cerr << "Error starting asynchronous read: " << strerror(errno)
                << std::endl;
      free(buffer);
      free(aiocbp);
      break;
    }
  }

  // Sleep to allow asynchronous reads to complete
  sleep(2);

  // Close the file
  close(fileDescriptor);

  return 0;
}
