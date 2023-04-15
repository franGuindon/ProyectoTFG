#-------------------------------------------------------------------------------
# Arg parse
if [[ 4 -ne $# ]]; then
  echo
  echo "Usage: generate_ground_truth.sh [REFERENCE_VIDEO] [LOSSY_VIDEO] [SNIP_SPECIFICATION] [OUTPUT_DIR]"
  echo "  REFERENCE_VIDEO    : H264 encoded 720p video"
  echo "  LOSSY_VIDEO        : Corresponing lossy video to REFERENCE_VIDEO"
  echo "  SNIP_SPECIFICATION : Text file specifying how to snip video"
  echo "  OUTPUT_DIR         : Directory where a subdirectory for each snip will be created,"
  echo "                       for each snip subdirectory, several files will be created"
  echo
  exit 1
fi

#-------------------------------------------------------------------------------
# Constants
RED="\x1b[31m"
GREEN="\x1b[32m"
RESET="\x1b[0m"

BASH_TOOLS_PATH="`dirname $0`/"
TOOLS_PATH="$BASH_TOOLS_PATH/../"
PYTHON_TOOLS_PATH="$TOOLS_PATH/python_tools/"

PYTHON_BIN="python3"
GT_SCRIPT="$PYTHON_TOOLS_PATH/ground_truth.py"
REF_VID=$1
PL_VID=$2

SNIP_SPEC=$3
SNIP_SPEC_PARSER="$PYTHON_TOOLS_PATH/parse_snip_spec.py"

OUT_DIR=$4

#-------------------------------------------------------------------------------
# Parse snip spec
PARSED_SNIP_SPEC=`$PYTHON_BIN $SNIP_SPEC_PARSER $SNIP_SPEC`
if [[ 0 -ne $? ]]; then
  printf $RED"Snip parser script returned error"$RESET
  echo $PARSED_SNIP_SPEC
  exit 1
fi

#-------------------------------------------------------------------------------
# Generate ground truths
for SNIP_ID_AND_START_TIME in $PARSED_SNIP_SPEC
do
  SNIP_ID=$(echo $SNIP_ID_AND_START_TIME | cut -d ":" -f 1)
  SNIP_START_TIME=$(echo $SNIP_ID_AND_START_TIME | cut -d ":" -f 2)
  echo "ID $SNIP_ID START_TIME $SNIP_START_TIME"

  SNIP_DIR="$OUT_DIR/$SNIP_ID"

  if [[ -d $SNIP_DIR ]]; then
    echo "$SNIP_DIR exists"
  else
    echo "making $SNIP_DIR directory"
    echo "mkdir $SNIP_DIR"  
    mkdir $SNIP_DIR
    CMD="$PYTHON_BIN $GT_SCRIPT $REF_VID $PL_VID $SNIP_DIR $SNIP_START_TIME"
    echo $CMD
    $CMD
  fi
  echo
done
