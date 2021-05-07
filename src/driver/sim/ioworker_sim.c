#include "driver.h"
#include "sim_common.h"

int ioworker_entry(struct spdk_nvme_ns* ns,
                   qpair_t *qpair,
                   struct ioworker_args* args,
                   struct ioworker_rets* rets)
{
    return DRVSIM_RETCODE_FAILURE;
}
