#!/bin/sh

# Runs python script for creating tf-record files.
# Should be run under "tensorflow" virtualenv!

if [ "`which python`" != "/home/roman/.virtualenvs/tensorflow/bin/python" ]; then
  echo "Bad python! Run it under 'tensorflow' virtualenv!"
  exit 1
fi

[ ! -d "../data" ] && mkdir ../data
python create_tf_record.py --input_dir=input --output_dir=../data
