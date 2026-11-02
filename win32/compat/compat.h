/* POSIX shims for building mtd-utils with MinGW.
 *
 * Included via -include into every source file. Only what MinGW
 * lacks: pread/pwrite, getline, readlink, lstat, dirfd, ioctl, uuid.
 * Behaviour is honest: missing functionality returns an error instead
 * of pretending to work.
 */
#ifndef MTD_COMPAT_H
#define MTD_COMPAT_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#ifndef __always_inline
#define __always_inline __inline__
#endif

#ifndef EUCLEAN
#define EUCLEAN 117
#endif
#ifndef ENODATA
#define ENODATA 120
#endif
#ifndef ENOTSUP
#define ENOTSUP 129
#endif
#ifndef ENOATTR
#define ENOATTR ENODATA
#endif

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef S_ISUID
#define S_ISUID 04000
#endif
#ifndef S_ISGID
#define S_ISGID 02000
#endif
#ifndef S_ISVTX
#define S_ISVTX 01000
#endif

#ifndef S_IFLNK
#define S_IFLNK 0120000
#endif
#ifndef S_IFSOCK
#define S_IFSOCK 0140000
#endif
#ifndef S_IFCHR
#define S_IFCHR 0020000
#endif
#ifndef S_IFBLK
#define S_IFBLK 0060000
#endif
#ifndef S_IFIFO
#define S_IFIFO 0010000
#endif

#ifndef S_ISLNK
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#endif
#ifndef VERSION
#define VERSION "2.3.1"
#endif

#include "sys/sysmacros.h"
#include "uuid.h"

ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
ssize_t getline(char **line, size_t *size, FILE *stream);
ssize_t readlink(const char *path, char *buf, size_t len);
int lstat(const char *path, struct stat *st);
int dirfd(DIR *dirp);
int ioctl(int fd, unsigned long request, ...);
int getpagesize(void);
int xasprintf(char **strp, const char *fmt, ...);
char *realpath(const char *path, char *resolved);

#endif /* MTD_COMPAT_H */
