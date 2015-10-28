
HERE=$(cd `dirname $0`; pwd)
. ${HERE}/common.sh

NOW=`date -u +%s`

TARGET=${TEST_DIR}/foo

touch ${TARGET}
INODE=$(ls -di "${TARGET}" | cut -d ' ' -f 1)
FS=$(df "${TARGET}"  | tail -1 | awk '{print $1}');
setuid
CMD="${CRTIME} ${FS} ${INODE}"
TIME=`${CMD}` || fail "${CMD} failed"
DIFF=$(($TIME-$NOW))
[ ${DIFF} -eq 0 -o ${DIFF} -eq 1 ] || fail 'Creation time was too different from now'

success
