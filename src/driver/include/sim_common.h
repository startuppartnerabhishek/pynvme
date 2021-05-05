#ifndef __DRIVER_SIM_COMMON_H__
#define __DRIVER_SIM_COMMON_H__

#include <stdio.h>

#define __SHORTED_FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define DRVSIM_LOG_UNDECORATED_TO_FILE(_FILE_STAR_, _FMT_, ...)                                                                  \
do {                                                                                            \
    fprintf(_FILE_STAR_, _FMT_, ##__VA_ARGS__);   \
    fflush(stdout);                               \
} while (0)

#define DRVSIM_LOG_TO_FILE(_FILE_STAR_, _FMT_, ...)                                                                  \
do {                                                                                            \
    fprintf(_FILE_STAR_, "%s:%u %s(): "_FMT_, __SHORTED_FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__);   \
    fflush(stdout);                                                                             \
} while (0)

#define DRVSIM_LOG(_FMT_, ...)  DRVSIM_LOG_TO_FILE(stdout, _FMT_, ##__VA_ARGS__)

#define DRVSIM_FATAL_ERROR(_FMT_, ...)  \
do {                                        \
    DRVSIM_LOG(_FMT_, ##__VA_ARGS__);       \
    fflush(stdout);                         \
    fflush(stderr);                         \
    assert(false);                          \
} while (0)

#define DRVSIM_NOT_IMPLEMENTED(_FMT_, ...) DRVSIM_FATAL_ERROR(_FMT_, ##__VA_ARGS__)

#define DRVSIM_NOT_IMPLEMENTED_BENIGN(_FMT_, ...) DRVSIM_LOG_TO_FILE(stderr, _FMT_, ##__VA_ARGS__)

#define DRVSIM_TBD(_FMT_, ...)                          \
do {                                                    \
    DRVSIM_LOG("TBD/TODO:" _FMT_, ##__VA_ARGS__);       \
} while (0)

#define DRVSIM_RETCODE_SUCCESS 0
#define DRVSIM_RETCODE_FAILURE -1

void hex_dump(const void* data, size_t size);

#endif