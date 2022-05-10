// based on cs3650 starter code

#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdio.h>

const int BLOCK_COUNT; // we split the "disk" into blocks (default = 256)
const int BLOCK_SIZE;  // default = 4K
const int NUFS_SIZE;   // default = 1MB

const int BLOCK_BITMAP_SIZE; // default = 256 / 8 = 32

int block_list[256]; // BLOCK_COUNT

// Get the number of blocks needed to store the given number of bytes.
int bytes_to_blocks(int bytes);

// Load and initialize the given disk image.
void blocks_init(const char *image_path);

// Close the disk image.
void blocks_free();

// Get the block with the given index, returning a pointer to its start.
void *blocks_get_block(int bnum);

// Return a pointer to the beginning of the block bitmap.
void *get_blocks_bitmap();

// Return a pointer to the beginning of the inode table bitmap.
void *get_inode_bitmap();

// Allocate a new block and return its index.
int alloc_block();

// Deallocate the block with the given index.
void free_block(int bnum);

#endif
