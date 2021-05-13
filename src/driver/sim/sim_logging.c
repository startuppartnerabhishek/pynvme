#include <uuid/uuid.h>

#include "sim_common.h"
#include "sim_types.h"

#include "spdk/nvme_spec.h"

#define LOG_TYPE_BANNER(__STRUCTNAME__, __CHECKPOINT__)     \
            DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, "\n<<<<<<<<<< %s (%s) >>>>>>>>>>>>>>>\n\n", __STRUCTNAME__, __CHECKPOINT__)

#define LOG_FIELD_LONGINT(__STRUCTPTR__, __FIELDNAME__, __CUSTOM_NAME__)        \
            DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, ">> %s = 0x%lx = %lu\n",     \
                __CUSTOM_NAME__ ? __CUSTOM_NAME__ : #__FIELDNAME__, (__STRUCTPTR__)->__FIELDNAME__, (__STRUCTPTR__)->__FIELDNAME__) 

#define LOG_FIELD_INT(__STRUCTPTR__, __FIELDNAME__, __CUSTOM_NAME__)            \
            DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, ">> %s = 0x%x = %u\n",       \
                __CUSTOM_NAME__ ? __CUSTOM_NAME__ : #__FIELDNAME__, (__STRUCTPTR__)->__FIELDNAME__, (__STRUCTPTR__)->__FIELDNAME__)

#define LOG_FIELD_HEX(__STRUCTPTR__, __FIELDNAME__, __CUSTOM_NAME__)                                                    \
do {                                                                                                                    \
    DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, ">> %s >>>>>>>>>>\n", __CUSTOM_NAME__ ? __CUSTOM_NAME__ : #__FIELDNAME__);   \
    sim_hex_dump(&((__STRUCTPTR__)->__FIELDNAME__), sizeof((__STRUCTPTR__)->__FIELDNAME__));                            \
} while (0)

////module: log
///////////////////////////////

char *log_buf_dump(const char* header, const void* buf, size_t len, size_t base)
{
  DRVSIM_NOT_IMPLEMENTED("not implemented\n");
  return NULL;
}

void log_dump_single_command(sim_cmd_log_entry_t *cmd_log)
{
    DRVSIM_LOG("\n>>>> Log Entry %p, prev %p, next %p, is_completed %u, is_processed %u, "
                    "resp_buf %p, resp_buf len %lu\n", cmd_log, cmd_log->prev, cmd_log->next,
                    cmd_log->is_completed, cmd_log->is_processed, cmd_log->response_buf,
                    cmd_log->response_buf_len);

    log_ctrlr_cmd(cmd_log->qpair, cmd_log);

    if (cmd_log->is_completed) {
        log_ctrlr_completion(cmd_log->qpair, cmd_log, false);
    }

    DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, "\n");
}

void log_cmd_dump(qpair_t* qpair, size_t count)
{
    sim_cmd_log_entry_t   *log_entry = qpair->log_list_head;
    uint16_t              qid = qpair->id;
    uint32_t              dump_count = count;
    const uint32_t        max_entries = qpair->log_entry_count;
    uint32_t             printed = 0;

    // print cmdlog from tail to head
    if (!log_entry) {
        DRVSIM_ASSERT((max_entries == 0),
                "entry count %u, but log_entry ptr is NULL\n", max_entries);
        DRVSIM_LOG("No log entries for qpair %p\n", qpair);
        return;
    }

    pthread_mutex_lock(&qpair->parent_controller->lock);

    if (count == 0 || count > max_entries)
    {
        dump_count = max_entries;
    }

    // cmdlog is NOT SQ/CQ. cmdlog keeps CMD/CPL for script test debug purpose
    DRVSIM_LOG("Dumping log entries: dump ctrlr %p, qpair %p, qid %d, count %d from tail\n",
                    qpair->parent_controller, qpair, qid, dump_count);

    // only send the most recent part of cmdlog
    for (log_entry = log_entry->prev; printed < dump_count; printed++, log_entry = log_entry->prev) {
        log_dump_single_command(log_entry);
    }

    pthread_mutex_unlock(&qpair->parent_controller->lock);

    DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, "\n\n");

    return;
}

void log_cmd_dump_admin(ctrlr_t* ctrlr, size_t count)
{
    log_cmd_dump(ctrlr->adminq, count);
    return;
}

void log_ctrlr_cmd(qpair_t *qp, sim_cmd_log_entry_t *cmd_log_entry)
{
    bool is_adminq = SIM_IS_ADMINQ(qp);
    const char *cmd_name_as_string = cmd_name(cmd_log_entry->cmd.opc, is_adminq ? 0 : 1);

    DRVSIM_LOG("ctrlr %p - Command on %s qp %p\n",
        qp->parent_controller, is_adminq ? "ADMIN" : "IO", qp);

    DRVSIM_LOG("Command opc %s (%u), fuse %u, cid 0x%x, nsid %u, cdw10 %u\n",
        cmd_name_as_string, cmd_log_entry->cmd.opc,
        cmd_log_entry->cmd.fuse, cmd_log_entry->cmd.cid, cmd_log_entry->cmd.nsid, cmd_log_entry->cmd.cdw10);

    sim_hex_dump(&cmd_log_entry->cmd, sizeof(cmd_log_entry->cmd));
 
    return;
}

void log_ctrlr_completion(qpair_t *qp, sim_cmd_log_entry_t *cmd_log_entry, bool print_cmd)
{
    bool is_adminq = SIM_IS_ADMINQ(qp);
    const char *cmd_name_as_string = cmd_name(cmd_log_entry->cmd.opc, is_adminq ? 0 : 1);

    DRVSIM_LOG("ctrlr %p Completion on %s qp %p\n",
        qp->parent_controller, is_adminq ? "ADMIN" : "IO", qp);

    if (print_cmd) {
        DRVSIM_LOG("|||| Command was opc %s (%u), fuse %u, cid 0x%x --------->\n",
            cmd_name_as_string, cmd_log_entry->cmd.opc,
            cmd_log_entry->cmd.fuse, cmd_log_entry->cmd.cid);
    }

    DRVSIM_LOG("|||| Response status-code %u, phase-tag %u, code-type %u, more %u, dnr %u\n",
        cmd_log_entry->cpl.status.sc, cmd_log_entry->cpl.status.p, cmd_log_entry->cpl.status.sct,
        cmd_log_entry->cpl.status.m, cmd_log_entry->cpl.status.dnr);

    DRVSIM_LOG("|||| cdw0 0x%x, sq-head %u, sq-id %u, cid 0x%x\n",
        cmd_log_entry->cpl.cdw0, cmd_log_entry->cpl.sqhd,
        cmd_log_entry->cpl.sqid, cmd_log_entry->cpl.cid);

    sim_hex_dump(&cmd_log_entry->cpl, sizeof(cmd_log_entry->cpl));

    return;
}

void log_ctrlr_completion_buf_id_controller(sim_cmd_log_entry_t *cmd_log_entry)
{
    struct spdk_nvme_ctrlr_data *resp = cmd_log_entry->response_buf;

    LOG_TYPE_BANNER("Identify Controller Response", "START");

    LOG_FIELD_INT(resp, vid, "PCI Vendor ID");
    LOG_FIELD_INT(resp, ssvid, "PCI subsystem vendor id");
    LOG_FIELD_HEX(resp, sn, "Serial Number");
    LOG_FIELD_HEX(resp, mn, "Model Number");
    LOG_FIELD_HEX(resp, fr, "Firmware Revision");
    LOG_FIELD_HEX(resp, ieee, "IEEE OUI Id");
    LOG_FIELD_INT(resp, mdts, "Max data transfer size");
    LOG_FIELD_INT(resp, cntlid, "Controller ID");
    LOG_FIELD_HEX(resp, ver, "NVME version");
    LOG_FIELD_INT(resp, aerl, "Async Event Request limit");
    LOG_FIELD_HEX(resp, lpa, "Log page attributes");
    LOG_FIELD_INT(resp, npss, "Number of power states supported");
    LOG_FIELD_INT(resp, hmpre, "Host Memory preferred size");
    LOG_FIELD_INT(resp, hmmin, "Host Memory min size");
    LOG_FIELD_HEX(resp, tnvmcap, "Total NVM capacity");
    LOG_FIELD_HEX(resp, unvmcap, "Unallocated NVM capacity");
    LOG_FIELD_HEX(resp, sqes, "Submission queue entry-size min-max");
    LOG_FIELD_HEX(resp, cqes, "Completion queue entry-size min-max");
    LOG_FIELD_INT(resp, maxcmd, NULL);
    LOG_FIELD_INT(resp, nn, "Number of namespaces");
    LOG_FIELD_HEX(resp, subnqn, "Subsystem NQN");

    LOG_TYPE_BANNER("Identify Controller Response", "END");

    return;
}

void log_ctrlr_completion_buf_id_namespace(sim_cmd_log_entry_t *cmd_log_entry)
{
    struct spdk_nvme_ns_data *resp = cmd_log_entry->response_buf;
    char decoded_nguid[UUID_STR_LEN] = {0};

    LOG_TYPE_BANNER("Identify Namepsace Response", "START");

    LOG_FIELD_LONGINT(resp, nsze, "Size");
    LOG_FIELD_LONGINT(resp, ncap, "Capacity");
    LOG_FIELD_LONGINT(resp, nuse, "Utilization");
    LOG_FIELD_INT(resp, flbas.format, "flbas.Format");
    LOG_FIELD_INT(resp, flbas.extended, "flbas.Extended");
    LOG_FIELD_HEX(resp, lbaf, "LBAF");
    LOG_FIELD_HEX(resp, nguid, "NGUN - Namespace Globally Unique ID");

    uuid_unparse((unsigned char *)&resp->nguid[0], decoded_nguid);

    DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, ">>>> decoded nguid = %s\n", decoded_nguid);

    LOG_TYPE_BANNER("Identify Namespace Response", "END");

    return;
}