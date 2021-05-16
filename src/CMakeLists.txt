cmake_minimum_required(VERSION  3.16.3)

project(pyNvmeDrvSim)

# make outputs verbose
set(CMAKE_VERBOSE_MAKEFILE ON)

set(DRIVER_SRC_BASE ./driver)

set(SIM_DIR ${DRIVER_SRC_BASE}/sim)
set(COMMON_DIR ${DRIVER_SRC_BASE}/common)
set(DRV_INCLUDE_DIR ${DRIVER_SRC_BASE}/include)
set(SPDK_INCLUDE_DIR ../spdk/include)
set(SIM_INCLUDE_DIR ${SIM_DIR}/cJSON)
set(CONF_FILE ../../conf/simconf.json)
set(AGENT_INCLUDE_DIR_CONF_KEY .global.nsv_test_root_path)

execute_process(COMMAND jq -rj ${AGENT_INCLUDE_DIR_CONF_KEY} ${CONF_FILE} OUTPUT_VARIABLE NSV_ROOT)
message(----NSV_ROOT----- = ${NSV_ROOT})

set(AGENT_LIB_INCLUDE_DIR "../${NSV_ROOT}/sim_libs")

message(---AGENT-LIB-INCLUDE-DIR------  = ${AGENT_LIB_INCLUDE_DIR})

set(ALL_SRC
    ${SIM_DIR}/driver_sim.c
    ${SIM_DIR}/intr_mgt_sim.c
    ${SIM_DIR}/ioworker_sim.c
    ${SIM_DIR}/sim_utils.c
    ${SIM_DIR}/sim_logging.c
    ${SIM_DIR}/sim_completions.c
    ${SIM_DIR}/cJSON/cJSON.c

    ${COMMON_DIR}/driver_common.c
)

include_directories(${DRV_INCLUDE_DIR} ${SPDK_INCLUDE_DIR} ${SIM_INCLUDE_DIR} ${AGENT_LIB_INCLUDE_DIR})

set(CMAKE_C_FLAGS "-fPIC -DSIM_MODE -Wall -Werror -g")

add_library(drvsim STATIC ${ALL_SRC})
