#!/bin/sh

cat images_list.txt | awk '!($0 in a) {a[$0];print}'
