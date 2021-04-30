#ifndef __DRIVER_SIM_COMMON_H__
#define __DRIVER_SIM_COMMON_H__

#include <stdio.h>

#define DRVSIM_LOG(_FMT_, ...) printf((_FMT_), __VA_ARGS__)

#define DRVSIM_NOT_IMPLEMENTED(_FMT_, ...)  \
do {                                        \
    DRVSIM_LOG((_FMT_), __VA_ARGS__);       \
    assert(false);                          \
} while (0)

#define DRVSIM_TBD(_FMT_, ...)             \
do {                                \
    DRVSIM_LOG("TBD/TODO:" _FMT_, __VA_ARGS__);     \
} while (0)

#define DRVSIM_RETCODE_SUCCESS 0
#define DRVSIM_RETCODE_FAILURE -1

#endif