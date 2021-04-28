import logging
import pytest

import nvme as driverIntfObj

def test_py_invocation():
    print("Your test was SUCCESSFULLY invoked. Congratulations!")
    print ("This is a rare direct print usage, prefer the logging object instead.")
    logging.info("test_py_invocation: SUCCESS")

def test_py_invocation_sample_failure():
    print("Your tests was SUCCESSFULLY invoked. However this is a test that is meant to FAIL!")
    logging.error("test_py_invocation_sample_failure - trying to force failure - this error log is expected for this test")
    pytest.fail("test_py_invocation_sample_failure: this test should fail, so here goes.");
    logging.debug("test_py_invocation_sample_failure: YOU SHOULD NEVER see this line - we were meant to fail earlier")

def test_driver_common_reachability():
    # the 'strings' are passed to cython interface functionn as byte arrays, to stay in tune with the way cython does with primitive char * conversions
    src = b"test input"
    dst = b"Target area badbad"
    askFor = int(20)
    count = int(len(src)) + 1

    logging.debug("Imported nvme driver-object")
    logging.debug(driverIntfObj)

    logging.info("Before first call [EQUAL TEST] - src %s, dst %s, count %u, asked for %u", src, dst, count, askFor)
    got = driverIntfObj.pen_common_connectivity_check(src, dst, count, askFor)
    
    assert got == askFor

    logging.debug("Second call returned %u", got)

    logging.info("Before second call [MISMATCH TEST] - src %s, dst %s, count 0, asked for %u, will compare against %u", src, dst, askFor + 90, askFor)
    got = driverIntfObj.pen_common_connectivity_check(src, dst, 0, askFor + 90)

    logging.debug("Second call returned %d", got)

    assert got != askFor
