
if [[ 3 -ne $# ]]; then
  echo "Usage: train_with_snippets.sh [TRAIN_BIN] [DATASET_DIR] [OUTPUT_DIR]"
  exit 1
fi

TRAIN_BIN=$1
DATASET_DIR=$2
OUTPUT_DIR=$3

for VID in {0..0..1} # {START..END..STEP}
do
  if [[ ! -d $DATASET_DIR/$VID ]]; then
    echo "Did not find $DATASET_DIR/$VID"
    continue
  fi

  for SNIP in {00..18..1}
  do
    if [[ ! -d $DATASET_DIR/$VID/$SNIP ]]; then
      echo "Did not find $DATASET_DIR/$VID/$SNIP"
      continue
    fi

    FEATURE_FILE="$DATASET_DIR/$VID/$SNIP/features.bytes"
    LABEL_FILE="$DATASET_DIR/$VID/$SNIP/block.bytes"
    OUT_PREFIX="$OUTPUT_DIR/vid${VID}_snip${SNIP}_ntree20_maxdepth100_unbalanced"

    CMD="$TRAIN_BIN $FEATURE_FILE $LABEL_FILE $OUT_PREFIX"
    echo $CMD
    $CMD
  done
done
