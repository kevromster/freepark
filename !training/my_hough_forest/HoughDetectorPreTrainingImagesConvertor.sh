#!/bin/sh

OUTDIR="$1"
if [ -z "$OUTDIR" ]; then
  echo "Usage: $0 <outdir name>"
  exit 1
fi

rm -rf $OUTDIR
mkdir $OUTDIR

for INDIR in `ls | grep ^screenshots`; do
  for level in `echo "bottom middle top_vertical"`; do
    for file in `find $INDIR/clipped/$level/neg/ -iname "*.png"`; do
#    for file in `find $INDIR/clipped/$level/ -iname "*.png" | grep -v "neg"`; do
      echo converting $file
      OPTS="-colorspace gray"
      if [ $level = "top_vertical" ]; then
        OPTS="$OPTS -rotate -90"
      fi
      #OPTS="$OPTS -resize 164x91!"
      OPTS="$OPTS -resize 164x82!"
      convert "$file" $OPTS $OUTDIR/`basename $file`
    done
  done
done
