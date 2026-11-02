#ifndef MTD_LINUX_KERNEL_H
#define MTD_LINUX_KERNEL_H
#include <stddef.h>
#include "linux/types.h"
#ifndef container_of
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
#endif
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif
#endif
