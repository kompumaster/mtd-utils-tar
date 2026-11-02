#ifndef MTD_SYS_SYSMACROS_H
#define MTD_SYS_SYSMACROS_H
typedef unsigned long long __mtd_dev_t;
#define major(dev) ((unsigned)(((__mtd_dev_t)(dev) >> 8) & 0xfff))
#define minor(dev) ((unsigned)(((__mtd_dev_t)(dev) & 0xff) | \
    (((__mtd_dev_t)(dev) >> 12) & 0xfff00)))
#define makedev(m, n) ((__mtd_dev_t)((((m) & 0xfff) << 8) | \
    ((n) & 0xff) | ((((__mtd_dev_t)(n)) & 0xfff00) << 12)))
#endif
