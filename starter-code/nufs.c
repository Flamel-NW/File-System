// based on cs3650 starter code

#include <assert.h>
#include <bsd/string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "storage.h"
#include "directory.h"
#include "inode.h"

// implementation for: man 2 access
// Checks if a file exists.
int nufs_access(const char *path, int mask) {
  printf("Debug: nufs_access\n");
  int rv = tree_lookup(path);

  // Only the root directory and our simulated file are accessible for now...
  if (rv >= 0) {
    inode_t* node = get_inode(rv);
    node->atime = time(NULL);
    rv = 0;
  } else { // ...others do not exist
    rv = -ENOENT;
  }

  printf("access(%s, %04o) -> %d\n", path, mask, rv);
  return rv;
}

// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
int nufs_getattr(const char *path, struct stat *st) {
  int rv = 0;

  // Return some metadata for the root directory...
  if (strcmp(path, "/") == 0) {
    st->st_mode = 040755; // directory
    st->st_size = 0;
    st->st_uid = getuid();
    st->st_nlink = 1;
  } else {
    rv = storage_stat(path, st);
    st->st_uid = getuid();
  }
  printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode,
         st->st_size);
  return rv;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {
  printf("Debug: nufs_readdir\n");
  struct stat st;
  int rv;

  rv = nufs_getattr(path, &st);
  assert(rv == 0);

  filler(buf, ".", &st, 0);

  slist_t* path_slist = storage_list(path);
  if (!path_slist)
  {
    printf("readdir(%s) -> %d\n", path, rv);
    return 0;
  }
  slist_t* pt_slist = path_slist;

  int path_len = strlen(path);
  while (pt_slist)
  {
    char* temp_path = (char*)malloc(path_len + 50);
    strncpy(temp_path, path, path_len);
    if (path[path_len - 1]  == '/')
      temp_path[path_len] = '\0';
    else
    {
      temp_path[path_len] = '/';
      temp_path[path_len + 1] = '\0';
    }
    strncat(temp_path, pt_slist->data, DIR_NAME_LENGTH);
    nufs_getattr(temp_path, &st);
    filler(buf, pt_slist->data, &st, 0);

    free(temp_path);
    pt_slist = pt_slist->next;
  }

  printf("readdir(%s) -> %d\n", path, rv);
  s_free(path_slist);
  return rv;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
// Note, for this assignment, you can alternatively implement the create
// function.
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
  int rv = storage_mknod(path, mode);
  printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int nufs_mkdir(const char *path, mode_t mode) {
  printf("Debug: nufs_mkdir\n");
  int rv = nufs_mknod(path, mode | 040000, 0);
  printf("mkdir(%s) -> %d\n", path, rv);
  return rv;
}

int nufs_unlink(const char *path) {
  printf("Debug: nufs_unlink\n");
  int rv = storage_unlink(path);
  printf("unlink(%s) -> %d\n", path, rv);
  return rv;
}

int nufs_link(const char *from, const char *to) {
  printf("Debug: nufs_link(%s, %s)\n", from, to);
  int rv = storage_link(from, to);
  printf("link(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

int nufs_rmdir(const char *path) {
  printf("Debug: nufs_rmdir\n");
  int rv = -1;
  printf("rmdir(%s) -> %d\n", path, rv);
  return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int nufs_rename(const char *from, const char *to) {
  printf("Debug: nufs_rename(%s, %s)\n", from, to);
  int rv = storage_rename(from, to);
  storage_update_ctime(to);
  printf("rename(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

int nufs_chmod(const char *path, mode_t mode) {
  printf("Debug: nufs_chmod\n");
  int rv = storage_chmod(path, mode);
  storage_update_ctime(path);
  printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

int nufs_truncate(const char *path, off_t size) {
  printf("Debug: nufs_truncate\n");
  int rv = storage_truncate(path, size);
  storage_update_ctime(path);
  printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
  return rv;
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi) {
  printf("Debug: nufs_open\n");
  int rv = 0;
  printf("open(%s) -> %d\n", path, rv);
  return rv;
}

// Actually read data
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
  int rv = storage_read(path, buf, size, offset);
  printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Actually write data
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {
  printf("Debug: nufs_write\n");
  int rv = storage_write(path, buf, size, offset);
  printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Update the timestamps on a file or directory.
int nufs_utimens(const char *path, const struct timespec ts[2]) {
  printf("Debug: nufs_utimens\n");
  int rv = storage_set_time(path, ts);
  storage_update_ctime(path);
  printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n", path, ts[0].tv_sec,
         ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
  return rv;
}

// Extended operations
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data) {
  printf("Debug: nufs_ioctl\n");
  int rv = -1;
  printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
  return rv;
}

void nufs_init_ops(struct fuse_operations *ops) {
  memset(ops, 0, sizeof(struct fuse_operations));
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->readdir = nufs_readdir;
  ops->mknod = nufs_mknod;
  // ops->create   = nufs_create; // alternative to mknod
  ops->mkdir = nufs_mkdir;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rmdir = nufs_rmdir;
  ops->rename = nufs_rename;
  ops->chmod = nufs_chmod;
  ops->truncate = nufs_truncate;
  ops->open = nufs_open;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int main(int argc, char *argv[]) {
  assert(argc > 2 && argc < 6);
  printf("TODO: mount %s as data file\n", argv[argc - 1]);
  storage_init(argv[--argc]);
  nufs_init_ops(&nufs_ops);
  return fuse_main(argc, argv, &nufs_ops, NULL);
}
