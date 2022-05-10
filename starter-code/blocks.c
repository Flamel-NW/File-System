// based on cs3650 starter code

#define _GNU_SOURCE
#include <string.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "blocks.h"
#include "bitmap.h"

const int BLOCK_COUNT = 256; // we split the "disk" into 256 blocks
const int BLOCK_SIZE = 4096; // = 4K
const int NUFS_SIZE = BLOCK_SIZE * BLOCK_COUNT; // = 1MB

const int BLOCK_BITMAP_SIZE = BLOCK_COUNT / 8;
// Note: assumes block count is divisible by 8

static int blocks_fd = -1;
static void *blocks_base = 0;

// Get the number of blocks needed to store the given number of bytes.
int bytes_to_blocks(int bytes) {
  int quo = bytes / BLOCK_SIZE;
  int rem = bytes % BLOCK_SIZE;
  if (rem == 0) {
    return quo;
  } else {
    return quo + 1;
  }
}

// Load and initialize the given disk image.
void blocks_init(const char *image_path) {
  blocks_fd = open(image_path, O_CREAT | O_RDWR, 0644);
  assert(blocks_fd != -1);

  // make sure the disk image is exactly 1MB
  int rv = ftruncate(blocks_fd, NUFS_SIZE);
  assert(rv == 0);

  // map the image to memory
  blocks_base =
      mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, blocks_fd, 0);
  assert(blocks_base != MAP_FAILED);

  // block 0 stores the block bitmap and the inode bitmap
  void *bbm = get_blocks_bitmap();
  bitmap_put(bbm, 0, 1);

  for (int i = 0; i < BLOCK_COUNT; i++)
    block_list[i] = i;
}

// Close the disk image.
void blocks_free() {
  int rv = munmap(blocks_base, NUFS_SIZE);
  assert(rv == 0);
}

// Get the given block, returning a pointer to its start.
void *blocks_get_block(int bnum) { return blocks_base + BLOCK_SIZE * bnum; }

// Return a pointer to the beginning of the block bitmap.
// The size is BLOCK_BITMAP_SIZE bytes.
void *get_blocks_bitmap() { return blocks_get_block(0); }

// Return a pointer to the beginning of the inode table bitmap.
void *get_inode_bitmap() {
  uint8_t *block = blocks_get_block(0);

  // The inode bitmap is stored immediately after the block bitmap
  return (void *) (block + BLOCK_BITMAP_SIZE);
}

// Allocate a new block and return its index.
int alloc_block() {
  void *bbm = get_blocks_bitmap();
  // bitmap_print(bbm, BLOCK_COUNT);

  for (int ii = 1; ii < BLOCK_COUNT; ++ii) {
    if (!bitmap_get(bbm, ii)) {
      bitmap_put(bbm, ii, 1);
      printf("+ alloc_block() -> %d\n", ii);
      return ii;
    }
  }

  return -1;
}

// Deallocate the block with the given index.
void free_block(int bnum) {
  printf("+ free_block(%d)\n", bnum);
  void *bbm = get_blocks_bitmap();
  bitmap_put(bbm, bnum, 0);
}
