BASE=$(cd `dirname $0`/..; pwd)
CRTIME=${BASE}/crtime

# The TEST_DIR must be in the filesystem under test. For now, we just assume
# that the filesystem under test is right here where the source is checked out.
# But we let the caller override it in case they need to
[[ $TEST_DIR ]] || TEST_DIR=${BASE}/.test

function fail {
    echo "FAIL: $*"
    exit 1
}

function success {
    echo "SUCCESS"
    exit 0
}

function clear_test_dir {
    rm -rf ${TEST_DIR}
    mkdir -p ${TEST_DIR}
}

function _sudo {
    MSG="Make a safe place and set USE_SUDO_FOR_TESTS to test for reals"
    [ "$USE_SUDO_FOR_TESTS" = "" ] && fail $MSG
    sudo $*
}

function setuid {
    _sudo chown root ${CRTIME}
    _sudo chmod u+s ${CRTIME}
}

function unsetuid {
    _sudo chmod u-s ${CRTIME}
    _sudo chown $USER ${CRTIME}
}

clear_test_dir
unsetuid
