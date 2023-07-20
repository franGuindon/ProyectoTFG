#!/bin/sh

if [ 2 -ne $# ]
then
  echo "Usage: build_thesis.sh [THESIS_DIRECTORY] [OUTPUT_DIRECTORY]";
  exit 1;
fi

CURRENT_DIRECTORY=`pwd`
THESIS_DIRECTORY=$1
OUTPUT_DIRECTORY=$2

echo "bin/sh$ cd ${THESIS_DIRECTORY}"
cd ${THESIS_DIRECTORY}

echo "bin/sh$ make clean && make && make clean"
touch main.tex && make && make clean # 1. touch main.tex to activate make target
                                     # 2. make to build
                                     # 3. make clean to clean

echo "bin/sh$ cd ${CURRENT_DIRECTORY}"
cd ${CURRENT_DIRECTORY}

echo "bin/sh$ cp ${THESIS_DIRECTORY}/main.pdf ${OUTPUT_DIRECTORY}"
cp "${THESIS_DIRECTORY}/main.pdf" ${OUTPUT_DIRECTORY}
