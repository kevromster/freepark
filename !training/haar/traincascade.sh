#!/bin/sh

HAAR_DIR="haarcascade"
[ -d "$HAAR_DIR" ] && rm -rf "$HAAR_DIR"
mkdir "$HAAR_DIR"

#numPos: 2140 (80% from whole count)
echo "start at:" `date`
opencv_traincascade -data "$HAAR_DIR" -vec samples.vec -bg `pwd`/bad.dat -numStages 20 -minhitrate 0.99 -maxFalseAlarmRate 0.5 -numPos 2140 -numNeg 1814 -w 48 -h 27 -mode ALL -precalcValBufSize 2048 -precalcIdxBufSize 2048
echo "finished at:" `date`
