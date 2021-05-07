#include <stdio.h>

#include "sim_common.h"

#define BYTES_BETWEEN_SEPARATOR 8
#define BYTES_PER_LINE          16

void sim_hex_dump(const void* data, size_t size)
{
	char line_as_string[BYTES_PER_LINE  + 1];
	size_t i, j;

	line_as_string[BYTES_PER_LINE] = '\0';
	for (i = 0; i < size; ++i) {
		DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, "%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			line_as_string[i % BYTES_PER_LINE] = ((unsigned char*)data)[i];
		} else {
			line_as_string[i % BYTES_PER_LINE] = '.';
		}
		if ((i+1) % BYTES_BETWEEN_SEPARATOR == 0 || i+1 == size) {
			DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, " ");
			if ((i+1) % BYTES_PER_LINE == 0) {
				DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, "|  %s \n", line_as_string);
			} else if (i+1 == size) {
				line_as_string[(i+1) % BYTES_PER_LINE] = '\0';
				if ((i+1) % BYTES_PER_LINE <= BYTES_BETWEEN_SEPARATOR) {
					DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, " ");
				}
				for (j = (i+1) % BYTES_PER_LINE; j < BYTES_PER_LINE; ++j) {
					DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, "   ");
				}
				DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, "|  %s \n", line_as_string);
			}
		}
	}
}