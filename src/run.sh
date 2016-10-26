#!/bin/bash

SCRIPTPATH=$(readlink -f ${BASH_SOURCE})
BASEDIR=$(dirname $(dirname ${SCRIPTPATH}))

export LD_LIBRARY_PATH=${BASEDIR}/lib:${LD_LIBRARY_PATH}

root -l -b $*
