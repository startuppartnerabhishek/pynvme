#!/bin/bash

CONF_DIR=./conf
CONF_FILE=${CONF_DIR}/simconf.json
AGENT_START_DELAY=5

NSV_ROOT_DIR=$(jq -rj .global.nsv_test_root_path ${CONF_FILE})
TARGET_CFG_CMD="tools/nvme_linux_tgt.sh"

# configure linux target
cd ${NSV_ROOT_DIR}
echo "Configuring target from ${PWD} - will report error if already configured"

${TARGET_CFG_CMD}