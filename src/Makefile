#
#  BSD LICENSE
#
#  Copyright (c) Crane Chu <cranechu@gmail.com>
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in
#      the documentation and/or other materials provided with the
#      distribution.
#    * Neither the name of Intel Corporation nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#SPDK infrastructure
SPDK_ROOT_DIR := $(abspath $(CURDIR)/../spdk)
include $(SPDK_ROOT_DIR)/mk/spdk.common.mk

DRIVER_SRC_DIR = ./driver

DRV_SPDK_EXT_DIR = $(DRIVER_SRC_DIR)/spdk-extensions
DRV_SIM_DIR = $(DRIVER_SRC_DIR)/sim
DRV_COMMON_DIR = $(DRIVER_SRC_DIR)/common
DRV_CYTHON_DIR = $(DRIVER_SRC_DIR)/cython

SPDK_EXT_SRCS = $(DRV_SPDK_EXT_DIR)/driver.c $(DRV_SPDK_EXT_DIR)/intr_mgt.c $(DRV_SPDK_EXT_DIR)/ioworker.c $(wildcard $(DRV_COMMON_DIR)/*.c)

CFLAGS += -I$(SPDK_ROOT_DIR)/include -I./driver/include

C_SRCS = $(SPDK_EXT_SRCS)
LIBNAME = pynvme

include $(SPDK_ROOT_DIR)/mk/spdk.lib.mk

$(info driver/src build ---- source files are $(SPDK_EXT_SRCS))

#cython part
clean: cython_clean c_objs_clean

cython_clean:
	@sudo rm -rf build $(DRV_CYTHON_DIR)/*.o nvme.*.so cdriver.c $(DRV_CYTHON_DIR)/driver_wrap.c unittest

cython_lib:
	@python3 setup.py build_ext -i --force

# sim and driver

c_objs_clean:
	@sudo rm -rf $(DRV_COMMON_DIR)/*.[od] $(DRV_SPDK_EXT_DIR)/*.[od]

.PHONY: all unittest clean cython_clean c_objs_clean

all: cython_lib

#unit test
UT_SRCS = $(wildcard $(DRV_SPDK_EXT_DIR)/*_ut.c)

unittest: $(UT_SRCS)
	gcc -o $@ $^ -lcunit -I../spdk/include -I../spdk/test -I./driver/include
	./$@

