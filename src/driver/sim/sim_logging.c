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
    sim_hex_dump(&((__STRUCTPTR__)->__FIELDNAME__), sizeof((__STRUCTPTR__)->__FIELDNAME__), stdout);                            \
} while (0)

////module: log
///////////////////////////////

char *log_buf_dump(const char* header, const void* buf, size_t len, size_t base)
{
  size_t size;
  FILE* fd = NULL;
  char* tmpname = "/tmp/pynvme_buf_dump.tmp";
  static char dump_buf[64*1024];

  // dump buf is limited
  assert(len <= 4096);

  errno = 0;
  fd = fopen(tmpname, "w+");

  if (fd == NULL)
  {
    DRVSIM_LOG_TO_FILE(stderr, "ERROR fopen: %s\n", strerror(errno));
    return NULL;
  }

  fprintf(fd, "%s: -----\n", header);
  sim_hex_dump(buf+base, len, fd);

  // get file size
  size = ftell(fd);

  errno = 0;
  if (-1 == fseek(fd, 0, SEEK_SET))
  {
    DRVSIM_LOG_TO_FILE(stderr, "ERROR lseek: %s\n", strerror(errno));
    return NULL;
  }

  // read the data from temporary file
  errno=0;
  if (fread(dump_buf, size, 1, fd) == 0)
  {
    DRVSIM_LOG_TO_FILE(stderr, "ERROR read: %s\n", strerror(errno));
    return NULL;
  }

  fclose(fd);
  dump_buf[size] = '\0';
  return dump_buf;
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

    DRVSIM_LOG("Command opc %s (%u), fuse %u, cid 0x%x, nsid 0x%x\n",
        cmd_name_as_string, cmd_log_entry->cmd.opc,
        cmd_log_entry->cmd.fuse, cmd_log_entry->cmd.cid, cmd_log_entry->cmd.nsid);

    DRVSIM_LOG("Command Words:\n"
        "\tcdw10 = 0x%08x\n"
        "\tcdw11 = 0x%08x\n"
        "\tcdw12 = 0x%08x\n"
        "\tcdw13 = 0x%08x\n"
        "\tcdw14 = 0x%08x\n"
        "\tcdw15 = 0x%08x\n",
    cmd_log_entry->cmd.cdw10,
    cmd_log_entry->cmd.cdw11,
    cmd_log_entry->cmd.cdw12,
    cmd_log_entry->cmd.cdw13,
    cmd_log_entry->cmd.cdw14,
    cmd_log_entry->cmd.cdw15);

    sim_hex_dump(&cmd_log_entry->cmd, sizeof(cmd_log_entry->cmd), NULL);
 
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

    DRVSIM_LOG("|||| cdw0 0x%08x, sq-head %u, sq-id %u, cid 0x%x\n",
        cmd_log_entry->cpl.cdw0, cmd_log_entry->cpl.sqhd,
        cmd_log_entry->cpl.sqid, cmd_log_entry->cpl.cid);

    sim_hex_dump(&cmd_log_entry->cpl, sizeof(cmd_log_entry->cpl), NULL);

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

void log_ctrlr_completion_get_log_page(sim_cmd_log_entry_t *cmd_log_entry)
{
    char *lid;

    LOG_TYPE_BANNER("GET LOG PAGE Response", "START");

    switch(cmd_log_entry->cmd.cdw10 & 0xff) {
        case 0x01:
            lid = "Error Information";
            break;

        case 0x2:
            lid = "SMART/Health Information";
            break;

        case 0x3:
            lid = "Firmware Slot Information";
            break;

        case 0x4:
            lid = "Changed Namespace list";
            break;

        case 0x5:
            lid = "Command Effects Log";
            break;

        default:
            lid = "Unknown/Unhandled LID";
            break;
    }

    DRVSIM_LOG_UNDECORATED_TO_FILE(stdout, "LID %s\n", lid);

    LOG_TYPE_BANNER("GET LOG PAGE Response", "END");
}