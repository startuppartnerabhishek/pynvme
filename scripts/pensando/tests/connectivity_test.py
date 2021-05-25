import logging
import pytest
import subprocess
import sys

additional_py_modules_path = "scripts/pensando/py-utils"

sys.path.append(additional_py_modules_path)

from conftest import globalNvmeModule as driverIntfObj
import cParser as P
import configStore as CfgStore
import validator as Validator

@pytest.mark.sim_test_trial_batch
def test_py_invocation():
    print("Your test was SUCCESSFULLY invoked. Congratulations!")
    print ("This is a rare direct print usage, prefer the logging object instead.")
    logging.info("test_py_invocation: SUCCESS")

@pytest.mark.sim_test_trial_batch
def test_py_invocation_sample_failure():
    print("Your tests was SUCCESSFULLY invoked. However this is a test that is meant to FAIL!")
    logging.error("test_py_invocation_sample_failure - trying to force failure - this error log is expected for this test")
    pytest.fail("test_py_invocation_sample_failure: this test should fail, so here goes.");
    logging.debug("test_py_invocation_sample_failure: YOU SHOULD NEVER see this line - we were meant to fail earlier")

@pytest.mark.sim_test_trial_batch
def test_driver_common_reachability():
    # the 'strings' are passed to cython interface functionn as byte arrays, to stay in tune with the way cython does with primitive char * conversions
    src = b"test input"
    dst = b"Target area badbad"
    askFor = int(20)
    delta = 90
    count = int(len(src))
    dst_len = len(dst)

    logging.info("test_driver_common_reachability: test STARTED =======================")

    logging.debug("Imported nvme driver-object")
    logging.debug(driverIntfObj)

    logging.info("Before first call [EQUAL TEST] - src %s, dst %s, count %u, asked for %u", src, dst, count, askFor)
    got = driverIntfObj.pen_common_connectivity_check(src, dst, count, askFor)

    assert got == askFor
    assert src == dst[0:count] # first n bytes of target must match with src
    assert src == b"test input" # src should not change
    assert dst[count:dst_len] == b'a badbad'

    logging.debug("First call returned %d, src %s, dst %s", got, src, dst)

    dst = b"CLEARCLEAR"

    logging.info("Before second call [MISMATCH TEST] - src %s, dst %s, count 0, asked for %u, will compare against %u", src, dst, askFor + delta, askFor)
    got = driverIntfObj.pen_common_connectivity_check(src, dst, 0, askFor + delta)

    logging.debug("Second call returned %d, src %s, target %s", got, src, dst)

    assert got == askFor + delta
    assert src == b"test input"
    assert dst == b"CLEARCLEAR"

    logging.info("test_driver_common_reachability: test completed SUCCESSFULLY =======================")

@pytest.mark.sim_test_trial_batch
def test_nvme_identify_controller(pcie):
    def nvme_custom_basic_init(nvme0):
        logging.info("user defined custom nvme init")

        # 1. wait for controller ready
        nvme0[0x14] = 0
        while not (nvme0[0x1c]&0x1) == 0: pass

    # 2. set admin queue registers
        nvme0.init_adminq()

        # 3. set register cc
        nvme0[0x14] = 0x00460000

        # 4. enable cc.en
        nvme0[0x14] = 0x00460001

        # 5. wait csts.rdy to 1
        while not (nvme0[0x1c]&0x1) == 1: pass

        logging.info("nvme_custom_basic_init: attempting Controller-Identify for controller nvme0")
        logging.info(nvme0)
        
        id_ctrlr_resp_buf = driverIntfObj.Buffer(nvme0, 4096)

        # 6. identify controller
        nvme0.identify(id_ctrlr_resp_buf).waitdone()

        logging.info("Parsing %u bytes of response buffer", P.struct_size("spdk_nvme_ctrlr_data"))
        parsed_response_id_ctrlr = P.struct_parse(id_ctrlr_resp_buf[0:P.struct_size("spdk_nvme_ctrlr_data")], "spdk_nvme_ctrlr_data")

        logging.info("ID Ctrlr Response buf - num-namespaces %u, sqes min-max (%u, %u), cqes min-max(%u, %u)",
            parsed_response_id_ctrlr.nn, parsed_response_id_ctrlr.sqes.min, parsed_response_id_ctrlr.sqes.max, parsed_response_id_ctrlr.cqes.min, parsed_response_id_ctrlr.cqes.max)

        logging.info("Serial Number")
        logging.info(parsed_response_id_ctrlr.sn)

        logging.info("Validating ID Controller Response")
        Validator.controllerValidateResponse("nvme0", id_ctrlr_resp_buf)

        # 7. create and identify all namespace
        logging.info("nvme_custom_basic_init: Identify namespaces (automatically)")
        nvme0.init_ns()

    logging.info("STARTED TEST test_nvme_identify_controller with nvme0 =========================")
    logging.info(pcie)

    nvme0 = driverIntfObj.Controller(pcie, nvme_init_func=nvme_custom_basic_init)

    logging.info("test_nvme_identify_controller: test completed SUCCESSFULLY =======================")

# pytest appears to ignore duplicate test-names, so we create wrappers to call this logic
# to get many fences in our batch of tests
def test_batch_fence(prevBatchInfo, currBatchInfo):
    logging.info("Batch marker (not a real test) - prev and current")
    logging.info(prevBatchInfo)
    logging.info(currBatchInfo)

    if (prevBatchInfo):
        logging.info(prevBatchInfo)
        if "cleanup" in prevBatchInfo:
            logging.info("Cleaning up previous batch")
            
            logging.info("Will invoke %s with args %s", prevBatchInfo['cleanup']['command'], prevBatchInfo['cleanup']['args'])
            full_args = [prevBatchInfo['cleanup']['command']]
            full_args.extend(prevBatchInfo['cleanup']['args'])
            logging.info("Prepared full_args")
            logging.info(full_args)
            exit_status = subprocess.run(full_args)
            logging.info("Exit status")
            logging.info(exit_status)
            assert exit_status.returncode == 0, "Non zero return code from cleanup"
        else:
            logging.info("No cleanup for previous batch command")
    else:
        logging.info("No previous batch info")

    assert currBatchInfo, "currBatchInfo cannot be NULL"

    logging.info(currBatchInfo)

    if "setup" in currBatchInfo:
        logging.info("Setting up batch %s", currBatchInfo['name'])
        logging.info("Will invoke %s with args %s", currBatchInfo['setup']['command'], currBatchInfo['setup']['args'])
        full_args = [currBatchInfo['setup']['command']]
        full_args.extend(currBatchInfo['setup']['args'])
        logging.info("Prepared full_args")
        logging.info(full_args)
        exit_status = subprocess.run(full_args)
        logging.info("Exit status")
        logging.info(exit_status)
        assert exit_status.returncode == 0, "Non zero return code from setup"
    else:
        logging.info("No setup for current batch command")

    if ("test_config" in currBatchInfo):
        CfgStore.refreshConfig(currBatchInfo['test_config'])

    logging.info("Fence completed")

def test_batch_final_cleanup(testFinalCleanup):
    logging.info("Cleaning up batch test setup")

    logging.info(testFinalCleanup)

    if None != testFinalCleanup:
        assert "command" in testFinalCleanup, "No command!"
        logging.info("Will invoke %s with args %s", testFinalCleanup['command'], testFinalCleanup['args'])
        full_args = [testFinalCleanup['command']]
        full_args.extend(testFinalCleanup['args'])
        logging.info("Prepared full_args")
        logging.info(full_args)
        exit_status = subprocess.run(full_args)

        assert exit_status.returncode == 0, "Non zero return code from command"

    logging.info("Batch Test setup cleaned up")

def test_batch_initial_setup(testInitialSetup):
    logging.info("Setting up batch tests")

    logging.info(testInitialSetup)

    if None != testInitialSetup:
        assert "command" in testInitialSetup, "No command!"
        logging.info("Will invoke %s with args %s", testInitialSetup['command'], testInitialSetup['args'])
        full_args = [testInitialSetup['command']]
        full_args.extend(testInitialSetup['args'])
        logging.info("Prepared full_args")
        logging.info(full_args)
        exit_status = subprocess.run(full_args)

        logging.info(exit_status)

        assert exit_status.returncode == 0, "Non zero return code from command"

    logging.info("Batch Test setup completed")

def test_batch_fence1(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence1 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence2(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence2 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence3(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence3 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence4(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence4 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence5(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence5 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence6(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence6 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence7(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence7 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence8(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence8 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence9(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence9 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence10(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence10 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence11(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence11 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence12(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence12 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence13(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence13 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence14(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence14 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence15(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence15 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)

def test_batch_fence16(prevBatchInfo, currBatchInfo):
    logging.info("test_batch_fence16 redirects to test_batch_fence")
    test_batch_fence(prevBatchInfo, currBatchInfo)