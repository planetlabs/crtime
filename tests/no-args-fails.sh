HERE=$(cd `dirname $0`; pwd)
. ${HERE}/common.sh

${CRTIME} 2> /dev/null && fail 'expected crtime with no args to fail'

success
