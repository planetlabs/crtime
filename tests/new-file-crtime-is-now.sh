
HERE=$(cd `dirname $0`; pwd)
. ${HERE}/common.sh

NOW=`date -u +%s`

TARGET=${TEST_DIR}/foo

touch ${TARGET}
FS=$(df "${TARGET}"  | tail -1 | awk '{print $1}');
setuid
CMD="${CRTIME} ${FS} ${TARGET}"
TIME=`${CMD}` || fail "${CMD} failed"
DIFF=$(($TIME-$NOW))
[ ${DIFF} -eq 0 -o ${DIFF} -eq 1 ] || fail 'Creation time was too different from now'

success
