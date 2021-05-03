#include <stdarg.h>

#include "driver.h"
#include "sim_common.h"
#include "../../../spdk/lib/nvme/nvme_internal.h"


struct spdk_log_flag SPDK_LOG_NVME = {
  .name = "nvme",
  .enabled = false,
};

////module: qpair
///////////////////////////////

struct spdk_nvme_qpair *qpair_create(ctrlr_t* ctrlr,
                                     unsigned int prio,
                                     unsigned int depth,
                                     bool ien,
                                     unsigned short iv)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return NULL;
}

int qpair_free(struct spdk_nvme_qpair* q)
{
  assert(q != NULL);
  DRVSIM_LOG("%s: no cleanup required at this time for qid %d\n", __FUNCTION__, q->id);
  return DRVSIM_RETCODE_FAILURE;
}

int qpair_wait_completion(struct spdk_nvme_qpair *qpair, uint32_t max_completions)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return DRVSIM_RETCODE_FAILURE;
}


uint16_t qpair_get_latest_cid(struct spdk_nvme_qpair* q,
                              ctrlr_t* c)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return (uint16_t)DRVSIM_RETCODE_FAILURE; 
}

uint32_t qpair_get_latest_latency(struct spdk_nvme_qpair* q,
                                  ctrlr_t* c)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return DRVSIM_RETCODE_FAILURE;
}

int nvme_set_ns(ctrlr_t *ctrlr)
{
  assert(ctrlr != NULL);
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return DRVSIM_RETCODE_FAILURE;
}

struct spdk_nvme_ns* nvme_get_ns(ctrlr_t* ctrlr,
                                 uint32_t nsid)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return NULL;
}

int nvme_set_reg32(ctrlr_t* ctrlr,
                   unsigned int offset,
                   unsigned int value)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return DRVSIM_RETCODE_FAILURE;
}

int nvme_send_cmd_raw(ctrlr_t* ctrlr,
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
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return DRVSIM_RETCODE_FAILURE;
}

int nvme_set_adminq(ctrlr_t *ctrlr)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return DRVSIM_RETCODE_FAILURE;
}

int nvme_wait_completion_admin(ctrlr_t* ctrlr)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return DRVSIM_RETCODE_FAILURE;
}

void nvme_register_timeout_cb(ctrlr_t* ctrlr,
                              spdk_nvme_timeout_cb timeout_cb,
                              unsigned int msec)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return;
}

struct spdk_nvme_ns* ns_init(ctrlr_t* ctrlr,
                             uint32_t nsid,
                             uint64_t nlba_verify)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return NULL;
}

int ns_refresh(struct spdk_nvme_ns *ns, uint32_t id,
               ctrlr_t *ctrlr)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return DRVSIM_RETCODE_FAILURE;
}

uint32_t ns_get_sector_size(struct spdk_nvme_ns* ns)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return 0;
}

int ns_cmd_io(uint8_t opcode,
              struct spdk_nvme_ns* ns,
              struct spdk_nvme_qpair* qpair,
              void* buf,
              size_t len,
              uint64_t lba,
              uint32_t lba_count,
              uint32_t io_flags,
              spdk_nvme_cmd_cb cb_fn,
              void* cb_arg,
              unsigned int dword13,
              unsigned int dword14,
              unsigned int dword15)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return DRVSIM_RETCODE_FAILURE;
}

int ns_fini(struct spdk_nvme_ns* ns)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return DRVSIM_RETCODE_FAILURE;
}

////module: log
///////////////////////////////

char *log_buf_dump(const char* header, const void* buf, size_t len, size_t base)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return NULL;
}

void log_cmd_dump(struct spdk_nvme_qpair* qpair, size_t count)
{
  // TODO/TBD
  DRVSIM_TBD("not implemented\n");
  return;
}

void log_cmd_dump_admin(ctrlr_t* ctrlr, size_t count)
{
  // TODO/TBD
  DRVSIM_TBD("not implemented\n");
  return;
}

void nvme_bar_remap(ctrlr_t* ctrlr)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return;
}

void nvme_bar_recover(ctrlr_t* ctrlr)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return;
}

int nvme_get_reg32(ctrlr_t* ctrlr,
                   unsigned int offset,
                   unsigned int* value)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return DRVSIM_RETCODE_FAILURE;
}

int nvme_get_reg64(ctrlr_t* ctrlr,
                   unsigned int offset,
                   unsigned long* value)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return DRVSIM_RETCODE_FAILURE;
}

int nvme_cpl_is_error(const struct spdk_nvme_cpl* cpl)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return DRVSIM_RETCODE_FAILURE;
}

int nvme_fini(ctrlr_t* ctrlr)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return DRVSIM_RETCODE_FAILURE;
}

ctrlr_t* nvme_init(char * traddr, unsigned int port)
{
    DRVSIM_LOG("traddr %s, port %u\n", traddr, port);
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return NULL;
}

////module: buffer
///////////////////////////////

void *buffer_init(size_t bytes, uint64_t *phys_addr,
                  uint32_t ptype, uint32_t pvalue)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return NULL;
}

void buffer_fini(void* buf)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return;
}

int driver_fini(void)
{
    // clear global shared data

    DRVSIM_LOG("%s: no cleanup required at this time\n", __FUNCTION__);

    return DRVSIM_RETCODE_FAILURE;
}

void driver_init_num_queues(ctrlr_t* ctrlr, uint32_t cdw0)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return;
}

int driver_init(void)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return DRVSIM_RETCODE_FAILURE;
}

////module: pcie ctrlr
///////////////////////////////

struct spdk_pci_device* pcie_init(ctrlr_t* ctrlr)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return NULL;
}

int pcie_cfg_read8(struct spdk_pci_device* pci,
                   unsigned char* value,
                   unsigned int offset)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return DRVSIM_RETCODE_FAILURE;
}

int pcie_cfg_write8(struct spdk_pci_device* pci,
                    unsigned char value,
                    unsigned int offset)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return DRVSIM_RETCODE_FAILURE;
}

bool driver_no_secondary(ctrlr_t* ctrlr)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return false;
}

void crc32_unlock_all(ctrlr_t* ctrlr)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return;
}

void spdk_log(enum spdk_log_level level, const char *file, const int line, const char *func,
         const char *format, ...)
{
    va_list args;
    char log_line_fmt[1024] = {};
    char log_line[2048] = {};

    va_start(args, format);
    
    sprintf(log_line_fmt, "[%s:%u] %s: %s", file, line, func, format);
    vsprintf(log_line, log_line_fmt, args);

    va_end(args);

    DRVSIM_LOG("%s\n", log_line);
}