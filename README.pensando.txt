(1) Please go through baseline knowhow and content from README.md. It focusse on stock pyNVME behaviour for testing
    NVME PCIE devices.

===================================

(2) This document contains information about running py-NVME in SIM mode against a Pensando Control-Plane running
    as a user-space application.

===================================

(3) Installation:
    PYNVME_ROOT_DIR$ ./install.sh

    Installation sets up SPDK and other dependencies to create a working build-and-run environment.

===================================

(4) Build:
    Pre-requisites:
      (a) See scripts/simconf.json and ensure that the nsv/nvme-test locations are correctly mentioned.
      (b) Ensure that the nvme-test location contains built binaries. pynvme links to the nvme-test SDK libraries in SIM mode.
    PYNVME_ROOT_DIR$ make

===================================

(5) Running:

   Note that an LD_LIBRARY_PATH export may be needed to ensure that NSV-TEST libraries can be located at runtime.

   E.g.
   export LD_LIBRARY_PATH=${AGENT_LIB_PATH}

    (a) PCIE mode (stock)
        No intentional changes were made to this environment, and so it is expected that procedures documented
        for stock pyNVME should work.


    (b) SIM mode
        Most SIM mode testing has been done using wrapper scripts.

        (i) One-test-at-a-time

            PYNVME_ROOT_DIR$ scripts/pensando/wrappers/run_sim_test.sh PYTHON_SRC_FILE::TEST_FUNCTION

            E.g.:
            PYNVME_ROOT_DIR$ scripts/pensando/wrappers/run_sim_test.sh scripts/pensando/tests/connectivity_test.py::test_py_invocation

        (ii) Batch-mode
 
             Invoke the run_sim_tests_batch.sh script without arguements to get up-to-date help. An example is shown below.

SYNOPSIS:
scripts/pensando/wrappers/run_sim_tests_batch.sh: Run multiple SIM tests
Run a number of test-functions (aka nodes), select a number of tests with one or more markers, or select tests/markers through files

OPTIONS:
-c CONFFILE (optional): JSON config file for test, default ./conf/simconf.json
-l LOGDIR (optional): Directory for saving logs, default ./logs
-t LOGFILETAG (optional): readable-tag for logfile
-m TESTMODE (compulsory): Test Mode = MARKERS/NODES
-b: remaining arguements are batch-files containing tests/markers, depending on -m option
tests/markers/files (at least one): List of tests/markers, or files containing tests/markers. Depends on combination of -m and -b options
-h: print this help message

EXAMPLES:

Run many tests: --->
    scripts/pensando/wrappers/run_sim_tests_batch.sh -m NODES path/to/file1.py::proc1 path/to/file2.py::class2::test2 ...

Run many tests, selected by markers: --->
    scripts/pensando/wrappers/run_sim_tests_batch.sh -m MARKERS marker1 marker2 ...

Run many tests, from file: --->
    scripts/pensando/wrappers/run_sim_tests_batch.sh -m NODES -b path/to/file1.txt path/to/file2.txt ...
where file*.txt mention a list of tests or markers, one per line

===================================


(6) Key points-of-interest

PYNVME_ROOT_DIR$ --->
.
├── conf
├── logs
├── scripts
└── src

conf - contains runtime configuration files. simconf.json is used by default to configure the test in SIM mode.

logs - by default (possible to override with conf file, the default conf-file being conf/simcnof.json) the logs go here

scripts ---> contains python test scripts, pensando extensions to python framework, and wrapper scripts for testing.
             See scripts/pensando/tests/connectivity_tests.py (test function test_nvme_identify_controller) for an example
             of parsing responses, and validating Command Responses against expected values.

├── ftl
├── pensando
│   ├── batch-tests
│   ├── py-utils ---------------> pensando framework additions
│   ├── tests
│   └── wrappers ------------------> wrapper scripts to invoke batch/sim tests etc.
├── performance
├── stress
└── trace

src -----> contains pynvme spdk extension for PCIE mode, and connecting logic with Pensando NVME Control Plane SDK for SIM modes
.
└── driver
    ├── common ----> common sources for SIM and PCIE modes
    ├── cython -----> cython layer connecting Python with C extensions
    ├── include
    ├── sim ----------> sources used only in SIM mode - contain many functions with names identical to spdk-extensions
    └── spdk-extensions  -----------> pynvme extensions over NVME-SPDK


For a design overview, see: https://docs.google.com/document/d/1WN6auMxVJ3kNI2fwQqmHaMY_bzhQwfvo_aAOjTYadvE/edit?usp=sharing

The python layers use a .so file (nvme.so or nvme_sim.so), loaded as a python module. The .so library implements the cython
interface to correctly communicate with a Pensando NVME Agent, or a PCIE device, depending on the test mode (PCIE/SIM).

nvme.so = driver/common + driver/cython + driver/spdk-extensions + statically linked SPDK
nvme_sim.so = driver/common + driver/cython + driver/sim + statically linked NVME-SDK (Pensando)

Both these libraries have a lot of common symbol-names, but they behave differently as they have different underlying layers.
The PCIE mode library nvme.so controls and directly drives a PCIE device.
The SIM mode library nvme_sim.so communicates with the NVME-SDK, which in turns talks to an Agent over shared-mem interfaces.


                                ____________________
                               |                    |
                               |  Pytest Framework  | ---> conftest.py, pytest.ini
                               |____________________|
                    
                                ____________________
                               |                    |
                               |  Python Test Logic | -------------> your test logic goes here
                               |____________________|
             
                                ____________________
                               |                    |
                               |      Cython        | ------> src/driver/cython
                               |____________________|
             


    nvme_sim.so (mode="SIM")                              nvme.so (mode = "PCIE")

    ____________________                               _____________________________
   |                    |                              |                            |
   | SIM glue=          |                              | pyNVME SPDK extensions =   | 
   | src/driver/sim     |                              |                            |
   |____________________|                              | src/driver/spdk-extensions |
                                                       |____________________________|

                                                       _____________________________
                                                       |                            |
                                                       |      NVME SPDK driver      |
                                                       |____________________________|
    _______________________
   |                       |                           _____________________________
   | NVME-Test SDK =       |                           |                            |
   | libpdsnvmedriver.so   |                           |      SPDK <---> DPDK       |
   | libpdsnvmepci.so      |                           |                            |
   |_______________________|                           |____________________________|
             

------ Shared Mem ------------                         ------------ PCIE ----------

          ^                                                          ^
          |                                                          |
          |                                                          |
          V                                                          V

       NVME - Agent                                               Device


