cmake_minimum_required(VERSION  3.16.3)

project(pyNvmeDrvSim)

set(DRIVER_SRC_BASE ./driver)

set(SIM_DIR ${DRIVER_SRC_BASE}/sim)
set(COMMON_DIR ${DRIVER_SRC_BASE}/common)
set(DRV_INCLUDE_DIR ${DRIVER_SRC_BASE}/include)
set(SPDK_INCLUDE_DIR ../spdk/include)

set(ALL_SRC
    ${SIM_DIR}/driver_sim.c
    ${SIM_DIR}/intr_mgt_sim.c
    
    ${COMMON_DIR}/driver_common.c
)

include_directories(${DRV_INCLUDE_DIR} ${SPDK_INCLUDE_DIR})

add_library(drvsim STATIC ${ALL_SRC})