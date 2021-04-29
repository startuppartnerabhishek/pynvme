/* driver.h interface implemenations that are common to SIM and PCIE-ASIC environments */

#include "driver.h"


int pen_common_connectivity_check(char *src, char *dst, unsigned int count, int return_this)
{
    // #error Woo-hoo - I can see me compile

    if (count > 0)
    {
        memcpy(dst, src, count);
    }

    return return_this;
}
