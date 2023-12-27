#include <spdk/blob.h>
#include <spdk/env.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_PATH "sample.txt"
#define PAGE_SIZE 4096

struct spdk_file_io_ctx {
  FILE *file;
  struct spdk_blob *blob;
  char *read_buffer;
};

static void read_complete(void *arg, int bserrno) {
  struct spdk_file_io_ctx *io_ctx = arg;

  if (bserrno != 0) {
    fprintf(stderr, "Async read failed: %s\n", spdk_strerror(-bserrno));
  } else {
    // Process or use the read data
    printf("Read data: %s\n", io_ctx->read_buffer);
  }

  // Stop the SPDK application
  spdk_app_stop(bserrno);
}

static void read_file(void *arg1, void *arg2) {
  struct spdk_file_io_ctx *io_ctx = arg1;

  // Submit an asynchronous read
  spdk_blob_io_read(io_ctx->blob, io_ctx->read_buffer, 0, PAGE_SIZE,
                    read_complete, io_ctx);
}

static void initialize_blob(void *arg1, void *arg2) {
  struct spdk_file_io_ctx *io_ctx = arg1;

  // Create a blob for reading
  io_ctx->blob = spdk_blob_open(io_ctx->file);
  if (io_ctx->blob == NULL) {
    fprintf(stderr, "Failed to open blob.\n");
    spdk_app_stop(-1);
    return;
  }

  // Allocate buffer for reading
  io_ctx->read_buffer = malloc(PAGE_SIZE);
  if (io_ctx->read_buffer == NULL) {
    fprintf(stderr, "Failed to allocate buffer.\n");
    spdk_app_stop(-1);
    return;
  }

  // Send a message to initialize the read operation
  spdk_thread_send_msg(spdk_get_thread(), read_file, io_ctx);
}

static void initialize_file(void *arg1, void *arg2) {
  struct spdk_file_io_ctx *io_ctx = arg1;

  // Open the file
  io_ctx->file = fopen(FILE_PATH, "r");
  if (io_ctx->file == NULL) {
    fprintf(stderr, "Failed to open file: %s\n", FILE_PATH);
    spdk_app_stop(-1);
    return;
  }

  // Send a message to initialize the blob and read data
  spdk_thread_send_msg(spdk_get_thread(), initialize_blob, io_ctx);
}

int main() {
  struct spdk_file_io_ctx io_ctx = {};

  if (spdk_env_init(NULL) < 0) {
    fprintf(stderr, "Unable to initialize SPDK env\n");
    return EXIT_FAILURE;
  }

  // Send a message to initialize the file
  spdk_thread_send_msg(spdk_get_thread(), initialize_file, &io_ctx);

  // Run the SPDK event loop
  spdk_app_start(NULL, NULL, NULL);

  // Clean up resources
  free(io_ctx.read_buffer);
  fclose(io_ctx.file);
  spdk_env_cleanup();

  return EXIT_SUCCESS;
}
