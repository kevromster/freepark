#!/bin/sh

> positives.dat
for file in `find cars_top_view/grayed -name "*.*"`; do
  echo $file...
  identify -format '%i 1 0 0 %w %h' $file >> positives.dat
  echo >> positives.dat
done
echo "done"
