if [[ $# -ne 3 ]]; then
  echo "Usage generate_feature_files.sh [BLACK_WHITE_BIN] [DATASET_DIR] [OUTFILE_PREFIX]";
  exit 1;
fi

BIN=$1
DATASET_DIR=$2
OUTFILE_PREFIX=$3

for VID_DIR in $DATASET_DIR/[0-9]
do for SNIP_DIR in $VID_DIR/[0-9][0-9]
  do
    LABEL_FILE=$SNIP_DIR/block.bytes
    OUTPUT_FILE=$SNIP_DIR/$OUTFILE_PREFIX.bytes
    echo $BIN $LABEL_FILE $OUTPUT_FILE
    $BIN $LABEL_FILE $OUTPUT_FILE
  done
done
