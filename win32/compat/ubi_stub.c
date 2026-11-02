/* UBI device stubs: mkfs.ubifs writes to a file, not to /dev/ubi*. */
#include <errno.h>
#include <stddef.h>
#include <libubi.h>
#include <libmtd.h>

libmtd_t libmtd_open(void)
{
    return NULL;
}

void libmtd_close(libmtd_t desc)
{
    (void)desc;
}

int mtd_get_dev_info2(libmtd_t desc, const char *node, struct mtd_dev_info *mtd)
{
    (void)desc; (void)node; (void)mtd;
    return -1;
}

libubi_t libubi_open(void)
{
    return NULL;
}

void libubi_close(libubi_t desc)
{
    (void)desc;
}

int ubi_get_dev_info1(libubi_t desc, int dev_num, struct ubi_dev_info *info)
{
    (void)desc; (void)dev_num; (void)info;
    return -1;
}

int ubi_get_vol_info(libubi_t desc, const char *node, struct ubi_vol_info *info)
{
    (void)desc; (void)node; (void)info;
    return -1;
}

int ubi_set_property(int fd, uint8_t property, uint64_t value)
{
    (void)fd; (void)property; (void)value;
    errno = ENOSYS;
    return -1;
}

int ubi_leb_change_start(libubi_t desc, int fd, int lnum, int bytes)
{
    (void)desc; (void)fd; (void)lnum; (void)bytes;
    errno = ENOSYS;
    return -1;
}

int ubi_leb_unmap(int fd, int lnum)
{
    (void)fd; (void)lnum;
    errno = ENOSYS;
    return -1;
}

int ubi_is_mapped(int fd, int lnum)
{
    (void)fd; (void)lnum;
    errno = ENOSYS;
    return -1;
}

int ubi_leb_map(int fd, int lnum)
{
    (void)fd; (void)lnum;
    errno = ENOSYS;
    return -1;
}
