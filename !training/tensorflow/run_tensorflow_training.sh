#!/bin/sh

# Should be run from the tensorflow/models/ directory and under "tensorflow" virtualenv.

PIPELINE_CONFIG="/home/roman/work/xproject/tensorflow/my_training/20170729/data/faster_rcnn_restnet101_cars_top_view.config"
TRAIN_DIR="/home/roman/work/xproject/tensorflow/my_training/20170729/train"
EVAL_DIR="/home/roman/work/xproject/tensorflow/my_training/20170729/eval"

if [ "`pwd`" != "/home/roman/work/xproject/tensorflow/models" ]; then
  echo "Wrong current directory or with script!"
  pwd
  exit 1
fi

if [ "`which python`" != "/home/roman/.virtualenvs/tensorflow/bin/python" ]; then
  echo "Bad python! Run it under 'tensorflow' virtualenv!"
  which python
  exit 1
fi

if [ "$1" = "--train" ]; then
  echo "Run training..."
  python object_detection/train.py \
      --logtostderr \
      --pipeline_config_path="${PIPELINE_CONFIG}" \
      --train_dir="${TRAIN_DIR}" 2>&1 | tee train.log

elif [ "$1" = "--eval" ]; then
  echo "Run evaluating..."
  python object_detection/eval.py \
      --logtostderr \
      --pipeline_config_path="${PIPELINE_CONFIG}" \
      --checkpoint_dir="${TRAIN_DIR}" \
      --eval_dir="${EVAL_DIR}" 2>&1 | tee eval.log
elif [ "$1" = "--export" -a "$2" != "" ]; then
  echo "Export checkpoint $2..."
  CHECKPOINT_NUMBER=$2
  python object_detection/export_inference_graph.py \
      --input_type image_tensor \
      --pipeline_config_path "${PIPELINE_CONFIG}" \
      --checkpoint_path "${TRAIN_DIR}/model.ckpt-${CHECKPOINT_NUMBER}" \
      --inference_graph_path output_cars_top_view_inference_graph${CHECKPOINT_NUMBER}.pb
else
  echo "Usage: run_mycar_training.sh (--train|--eval|--export <checkpoint_number>)"
  exit 1
fi

echo "Finished"
exit 0
