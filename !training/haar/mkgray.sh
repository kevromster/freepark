#!/bin/sh

INDIR=$1
OUTDIR=grayed

if [ -z "$INDIR" ]; then
  echo "usage: mkgray.sh <dir_with_images>"
  exit 1
fi

rm -rf $OUTDIR
mkdir $OUTDIR

for file in `find $INDIR -iname "*.*"`; do

  echo converting $file...
  convert $file -colorspace gray $OUTDIR/`basename $file`

done
