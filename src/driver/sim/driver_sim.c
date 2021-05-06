#include <stdarg.h>

#include "driver.h"
#include "sim_common.h"
#include "../../../spdk/lib/nvme/nvme_internal.h"

#include "client_interface/agent_interface.h"

#include "cJSON.h"

#define MAX_STRING_LEN  1024

typedef struct sim_config_item_s {
    char *key_name;
    bool isString;
    void *save_to;
} sim_config_item_t;

#define CONF_FILE_PARSE_ENTRY(__field__, __is_string__) {#__field__, __is_string__, &g_sim_config.__field__}

typedef struct sim_config_s { 
    char agent_runtime_rootpath[MAX_STRING_LEN];
    unsigned int dev_no;
    unsigned int vf_no;
    unsigned int sq_size;
    unsigned int cq_size;
    unsigned int nr_cmds;
    unsigned int log_register_reads;
    unsigned int log_register_writes;
    unsigned int log_buf_alloc_free;
    ctrlr_t *p_default_controller;
} sim_config_t;


struct spdk_log_flag SPDK_LOG_NVME = {
  .name = "nvme",
  .enabled = false,
};

/*** global config *****/
static sim_config_t g_sim_config;


static sim_config_item_t g_simcfg_file_items[] = {
    CONF_FILE_PARSE_ENTRY(agent_runtime_rootpath, true),
    CONF_FILE_PARSE_ENTRY(dev_no, false),
    CONF_FILE_PARSE_ENTRY(vf_no, false),
    CONF_FILE_PARSE_ENTRY(sq_size, false),
    CONF_FILE_PARSE_ENTRY(cq_size, false),
    CONF_FILE_PARSE_ENTRY(nr_cmds, false),
    CONF_FILE_PARSE_ENTRY(log_register_reads, false),
    CONF_FILE_PARSE_ENTRY(log_register_writes, false),
    CONF_FILE_PARSE_ENTRY(log_buf_alloc_free, false),
};

static void init_sim_config(char *json_string);

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
    assert(ctrlr && ctrlr->ctrlr_api_handle);
    
    // admin qpair reset is TODO, it is their in pynvme driver.c 
    return enable_driver(ctrlr->ctrlr_api_handle); 
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
  DRVSIM_NOT_IMPLEMENTED_BENIGN("TODO for later: not implemented - needed to handle qpair timeouts\n");
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
    int ret = 0;
    assert(ctrlr && ctrlr->ctrlr_api_handle);

#if 0
    /* this is for raw pci-memory access - not what the test logic expects */
    ret = nvme_ctrlr_get_pcie_registers(
        ctrlr->ctrlr_api_handle, offset, (void *)value, sizeof(unsigned int));
#else
    /* test logic expects BAR access */
    unsigned char *region = (unsigned char *)nvme_ctrlr_get_pci_cmd_region_ptr(ctrlr->ctrlr_api_handle);

    if (g_sim_config.log_register_reads) {
        DRVSIM_LOG("BAR base %p, offset 0x%x => %p\n", region, offset, &region[offset]);
    }

    memcpy(value, &region[offset], sizeof(unsigned int));
#endif

    if (0 == ret) {
        if (g_sim_config.log_register_reads) {
            DRVSIM_LOG("Register Read (32): ctrlr %p, offset 0x%x\n", ctrlr, offset);
            hex_dump(&value, sizeof(unsigned int));
        }
    } else {
        DRVSIM_LOG_TO_FILE(stderr, "Register Read (32) FAILED: ctrlr %p, offset 0x%x, ret-code %d", ctrlr, offset, ret);
    }

    return ret;
}

int nvme_get_reg64(ctrlr_t* ctrlr,
                   unsigned int offset,
                   unsigned long* value)
{
    int ret = 0;

    assert(ctrlr && ctrlr->ctrlr_api_handle);

#if 0
    /* this is for raw pci-memory access - not what the test logic expects */
    ret = nvme_ctrlr_get_pcie_registers(
        ctrlr->ctrlr_api_handle, offset, (void *)value, sizeof(unsigned long));
#else
    /* test logic expects BAR access */
    unsigned char *region = (unsigned char *)nvme_ctrlr_get_pci_cmd_region_ptr(ctrlr->ctrlr_api_handle);

    if (g_sim_config.log_register_reads) {
        DRVSIM_LOG("BAR base %p, offset 0x%x => %p\n", region, offset, &region[offset]);
    }

    memcpy(value, &region[offset], sizeof(unsigned long));
#endif

    if (0 == ret) {
        if (g_sim_config.log_register_reads) {
            DRVSIM_LOG("Register Read (64): ctrlr %p, offset 0x%x\n", ctrlr, offset);
            hex_dump(&value, sizeof(unsigned long));
        }
    } else {
        DRVSIM_LOG_TO_FILE(stderr, "Register Read (64) FAILED: ctrlr %p, offset 0x%x, ret-code %d", ctrlr, offset, ret);
    }

    return ret;
}

int nvme_set_reg32(ctrlr_t* ctrlr,
                   unsigned int offset,
                   unsigned int value)
{
    int ret = 0;

    assert(ctrlr && ctrlr->ctrlr_api_handle);

#if 0
    /* this is for raw pci-memory access - not what the test logic expects */
    ret = nvme_ctrlr_set_pcie_registers(
        ctrlr->ctrlr_api_handle, offset, (void *)&value, sizeof(unsigned int));
#else
    /* test logic expects BAR access */
    unsigned char *region = (unsigned char *)nvme_ctrlr_get_pci_cmd_region_ptr(ctrlr->ctrlr_api_handle);

    if (g_sim_config.log_register_writes) {
        DRVSIM_LOG("BAR base %p, offset 0x%x => %p\n", region, offset, &region[offset]);
    }

    memcpy(&region[offset], &value, sizeof(unsigned int));
#endif

    if (0 == ret) {
        if (g_sim_config.log_register_writes) {
            DRVSIM_LOG("Register Write (32): ctrlr %p, offset 0x%x\n", ctrlr, offset);
            hex_dump(&value, sizeof(unsigned int));
        }
    } else {
        DRVSIM_LOG_TO_FILE(stderr, "Register Write (32) FAILED: ctrlr %p, offset 0x%x, ret-code %d", ctrlr, offset, ret);
    }

    return ret;
}

int nvme_set_reg64(ctrlr_t* ctrlr,
                   unsigned int offset,
                   unsigned long value)
{
    int ret = 0;

    assert(ctrlr && ctrlr->ctrlr_api_handle);

#if 0
    /* this is for raw pci-memory access - not what the test logic expects */
    ret = nvme_ctrlr_set_pcie_registers(
        ctrlr->ctrlr_api_handle, offset, (void *)&value, sizeof(unsigned long));
#else
    /* test logic expects BAR access */
    unsigned char *region = (unsigned char *)nvme_ctrlr_get_pci_cmd_region_ptr(ctrlr->ctrlr_api_handle);

    if (g_sim_config.log_register_writes) {
        DRVSIM_LOG("BAR base %p, offset 0x%x => %p\n", region, offset, &region[offset]);
    }

    memcpy(&region[offset], &value, sizeof(unsigned long));
#endif

    if (0 == ret) {
        if (g_sim_config.log_register_writes) {
            DRVSIM_LOG("Register Write (64): ctrlr %p, offset 0x%x\n", ctrlr, offset);
            hex_dump(&value, sizeof(unsigned long));
        }
    } else {
        DRVSIM_LOG_TO_FILE(stderr, "Register Write (64) FAILED: ctrlr %p, offset 0x%x, ret-code %d", ctrlr, offset, ret);
    }

    return ret;

}

int nvme_cpl_is_error(const struct spdk_nvme_cpl* cpl)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return DRVSIM_RETCODE_FAILURE;
}

static void init_sim_config(char *json_string)
{
    cJSON *conf = cJSON_Parse(json_string);
    cJSON *aConfig;
    unsigned int i;
    const unsigned int max = sizeof(g_simcfg_file_items) / sizeof(g_simcfg_file_items[0]);

    if (NULL == conf) {
        DRVSIM_FATAL_ERROR("Invalid json-config %s\n", json_string);
        return;
    }

    for (i = 0; i < max; i++) {
        aConfig = cJSON_GetObjectItemCaseSensitive(conf, g_simcfg_file_items[i].key_name);

        if (aConfig) {
            if (g_simcfg_file_items[i].isString) {
                if (cJSON_IsString(aConfig) && NULL != aConfig->valuestring) {
                    strcpy((char *)g_simcfg_file_items[i].save_to, aConfig->valuestring);
                    DRVSIM_LOG("[JSON-CFG] %s (STRING) -> %s\n", g_simcfg_file_items[i].key_name,
                        (char *)g_simcfg_file_items[i].save_to);
                    continue;
                }
            } else {
                if (cJSON_IsNumber(aConfig)) {
                    *(int *)g_simcfg_file_items[i].save_to = aConfig->valueint;
                    DRVSIM_LOG("[JSON-CFG] %s (INT) -> %d\n", g_simcfg_file_items[i].key_name,
                        *(int *)g_simcfg_file_items[i].save_to);
                    continue;
                }
            }
        }

        DRVSIM_LOG("[JSON-CFG] %s NOT FOUND\n", g_simcfg_file_items[i].key_name);
    }

    cJSON_Delete(conf);
}

ctrlr_t* nvme_init(char * traddr, unsigned int port)
{
    ctrlr_t *ctrl_opaque_handle = NULL;

    DRVSIM_LOG("[ENTERING] traddr %s, port %u\n", traddr, port);

    init_sim_config(traddr);

    ctrl_opaque_handle = (ctrlr_t *)malloc(sizeof(ctrlr_t));

    assert(ctrl_opaque_handle);

    ctrl_opaque_handle->ctrlr_api_handle =
        create_driver(
            (const char *)&g_sim_config.agent_runtime_rootpath,
            g_sim_config.dev_no,
            g_sim_config.vf_no,
            g_sim_config.sq_size,
            g_sim_config.cq_size,
            g_sim_config.nr_cmds);

    if (!ctrl_opaque_handle->ctrlr_api_handle) {
        free(ctrl_opaque_handle);
        return NULL;
    }

    g_sim_config.p_default_controller = ctrl_opaque_handle;

    DRVSIM_LOG("Returning Driver-API handle %p (wraps api_handle %p)\n",
        ctrl_opaque_handle, ctrl_opaque_handle->ctrlr_api_handle);

    return ctrl_opaque_handle;
}

int nvme_fini(ctrlr_t* ctrlr)
{
    assert(ctrlr && ctrlr->ctrlr_api_handle);

    deallocate_driver(ctrlr->ctrlr_api_handle);

    if (g_sim_config.p_default_controller == ctrlr) {
        g_sim_config.p_default_controller = NULL;
    }

    free(ctrlr);

    DRVSIM_LOG("Driver resources freed\n");

    return DRVSIM_RETCODE_SUCCESS;
}

////module: buffer
///////////////////////////////

void *buffer_init(size_t bytes, uint64_t *phys_addr,
                  uint32_t ptype, uint32_t pvalue)
{
    void *buf;

    assert(g_sim_config.p_default_controller && g_sim_config.p_default_controller->ctrlr_api_handle);

    buf = nvme_driver_buffer_alloc(
            g_sim_config.p_default_controller->ctrlr_api_handle,
                bytes, phys_addr);

    if (g_sim_config.log_buf_alloc_free) {
        DRVSIM_LOG("buf = %p, api-handle %p, size %lu\n",
            buf, g_sim_config.p_default_controller->ctrlr_api_handle, bytes);
    }

    if (!buf) {
        return buf;
    }

    buffer_pattern_init(buf, bytes, ptype, pvalue);

    return buf;
}

void buffer_fini(void* buf)
{
    assert(g_sim_config.p_default_controller && g_sim_config.p_default_controller->ctrlr_api_handle);

    if (g_sim_config.log_buf_alloc_free) {
        DRVSIM_LOG("buf = %p, api-handle %p\n",
            buf, g_sim_config.p_default_controller->ctrlr_api_handle);
    }

    nvme_driver_buffer_free(g_sim_config.p_default_controller->ctrlr_api_handle, buf);

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

pcie_t* pcie_init(ctrlr_t* ctrlr)
{
  return ctrlr; // in SIM mode - these are identical
}

int pcie_cfg_read8(pcie_t* pci,
                   unsigned char* value,
                   unsigned int offset)
{
    unsigned char *region;
    ctrlr_t *ctrlr = pci; 

    assert(ctrlr && ctrlr->ctrlr_api_handle);

    region = (unsigned char *)nvme_ctrlr_get_pci_cmd_region_ptr(ctrlr->ctrlr_api_handle);

    *value = region[offset];

    if (g_sim_config.log_register_reads) {
        DRVSIM_LOG("BAR base %p, offset 0x%x => %p\n", region, offset, &region[offset]);
    }

    if (g_sim_config.log_register_reads) {
        DRVSIM_LOG("CFG Read CMD Region (8): ctrlr %p, offset 0x%x, value 0x%x\n",
            pci, offset, *value);
    }

    return 0;
}

int pcie_cfg_write8(pcie_t* pci,
                    unsigned char value,
                    unsigned int offset)
{
    unsigned char *region;
    ctrlr_t *ctrlr = pci; 

    assert(ctrlr && ctrlr->ctrlr_api_handle);

    region = (unsigned char *)nvme_ctrlr_get_pci_cmd_region_ptr(ctrlr->ctrlr_api_handle);

    region[offset] = value;

    if (g_sim_config.log_register_writes) {
        DRVSIM_LOG("BAR base %p, offset 0x%x => %p\n", region, offset, &region[offset]);
    }

    if (g_sim_config.log_register_writes) {
        DRVSIM_LOG("CFG Write CMD Region (8): ctrlr %p, offset 0x%x, value 0x%x\n",
            pci, offset, value);
    }

    return 0;
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