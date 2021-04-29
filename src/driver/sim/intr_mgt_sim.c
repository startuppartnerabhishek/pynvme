#include "intr_mgt.h"
#include "sim_common.h"
#include "../../../spdk/lib/nvme/nvme_internal.h"

void intc_clear(struct spdk_nvme_qpair* q)
{
  DRVSIM_LOG("%s: not implemented\n", __FUNCTION__);
  return;
}

void intc_mask(struct spdk_nvme_qpair *q)
{
  DRVSIM_LOG("%s: not implemented\n", __FUNCTION__);
  return;
}

void intc_unmask(struct spdk_nvme_qpair *q)
{
  DRVSIM_LOG("%s: not implemented\n", __FUNCTION__);
  return;
}