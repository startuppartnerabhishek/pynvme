#ifndef __SIM_TYPES_H__
#define __SIM_TYPES_H__

#include "sim_common.h"
#include "driver.h"
#include "intr_mgt.h"

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
} sim_config_t;

extern sim_config_t g_sim_config;

void sim_process_completion(struct sim_cmd_log_entry_s *completed_cmd_log);


/********************* </end> externs, types and declarations ***********************/

#endif