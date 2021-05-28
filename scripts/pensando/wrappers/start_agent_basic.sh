#!/bin/bash

CONF_DIR=./conf
CONF_FILE=${CONF_DIR}/simconf.json
AGENT_START_DELAY=5

NSV_ROOT_DIR=$(jq -rj .global.nsv_test_root_path ${CONF_FILE})
AGENT_EXEC_CMD="build/nvme_agent"
AGENT_EXEC_ARGS="--run_dir run --config_dir run/conf"

AGENT_CONF_FILE="tools/nvme_testcfg.json"

#NSV_CFG_CMD="tools/nsv_scale_cfg.sh"

NSV_CFG_CMD="build/nsvcfg load --json ${AGENT_CONF_FILE}"

# start the agent
cd ${NSV_ROOT_DIR}
echo "Starting agent from ${PWD}"
# cannot nohup - nvme_agent needs stdout at the moment
#nohup ${AGENT_EXEC_CMD} ${AGENT_EXEC_ARGS}
${AGENT_EXEC_CMD} ${AGENT_EXEC_ARGS} &

echo "Waiting for Agent to come up - ${AGENT_START_DELAY} second(s)"
sleep ${AGENT_START_DELAY}

export PATH="${PATH}:${PWD}/build/"

echo "PATH is now ${PATH}"

echo "Configuring agent from ${PWD}"
${NSV_CFG_CMD}
cd -