import logging
import subprocess
import sys

additional_py_modules_path = "scripts/pensando/py-utils"
sys.path.append(additional_py_modules_path)

import configStore as CfgStore

def apply_batch_fence(prevBatchInfo, currBatchInfo):

    logging.info("Applying batch fence (prevBatch, currBatch)")
    logging.debug(prevBatchInfo)
    logging.debug(currBatchInfo)

    if (prevBatchInfo):
        if "cleanup" in prevBatchInfo:
            logging.info("Cleaning up previous batch")
            
            logging.info("Will invoke %s with args %s", prevBatchInfo['cleanup']['command'], prevBatchInfo['cleanup']['args'])
            full_args = [prevBatchInfo['cleanup']['command']]
            full_args.extend(prevBatchInfo['cleanup']['args'])
            logging.debug("Prepared full_args")
            logging.debug(full_args)
            exit_status = subprocess.run(full_args)
            logging.debug("Exit status")
            logging.debug(exit_status)
            assert exit_status.returncode == 0, "Non zero return code from cleanup"
        else:
            logging.info("No cleanup for previous batch command")
    else:
        logging.info("No previous batch info")

    assert currBatchInfo, "currBatchInfo cannot be NULL"

    if "setup" in currBatchInfo:
        logging.info("Setting up batch %s", currBatchInfo['name'])
        logging.info("Will invoke %s with args %s", currBatchInfo['setup']['command'], currBatchInfo['setup']['args'])
        full_args = [currBatchInfo['setup']['command']]
        full_args.extend(currBatchInfo['setup']['args'])
        logging.debug("Prepared full_args")
        logging.debug(full_args)
        exit_status = subprocess.run(full_args)
        logging.debug("Exit status")
        logging.debug(exit_status)
        assert exit_status.returncode == 0, "Non zero return code from setup"
    else:
        logging.info("No setup for current batch command")

    if ("test_config" in currBatchInfo):
        CfgStore.refreshConfig(currBatchInfo['test_config'])

    logging.info("Fence completed")

def apply_final_global_fence(testFinalCleanup):

    logging.info("Applying final global fence (finalCleanup)")
    logging.debug(testFinalCleanup)

    if None != testFinalCleanup:
        assert "command" in testFinalCleanup, "No command!"
        logging.info("Will invoke %s with args %s", testFinalCleanup['command'], testFinalCleanup['args'])
        full_args = [testFinalCleanup['command']]
        full_args.extend(testFinalCleanup['args'])
        logging.debug("Prepared full_args")
        logging.debug(full_args)
        exit_status = subprocess.run(full_args)

        assert exit_status.returncode == 0, "Non zero return code from command"

def apply_first_global_fence(testInitialSetup):

    logging.info("Applying initial global fence (initialSetup)")
    logging.debug(testInitialSetup)

    if None != testInitialSetup:
        assert "command" in testInitialSetup, "No command!"
        logging.info("Will invoke %s with args %s", testInitialSetup['command'], testInitialSetup['args'])
        full_args = [testInitialSetup['command']]
        full_args.extend(testInitialSetup['args'])
        logging.debug("Prepared full_args")
        logging.debug(full_args)
        exit_status = subprocess.run(full_args)

        logging.debug(exit_status)

        assert exit_status.returncode == 0, "Non zero return code from command"
