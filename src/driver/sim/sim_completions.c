#include "sim_common.h"
#include "sim_types.h"
#include "driver.h"

static void sim_process_completion(struct sim_cmd_log_entry_s *cmd_log);
static void sim_process_completion_identify_controller(struct sim_cmd_log_entry_s *cmd_log);
static void sim_process_completion_identify_namespace(struct sim_cmd_log_entry_s *cmd_log);

static void sim_process_completion(struct sim_cmd_log_entry_s *cmd_log)
{
    bool is_adminq = SIM_IS_ADMINQ(cmd_log->qpair);
    const char *q_type = is_adminq ? "ADMIN" : "IO";

    cmd_log->is_processed = true;

    cmd_log->qpair->completions_collected++;

    if (g_sim_config.log_dump_adminq_completion_len) {
        DRVSIM_LOG("%s commmand:\n", q_type);
        sim_hex_dump(&cmd_log->cmd, sizeof(cmd_log->cmd));
        DRVSIM_LOG("completion:\n");
        sim_hex_dump(&cmd_log->cpl, sizeof(cmd_log->cpl));
        if (cmd_log->response_buf && cmd_log->response_buf_len) {
            DRVSIM_LOG("response buffer:\n");
            sim_hex_dump(cmd_log->response_buf,
                cmd_log->response_buf_len > g_sim_config.log_dump_adminq_completion_len ?
                    g_sim_config.log_dump_adminq_completion_len : cmd_log->response_buf_len);
        }
    }

    if (is_adminq) {

        switch (cmd_log->cmd.opc) {
            case SPDK_NVME_OPC_IDENTIFY:
                DRVSIM_LOG("IDENTIFY completion\n");
                switch(cmd_log->cmd.cdw10) {
                    
                    case SPDK_NVME_IDENTIFY_CTRLR:
                        sim_process_completion_identify_controller(cmd_log);
                        break;

                    case SPDK_NVME_IDENTIFY_NS:
                        sim_process_completion_identify_namespace(cmd_log);
                        break;

                    default:
                        DRVSIM_LOG("do not know how to process cdw10 = %u for OPC IDENTIFY (6)\n",
                            cmd_log->cmd.cdw10);
                } /* switch cdw10 */
            break;

            case SPDK_NVME_OPC_SET_FEATURES:
                DRVSIM_NOT_IMPLEMENTED_BENIGN("SET Features completion - processing not implemented\n");
                break;

            default:
                DRVSIM_NOT_IMPLEMENTED_BENIGN("ADMIN cmd %u has no special handling\n",
                        cmd_log->cmd.opc);
        } /* switch OPC */

    } else {
        switch (cmd_log->cmd.opc) {
            default:
                DRVSIM_NOT_IMPLEMENTED_BENIGN("IO cmd %u has no special handling\n",
                        cmd_log->cmd.opc);
        }
    }

    if (cmd_log->response_buf && cmd_log->free_buf_on_completion) {
        buffer_fini(cmd_log->response_buf);
    }

    return;
}

int sim_qpair_process_completions(qpair_t *q, unsigned int max)
{
    int processed = 0;
    int scanned = 0;
    DRVSIM_ASSERT((q), "q cannot be NULL\n");
    sim_cmd_log_entry_t *e = q->log_list_head;
    const unsigned int log_entry_count = q->log_entry_count;

    pthread_mutex_lock(&q->parent_controller->lock);

    while (processed < max && scanned < log_entry_count) {
        scanned++;

        DRVSIM_ASSERT((e), "qpair %p has unexpected NULL cmd entry\n", q);

        if (e->is_completed) {
            if (!e->is_processed) {
                processed++;
                sim_process_completion(e);
            }
        }

        e = e->next;
    }

    if (processed) {
        DRVSIM_LOG("QP %p counters: cmds sent %u, responses received %u, log entry count %u, completions collected %u\n",
            q, q->commands_sent, q->responses_received, q->log_entry_count, q->completions_collected);
    }

    pthread_mutex_unlock(&q->parent_controller->lock);

    return processed;
}

static void sim_process_completion_identify_controller(struct sim_cmd_log_entry_s *cmd_log)
{
    if (spdk_nvme_cpl_is_error(&cmd_log->cpl)) {
        DRVSIM_LOG("sc %u, sct %u => error (should be SUCCESS, GENERIC SUCCESS), no further processing\n",
            cmd_log->cpl.status.sc, cmd_log->cpl.status.sct);
        return;
    } else if (cmd_log->response_buf_len < sizeof(struct spdk_nvme_ctrlr_data)) {
        DRVSIM_LOG("buffer of len %lu too small to parse as id-ctrlr response (min %lu)\n",
            cmd_log->response_buf_len, sizeof(struct spdk_nvme_ctrlr_data));
            return;
    }

    log_ctrlr_completion_buf_id_controller(cmd_log);

    struct spdk_nvme_ctrlr_data *resp = cmd_log->response_buf;
    ctrlr_t *ctrlr = cmd_log->qpair->parent_controller;

    if (resp->nn > ctrlr->num_namespaces) {
        ctrlr->namespaces = (namespace_t *)realloc(ctrlr->namespaces, resp->nn * sizeof(namespace_t));
    }

    ctrlr->num_namespaces = resp->nn;
    memset(ctrlr->namespaces, 0, resp->nn * sizeof(namespace_t));

    return;
}

static void sim_process_completion_identify_namespace(struct sim_cmd_log_entry_s *cmd_log)
{
    uint32_t ns_id = cmd_log->cmd.nsid;
    namespace_t *n = &cmd_log->qpair->parent_controller->namespaces[ns_id - 1];
    struct spdk_nvme_ns_data *resp = (struct spdk_nvme_ns_data *)cmd_log->response_buf;

    if (spdk_nvme_cpl_is_error(&cmd_log->cpl)) {
        n->state = SIM_NS_STATE_IDENTIFY_FAILED;
        DRVSIM_LOG("sc %u, sct %u => error (should be SUCCESS, GENERIC SUCCESS), no further processing\n",
            cmd_log->cpl.status.sc, cmd_log->cpl.status.sct);
        return;
    } else if (cmd_log->response_buf_len < sizeof(struct spdk_nvme_ns_data)) {
        DRVSIM_LOG("buffer of len %lu too small to parse as id-ns response (min %lu)\n",
            cmd_log->response_buf_len, sizeof(struct spdk_nvme_ns_data));
            return;
    }

    log_ctrlr_completion_buf_id_namespace(cmd_log);

    n->state = SIM_NS_STATE_IDENTIFY_COMPLETE;

    n->size = resp->nsze;
    n->capacity = resp->ncap;
    n->utilization = resp->nuse;
    n->sector_size = 1 << resp->lbaf[resp->flbas.format].lbads;

    memcpy(&n->nguid, &resp->nguid, sizeof(n->nguid));

    DRVSIM_LOG("Namespace %p with ns_id %d, saved with sector_size %u\n",
                    n, ns_id, n->sector_size);

    return;
}