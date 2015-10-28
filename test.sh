#!/bin/bash

HERE=$(cd `dirname $0`; pwd)

failed=0
for f in `find ${HERE}/tests -not -name common.sh -type f`; do
    echo -n `basename $f`...
    $f
    failed=$(($failed + $?))
done
exit $failed
