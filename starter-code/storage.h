// Disk storage manipulation.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "slist.h"

void storage_init(const char *path);
int storage_stat(const char *path, struct stat *st);
int storage_read(const char *path, char *buf, size_t size, off_t offset);
int storage_write(const char *path, const char *buf, size_t size, off_t offset);
int storage_truncate(const char *path, off_t size);
int storage_mknod(const char *path, mode_t mode);
int storage_unlink(const char *path);
int storage_link(const char *from, const char *to);
int storage_rename(const char *from, const char *to);
int storage_set_time(const char *path, const struct timespec ts[2]);
slist_t *storage_list(const char *path);

int storage_update_ctime(const char* path);
int storage_chmod(const char* path, mode_t mode);

#endif
