/* Implementation of POSIX functions missing in MinGW. */
#include "compat.h"

#include <fcntl.h>
#include <io.h>
#include <time.h>
#include <stdarg.h>
#include <limits.h>

/* MinGW opens files in text mode; this corrupts 0x0A bytes in
 * images. Switch the default mode to binary. */
__attribute__((constructor))
static void compat_binary_mode(void)
{
    _set_fmode(_O_BINARY);
}

ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
    __int64 save = _lseeki64(fd, 0, SEEK_CUR);
    int got;
    if (save < 0)
        return -1;
    if (_lseeki64(fd, (__int64)offset, SEEK_SET) < 0)
        return -1;
    got = _read(fd, buf, (unsigned int)count);
    _lseeki64(fd, save, SEEK_SET);
    return got;
}

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    __int64 save = _lseeki64(fd, 0, SEEK_CUR);
    int put;
    if (save < 0)
        return -1;
    if (_lseeki64(fd, (__int64)offset, SEEK_SET) < 0)
        return -1;
    put = _write(fd, buf, (unsigned int)count);
    _lseeki64(fd, save, SEEK_SET);
    return put;
}

ssize_t getline(char **line, size_t *size, FILE *stream)
{
    size_t used = 0;
    int ch;

    if (!line || !size || !stream) {
        errno = EINVAL;
        return -1;
    }
    if (!*line || !*size) {
        *size = 256;
        *line = malloc(*size);
        if (!*line)
            return -1;
    }
    while ((ch = fgetc(stream)) != EOF) {
        if (used + 1 >= *size) {
            size_t want = *size * 2;
            char *grown = realloc(*line, want);
            if (!grown)
                return -1;
            *line = grown;
            *size = want;
        }
        (*line)[used++] = (char)ch;
        if (ch == '\n')
            break;
    }
    if (used == 0 && ch == EOF)
        return -1;
    (*line)[used] = '\0';
    return (ssize_t)used;
}

ssize_t readlink(const char *path, char *buf, size_t len)
{
    (void)path; (void)buf; (void)len;
    errno = ENOSYS;
    return -1;
}

int lstat(const char *path, struct stat *st)
{
    return stat(path, st);
}

int dirfd(DIR *dirp)
{
    (void)dirp;
    return 0;
}

int ioctl(int fd, unsigned long request, ...)
{
    (void)fd; (void)request;
    errno = ENOTTY;
    return -1;
}

int xasprintf(char **strp, const char *fmt, ...)
{
    va_list ap;
    int need;

    va_start(ap, fmt);
    need = _vscprintf(fmt, ap);
    va_end(ap);
    if (need < 0)
        return -1;
    *strp = malloc((size_t)need + 1);
    if (!*strp)
        return -1;
    va_start(ap, fmt);
    vsnprintf(*strp, (size_t)need + 1, fmt, ap);
    va_end(ap);
    return need;
}

char *realpath(const char *path, char *resolved)
{
    return _fullpath(resolved, path, PATH_MAX);
}

int getpagesize(void)
{
    return 4096;
}

void uuid_generate_random(unsigned char out[16])
{
    static unsigned int seeded;
    size_t i;

    if (!seeded) {
        srand((unsigned)time(NULL) ^ (unsigned)(size_t)&seeded);
        seeded = 1;
    }
    for (i = 0; i < 16; i++)
        out[i] = (unsigned char)(rand() & 0xff);
    out[6] = (unsigned char)((out[6] & 0x0f) | 0x40);
    out[8] = (unsigned char)((out[8] & 0x3f) | 0x80);
}

void uuid_unparse_upper(const unsigned char u[16], char *out)
{
    sprintf(out,
            "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-"
            "%02X%02X%02X%02X%02X%02X",
            u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7],
            u[8], u[9], u[10], u[11], u[12], u[13], u[14], u[15]);
}
