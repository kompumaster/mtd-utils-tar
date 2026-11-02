/* Parse a tar archive into an index. No external dependencies. */
#include "tar_index.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK 512

struct entry {
    char *path;
    char type;              /* '0' file, '5' dir, '2' symlink, '1' hardlink */
    mode_t mode;
    unsigned int uid;
    unsigned int gid;
    unsigned long nlink;
    off_t size;
    time_t mtime;
    off_t data;
    char *link;
    ino_t ino;
};

struct tar_index {
    int fd;
    struct entry *items;
    int count;
    int capacity;
    char **skip;
    int skip_count;
};

static char *dup_str(const char *s)
{
    size_t n = strlen(s) + 1;
    char *out = malloc(n);
    if (out)
        memcpy(out, s, n);
    return out;
}

static long long parse_octal(const char *field, size_t len)
{
    long long value = 0;
    size_t i = 0;

    while (i < len && (field[i] == ' ' || field[i] == '\0'))
        i++;
    if (i < len && (field[i] & 0x80)) {
        /* base-256 (GNU) */
        value = field[i] & 0x7f;
        for (i++; i < len; i++)
            value = (value << 8) | (unsigned char)field[i];
        return value;
    }
    for (; i < len && field[i] >= '0' && field[i] <= '7'; i++)
        value = value * 8 + (field[i] - '0');
    return value;
}

static int is_zero_block(const unsigned char *block)
{
    int i;
    for (i = 0; i < BLOCK; i++)
        if (block[i])
            return 0;
    return 1;
}

static int add_entry(struct tar_index *ix)
{
    if (ix->count == ix->capacity) {
        int want = ix->capacity ? ix->capacity * 2 : 128;
        struct entry *grown = realloc(ix->items,
                                      (size_t)want * sizeof(*grown));
        if (!grown)
            return -1;
        ix->items = grown;
        ix->capacity = want;
    }
    memset(&ix->items[ix->count], 0, sizeof(ix->items[0]));
    return ix->count++;
}

static int cmp_path(const void *a, const void *b)
{
    const struct entry *ea = a;
    const struct entry *eb = b;
    return strcmp(ea->path, eb->path);
}

static struct entry *find_entry(struct tar_index *ix, const char *path)
{
    int lo = 0, hi = ix->count - 1;

    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        int cmp = strcmp(ix->items[mid].path, path);
        if (cmp == 0)
            return &ix->items[mid];
        if (cmp < 0)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return NULL;
}

static void strip_prefix(const char *in, char *out, size_t out_len)
{
    size_t i = 0;

    while (in[i] == '/' || (in[i] == '.' && in[i + 1] == '/'))
        i += (in[i] == '/') ? 1 : 2;
    snprintf(out, out_len, "%s", in + i);
}

static int is_skipped(const struct tar_index *ix, const char *path)
{
    char clean[4096];
    int i;

    strip_prefix(path, clean, sizeof(clean));
    for (i = 0; i < ix->skip_count; i++) {
        const char *skip = ix->skip[i];
        size_t n = strlen(skip);

        if (!n)
            continue;
        if (strncmp(clean, skip, n) == 0 &&
            (clean[n] == '\0' || clean[n] == '/'))
            return 1;
    }
    return 0;
}

struct tar_index *tar_open(const char *path, const char *const *skip,
                           int skip_count)
{
    struct tar_index *ix;
    unsigned char header[BLOCK];
    char longname[4096];
    char longlink[4096];
    int pending_name = 0, pending_link = 0;
    int i;

    ix = calloc(1, sizeof(*ix));
    if (!ix)
        return NULL;
    ix->fd = open(path, O_RDONLY);
    if (ix->fd < 0)
        goto fail;

    if (skip_count > 0) {
        ix->skip = calloc((size_t)skip_count, sizeof(*ix->skip));
        if (!ix->skip)
            goto fail;
        for (i = 0; i < skip_count; i++) {
            char clean[4096];

            strip_prefix(skip[i], clean, sizeof(clean));
            ix->skip[i] = dup_str(clean);
            if (!ix->skip[i])
                goto fail;
            ix->skip_count++;
        }
    }

    longname[0] = 0;
    longlink[0] = 0;

    for (;;) {
        ssize_t got = read(ix->fd, header, BLOCK);
        char name[256], link[256], prefix[156];
        long long size, mode, uid, gid, mtime;
        char type;
        struct entry *e;
        size_t name_len;

        if (got == 0)
            break;
        if (got != BLOCK || is_zero_block(header))
            break;

        memcpy(name, header + 0, 100);
        name[100] = 0;
        memcpy(prefix, header + 345, 155);
        prefix[155] = 0;
        memcpy(link, header + 157, 100);
        link[100] = 0;
        mode = parse_octal((char *)header + 100, 8);
        uid = parse_octal((char *)header + 108, 8);
        gid = parse_octal((char *)header + 116, 8);
        size = parse_octal((char *)header + 124, 12);
        mtime = parse_octal((char *)header + 136, 12);
        type = (char)header[156];

        if (type == 'L' || type == 'K') {
            char *target = (type == 'L') ? longname : longlink;
            size_t want = (size_t)((size < 4095) ? size : 4095);
            if (read(ix->fd, target, want) != (ssize_t)want)
                break;
            target[want] = 0;
            if (type == 'L')
                pending_name = 1;
            else
                pending_link = 1;
            /* skip the data and padding */
            if (lseek(ix->fd, (off_t)((size + 511) & ~511LL) - (off_t)want,
                      SEEK_CUR) < 0)
                break;
            continue;
        }
        if (type == 'x' || type == 'g') {
            /* pax: read as-is and extract path/linkpath */
            char *buf = calloc(1, (size_t)size + 1);
            if (buf) {
                if (read(ix->fd, buf, (size_t)size) != (ssize_t)size) {
                    free(buf);
                    break;
                }
                char *p = strstr(buf, "path=");
                if (p && p == buf) {
                    char *v = p + 5, *nl = strchr(v, '\n');
                    if (nl) *nl = 0;
                    snprintf(longname, sizeof(longname), "%s", v);
                    pending_name = 1;
                }
                p = strstr(buf, "linkpath=");
                if (p && p == buf) {
                    char *v = p + 9, *nl = strchr(v, '\n');
                    if (nl) *nl = 0;
                    snprintf(longlink, sizeof(longlink), "%s", v);
                    pending_link = 1;
                }
                free(buf);
                if (lseek(ix->fd,
                          (off_t)((size + 511) & ~511LL) - (off_t)size,
                          SEEK_CUR) < 0)
                    break;
            }
            continue;
        }
        if (type == 0 || type == '0')
            type = '0';

        {
            char final_path[4096];
            if (pending_name) {
                snprintf(final_path, sizeof(final_path), "%s", longname);
                pending_name = 0;
            } else if (prefix[0]) {
                snprintf(final_path, sizeof(final_path), "%s/%s",
                         prefix, name);
            } else {
                snprintf(final_path, sizeof(final_path), "%s", name);
            }
            /* Skip paths requested via --tar-skip. */
            if (is_skipped(ix, final_path)) {
                if (lseek(ix->fd,
                          (off_t)((size + 511) & ~511LL), SEEK_CUR) < 0)
                    break;
                continue;
            }
            if (add_entry(ix) < 0)
                break;
            e = &ix->items[ix->count - 1];
            e->path = dup_str(final_path);
        }
        if (!e->path)
            break;
        /* GNU tar writes directories with a trailing '/'; drop it. */
        {
            size_t plen = strlen(e->path);
            while (plen > 1 && e->path[plen - 1] == '/')
                e->path[--plen] = 0;
        }

        e->type = type;
        e->mode = (mode_t)mode;
        e->uid = (unsigned int)uid;
        e->gid = (unsigned int)gid;
        e->mtime = (time_t)mtime;
        e->size = (off_t)size;
        e->data = lseek(ix->fd, 0, SEEK_CUR);
        if (pending_link) {
            e->link = dup_str(longlink);
            pending_link = 0;
        } else if (link[0]) {
            e->link = dup_str(link);
        }

        if (lseek(ix->fd, (off_t)((size + 511) & ~511LL), SEEK_CUR) < 0)
            break;
        name_len = strlen(e->path);
        (void)name_len;
    }

    if (ix->count > 1)
        qsort(ix->items, (size_t)ix->count, sizeof(ix->items[0]), cmp_path);

    /* Resolve hardlinks: same inode, data and size as the target. */
    {
        int i;
        ino_t next_ino = 2;

        for (i = 0; i < ix->count; i++) {
            struct entry *e = &ix->items[i];
            if (e->type == '0') {
                e->ino = next_ino++;
                e->nlink = 1;
            } else if (e->type == '5') {
                e->ino = next_ino++;
                e->nlink = 2;
            } else if (e->type == '2') {
                e->ino = next_ino++;
                e->nlink = 1;
            }
        }
        /* first pass: count links to the targets */
        for (i = 0; i < ix->count; i++) {
            struct entry *e = &ix->items[i];
            char target[4096];
            struct entry *t;
            if (e->type != '1' || !e->link)
                continue;
            strip_prefix(e->link, target, sizeof(target));
            t = find_entry(ix, target);
            if (t && t->type == '0')
                t->nlink += 1;
        }
        /* second pass: a hardlink gets the same inode and data */
        for (i = 0; i < ix->count; i++) {
            struct entry *e = &ix->items[i];
            char target[4096];
            struct entry *t;
            if (e->type != '1' || !e->link)
                continue;
            strip_prefix(e->link, target, sizeof(target));
            t = find_entry(ix, target);
            if (!t || t->type != '0')
                continue;
            e->ino = t->ino;
            e->data = t->data;
            e->size = t->size;
            e->mode = t->mode;
            e->uid = t->uid;
            e->gid = t->gid;
            e->nlink = t->nlink;
        }
    }

    return ix;

fail:
    tar_close(ix);
    return NULL;
}

void tar_close(struct tar_index *ix)
{
    int i;

    if (!ix)
        return;
    for (i = 0; i < ix->count; i++) {
        free(ix->items[i].path);
        free(ix->items[i].link);
    }
    free(ix->items);
    for (i = 0; i < ix->skip_count; i++)
        free(ix->skip[i]);
    free(ix->skip);
    if (ix->fd >= 0)
        close(ix->fd);
    free(ix);
}

int tar_count(const struct tar_index *ix)
{
    return ix ? ix->count : 0;
}

static struct entry *lookup(struct tar_index *ix, const char *path)
{
    char clean[4096];
    strip_prefix(path, clean, sizeof(clean));
    return find_entry(ix, clean);
}

int tar_fill_stat(const struct tar_index *ix, const char *path,
                  struct stat *st)
{
    struct entry *e = lookup((struct tar_index *)ix, path);

    if (!e)
        return -1;
    memset(st, 0, sizeof(*st));
    st->st_uid = e->uid;
    st->st_gid = e->gid;
    st->st_nlink = e->nlink ? e->nlink : 1;
    st->st_size = e->size;
    st->st_mtime = e->mtime;
    st->st_ino = e->ino;
    st->st_dev = 0;
    switch (e->type) {
    case '5': st->st_mode = S_IFDIR | (e->mode & 07777); break;
    case '2': st->st_mode = S_IFLNK | 0777; break;
    default:  st->st_mode = S_IFREG | (e->mode & 07777); break;
    }
    return 0;
}

ssize_t tar_read(const struct tar_index *ix, const char *path,
                 void *buf, size_t len, off_t off)
{
    struct entry *e = lookup((struct tar_index *)ix, path);

    if (!e)
        return -1;
    if ((off_t)(off + (off_t)len) > e->size)
        len = (size_t)(e->size - off);
    if ((off_t)len <= 0)
        return 0;
    return pread(ix->fd, buf, len, e->data + off);
}

ssize_t tar_target(const struct tar_index *ix, const char *path,
                   char *buf, size_t len)
{
    struct entry *e = lookup((struct tar_index *)ix, path);
    size_t n;

    if (!e || !e->link)
        return -1;
    n = strlen(e->link);
    if (n >= len)
        n = len - 1;
    memcpy(buf, e->link, n);
    buf[n] = 0;
    return (ssize_t)n;
}

void tar_dir_begin(const struct tar_index *ix, const char *dir,
                   struct tar_dir *it)
{
    char clean[4096];

    (void)ix;

    strip_prefix(dir, clean, sizeof(clean));
    it->pos = 0;
    it->dir = dup_str(clean);
}

static int is_child(const char *path, const char *dir)
{
    size_t n = strlen(dir);

    if (!n)
        return strchr(path, '/') == NULL;
    if (strncmp(path, dir, n) != 0 || path[n] != '/')
        return 0;
    return strchr(path + n + 1, '/') == NULL;
}

const char *tar_dir_next(const struct tar_index *ix, struct tar_dir *it)
{
    struct tar_index *m = (struct tar_index *)ix;

    while (it->pos < m->count) {
        struct entry *e = &m->items[it->pos++];
        if (is_child(e->path, it->dir))
            return e->path + (it->dir[0] ? strlen(it->dir) + 1 : 0);
    }
    return NULL;
}

void tar_dir_end(struct tar_dir *it)
{
    free(it->dir);
    it->dir = NULL;
}
