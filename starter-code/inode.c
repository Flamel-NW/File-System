#include <stdio.h>
#include <math.h>
#include <time.h>

#include "inode.h"
#include "blocks.h"
#include "bitmap.h"

void print_inode(inode_t* node)
{
    printf("inode.refs = %d\n", node->refs);
    printf("inode.mode = %d\n", node->mode);
    printf("inode.size = %d\n", node->size);
    printf("inode.block = %d\n", node->block);
    int block = node->block;
    while (block != block_list[block]) {
        block = block_list[block];
        printf("\tblock_list = %d\n", block);
    }

    printf("inode.atime = %d\n", node->atime);
    printf("inode.mtime = %d\n", node->mtime);
    printf("inode.ctime = %d\n", node->ctime);
}

inode_t* get_inode(int inum)
{
    inode_t* nodes = get_inode_bitmap() + BLOCK_BITMAP_SIZE;
    return &nodes[inum];
}

int alloc_inode()
{
    void *ibm = get_inode_bitmap();
    // bitmap_print(ibm, BLOCK_COUNT);
    
    int inode_num = 0;
    for (int ii = 0; ii < BLOCK_COUNT; ++ii) {
        if (!bitmap_get(ibm, ii)) {
            bitmap_put(ibm, ii, 1);
            printf("+ alloc_inode() -> %d\n", ii);
            inode_num = ii;
            break;
        }
    }
    inode_t* new_node = get_inode(inode_num);
    new_node->refs = 1;
    new_node->mode = 0;
    new_node->size = 0;
    new_node->block = alloc_block();
    new_node->atime = 
        new_node->ctime = 
        new_node->mtime = time(NULL);
    return inode_num;
}

void free_inode(int inum)
{
    printf("+ free_inode(%d)\n", inum);

    inode_t* node = get_inode(inum);
    shrink_inode(node, 0);
    free_block(node->block);

    void *ibm = get_inode_bitmap();
    bitmap_put(ibm, inum, 0);
}

int grow_inode(inode_t* node, int size)
{
    int end_block = node->block;
    while (end_block != block_list[end_block])
        end_block = block_list[end_block];

    int grow_block_num = (size - 1) / BLOCK_SIZE - (node->size - 1) / BLOCK_SIZE;
    for (int i = 0; i < grow_block_num; i++)
    {
        int inum = alloc_block();
        block_list[end_block] = inum;
        end_block = inum;
    }

    node->size = size;
    return 0;
}

int shrink_inode(inode_t* node, int size)
{
    int block_num = size / BLOCK_SIZE;

    int i = 0;
    int block = node->block;
    while (block != block_list[block])
    {
        if (i > block_num)
        {
            free_block(block);
            int t = block_list[block];
            block = t;
        }
        i++;
    }

    node->size = size;
    return 0;
}

int inode_get_pnum(inode_t* node, int fpn)
{
    int block_num = fpn / BLOCK_COUNT;
    int block = node->block;
    for (int i = 0; i < block_num; i++)
        block = block_list[block];
    return block;
}
