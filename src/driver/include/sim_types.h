#ifndef __SIM_TYPES_H__
#define __SIM_TYPES_H__

#include "sim_common.h"
#include "driver.h"
#include "intr_mgt.h"

/***** general defines *************************/

#define SIM_IS_ADMINQ(_A_QUEUE_) (((_A_QUEUE_)->parent_controller->adminq) == (_A_QUEUE_))

/**** </end> general defines ****************/

/********************* externs, types and declarations ******************************/

/*** global sim config ***********/

typedef struct sim_config_s { 
    char agent_runtime_rootpath[SIM_MAX_STRING_LEN];
    unsigned int dev_no;
    unsigned int vf_no;
    unsigned int sq_size;
    unsigned int cq_size;
    unsigned int nr_cmds;
    unsigned int log_register_reads;
    unsigned int log_register_writes;
    unsigned int log_buf_alloc_free;
    unsigned int max_log_entries_per_qpair;
    unsigned int log_dump_adminq_completion_len;
    ctrlr_t *p_default_controller;
    ctrlr_t *p_most_recent_cleaned_up_default_ctrlr;
} sim_config_t;

extern sim_config_t g_sim_config;
int sim_qpair_process_completions(qpair_t *q, unsigned int max);

/* logging utility functions */
void log_ctrlr_cmd(qpair_t *qp, sim_cmd_log_entry_t *cmd_log_entry);
void log_ctrlr_completion(qpair_t *qp, sim_cmd_log_entry_t *cmd_log_entry);
void log_ctrlr_completion_buf_id_controller(sim_cmd_log_entry_t *cmd_log_entry);
void log_ctrlr_completion_buf_id_namespace(sim_cmd_log_entry_t *cmd_log_entry);
void sim_sleep(unsigned int seconds, unsigned int nanoseconds);

/* command logs */
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
                            void* cb_arg, bool free_buf_on_completion_processing);

int sim_receive_and_note_completion(sim_cmd_log_entry_t *completion_ctx, cpl *cqe);

int prune_completion_table(qpair_t *qpair, unsigned int max_clean);

int free_completion_table(qpair_t *qpair);

void free_log_entry(sim_cmd_log_entry_t *e);

/********************* </end> externs, types and declarations ***********************/

#endif