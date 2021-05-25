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

# -*- coding: utf-8 -*-


import time
import pytest
import random
import logging
import inspect
import importlib
import json
import sys

additional_py_modules_path = "scripts/pensando/py-utils"

sys.path.append(additional_py_modules_path)

import batch as B
import configStore as CfgStore

# defer this binding
# import nvme as d
d=None
globalTestOptions = None
globalNvmeModule = None
globalBatchCtrl = None

def pytest_addoption(parser):
    parser.addoption(
        "--pciaddr", action="store", default="", help="pci (BDF) address of the device under test, e.g.: 02:00.0"
    )
    parser.addoption(
        "--deviceMode", action="store", default="SIM", help="choose test device mode - run against PCIE or SIM (simulation)"
    )
    parser.addoption(
        "--conf", action="store", default="conf/simconf.json", help="environment configuration"
    )
    parser.addoption(
        "--agentconf", action="store", default="conf/agentconf.json", help="environment configuration"
    )

def pytest_configure(config):
    global globalTestOptions
    global d
    global globalNvmeModule
    global globalBatchCtrl
    sim_config = None
    sim_config_as_string = None

    deviceMode = config.getoption("--deviceMode")
    conf = config.getoption("--conf")
    agentconf = config.getoption("--agentconf")
    driverModule = "nvme_sim"
    
    if deviceMode == "PCIE":
        logging.info("Switching to PCIE mode")
        driverModule = "nvme"
    else:
        assert conf != None, "SIM could not find conf-file name"
        deviceMode = "SIM"
        with open(conf, "r") as f:
            sim_config = json.load(f)

        sim_config_as_string = json.dumps(sim_config)
        assert sim_config_as_string != None, "SIM mode requires a valid json conf-file"

    globalTestOptions = {
        "mode": deviceMode,
        "config_file": conf,
        "driverModule": driverModule,
        "config_json": sim_config,
        "config_as_string": sim_config_as_string,
        "initial_agent_conf_file": agentconf
    }

    globalBatchCtrl = B.BatchControl(sim_config)

    CfgStore.refreshConfig(agentconf)

    globalBatchCtrl.print()

    logging.info("pytest_configure: loading nvme module %s" % globalTestOptions["driverModule"])
    d = importlib.import_module(globalTestOptions["driverModule"])
    globalNvmeModule = d

    logging.info("Global Config from conftest")
    logging.info(globalTestOptions)

@pytest.fixture(scope="function", autouse=True)
def script(request):
    # skip empty tests
    sourcecode = inspect.getsourcelines(request.function)[0]
    if 'pass' in sourcecode[-1] and len(sourcecode) < 5:
        pytest.skip("empty test function")

    # measure test time, and set random seed by time
    start_time = time.time()
    d.srand(int(start_time*1000000)&0xffffffff)
    yield
    logging.info("test duration: %.3f sec" % (time.time()-start_time))


@pytest.fixture(scope="session")
def pciaddr(request):
    logging.debug("fixture pciaddr(request %s)" % request)
    if globalTestOptions["mode"] == "PCIE":
        logging.debug("returning pciaddr %s" % request.config.getoption("--pciaddr"))
        return request.config.getoption("--pciaddr")
    else:
        logging.debug("returning config instead of pciaddr %s" % globalTestOptions["config_as_string"])
        return globalTestOptions["config_as_string"]

@pytest.fixture(scope="function")
def pcie(pciaddr):
    logging.debug("fixture pcie(pcieaddr %s)" % pciaddr); sys.stdout.flush();

    if globalTestOptions["mode"] == "PCIE":
        logging.debug("fixture pcie [PCIE] - creating Pcie object with addr %s" % pciaddr); sys.stdout.flush();
        ret = d.Pcie(pciaddr, 0, "PCIE")
    else:
        logging.debug("fixture pcie [SIM] - creating Pcie object to access nvme0 with config %s" % globalTestOptions["config_json"]["nvme0"]); sys.stdout.flush();
        ret = d.Pcie(json.dumps(globalTestOptions["config_json"]["nvme0"]), 0, "SIM")
    yield ret
    ret.close()

@pytest.fixture(scope="function")
def nvme0(pcie):
    ret = None
    logging.debug("fixture nvme0(pcie)"); sys.stdout.flush();
    ret = d.Controller(pcie)
    logging.debug("fixture nvme0 - got controller object"); sys.stdout.flush();
    logging.debug(ret); sys.stdout.flush();
    yield ret

@pytest.fixture(scope="function")
def subsystem(nvme0):
    ret = d.Subsystem(nvme0)
    yield ret


@pytest.fixture(scope="function")
def nvme0n1(nvme0):
    ret = d.Namespace(nvme0)
    yield ret
    ret.close()


@pytest.fixture(scope="function")
def qpair(nvme0):
    num_of_entry = (nvme0.cap & 0xffff) + 1
    num_of_entry = min(1024, num_of_entry)
    ret = d.Qpair(nvme0, num_of_entry)
    yield ret
    ret.delete()

    
@pytest.fixture(scope="function")
def tcg(nvme0):
    ret = d.Tcg(nvme0)
    yield ret
    ret.close()


@pytest.fixture(scope="session")
def buf(nvme0):
    ret = d.Buffer(nvme0, 4096, "pynvme buffer")
    yield ret
    del ret


@pytest.fixture(scope="function")
def verify(nvme0n1):
    ret = nvme0n1.verify_enable(True)
    yield ret


@pytest.fixture(scope="function")
def aer():
    assert False, "aer fixture is replaced by admin command nvme0.aer()"


@pytest.hookimpl(tryfirst=True, hookwrapper=True)
def pytest_runtest_makereport(item, call):
    # execute all other hooks to obtain the report object
    outcome = yield
    rep = outcome.get_result()
    # set a report attribute for each phase of a call, which can
    # be "setup", "call", "teardown"
    setattr(item, "rep_" + rep.when, rep)

@pytest.fixture(scope="function")
def currBatchInfo():
    logging.info("Fixture currBatchInfo: evaluating for batch %d", globalBatchCtrl.getCurrentBatchNumber())
    ret = globalBatchCtrl.moveToNextBatch()
    logging.info("currBatchInfo = ")
    logging.info(ret)
    yield ret

@pytest.fixture(scope="function")
def prevBatchInfo():
    logging.info("Fixture prevBatchInfo: evaluating for batch %d", globalBatchCtrl.getCurrentBatchNumber())
    if (globalBatchCtrl.getCurrentBatchNumber() < 0):
        ret = None
    else:
        ret = globalBatchCtrl.getCurrentBatch(False)
    logging.info("prevBatchInfo = ")
    logging.info(ret)
    yield ret

@pytest.fixture(scope="function")
def testFinalCleanup():
    ret = globalBatchCtrl.getFinalCleanup()

    yield ret

@pytest.fixture(scope="function")
def testInitialSetup():
    ret = globalBatchCtrl.getInitialSetup()

    yield ret