#!/bin/sh

rm -f samples.vec

opencv_createsamples -info `pwd`/good.dat -num 2677 -vec samples.vec -w 48 -h 27
