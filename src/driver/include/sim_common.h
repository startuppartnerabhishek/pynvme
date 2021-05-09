#ifndef __DRIVER_SIM_COMMON_H__
#define __DRIVER_SIM_COMMON_H__

#include <stdio.h>
#include <pthread.h>

#include "driver.h"

/************************  asserting and printing - low level *********************************/

#define __SHORTENED_FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define DRVSIM_LOG_UNDECORATED_TO_FILE(_FILE_STAR_, _FMT_, ...)                                                                  \
do {                                                                                            \
    fprintf(_FILE_STAR_, _FMT_, ##__VA_ARGS__);   \
    fflush(stdout);                               \
} while (0)

#define DRVSIM_LOG_TO_FILE(_FILE_STAR_, _FMT_, ...)                                                                  \
do {                                                                                            \
    fprintf(_FILE_STAR_, "[0x%08lX] %s:%u %s(): "_FMT_, pthread_self(), __SHORTENED_FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__);   \
    fflush(_FILE_STAR_);                                                                             \
} while (0)

#define DRVSIM_LOG(_FMT_, ...)  DRVSIM_LOG_TO_FILE(stdout, _FMT_, ##__VA_ARGS__)

#define DRVSIM_FATAL_ERROR(_FMT_, ...)      \
do {                                        \
    DRVSIM_LOG(_FMT_, ##__VA_ARGS__);       \
    fflush(stdout);                         \
    fflush(stderr);                         \
    assert(false);                          \
} while (0)

#define DRVSIM_ASSERT(COND, _FMT_, ...)                 \
do {                                                    \
    if (!(COND)) {                                      \
        DRVSIM_FATAL_ERROR("SIM SSERTION FAILED ---> " _FMT_, ##__VA_ARGS__);       \
    }                                                   \
} while (0)

#define DRVSIM_NOT_IMPLEMENTED(_FMT_, ...) DRVSIM_FATAL_ERROR(_FMT_, ##__VA_ARGS__)

#define DRVSIM_NOT_IMPLEMENTED_BENIGN(_FMT_, ...) DRVSIM_LOG_TO_FILE(stderr, _FMT_, ##__VA_ARGS__)

#define DRVSIM_TBD(_FMT_, ...)                          \
do {                                                    \
    DRVSIM_LOG("TBD/TODO:" _FMT_, ##__VA_ARGS__);       \
} while (0)


void sim_hex_dump(const void* data, size_t size);

/************************ </end> asserting and printing low level *****************************/



/************************* general defines ***********************************/

#define DRVSIM_RETCODE_SUCCESS 0
#define DRVSIM_RETCODE_FAILURE -1

#define DRVSIM_VERY_LARGE_NUMBER ((unsigned int)(-1))


#define SIM_MAX_STRING_LEN  1024


/**************************** </end> general defines ********************************/


#endif