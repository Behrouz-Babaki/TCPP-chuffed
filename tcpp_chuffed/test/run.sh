#!/bin/bash

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DATADIR=$SCRIPTDIR/data
OUTDIR=$SCRIPTDIR/output

if [ $# -eq 0 ]; then
    pid="01"
else
    pid=$1
fi

cd $SCRIPTDIR/..

./tcpp                                  \
        $DATADIR/domain.pddl            \
        $DATADIR/p${pid}.pddl           \
        -varorder conflict              \
        -preprocess SAC                 \
        -prop-node SAC                  \
        $OUTDIR/out${pid}               \
        | tee $OUTDIR/stdout${pid}.txt

rm output.sas