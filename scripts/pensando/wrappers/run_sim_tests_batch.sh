#!/bin/bash

PROJECT_ROOT_DIR=.
LOGDIR=${PROJECT_ROOT_DIR}/logs
CONFDIR=${PROJECT_ROOT_DIR}/conf
CONFFILE=${CONFDIR}/simconf.json
TESTMODE=
BATCHFILEMODE=0
TESTS=
MARKEROPTION=
LOGDIR=./logs
LOGFILE_NAME_TIMESTAMP_FORMAT='+%Y:%m:%d-%R:%S'
NSV_DIR_CONF_TAG=global.nsv_test_root_path
NSV_BUILD_RELATIVE_PATH=./build
EXTRA_LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:.
LOGTAG="-IMMEDIATE-CMD-LINE-"
LOGSUFFIX=

# always eat the script-name
EATENARGS=1

# For marker mode to work, we need to configure marker information and script-path information
# in pytest.ini

mkdir -p ${LOGDIR}

python --version

echo PROJECT_ROOT_DIR = ${PROJECT_ROOT_DIR}

echo You are running from ${PWD}

printUsageAndExit() {
  echo "" >&2
  echo "SYNOPSIS:"
  echo "$0: Run multiple SIM tests" >&2
  echo "Run a number of test-functions (aka nodes), select a number of tests with one or more markers, or select tests/markers through files" >&2
  echo "" >&2
  echo "OPTIONS:" >&2
  echo "-c CONFFILE (optional): JSON config file for test, default ${CONFFILE}" >&2
  echo "-l LOGDIR (optional): Directory for saving logs, default ${LOGDIR}" >&2
  echo "-t LOGFILETAG (optional): readable-tag for logfile" >&2
  echo "-m TESTMODE (compulsory): Test Mode = MARKERS/NODES" >&2
  echo "-b: remaining arguements are batch-files containing tests/markers, depending on -m option" >&2
  echo "tests/markers/files (at least one): List of tests/markers, or files containing tests/markers. Depends on combination of -m and -b options" >&2
  echo "-h: print this help message" >&2
  echo >&2
  echo "EXAMPLES:" >&2
  echo >&2
  echo "Run many tests: --->" >&2
  echo "    $0 -m NODES path/to/file1.py::proc1 path/to/file2.py::class2::test2 ..." >&2
  echo >&2
  echo "Run many tests, selected by markers: --->" >&2
  echo "    $0 -m MARKERS marker1 marker2 ..." >&2
  echo >&2
  echo "Run many tests, from file: --->" >&2
  echo "    $0 -m NODES -b path/to/file1.txt path/to/file2.txt ..." >&2
  echo "where file*.txt mention a list of tests or markers, one per line" >&2
  echo >&2
  exit 1;
}

setMode() {
    if [ "$1" = "NODES" ] || [ "$1" == "MARKERS" ] ; then
        TESTMODE=$1
        echo TESTMODE set to ${TESTMODE}

        if [ "${TESTMODE}" = "MARKERS" ]; then
            MARKEROPTION='-m '
        fi
    else
        echo "Do not recognize mode with -m $1" >&2
        echo "Allowed values are" >&2
        echo "-m NODES|MARKERS" >&2
        printUsageAndExit
    fi

    EATENARGS=`expr ${EATENARGS} + 2`
}

configLibPath() {
    NSV_BUILD_DIR=$(jq -r .${NSV_DIR_CONF_TAG} ${CONFFILE})${NSV_BUILD_RELATIVE_PATH}
    echo "Extracted relative nsv-test build path using tag ${NSV_DIR_CONF_TAG} as ${NSV_BUILD_DIR}"
    EXTRA_LD_LIBRARY_PATH=${EXTRA_LD_LIBRARY_PATH}:${NSV_BUILD_DIR}
    export LD_LIBRARY_PATH=${EXTRA_LD_LIBRARY_PATH}
}

while getopts "hm:c:bl:t:" opt; do
  case $opt in
    h)
      printUsageAndExit
      ;;

    m)
      echo Got Mode $OPTARG
      setMode "$OPTARG"
      ;;

    c)
      echo Conf File $OPTARG
      CONFFILE="$OPTARG"
      EATENARGS=`expr ${EATENARGS} + 2`
      ;;

    b)
      echo treating list as batch-files
      BATCHFILEMODE=1
      LOGTAG="-BATCH-FILE-"
      EATENARGS=`expr ${EATENARGS} + 1`
      ;;

    l)
      echo got log-dir ${OPTARG}
      LOGDIR="${OPTARG}"
      EATENARGS=`expr ${EATENARGS} + 2`
      ;;

    t)
      echo got log-tag ${OPTARG}
      LOGSUFFIX="${OPTARG}"
      EATENARGS=`expr ${EATENARGS} + 2`
      ;;

    \?)
      echo "Invalid option: -$opt" >&2
      printUsageAndExit
      ;;

    :)
      echo "Option -$opt requires an argument." >&2
      printUsageAndExit
      ;;
  esac
done

if [ -z "${TESTMODE}" ] ; then
    echo "Could not determine test-mode, -m missing?" >&2
    printUsageAndExit
fi

REMAININGARGS="${@:${EATENARGS}}"

if [ ${BATCHFILEMODE} -eq 1 ]; then
    for file in ${REMAININGARGS[@]}; do
        echo "Considering ${TESTMODE} file ${file}"
        if [ -f ${file} ]; then
            
            if [ -z "${TESTS}" ]; then
                PRESTRING=""
            else
                PRESTRING=" "
            fi
            TESTS="${PRESTRING}"$(cat ${file} | tr "\n" " ")
        else
            echo "in batch mode, ${file} should be a file" >&2
            printUsageAndExit
        fi
    done
else
    TESTS="${REMAININGARGS}"
fi

if [ -z "${TESTS}" ] ; then
    echo "No tests found" >&2
    printUsageAndExit
fi

if [ -z "${LOGSUFFIX}" ] ; then
    LOGSUFFIX=${TESTMODE}
fi

configLibPath

OUTFILE_PREFIX=${LOGDIR}/`date ${LOGFILE_NAME_TIMESTAMP_FORMAT}`${LOGTAG}${LOGSUFFIX}
LOGFILE=${OUTFILE_PREFIX}.log
REPORTFILE=${OUTFILE_PREFIX}.xls

echo TESTS are "${TESTS}"

COMMAND_LINE="python3 -B -m pytest ${MARKEROPTION} ${TESTS} --deviceMode=SIM --conf=${CONFFILE} -s -v -r Efsx --excelreport=${REPORTFILE} --verbose"

echo Prepared command-line
echo -e "Wrapper script invoked with:\n$0 $*\n\n" >> ${LOGFILE}
echo -e "Command Line:\n$COMMAND_LINE\n\n" | tee -a ${LOGFILE}

echo

${COMMAND_LINE} 2>&1 | tee -a ${LOGFILE}