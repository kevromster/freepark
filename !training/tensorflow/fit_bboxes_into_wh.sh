#!/bin/bash

PROCESS_FILE="images_list.no_dups_and_backslashes_and_falsed.txt"
PROCESSED_FILE="images_list.no_dups_and_backslashes_and_falsed_and_fit_bbox.txt"

gawk '{

  filename=$1;
  xmin=$2;
  ymin=$3;
  xmax=$4;
  ymax=$5;

  if (xmin < 0)
    xmin=0;
  if (ymin < 0)
    ymin=0;

  realw=0;
  cmd="identify -format \"%w\" " filename;
  while ( ( cmd | getline result ) > 0 ) {
    realw=result;
  }
  close(cmd);

  realh=0;
  cmd="identify -format \"%h\" " filename;
  while ( ( cmd | getline result ) > 0 ) {
    realh=result;
  }
  close(cmd);

  w=xmax-xmin;
  h=ymax-ymin;

  if (w > realw)
    w=realw;
  if (h > realh)
    h=realh;

  print filename " " xmin " " ymin " " xmax " " ymax
}' $PROCESS_FILE > $PROCESSED_FILE
