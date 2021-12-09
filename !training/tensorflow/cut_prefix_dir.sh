#!/bin/sh

cat images_list.no_dups_and_backslashes_and_falsed_and_fit_bbox.txt | awk -F/ '{print $NF}' > images_list.no_dups_and_backslashes_and_falsed_and_fit_bbox_and_prefix.txt
