
#include "driver.h"
#include "sim_common.h"
#include "../../../spdk/lib/nvme/nvme_internal.h"

int driver_fini(void)
{
    // clear global shared data

    DRVSIM_LOG("%s: no cleanup required at this time\n", __FUNCTION__);

    return DRVSIM_RETCODE_SUCCESS;
}

int qpair_free(struct spdk_nvme_qpair* q)
{
  assert(q != NULL);
  DRVSIM_LOG("%s: no cleanup required at this time for qid %d\n", __FUNCTION__, q->id);
  return DRVSIM_RETCODE_SUCCESS;
}

int nvme_set_ns(struct spdk_nvme_ctrlr *ctrlr)
{
  assert(ctrlr != NULL);
  DRVSIM_NOT_IMPLEMENTED("%s: not implemented\n", __FUNCTION__);
  return DRVSIM_RETCODE_FAILURE;
}

int nvme_send_cmd_raw(struct spdk_nvme_ctrlr* ctrlr,
                      struct spdk_nvme_qpair *qpair,
                      unsigned int cdw0,
                      unsigned int nsid,
                      void* buf, size_t len,
                      unsigned int cdw10,
                      unsigned int cdw11,
                      unsigned int cdw12,
                      unsigned int cdw13,
                      unsigned int cdw14,
                      unsigned int cdw15,
                      spdk_nvme_cmd_cb cb_fn,
                      void* cb_arg)
{
  DRVSIM_NOT_IMPLEMENTED("%s: not implemented\n", __FUNCTION__);
  return DRVSIM_RETCODE_FAILURE;
}

struct spdk_nvme_ns* ns_init(struct spdk_nvme_ctrlr* ctrlr,
                             uint32_t nsid,
                             uint64_t nlba_verify)
{
  DRVSIM_NOT_IMPLEMENTED("%s: not implemented\n", __FUNCTION__);
  return NULL;
}

void log_cmd_dump_admin(struct spdk_nvme_ctrlr* ctrlr, size_t count)
{
  // TODO/TBD
  DRVSIM_TBD("%s: not implemented\n", __FUNCTION__);
  return;
}

void nvme_bar_remap(struct spdk_nvme_ctrlr* ctrlr)
{
  DRVSIM_NOT_IMPLEMENTED("%s: not implemented\n", __FUNCTION__);
  return;
}

void nvme_bar_recover(struct spdk_nvme_ctrlr* ctrlr)
{
  DRVSIM_NOT_IMPLEMENTED("%s: not implemented\n", __FUNCTION__);
  return;
}

int nvme_get_reg32(struct spdk_nvme_ctrlr* ctrlr,
                   unsigned int offset,
                   unsigned int* value)
{
    DRVSIM_NOT_IMPLEMENTED("%s: not implemented\n", __FUNCTION__);
    return DRVSIM_RETCODE_FAILURE;
}