#ifndef __DRIVER_SIM_COMMON_H__
#define __DRIVER_SIM_COMMON_H__

#include <stdio.h>

#define __SHORTED_FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define DRVSIM_LOG(_FMT_, ...) printf("%s:%u %s(): "_FMT_, __SHORTED_FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define DRVSIM_NOT_IMPLEMENTED(_FMT_, ...)  \
do {                                        \
    DRVSIM_LOG(_FMT_, ##__VA_ARGS__);     \
    fflush(stdout);                         \
    fflush(stderr);                         \
    assert(false);                          \
} while (0)

#define DRVSIM_TBD(_FMT_, ...)                          \
do {                                                    \
    DRVSIM_LOG("TBD/TODO:" _FMT_, ##__VA_ARGS__);       \
} while (0)

#define DRVSIM_RETCODE_SUCCESS 0
#define DRVSIM_RETCODE_FAILURE -1

#endif