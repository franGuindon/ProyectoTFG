if [[ $# -ne 3 ]]; then
  echo "Usage generate_feature_files.sh [FEATURE_GENERATOR_BIN] [DATASET_DIR] [OUTFILE_PREFIX]";
  exit 1;
fi

BIN=$1
DATASET_DIR=$2
OUTFILE_PREFIX=$3

for VID_DIR in $DATASET_DIR/[0-9]
do for SNIP_DIR in $VID_DIR/[0-9][0-9]
  do
    LOSSY_VID_FILE=$SNIP_DIR/pl.mp4
    LABEL_FILE=$SNIP_DIR/block.bytes
    OUTPUT_FILE=$SNIP_DIR/$OUTFILE_PREFIX.bytes
    echo $BIN $LOSSY_VID_FILE $LABEL_FILE $OUTPUT_FILE
    $BIN $LOSSY_VID_FILE $LABEL_FILE $OUTPUT_FILE
  done
done
