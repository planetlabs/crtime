
HERE=$(cd `dirname $0`; pwd)
. ${HERE}/common.sh

TARGET=${TEST_DIR}/foo
touch ${TARGET}
chmod 220 ${TARGET}

setuid
CMD="${CRTIME} ${TARGET}"
TIME=`${CMD} 2>/dev/null` && fail "Expected failure for file w/o read permission"
success
