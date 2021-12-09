#!/bin/bash

FALSED_DIR="cropped_inputs/falsed"
PROCESS_FILE="images_list.no_dups_and_backslashes_and_falsed.txt"

cat images_list.no_dups_and_backslashes.txt > $PROCESS_FILE

for falsefile in `ls $FALSED_DIR`; do
  # find string like:
  #   cropped_inputs/screenshots_20170424/bottom/bottom_scr_20170424_20-00-01.png 41 88 322 234
  # falsefile here is like:
  #   bottom_scr_20170514_11-52-38_10_80_269_121.png

  name=`echo $falsefile | awk -F. '{print $1}'`

  if [[ $name == top_vertical_* ]]; then
    findname=`echo $name | awk -F_ '{print $1 "_" $2 "_" $3 "_" $4 "_" $5 ".png"}'`
    xmin=`echo $name | awk -F_ '{print $6}'`
    ymin=`echo $name | awk -F_ '{print $7}'`
    width=`echo $name | awk -F_ '{print $8}'`
    height=`echo $name | awk -F_ '{print $9}'`
  else
    findname=`echo $name | awk -F_ '{print $1 "_" $2 "_" $3 "_" $4 ".png"}'`
    xmin=`echo $name | awk -F_ '{print $5}'`
    ymin=`echo $name | awk -F_ '{print $6}'`
    width=`echo $name | awk -F_ '{print $7}'`
    height=`echo $name | awk -F_ '{print $8}'`
  fi

  xmax=$(($xmin+$width))
  ymax=$(($ymin+$height))

  findstr="$findname $xmin $ymin $xmax $ymax"

  #echo "looking for ${findstr}"
  cat $PROCESS_FILE | sed "/${findstr}$/!d"

  sed -i "/${findstr}$/d" $PROCESS_FILE

done
