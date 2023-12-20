#include <spdk/env.h>
#include <spdk/nvme.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NVME_NSID 1
#define BUFFER_SIZE (4096 * 4) // 4 KB

struct nvme_read_ctx {
  spdk_nvme_qpair *qpair;
  char *buffer;
};

static void read_complete(void *arg, const struct spdk_nvme_cpl *completion);

static void read_data(struct nvme_read_ctx *ctx, uint64_t lba,
                      uint32_t lba_count) {
  spdk_nvme_ns_cmd_read(ctx->qpair, ctx->buffer, lba, lba_count, read_complete,
                        ctx, 0);
}

static void read_complete(void *arg, const struct spdk_nvme_cpl *completion) {
  struct nvme_read_ctx *ctx = (struct nvme_read_ctx *)arg;

  if (spdk_nvme_cpl_is_error(completion)) {
    fprintf(stderr, "NVMe read error: SC %x\n",
            spdk_nvme_cpl_get_status_code(completion));
    spdk_nvme_qpair_abort_reqs(ctx->qpair, 0);
    spdk_nvme_ctrlr_free_io_qpair(ctx->qpair);
    spdk_free(ctx->buffer);
    spdk_nvme_detach(spdk_nvme_ns_get_ctrlr(spdk_nvme_ns_list_get_first()));
    spdk_nvme_cleanup();
    exit(EXIT_FAILURE);
  }

  // Process the read data as needed
  // ...

  // Clean up resources
  spdk_nvme_qpair_abort_reqs(ctx->qpair, 0);
  spdk_nvme_ctrlr_free_io_qpair(ctx->qpair);
  spdk_free(ctx->buffer);
  spdk_nvme_detach(spdk_nvme_ns_get_ctrlr(spdk_nvme_ns_list_get_first()));
  spdk_nvme_cleanup();

  exit(EXIT_SUCCESS);
}

int main() {
  struct nvme_read_ctx ctx = {0};
  uint64_t lba = 0;       // Starting Logical Block Address
  uint32_t lba_count = 8; // Number of Logical Blocks to read

  // Initialize SPDK environment
  if (spdk_env_init(NULL) < 0) {
    fprintf(stderr, "Unable to initialize SPDK environment\n");
    return EXIT_FAILURE;
  }

  // Initialize NVMe subsystem
  if (spdk_nvme_init() < 0) {
    fprintf(stderr, "Unable to initialize NVMe\n");
    spdk_env_cleanup();
    return EXIT_FAILURE;
  }

  // Enumerate NVMe controllers
  if (spdk_nvme_probe(NULL, NULL, NULL, NULL) != 0) {
    fprintf(stderr, "No NVMe controllers found\n");
    spdk_nvme_cleanup();
    spdk_env_cleanup();
    return EXIT_FAILURE;
  }

  // Attach to the first NVMe controller
  if (spdk_nvme_attach(spdk_nvme_probe(NULL, NULL, NULL, NULL)) != 0) {
    fprintf(stderr, "Failed to attach to NVMe controller\n");
    spdk_nvme_cleanup();
    spdk_env_cleanup();
    return EXIT_FAILURE;
  }

  // Get the first NVMe namespace
  struct spdk_nvme_ns *ns =
      spdk_nvme_ctrlr_get_ns(spdk_nvme_ns_list_get_first(), NVME_NSID);
  if (!ns) {
    fprintf(stderr, "No NVMe namespace found\n");
    spdk_nvme_cleanup();
    spdk_env_cleanup();
    return EXIT_FAILURE;
  }

  // Allocate buffer for read data
  ctx.buffer = spdk_zmalloc(BUFFER_SIZE, 0, NULL, SPDK_ENV_LCORE_ID_ANY,
                            SPDK_MALLOC_DMA);
  if (!ctx.buffer) {
    fprintf(stderr, "Failed to allocate buffer for read data\n");
    spdk_nvme_cleanup();
    spdk_env_cleanup();
    return EXIT_FAILURE;
  }

  // Create I/O queue pair
  ctx.qpair =
      spdk_nvme_ctrlr_alloc_io_qpair(spdk_nvme_ns_get_ctrlr(ns), NULL, 0);
  if (!ctx.qpair) {
    fprintf(stderr, "Failed to create I/O queue pair\n");
    spdk_free(ctx.buffer);
    spdk_nvme_cleanup();
    spdk_env_cleanup();
    return EXIT_FAILURE;
  }

  // Perform asynchronous read
  read_data(&ctx, lba, lba_count);

  // Start the SPDK event loop
  spdk_env_run();

  return EXIT_SUCCESS;
}
