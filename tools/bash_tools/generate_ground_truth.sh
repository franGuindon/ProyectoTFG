if [[ 3 -ne $# ]]; then
  echo
  echo "Usage: generate_ground_truth.sh [REFERENCE_VIDEO] [SNIP_SPECIFICATION] [OUTPUT_DIR]"
  echo "  REFERENCE_VIDEO    : H264 encoded 720p video"
  echo "  SNIP_SPECIFICATION : Text file specifying how to snip encoded video"
  echo "  OUTPUT_DIR         : Directory where a subdirectory for each snip will be created,"
  echo "                       for each snip subdirectory, several files will be created"
  echo
  exit 1
fi
BASH_TOOLS_PATH=`dirname $0`/
TOOLS_PATH=$BASH_TOOLS_PATH/../
PYTHON_TOOLS_PATH=$TOOLS_PATH/python_tools/

PYTHON_BIN=python3
GT_SCRIPT=$PYTHON_TOOLS_PATH/ground_truth.py
REF_VID=$1
# PL_VID=lossy_walking_tour_720p.mov
SNIP_SPEC_PARSER="$PYTHON_TOOLS_PATH/parse_readme_into_list.py"

for ID_AND_START_TIME in `$PYTHON_BIN $SNIP_SPEC_PARSER`
do
  echo spliting ID_AND_START_TIME
  continue
  ID=$(echo $ID_AND_START_TIME | cut -d ":" -f 1)
  START_TIME=$(echo $ID_AND_START_TIME | cut -d ":" -f 2)
  echo ID $ID
  echo START_TIME $START_TIME
  if [[ -d "$ID" ]]; then
    echo $ID exists
  else
    echo making "$ID" directory
    echo mkdir "$ID"
    mkdir "$ID"
    echo $PYTHON_BIN $SCRIPT $REF_VID $PL_VID $ID $START_TIME
    $PYTHON_BIN $SCRIPT $REF_VID $PL_VID $ID $START_TIME
  fi
done
