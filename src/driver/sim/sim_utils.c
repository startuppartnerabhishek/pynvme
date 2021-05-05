#include <stdio.h>

#include "sim_common.h"

void hex_dump(const void* data, size_t size)
{
	char line_as_string[17];
	size_t i, j;

	line_as_string[16] = '\0';
	for (i = 0; i < size; ++i) {
		DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, "%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			line_as_string[i % 16] = ((unsigned char*)data)[i];
		} else {
			line_as_string[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, " ");
			if ((i+1) % 16 == 0) {
				DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, "|  %s \n", line_as_string);
			} else if (i+1 == size) {
				line_as_string[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, " ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, "   ");
				}
				DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, "|  %s \n", line_as_string);
			}
		}
	}
}