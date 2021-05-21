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

#define CONF_FILE_PARSE_ENTRY(__conf_tgt_struct__, __field__, __is_string__) {#__field__, __is_string__, &__conf_tgt_struct__->__field__}

/*** global config *****/
sim_config_t g_sim_config;

static void init_sim_config(char *json_string);
static void init_sim_controller_config(char *json_string, ctrlr_t *ctrlr);
static void __init_config_from_conf_file(const char *json_string, const sim_config_item_t *cfg_items, unsigned int num_items);
static void drvsim_completion_callback(void *cb_args, nvme_ctrlr_completion_t *cmpl);
static qpair_t *sim_allocate_qpair(ctrlr_t *ctrlr, bool is_adminq);
static void sim_free_qpair(qpair_t *q);
static int sim_sync_namespace(ctrlr_t *ctrlr, unsigned int ns_array_idx);
static void wait_for_all_adminq_completions(ctrlr_t *ctrlr);
static int nvme_send_cmd_raw_internal(
                        ctrlr_t* ctrlr,
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
                        void* cb_arg,
                        bool free_buf_on_completion_processing);
static void sim_ctrlr_get_num_queues_done(ctrlr_t *ctrlr, struct spdk_nvme_cpl *cpl);
static void free_controller(ctrlr_t *ctrlr);

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

static void wait_for_all_adminq_completions(ctrlr_t *ctrlr)
{
    qpair_t *adminq = ctrlr->adminq;

    do {
        pthread_mutex_lock(&ctrlr->lock);

        if ((adminq->commands_sent - adminq->aers_sent) > (adminq->responses_received - adminq->aer_completions_received)) {
            pthread_mutex_unlock(&ctrlr->lock);
            sim_sleep(0, 10000);
            continue;   
        } else {
            pthread_mutex_unlock(&ctrlr->lock);
            break;
        }
    } while (true);

    sim_qpair_process_completions(adminq, DRVSIM_VERY_LARGE_NUMBER);
}

static int sim_sync_namespace(ctrlr_t *ctrlr, unsigned int ns_array_idx)
{
    int ret;
    namespace_t *n = &ctrlr->namespaces[ns_array_idx];
    void *buf;
    uint64_t phys_addr;
    struct spdk_nvme_cmd cmd = {0};
    uint32_t *cmd_as_arr = (uint32_t *)&cmd;
    const size_t buf_size = sizeof(struct spdk_nvme_ns_data);

    buf = buffer_init(ctrlr, buf_size, &phys_addr, 32, 0x00DDBA11);

    /* what if we are racing with a completion for this namespace entry? */

    n->parent_controller = ctrlr;
    n->id = ns_array_idx + 1;
    n->state = SIM_NS_STATE_CREATED;

    cmd.opc = SPDK_NVME_OPC_IDENTIFY;
    cmd.cdw10 = SPDK_NVME_IDENTIFY_NS;

    ret = nvme_send_cmd_raw(ctrlr, NULL, cmd_as_arr[0], n->id, buf, buf_size, cmd.cdw10,
                        cmd.cdw11, cmd.cdw12, cmd.cdw13, cmd.cdw14, cmd.cdw15, NULL, NULL);

    if (DRVSIM_RETCODE_SUCCESS != ret) {
        buffer_fini(ctrlr, buf);
        n->state = SIM_NS_STATE_IDENTIFY_FAILED;

        return ret;
    }

    // wait for the response so that the caller can treat us as synchronous
    wait_for_all_adminq_completions(ctrlr);

    return DRVSIM_RETCODE_SUCCESS;
}

int nvme_set_ns(ctrlr_t *ctrlr)
{
    int ret = DRVSIM_RETCODE_SUCCESS;

    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle), "ctrlr %p or api-handle cannot be NULL\n", ctrlr);

    const int nn_count = ctrlr->num_namespaces;
    int i;
    unsigned int success = 0;

    /* send out identify-namespace on all the namespaces */
    for (i = 0; i < nn_count; i++) {
        int lret;
        
        lret = sim_sync_namespace(ctrlr, i);

        if (DRVSIM_RETCODE_SUCCESS != lret) {
            ret = lret;
        } else {
            success++;
            DRVSIM_ASSERT((ctrlr->namespaces[i].state == SIM_NS_STATE_IDENTIFY_COMPLETE),
                            "namespace %u, state %u is not IDENTIFIED\n", i, ctrlr->namespaces[i].state);
        }
    }

    DRVSIM_LOG("ctrlr %p, %u namespace(s) identified, %u success\n", ctrlr, nn_count, success);

    return ret;
}

namespace_t* nvme_get_ns(ctrlr_t* ctrlr, uint32_t nsid)
{
	if (nsid < 1 || nsid > ctrlr->num_namespaces) {
		return NULL;
	}

	return &ctrlr->namespaces[nsid - 1];
}

static void drvsim_completion_callback(void *cb_args, nvme_ctrlr_completion_t *cmpl)
{
    completion_cb_context_t *compl_ctx = (completion_cb_context_t *)cb_args;
    struct spdk_nvme_cpl *spec_completion = (struct spdk_nvme_cpl *)cmpl;    

    DRVSIM_LOG("Got completion with args %p -> translate to %p, "
        "cmd-id 0x%x, phase-tag %u, status code %u, type %u\n",
        cb_args, compl_ctx->response_args, spec_completion->cid,
        spec_completion->status.p, spec_completion->status.sc, spec_completion->status.sct);

    /*  both agent's and spdk's completion structures are from the spec, so
        we should be ok to just type-cast and pass on. */
    sim_receive_and_note_completion((sim_cmd_log_entry_t *)cb_args, (cpl *)cmpl);

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
                            void* cb_arg, bool free_buf_on_completion_processing)
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
    e->free_buf_on_completion = free_buf_on_completion_processing;

    DRVSIM_LOG("For qp %p, adding cmd entry for opc %u, cid 0x%x - expect callback to fn %p with ctx %p\n",
                    qpair, e->cmd.opc, e->cmd.cid, cb_fn, e);    

    pthread_mutex_lock(&qpair->parent_controller->lock);

    /* connect to qpair - just behind the tail */
    qpair->commands_sent++;

    if (e->cmd.opc == SPDK_NVME_OPC_ASYNC_EVENT_REQUEST) {
        qpair->aers_sent++;
    }

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

    log_ctrlr_cmd(qpair, e);

    DRVSIM_LOG("QP %p counters: cmds sent %u, responses received %u, log entry count %u, completions collected %u\n",
        qpair, qpair->commands_sent, qpair->responses_received, qpair->log_entry_count, qpair->completions_collected);

    pthread_mutex_unlock(&qpair->parent_controller->lock);

    return e;
}

int sim_receive_and_note_completion(sim_cmd_log_entry_t *completion_ctx, cpl *cqe)
{
    DRVSIM_ASSERT((completion_ctx && completion_ctx->qpair),
        "completion_ctx %p && completion_ctx->qpair\n", completion_ctx);

    qpair_t *qpair = completion_ctx->qpair;

    DRVSIM_ASSERT((completion_ctx->is_completed == false),
        "completion_ctx %p with cid %x is already completed\n", completion_ctx, completion_ctx->cmd.cid);

    DRVSIM_ASSERT((completion_ctx->cmd.cid == cqe->cid),  // wrong context?
        "completion ctx %p cmd.cid 0x%x does not match cid in completion 0x%x\n",
        completion_ctx, completion_ctx->cmd.cid, cqe->cid);

    DRVSIM_ASSERT((qpair->parent_controller),  // wrong context?
        "qpair %p has null parent controller\n", qpair);

    /* grab controoller level lock to avoid completion side-effects before the submission call stack has unwound */
    pthread_mutex_lock(&qpair->parent_controller->lock);

    completion_ctx->is_completed = true;

    completion_ctx->qpair->responses_received++;

    memcpy(&completion_ctx->cpl, cqe, sizeof(cpl));

    if (completion_ctx->cb_ctx.cb_fn) {
        completion_ctx->cb_ctx.cb_fn(completion_ctx->cb_ctx.response_args, cqe);
    }

    pthread_mutex_unlock(&qpair->parent_controller->lock);

    if (g_sim_config.max_log_entries_per_qpair > 0 &&
            completion_ctx->qpair->log_entry_count > g_sim_config.max_log_entries_per_qpair) {
        prune_completion_table(
            completion_ctx->qpair,
            completion_ctx->qpair->log_entry_count - g_sim_config.max_log_entries_per_qpair
        );
    }

    // log_ctrlr_completion(completion_ctx->qpair, completion_ctx, true);

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

    pthread_mutex_lock(&qpair->parent_controller->lock);

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
            free_log_entry(e);

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

    pthread_mutex_unlock(&qpair->parent_controller->lock);

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

    free(e);

    qpair->log_entry_count--;

    if (qpair->log_entry_count) {
        next->prev = prev;
        prev->next = next;
    } else {
        qpair->log_list_head = NULL;
    }

    return;
}

int free_completion_table(qpair_t *qpair)
{
    int cleaned = 0;
    const unsigned int initial_entry_count = qpair->log_entry_count;

    pthread_mutex_lock(&qpair->parent_controller->lock);

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

    pthread_mutex_unlock(&qpair->parent_controller->lock);

    return cleaned;
}

static int nvme_send_cmd_raw_internal(
                        ctrlr_t* ctrlr,
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
                        void* cb_arg,
                        bool free_buf_on_completion_processing)
{
    int ret;
    sim_cmd_log_entry_t *completion_ctx = NULL;

    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle), "invalid ctrlr %p or uninitialized api handle\n", ctrlr);

    DRVSIM_LOG("ENTERED with ctrlr %p, qpair %p, buf %p of len %lu, cdw0 0x%08x, "
                    "nsid 0x%08x, cdw10 = 0x%08x, cb_fn %p, cb_args %p\n", ctrlr, qpair,
                    buf, len, cdw0, nsid, cdw10, cb_fn, cb_arg);

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
                      cb_fn, cb_arg, free_buf_on_completion_processing);
    }

    DRVSIM_ASSERT((completion_ctx), "could not allocate completion ctx\n");

    /* Grab an api lock or else we do not want to process this command's completion in a different thread
     * till this command submission call stack has unwound and updated its book-keeping.
     */

    pthread_mutex_lock(&ctrlr->lock);

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
        drvsim_completion_callback,
        completion_ctx
    );

    pthread_mutex_unlock(&ctrlr->lock);

    if (0 != ret) {
        free_log_entry(completion_ctx);

        if (free_buf_on_completion_processing) {
            buffer_fini(ctrlr, buf);
        }

        return ret;
    }

    return ret;
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
 
    return nvme_send_cmd_raw_internal(
                ctrlr, qpair, cdw0, nsid, buf, len,
                cdw10, cdw11, cdw12, cdw13, cdw14, cdw15,
                cb_fn, cb_arg, false);
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

    // DRVSIM_LOG("Test requested waitdone on ctrlr %p, adminq %p\n", ctrlr, ctrlr->adminq);

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

////module: namespace
///////////////////////////////

namespace_t* ns_init(ctrlr_t* ctrlr,
                             uint32_t nsid,
                             uint64_t nlba_verify)
{
    namespace_t *ns = &ctrlr->namespaces[nsid - 1];

    assert(ctrlr != NULL);
    assert(nsid > 0);
    assert(ns != NULL);
    assert(nsid <= ctrlr->num_namespaces);

    // ns_table_init not implemented in sim till we start doing IOs
#if 0
    uint64_t nsze = spdk_nvme_ns_get_num_sectors(ns);
    if (nlba_verify > 0)
    {
        // limit verify area to save memory usage
        nsze = MIN(nsze, nlba_verify);
    }

    if (0 != ns_table_init(ns, sizeof(uint32_t)*nsze))
    {
        return NULL;
    }
#endif

    DRVSIM_LOG("ctrlr %p, nsid %d, ns %p\n", ctrlr, nsid, ns);
    return ns;
}

int ns_refresh(namespace_t *ns, uint32_t id,
               ctrlr_t *ctrlr)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return DRVSIM_RETCODE_FAILURE;
}

uint32_t ns_get_sector_size(namespace_t* ns)
{
  return ns->sector_size;
}

bool ns_verify_enable(namespace_t* ns, bool enable)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");

  return false;
}

int ns_cmd_io(uint8_t opcode,
              namespace_t* ns,
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

int ns_fini(namespace_t* ns)
{
  // 'ns_table_fini(ns);' not implemented till we start doing IOs on the namespace

  // driver.c does not cleanup the ns itself, so for us there is nothing else to do

  return DRVSIM_RETCODE_SUCCESS;
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

static void __init_config_from_conf_file(const char *json_string, const sim_config_item_t *cfg_items, unsigned int num_items)
{
    cJSON *conf = cJSON_Parse(json_string);
    cJSON *aConfig;
    unsigned int i;

    if (NULL == conf) {
        DRVSIM_FATAL_ERROR("Invalid json-config %s\n", json_string);
        return;
    }

    for (i = 0; i < num_items; i++) {
        aConfig = cJSON_GetObjectItemCaseSensitive(conf, cfg_items[i].key_name);

        if (aConfig) {
            if (cfg_items[i].isString) {
                if (cJSON_IsString(aConfig) && NULL != aConfig->valuestring) {
                    strcpy((char *)cfg_items[i].save_to, aConfig->valuestring);
                    DRVSIM_LOG("[JSON-CFG] %s (STRING) -> %s\n", cfg_items[i].key_name,
                        (char *)cfg_items[i].save_to);
                    continue;
                }
            } else {
                if (cJSON_IsNumber(aConfig)) {
                    *(int *)cfg_items[i].save_to = aConfig->valueint;
                    DRVSIM_LOG("[JSON-CFG] %s (INT) -> %d\n", cfg_items[i].key_name,
                        *(int *)cfg_items[i].save_to);
                    continue;
                }
            }
        }

        DRVSIM_LOG("[JSON-CFG] %s NOT FOUND\n", cfg_items[i].key_name);
    }

    cJSON_Delete(conf);
}

static void init_sim_controller_config(char *json_string, ctrlr_t *ctrlr)
{
    const sim_config_item_t sim_ctrlr_cfg_file_items[] = {
        CONF_FILE_PARSE_ENTRY(ctrlr, dev_no, false),
        CONF_FILE_PARSE_ENTRY(ctrlr, vf_no, false),
        CONF_FILE_PARSE_ENTRY(ctrlr, sq_size, false),
        CONF_FILE_PARSE_ENTRY(ctrlr, cq_size, false),
        CONF_FILE_PARSE_ENTRY(ctrlr, nr_cmds, false)
    };

    const unsigned int num_items = sizeof(sim_ctrlr_cfg_file_items) / sizeof(sim_config_item_t);

    __init_config_from_conf_file(json_string, &sim_ctrlr_cfg_file_items[0], num_items);

    return;
}

ctrlr_t* nvme_init(char * traddr, unsigned int port)
{
    ctrlr_t *ctrlr_opaque_handle = NULL;

    DRVSIM_LOG("[ENTERING] traddr %s, port %u\n", traddr, port);

    ctrlr_opaque_handle = (ctrlr_t *)malloc(sizeof(ctrlr_t));

    DRVSIM_ASSERT((ctrlr_opaque_handle), "opaque handle alloc failed\n");

    memset(ctrlr_opaque_handle, 0, sizeof(ctrlr_t));

    init_sim_controller_config(traddr, ctrlr_opaque_handle);

    ctrlr_opaque_handle->ctrlr_api_handle =
        create_driver(
            (const char *)&g_sim_config.agent_runtime_rootpath,
            ctrlr_opaque_handle->dev_no,
            ctrlr_opaque_handle->vf_no,
            ctrlr_opaque_handle->sq_size,
            ctrlr_opaque_handle->cq_size,
            ctrlr_opaque_handle->nr_cmds);

    if (!ctrlr_opaque_handle->ctrlr_api_handle) {
        free(ctrlr_opaque_handle);
        return NULL;
    }

    pthread_mutex_init(&ctrlr_opaque_handle->lock, NULL);

    DRVSIM_LOG("Returning Driver-API handle %p (wraps api_handle %p)\n",
        ctrlr_opaque_handle, ctrlr_opaque_handle->ctrlr_api_handle);

    return ctrlr_opaque_handle;
}

static void free_controller(ctrlr_t *ctrlr)
{
    pthread_mutex_destroy(&ctrlr->lock);

    if (ctrlr->num_namespaces > 0) {
        free(ctrlr->namespaces);
    }

    deallocate_driver(ctrlr->ctrlr_api_handle);

    free(ctrlr);

    DRVSIM_LOG("Driver resources freed\n");

    return;
}

int nvme_fini(ctrlr_t* ctrlr)
{
    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle), "invalid ctrlr %p or uninitialized api handle\n", ctrlr);

#if 0
    DRVSIM_ASSERT((ctrlr->num_alloocated_buffers == ctrlr->num_freed_buffers),
        "ctrlr %p has outstanding allocated buffers, alloc %u, free %u\n",
        ctrlr, ctrlr->num_alloocated_buffers, ctrlr->num_freed_buffers);
#endif

    // maybe we should remove the assert, and allow this ctrlr to exist for some time

    if (ctrlr->num_allocated_buffers != ctrlr->num_freed_buffers) {
        DRVSIM_LOG_TO_FILE(stderr,
            "WARNING: ctrlr %p has outstanding allocated buffers, alloc %u, free %u\n",
            ctrlr, ctrlr->num_allocated_buffers, ctrlr->num_freed_buffers);
        
        ctrlr->is_destroyed = true;

        return DRVSIM_RETCODE_SUCCESS;
    }

    free_controller(ctrlr);

    return DRVSIM_RETCODE_SUCCESS;
}

////module: buffer
///////////////////////////////

void *buffer_init(ctrlr_t *ctrlr, size_t bytes, uint64_t *phys_addr,
                  uint32_t ptype, uint32_t pvalue)
{
    void *buf;

    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle),
        "malformed/absent controller %p\n", ctrlr);

    buf = nvme_driver_buffer_alloc(ctrlr->ctrlr_api_handle, bytes, phys_addr);

    if (g_sim_config.log_buf_alloc_free) {
        DRVSIM_LOG("buf = %p, api-handle %p, size %lu\n",
            buf, ctrlr->ctrlr_api_handle, bytes);
    }

    if (!buf) {
        return buf;
    }

    ctrlr->num_allocated_buffers++;

    DRVSIM_LOG("Intilializing allocated buffer %p, ptype %u, pvalue 0x%x\n",
                    buf, ptype, pvalue);

    buffer_pattern_init(buf, bytes, ptype, pvalue);

    // sim_hex_dump(buf, bytes);

    return buf;
}

void buffer_fini(ctrlr_t *ctrlr, void* buf)
{
    DRVSIM_ASSERT((ctrlr && ctrlr->ctrlr_api_handle),
        "malformed/absent controller %p\n", ctrlr);

    if (g_sim_config.log_buf_alloc_free) {
        DRVSIM_LOG("buf = %p, ctrlr %p, api-handle %p\n",
                buf, ctrlr, ctrlr->ctrlr_api_handle);
    }

    nvme_driver_buffer_free(ctrlr->ctrlr_api_handle, buf);

    ctrlr->num_freed_buffers++;

    if (ctrlr->num_freed_buffers == ctrlr->num_allocated_buffers && ctrlr->is_destroyed) {
        // a free was deferred, probably becausee of a test bug
        free_controller(ctrlr);
    }

    return;
}

int driver_fini(void)
{
    // clear global shared data

    return DRVSIM_RETCODE_SUCCESS;
}

static void sim_ctrlr_get_num_queues_done(ctrlr_t *ctrlr, struct spdk_nvme_cpl *cpl)
{
    /******** (START) from nvme_ctrlr_get_num_queues_done ******/
	uint32_t cq_allocated, sq_allocated, min_allocated;

	if (spdk_nvme_cpl_is_error(cpl)) {
		DRVSIM_LOG("Get Features - Number of Queues failed!\n");
		ctrlr->opts.num_io_queues = 0;
	} else {
		/*
		 * Data in cdw0 is 0-based.
		 * Lower 16-bits indicate number of submission queues allocated.
		 * Upper 16-bits indicate number of completion queues allocated.
		 */
		sq_allocated = (cpl->cdw0 & 0xFFFF) + 1;
		cq_allocated = (cpl->cdw0 >> 16) + 1;

		/*
		 * For 1:1 queue mapping, set number of allocated queues to be minimum of
		 * submission and completion queues.
		 */
		min_allocated = spdk_min(sq_allocated, cq_allocated);

		/* Set number of queues to be minimum of requested and actually allocated. */
		ctrlr->opts.num_io_queues = spdk_min(min_allocated, ctrlr->opts.num_io_queues);
	}

/* for now - this path does not use IO-queues */
#if 0

	ctrlr->free_io_qids = spdk_bit_array_create(ctrlr->opts.num_io_queues + 1);
	if (ctrlr->free_io_qids == NULL) {
		nvme_ctrlr_set_state(ctrlr, NVME_CTRLR_STATE_ERROR, NVME_TIMEOUT_INFINITE);
		return;
	}

	/* Initialize list of free I/O queue IDs. QID 0 is the admin queue. */
	spdk_bit_array_clear(ctrlr->free_io_qids, 0);
	for (i = 1; i <= ctrlr->opts.num_io_queues; i++) {
		spdk_bit_array_set(ctrlr->free_io_qids, i);
	}
	nvme_ctrlr_set_state(ctrlr, NVME_CTRLR_STATE_CONSTRUCT_NS,
			     ctrlr->opts.admin_timeout_ms);
#endif

    /******** (END) from nvme_ctrlr_get_num_queues_done ******/

	// nvme_ctrlr_set_state(ctrlr, NVME_CTRLR_STATE_READY, NVME_TIMEOUT_INFINITE);

    return;
}

void driver_init_num_queues(ctrlr_t* ctrlr, uint32_t cdw0)
{
    struct spdk_nvme_cpl cpl;

    DRVSIM_LOG("ctrlr %p, cdw0 0x%x\n", ctrlr, cdw0);

    memset(&cpl, 0, sizeof(cpl));
    cpl.cdw0 = cdw0;

    sim_ctrlr_get_num_queues_done(ctrlr, &cpl);
    return;
}

static void init_sim_config(char *json_string)
{
    const sim_config_item_t sim_global_cfg_file_items[] = {
        CONF_FILE_PARSE_ENTRY((&g_sim_config), agent_runtime_rootpath, true),
        CONF_FILE_PARSE_ENTRY((&g_sim_config), log_register_reads, false),
        CONF_FILE_PARSE_ENTRY((&g_sim_config), log_register_writes, false),
        CONF_FILE_PARSE_ENTRY((&g_sim_config), log_buf_alloc_free, false),
        CONF_FILE_PARSE_ENTRY((&g_sim_config), max_log_entries_per_qpair, false),
        CONF_FILE_PARSE_ENTRY((&g_sim_config), log_dump_adminq_completion_len, false)
    };

    const unsigned int num_items = sizeof(sim_global_cfg_file_items) / sizeof(sim_config_item_t);

    __init_config_from_conf_file(json_string, &sim_global_cfg_file_items[0], num_items);

    return;
}

int driver_init(char *conf_json)
{
    DRVSIM_LOG("Got config %s\n", conf_json);

    init_sim_config(conf_json);

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