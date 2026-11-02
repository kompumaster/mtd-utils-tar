/* What autotools normally passes via config.h, for the manual build. */
#ifndef MTD_LINUX_BUILD_H
#define MTD_LINUX_BUILD_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#include <stdint.h>

typedef int64_t loff_t;

#ifndef VERSION
#define VERSION "2.3.1"
#endif


#endif
