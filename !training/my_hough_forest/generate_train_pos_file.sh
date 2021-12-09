#!/bin/sh

TRAIN_FILE=train_pos.txt

> "$TRAIN_FILE"
for file in `find trainimages/pos -name "*.*"`; do
  echo $file...
  IDENTIFY_STRING="`identify -format '%f 0 0 %w %h' $file`"
  IMG_WIDTH=`echo $IDENTIFY_STRING | awk '{print $4}'`
  IMG_HEIGHT=`echo $IDENTIFY_STRING | awk '{print $5}'`
  IDENTIFY_STRING="$IDENTIFY_STRING $(($IMG_WIDTH/2)) $(($IMG_HEIGHT/2))"
  echo "$IDENTIFY_STRING" >> "$TRAIN_FILE"
done
echo "done"
