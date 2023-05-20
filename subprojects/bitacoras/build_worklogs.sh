#!/bin/sh

if [ 3 -ne $# ]
then
  echo "Usage: build_worklogs.sh [WORKLOGS_DIRECTORY] [OUTPUT_DIRECTORY] [TARGET]";
  exit 1;
fi

CURRENT_DIRECTORY=`pwd`
WORKLOGS_DIRECTORY=$1
OUTPUT_DIRECTORY=$2
TGT=$3
TGT_OUT=${TGT}.pdf

echo "bin/sh$ cd ${WORKLOGS_DIRECTORY}"
cd ${WORKLOGS_DIRECTORY}

echo "bin/sh$ export MAINFILE=${TGT}"
export MAINFILE=${TGT}

echo "bin/sh$ make clean && make semana_ && make clean"
make clean && make && make clean

echo "bin/sh$ cd ${CURRENT_DIRECTORY}"
cd ${CURRENT_DIRECTORY}

if [ ! -d ${WORKLOGS_DIRECTORY}/${TGT_OUT} ]
then
  echo "Target '${TGT}' construction failed"
  exit 1;
fi

echo "bin/sh$ cp ${WORKLOGS_DIRECTORY}/${TGT_OUT} ${OUTPUT_DIRECTORY}"
cp "${WORKLOGS_DIRECTORY}/${TGT_OUT}" ${OUTPUT_DIRECTORY}
