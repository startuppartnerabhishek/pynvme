#include <stdarg.h>

#include "sim_common.h"
#include "sim_types.h"
#include "../../../spdk/lib/nvme/nvme_internal.h"

#include "client_interface/agent_interface.h"

#include "cJSON.h"


struct spdk_log_flag SPDK_LOG_NVME = {
  .name = "nvme",
  .enabled = false,
};

typedef struct sim_config_item_s {
    char *key_name;
    bool isString;
    void *save_to;
} sim_config_item_t;

#define CONF_FILE_PARSE_ENTRY(__field__, __is_string__) {#__field__, __is_string__, &g_sim_config.__field__}

/*** global config *****/
sim_config_t g_sim_config;

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
    CONF_FILE_PARSE_ENTRY(max_log_entries_per_qpair, false),
    CONF_FILE_PARSE_ENTRY(log_dump_adminq_completion_len, false)
};

static void init_sim_config(char *json_string);
static void drvsim_handle_completion(void *cb_args, nvme_ctrlr_completion_t *cmpl);
static qpair_t *sim_allocate_qpair(ctrlr_t *ctrlr, bool is_adminq);
static void sim_free_qpair(qpair_t *q);
static int sim_qpair_process_completions(qpair_t *q, unsigned int max);

////module: qpair
///////////////////////////////

qpair_t *qpair_create(
                    ctrlr_t* ctrlr,
                    unsigned int prio,
                    unsigned int depth,
                    bool ien,
                    unsigned short iv)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");

  return NULL;
}

int qpair_free(qpair_t* q)
{
  DRVSIM_ASSERT((q != NULL), "q cannot be NULL\n");
  DRVSIM_NOT_IMPLEMENTED("%s: no cleanup required at this time for qid %d\n", __FUNCTION__, q->id);

  /* call agent-api first */
  sim_free_qpair(q);

  return DRVSIM_RETCODE_FAILURE;
}

int qpair_wait_completion(qpair_t *qpair, uint32_t max_completions)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return DRVSIM_RETCODE_FAILURE;
}


uint16_t qpair_get_latest_cid(qpair_t* q,
                              ctrlr_t* c)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return (uint16_t)DRVSIM_RETCODE_FAILURE; 
}

uint32_t qpair_get_latest_latency(qpair_t* q,
                                  ctrlr_t* c)
{
    DRVSIM_NOT_IMPLEMENTED("not implemented\n");
    return DRVSIM_RETCODE_FAILURE;
}

int nvme_set_ns(ctrlr_t *ctrlr)
{
  DRVSIM_ASSERT((ctrlr != NULL), "ctrlr cannot be NULL\n");
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return DRVSIM_RETCODE_FAILURE;
}

struct spdk_nvme_ns* nvme_get_ns(ctrlr_t* ctrlr,
                                 uint32_t nsid)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return NULL;
}

static void drvsim_handle_completion(void *cb_args, nvme_ctrlr_completion_t *cmpl)
{
    completion_cb_context_t *compl_ctx = (completion_cb_context_t *)cb_args;
    struct spdk_nvme_cpl *spec_completion = (struct spdk_nvme_cpl *)cmpl;    

    DRVSIM_LOG("Got completion with args %p -> translate to %p, "
        "cmd-id 0x%x, phase-tag %u, status code %u, type %u\n",
        cb_args, compl_ctx->response_args, spec_completion->cid,
        spec_completion->status.p, spec_completion->status.sc, spec_completion->status.sct);

    /*  both agent's and spdk's completion structures are from the spec, so
        we should be ok to just type-cast and pass on. */
    sim_handle_completion((sim_cmd_log_entry_t *)cb_args, (cpl *)cmpl);

    return;
}

sim_cmd_log_entry_t *sim_add_cmd_log_entry(
                            qpair_t *qpair,
                            unsigned int cdw0,
                            unsigned int nsid,
                            void* buf, size_t len,
                            unsigned int cdw10,
                            unsigned int cdw11,
                            unsigned int cdw12,
                            unsigned int cdw13,
                            unsigned int cdw14,
                            unsigned int cdw15,
                            cmd_cb_func cb_fn,
                            void* cb_arg)
{
    sim_cmd_log_entry_t *e = (sim_cmd_log_entry_t *)malloc(sizeof(sim_cmd_log_entry_t));
    
    if (!e) {
        return NULL;
    }

    memset(e, 0, sizeof(sim_cmd_log_entry_t));

    *(unsigned int*)&e->cmd = cdw0;
    e->cmd.nsid = nsid;
    e->cmd.cdw10 = cdw10;
    e->cmd.cdw11 = cdw11;
    e->cmd.cdw12 = cdw12;
    e->cmd.cdw13 = cdw13;
    e->cmd.cdw14 = cdw14;
    e->cmd.cdw15 = cdw15;

    e->qpair = qpair;

    e->cb_ctx.cb_fn = cb_fn;
    e->cb_ctx.response_args = cb_arg;

    e->response_buf = buf;
    e->response_buf_len = len;

    DRVSIM_LOG("For qp %p, adding cmd entry for opc %u, cid 0x%x - expect callback with ctx %p\n",
                    qpair, e->cmd.opc, e->cmd.cid, e);    

    pthread_mutex_lock(&qpair->lock);

    /* connect to qpair - just behind the tail */
    qpair->commands_sent++;

    if (qpair->log_list_head) {
        sim_cmd_log_entry_t *prev = qpair->log_list_head->prev; /* => tail */
        sim_cmd_log_entry_t *next = qpair->log_list_head;

        e->next = next;
        e->prev = prev;
        prev->next = e;
        next->prev = e;
    } else {
        qpair->log_list_head = e;
        e->next = e;
        e->prev = e;
    }

    qpair->log_entry_count++;

    DRVSIM_LOG("QP %p counters: cmds sent %u, responses received %u, log entry count %u, completions collected %u\n",
        qpair, qpair->commands_sent, qpair->responses_received, qpair->log_entry_count, qpair->completions_collected);

    pthread_mutex_unlock(&qpair->lock);

    return e;
}

int sim_handle_completion(sim_cmd_log_entry_t *completion_ctx, cpl *cqe)
{
    DRVSIM_ASSERT((completion_ctx && completion_ctx->qpair),
        "completion_ctx %p && completion_ctx->qpair\n", completion_ctx);

    qpair_t *qpair = completion_ctx->qpair;

    DRVSIM_ASSERT((completion_ctx->is_completed == false),
        "completion_ctx %p with cid %x is already completed\n", completion_ctx, completion_ctx->cmd.cid);

    DRVSIM_ASSERT((completion_ctx->cmd.cid == cqe->cid),  // wrong context?
        "completion ctx %p cmd.cid 0x%x does not match cid in completion 0x%x\n",
        completion_ctx, completion_ctx->cmd.cid, cqe->cid);

    pthread_mutex_lock(&qpair->lock);

    completion_ctx->is_completed = true;

    completion_ctx->qpair->responses_received++;

    memcpy(&completion_ctx->cpl, cqe, sizeof(cpl));

    if (completion_ctx->cb_ctx.cb_fn) {
        completion_ctx->cb_ctx.cb_fn(completion_ctx->cb_ctx.response_args, cqe);
    }

    if (g_sim_config.max_log_entries_per_qpair > 0 &&
            completion_ctx->qpair->log_entry_count > g_sim_config.max_log_entries_per_qpair) {
        prune_completion_table(
            completion_ctx->qpair,
            completion_ctx->qpair->log_entry_count - g_sim_config.max_log_entries_per_qpair
        );
    }

    pthread_mutex_unlock(&qpair->lock);

    DRVSIM_LOG("QP %p counters: cmds sent %u, responses received %u, "
            "log entry count %u, completions collected %u\n",
        qpair, qpair->commands_sent, qpair->responses_received,
        qpair->log_entry_count, qpair->completions_collected);

    return 0;
}

/* delete a few completed log-entries - for now in addition order,
 * but it may make sense to consider cleaning in completion order
 */
int prune_completion_table(qpair_t *qpair, unsigned int max_clean)
{
    int cleaned = 0;
    sim_cmd_log_entry_t *e = qpair->log_list_head;
    sim_cmd_log_entry_t *next;
    const unsigned int max_entries_to_scan = qpair->log_entry_count;
    unsigned int scanned = 0;

    pthread_mutex_lock(&qpair->lock);

    while (cleaned < max_clean && scanned < max_entries_to_scan) {
        if (!e) {
            // how can this happen - caller made a mistake?
            break;
        }

        next = e->next;

        scanned++;

        if (e->is_completed && e->is_processed) { // actually is_processed implies is completed
            sim_cmd_log_entry_t *prev = e->prev;

            cleaned++;
            qpair->log_entry_count--;
            free(e);

            if (qpair->log_entry_count > 0) {
                /* detach e from linked list */
                prev->next = next;
                next->prev = prev;
            } else {
                qpair->log_list_head = NULL;
            }
        }

        e = next;
    }

    pthread_mutex_unlock(&qpair->lock);

    return cleaned;
}

static qpair_t *sim_allocate_qpair(ctrlr_t *ctrlr, bool is_adminq)
{
    qpair_t *q = (qpair_t *)malloc(sizeof(qpair_t));

    DRVSIM_ASSERT((q), "q alloc failed\n");

    memset(q, 0, sizeof(qpair_t));

    q->parent_controller = ctrlr;

    pthread_mutex_init(&q->lock, NULL);

    if (is_adminq) {
        DRVSIM_ASSERT((ctrlr->adminq == NULL),
            "ctrlr %p, admin q %p was already allocated\n", ctrlr, ctrlr->adminq);
        ctrlr->adminq = q;
    } else {
        if (ctrlr->other_queues_list) {
            qpair_t *next = ctrlr->other_queues_list;
            qpair_t *prev = ctrlr->other_queues_list->prev;

            q->next = next;
            q->prev = prev;

            next->prev = q;
            prev->next = q;
        } else {
            ctrlr->other_queues_list = q;
            q->next = q;
            q->prev = q;
        }
    }

    return q;
}

static int sim_qpair_process_completions(qpair_t *q, unsigned int max)
{
    int processed = 0;
    int scanned = 0;
    DRVSIM_ASSERT((q), "q cannot be NULL\n");
    sim_cmd_log_entry_t *e = q->log_list_head;
    const unsigned int log_entry_count = q->log_entry_count;

    pthread_mutex_lock(&q->lock);

    while (processed < max && scanned < log_entry_count) {
        scanned++;

        DRVSIM_ASSERT((e), "qpair %p has unexpected NULL cmd entry\n", q);

        if (e->is_completed) {
            if (!e->is_processed) {
                // whatever processing means, goes here -> log it, maybe?
                e->is_processed = true;
                processed++;
                q->completions_collected++;

                if (g_sim_config.log_dump_adminq_completion_len) {
                    DRVSIM_LOG("commmand:\n");
                    sim_hex_dump(&e->cmd, sizeof(e->cmd));
                    DRVSIM_LOG("completion:\n");
                    sim_hex_dump(&e->cpl, sizeof(e->cpl));
                    if (e->response_buf && e->response_buf_len) {
                        DRVSIM_LOG("response buffer:\n");
                        sim_hex_dump(e->response_buf,
                            e->response_buf_len > g_sim_config.log_dump_adminq_completion_len ?
                                g_sim_config.log_dump_adminq_completion_len : e->response_buf_len);
                    }
                }
            }
        }

        e = e->next;
    }

    if (processed) {
        DRVSIM_LOG("QP %p counters: cmds sent %u, responses received %u, log entry count %u, completions collected %u\n",
            q, q->commands_sent, q->responses_received, q->log_entry_count, q->completions_collected);
    }

    pthread_mutex_unlock(&q->lock);

    return processed;
}

static void sim_free_qpair(qpair_t *q)
{
    DRVSIM_ASSERT((q), "q cannot be NULL\n");
    DRVSIM_ASSERT((q->parent_controller), "q %p has NULL controller\n", q);

    qpair_t *next = q->next;
    qpair_t *prev = q->prev;

    free_completion_table(q);

    pthread_mutex_destroy(&q->lock);

    if (q == next) {
        // there was only this entry in the queue
        q->parent_controller = NULL;
    } else {
        prev->next = next;
        next->prev = prev;
    }

    free(q);

    return;
}

void free_log_entry(sim_cmd_log_entry_t *e)
{
    DRVSIM_ASSERT((e), "NULL free");

    qpair_t *qpair = e->qpair;
    sim_cmd_log_entry_t *next = e->next;
    sim_cmd_log_entry_t *prev = e->prev;

    DRVSIM_ASSERT((qpair), "log entry %p associated with NULL qp\n", e);
    DRVSIM_ASSERT((qpair->log_entry_count), "log entry %p of qp %p, but qp has no log entries\n", e, qpair);

    pthread_mutex_lock(&qpair->lock);

    free(e);

    qpair->log_entry_count--;

    if (qpair->log_entry_count) {
        next->prev = prev;
        prev->next = next;
    } else {
        qpair->log_list_head = NULL;
    }

    pthread_mutex_unlock(&qpair->lock);

    return;
}

int free_completion_table(qpair_t *qpair)
{
    int cleaned = 0;
    const unsigned int initial_entry_count = qpair->log_entry_count;

    pthread_mutex_lock(&qpair->lock);

    while (qpair->log_list_head) {
        DRVSIM_ASSERT((qpair->log_entry_count > 0),
            "qpair %p, list head %p, log entry count %u at 0, cleaned %d, max %u\n",
            qpair, qpair->log_list_head, qpair->log_entry_count, cleaned, initial_entry_count);
        cleaned++;
        free_log_entry(qpair->log_list_head);
    }

    DRVSIM_ASSERT((cleaned == initial_entry_count),
        "qpair %p, cleaned only %d out of max %u\n", qpair, cleaned, initial_entry_count);
    
    DRVSIM_ASSERT((qpair->log_entry_count == 0),
        "qpair %p still reports %u after cleaning %d entry(s), max %u\n",
        qpair, qpair->log_entry_count, cleaned, initial_entry_count);

    pthread_mutex_unlock(&qpair->lock);

    return cleaned;
}

int nvme_send_cmd_raw(ctrlr_t* ctrlr,
                      qpair_t *qpair,
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
    int ret;
    sim_cmd_log_entry_t *completion_ctx = NULL;

    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle), "invalid ctrlr %p or uninitialized api handle\n", ctrlr);

    DRVSIM_LOG("ENTERED with ctrlr %p, qpair %p, buf %p of len %lu, cdw0 0x%08x, "
                    "nsid 0x%08x, cdw10 = 0x%08x, cb_args %p\n", ctrlr, qpair,
                    buf, len, cdw0, nsid, cdw10, cb_arg);

    if (qpair) {
        DRVSIM_NOT_IMPLEMENTED("qpair commands are not implemented\n");
        return DRVSIM_RETCODE_FAILURE;
    } else {
        DRVSIM_ASSERT((ctrlr->adminq), "ctrlr %p does not have adminq\n", ctrlr);

        completion_ctx = sim_add_cmd_log_entry(
                      ctrlr->adminq,
                      cdw0,
                      nsid,
                      buf, len,
                      cdw10,
                      cdw11,
                      cdw12,
                      cdw13,
                      cdw14,
                      cdw15,
                      cb_fn, cb_arg);
    }

    DRVSIM_ASSERT((completion_ctx), "could not allocate completion ctx\n");

    ret = nvme_ctrlr_submit_command(
        ctrlr->ctrlr_api_handle,
        cdw0,
        nsid,
        buf, len,
        cdw10,
        cdw11,
        cdw12,
        cdw13,
        cdw14,
        cdw15,
        drvsim_handle_completion,
        completion_ctx
    );

    if (0 != ret) {
        free_log_entry(completion_ctx);
        return DRVSIM_RETCODE_FAILURE;
    }

    return ret;
}

int nvme_set_adminq(ctrlr_t *ctrlr)
{
    int ret = 0;

    DRVSIM_ASSERT((ctrlr), "NULL ctrlr\n");

    DRVSIM_ASSERT((ctrlr->ctrlr_api_handle && NULL == ctrlr->adminq),
        "api handle %p should not be NULL, and adminq %p should be NULL!\n",
        ctrlr->ctrlr_api_handle, ctrlr->adminq);
    
    // admin qpair reset is TODO, it is their in pynvme driver.c 
    ret = enable_driver(ctrlr->ctrlr_api_handle);

    qpair_t *adminq = sim_allocate_qpair(ctrlr, true);

    DRVSIM_ASSERT((adminq), "ctrlr %p, adminq alloc failed\n", ctrlr);

    return ret;
}

int nvme_wait_completion_admin(ctrlr_t* ctrlr)
{
    int processed = 0;

    DRVSIM_ASSERT((ctrlr && ctrlr->adminq), "ctrlr %p and adminq should be non-NULL\n", ctrlr);

    processed = sim_qpair_process_completions(ctrlr->adminq, DRVSIM_VERY_LARGE_NUMBER);

    if (processed) {
        DRVSIM_LOG("ctrlr %p, adminq %p, collected %d completion(s)\n", ctrlr, ctrlr->adminq, processed);
    }

    return processed;
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
              qpair_t* qpair,
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
    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle), "invalid ctrlr %p or uninitialized api handle\n", ctrlr);

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
            sim_hex_dump(&value, sizeof(unsigned int));
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

    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle), "invalid ctrlr %p or uninitialized api handle\n", ctrlr);

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
            sim_hex_dump(&value, sizeof(unsigned long));
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

    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle), "invalid ctrlr %p or uninitialized api handle\n", ctrlr);

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
            sim_hex_dump(&value, sizeof(unsigned int));
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

    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle), "invalid ctrlr %p or uninitialized api handle\n", ctrlr);

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
            sim_hex_dump(&value, sizeof(unsigned long));
        }
    } else {
        DRVSIM_LOG_TO_FILE(stderr, "Register Write (64) FAILED: ctrlr %p, offset 0x%x, ret-code %d", ctrlr, offset, ret);
    }

    return ret;

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
    ctrlr_t *ctrlr_opaque_handle = NULL;

    DRVSIM_LOG("[ENTERING] traddr %s, port %u\n", traddr, port);

    init_sim_config(traddr);

    ctrlr_opaque_handle = (ctrlr_t *)malloc(sizeof(ctrlr_t));

    DRVSIM_ASSERT((ctrlr_opaque_handle), "opaque handle alloc failed\n");

    memset(ctrlr_opaque_handle, 0, sizeof(ctrlr_t));

    ctrlr_opaque_handle->ctrlr_api_handle =
        create_driver(
            (const char *)&g_sim_config.agent_runtime_rootpath,
            g_sim_config.dev_no,
            g_sim_config.vf_no,
            g_sim_config.sq_size,
            g_sim_config.cq_size,
            g_sim_config.nr_cmds);

    if (!ctrlr_opaque_handle->ctrlr_api_handle) {
        free(ctrlr_opaque_handle);
        return NULL;
    }

    g_sim_config.p_default_controller = ctrlr_opaque_handle;

    pthread_mutex_init(&ctrlr_opaque_handle->lock, NULL);

    DRVSIM_LOG("Returning Driver-API handle %p (wraps api_handle %p)\n",
        ctrlr_opaque_handle, ctrlr_opaque_handle->ctrlr_api_handle);

    return ctrlr_opaque_handle;
}

int nvme_fini(ctrlr_t* ctrlr)
{
    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle), "invalid ctrlr %p or uninitialized api handle\n", ctrlr);

    deallocate_driver(ctrlr->ctrlr_api_handle);

    if (g_sim_config.p_default_controller == ctrlr) {
        g_sim_config.p_default_controller = NULL;
    }

    pthread_mutex_destroy(&ctrlr->lock);

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

    DRVSIM_ASSERT((g_sim_config.p_default_controller && g_sim_config.p_default_controller->ctrlr_api_handle),
        "malformed/absent default controller %p\n", g_sim_config.p_default_controller);

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
    DRVSIM_ASSERT((g_sim_config.p_default_controller && g_sim_config.p_default_controller->ctrlr_api_handle),
        "malformed/absent default controller %p\n", g_sim_config.p_default_controller);

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

    g_sim_config.p_default_controller = NULL; // to prevent a leak, the test should have cleaned up correctly

    return DRVSIM_RETCODE_SUCCESS;
}

void driver_init_num_queues(ctrlr_t* ctrlr, uint32_t cdw0)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return;
}

int driver_init(void)
{
    driver_init_common();

    return DRVSIM_RETCODE_SUCCESS;
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

    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle), "invalid ctrlr %p or uninitialized api handle\n", ctrlr);

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

    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle), "invalid ctrlr %p or uninitialized api handle\n", ctrlr);

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