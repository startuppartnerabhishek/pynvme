#!/bin/bash

# example scripts/pensando/wrappers/run_sim_test.sh scripts/pensando/tests/connectivity_test.py::test_driver_common_reachability

RED='\033[0;31m'
LIGHT_GREEN='\033[0;32m'
NC='\033[0m' # No Color
YELLOW='\033[0;33m'
LOGDIR=./logs
LOGFILE_NAME_TIMESTAMP_FORMAT='+%Y:%m:%d-%R:%S'
CONF_DIR=./conf
CONF_FILE=${CONF_DIR}/simconf.json
AGENT_CONF_FILE=${CONF_DIR}/sample_nvme_testcfg.json
NSV_ROOT_DIR=$(jq -rj .global.nsv_test_root_path ${CONF_FILE})
AGENT_LIB_PATH=${NSV_ROOT_DIR}/build

if [ "$#" -ne 1 ] && [ "$#" -ne 2 ];
then
    echo -e "$0: ${RED}invalid arguments ${NC}">&2
    echo >&2
    echo Usage is >&2
    echo $0 \"script_file::script_function\" [JSON_CONF_FILE] >&2
    echo >&2
    echo -e "${YELLOW}EXAMPLE (try it from the pynvme base directory):${NC}" >&2
    echo -e "${LIGHT_GREEN}$0 scripts/pensando/tests/connectivity_test.py::test_py_invocation${NC}" >&2
    echo >&2
    exit -1
fi

echo -e "Working with ${LIGHT_GREEN} TESTS=$1 ${NC}"

PY_FILE=$(basename $(echo $1 | cut -d ':' -f1))
PY_PROC=$(echo $1 | cut -d ':' -f3)

OUTFILE_PREFIX=${LOGDIR}/`date ${LOGFILE_NAME_TIMESTAMP_FORMAT}`-${PY_FILE}-${PY_PROC}
LOGFILE=${OUTFILE_PREFIX}.log
REPORTFILE=${OUTFILE_PREFIX}.xls

echo -e "Invoking ${LIGHT_GREEN}PROCEDURE=${PY_PROC} from\nFILE=${PY_FILE}\nLOGFILE=${LOGFILE}\nREPORTFILE=${REPORTFILE}${NC}"
echo "Will export LD_LIBRARY_PATH = ${AGENT_LIB_PATH}"

BASE_COMMAND='make sim_test'

# set -x

export LD_LIBRARY_PATH=${AGENT_LIB_PATH}
$BASE_COMMAND TESTS=\"$1\" LOGFILE=${LOGFILE} CONF_FILE=${CONF_FILE} AGENT_CONF_FILE=${AGENT_CONF_FILE} REPORTFILE=${REPORTFILE}
