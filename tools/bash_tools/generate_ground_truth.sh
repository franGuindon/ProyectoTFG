PYTHON_BIN=python3
SCRIPT=ground_truth.py  
REF_VID=walking_tour_720p.mp4
PL_VID=lossy_walking_tour_720p.mov
#START_TIME=0

for ID_AND_START_TIME in $(python3 parse_readme_into_list.py)
do
  echo spliting ID_AND_START_TIME
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
