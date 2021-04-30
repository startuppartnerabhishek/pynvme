#include "intr_mgt.h"
#include "sim_common.h"
#include "../../../spdk/lib/nvme/nvme_internal.h"

void intc_clear(struct spdk_nvme_qpair* q)
{
  DRVSIM_NOT_IMPLEMENTED("%s: not implemented\n", __FUNCTION__);
  return;
}

void intc_mask(struct spdk_nvme_qpair *q)
{
  DRVSIM_NOT_IMPLEMENTED("%s: not implemented\n", __FUNCTION__);
  return;
}

void intc_unmask(struct spdk_nvme_qpair *q)
{
  DRVSIM_NOT_IMPLEMENTED("%s: not implemented\n", __FUNCTION__);
  return;
}

bool intc_isset(struct spdk_nvme_qpair *q)
{
  DRVSIM_NOT_IMPLEMENTED("%s: not implemented\n", __FUNCTION__);
  return false;
}