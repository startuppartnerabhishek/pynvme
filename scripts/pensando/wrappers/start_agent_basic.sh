#!/bin/bash

CONF_DIR=./conf
CONF_FILE=${CONF_DIR}/simconf.json
AGENT_CONF_FILE=${CONF_DIR}/agentconf.json
AGENT_START_DELAY=5

NSV_ROOT_DIR=$(jq -rj .global.nsv_test_root_path ${CONF_FILE})
AGENT_EXEC_CMD="build/nvme_agent --run_dir run --config_dir run/conf"


NSV_CFG_CMD="tools/nsv_scale_cfg.sh"

# start the agent
cd ${NSV_ROOT_DIR}
echo 'Starting agent from ${PWD}'
${AGENT_EXEC_CMD} &

echo 'Waiting for Agent to come up - ${AGENT_START_DELAY} second\(s\)'
sleep ${AGENT_START_DELAY}

export PATH=${PATH}:${PWD}/build/

echo "Configuring agent from ${PWD}"
${NSV_CFG_CMD}
cd -