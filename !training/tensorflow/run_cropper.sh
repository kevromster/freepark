#!/bin/sh

# Runs TrainImageCropper to extract parking images from original screenshots.
# Adapted for NB5 camera screenshots processing.

BASE_DIR="/mnt/bigstore/xproject/TensorFlowTraining/Msk-Dolgoprudnyi-NB5-201707"
SCREENSHOTS_DIR="$BASE_DIR/screenshots"
CROPPER_DIR="$BASE_DIR/cropped_inputs"
CONFIG_FILE="msk_dolgoprudny_nb5.txt"

rm -rf "$CROPPER_DIR"
mkdir "$CROPPER_DIR"

for INDIR in `ls $SCREENSHOTS_DIR | grep ^screenshots`; do
  CROPPER_OUT_DIR="$CROPPER_DIR/$INDIR"
  mkdir "$CROPPER_OUT_DIR"

  for file in `ls $SCREENSHOTS_DIR/$INDIR/*.png`; do
    echo "$file"
    ./TrainImageCropper "$CONFIG_FILE" "$file" "$CROPPER_OUT_DIR"
  done

  for prefix in `cat "$CONFIG_FILE" | awk '/ = / {print $1}'`; do
    mkdir "$CROPPER_OUT_DIR/$prefix"
    mv "$CROPPER_OUT_DIR/$prefix"_*png "$CROPPER_OUT_DIR/$prefix/"
  done
done
echo "done"
