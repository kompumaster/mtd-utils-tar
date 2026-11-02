/* Tar index for mkfs.ubifs: path -> metadata and data offset.
 *
 * Supports ustar, GNU longname (L/K) and the basic pax fields (x/g).
 * The goal is to give mkfs.ubifs the same information a file system
 * would provide: type, mode, uid, gid, nlink, mtime, size, symlink
 * target and file data.
 */
#ifndef MTD_TAR_INDEX_H
#define MTD_TAR_INDEX_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct tar_index;

/* Open an archive and index its entries.  Entries matching one of the
 * skip prefixes (and everything below them) are ignored. */
struct tar_index *tar_open(const char *path, const char *const *skip,
                           int skip_count);
void tar_close(struct tar_index *ix);
int tar_count(const struct tar_index *ix);

/* Fill struct stat for path ("/etc/x" or "etc/x"). Returns 0 on success. */
int tar_fill_stat(const struct tar_index *ix, const char *path,
                  struct stat *st);

/* Read file data at offset. Returns the number of bytes or -1. */
ssize_t tar_read(const struct tar_index *ix, const char *path,
                 void *buf, size_t len, off_t off);

/* Symlink target. Returns the length or -1. */
ssize_t tar_target(const struct tar_index *ix, const char *path,
                   char *buf, size_t len);

/* Directory child iteration: dir is "" or "/etc". */
struct tar_dir {
    int pos;
    char *dir;
};

void tar_dir_begin(const struct tar_index *ix, const char *dir,
                   struct tar_dir *it);
const char *tar_dir_next(const struct tar_index *ix, struct tar_dir *it);
void tar_dir_end(struct tar_dir *it);

#endif /* MTD_TAR_INDEX_H */
