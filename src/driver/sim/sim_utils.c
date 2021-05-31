#include <stdio.h>
#include <time.h>

#include "sim_common.h"

#define BYTES_BETWEEN_SEPARATOR 8
#define BYTES_PER_LINE          16

void sim_hex_dump(const void *data, size_t size, FILE *out_fp)
{
	char line_as_string[BYTES_PER_LINE  + 1];
	size_t i, j;

    if (!size) {
        return;
    }

    out_fp = (NULL == out_fp) ? stdout : out_fp;

    DRVSIM_LOG_UNDECORATED_TO_FILE(out_fp, "0x%016lX: %08lx  ", (size_t)data, (size_t)0);

	line_as_string[BYTES_PER_LINE] = '\0';
	for (i = 0; i < size; ++i) {
		DRVSIM_LOG_UNDECORATED_TO_FILE(out_fp, "%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			line_as_string[i % BYTES_PER_LINE] = ((unsigned char*)data)[i];
		} else {
			line_as_string[i % BYTES_PER_LINE] = '.';
		}
		if ((i+1) % BYTES_BETWEEN_SEPARATOR == 0 || i+1 == size) {
			DRVSIM_LOG_UNDECORATED_TO_FILE(out_fp, " ");
			if ((i+1) % BYTES_PER_LINE == 0) {

                DRVSIM_LOG_UNDECORATED_TO_FILE(out_fp, "|  %s \n", line_as_string);

                if (i + 1 < size) { // at least one more line is needed
                    DRVSIM_LOG_UNDECORATED_TO_FILE(out_fp, "0x%016lX: %08lx  ", (size_t)data + i, i + 1);
                }

			} else if (i+1 == size) {
				line_as_string[(i+1) % BYTES_PER_LINE] = '\0';
				if ((i+1) % BYTES_PER_LINE <= BYTES_BETWEEN_SEPARATOR) {
					DRVSIM_LOG_UNDECORATED_TO_FILE(out_fp, " ");
				}
				for (j = (i+1) % BYTES_PER_LINE; j < BYTES_PER_LINE; ++j) {
					DRVSIM_LOG_UNDECORATED_TO_FILE(out_fp, "   ");
				}
				DRVSIM_LOG_UNDECORATED_TO_FILE(out_fp, "|  %s \n", line_as_string);
			}
		}
	}
}

void sim_sleep(unsigned int seconds, unsigned int nanoseconds)
{
    struct timespec req, rem;
    int ret;

    req.tv_sec = seconds;
    req.tv_nsec = nanoseconds;

    ret = nanosleep(&req, &rem);

    if (ret) {
        DRVSIM_LOG_TO_FILE(stderr, "WARNING - nanosleep returned %d\n", ret);
    }

    return;
}