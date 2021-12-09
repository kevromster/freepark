"""
  Creates tf-record file from input images for TensorFlow training.
  Currently we work with .png files and with 'Car' label only.

  Assumed structure of input data:
  input_dir/
    *--images/ - training images are located here
    *--images_list.txt - list of training images
    *--mscoco_label_map.pbtxt - labels file

  The script assumes that input images_list.txt file contains records in the following format:
    filename xmin ymin xmax ymax

  There can be many lines for one image file with different bounding boxes described.
  The script collects them info one piece of information about the specified image.
"""

import hashlib
import io
import logging
import os
import PIL.Image
import random
import tensorflow as tf

from object_detection.utils import dataset_util
from object_detection.utils import label_map_util


logging.basicConfig(level=logging.DEBUG)

flags = tf.app.flags
flags.DEFINE_string('input_dir', '', 'Path to input training images')
flags.DEFINE_string('output_dir', '', 'Path to output TFRecord')
FLAGS = flags.FLAGS

IMAGES_SUBDIR = 'images'
IMAGES_LIST_FILE = 'images_list.txt'
LABELS_FILE = 'mscoco_label_map.pbtxt'
OUTPUT_TRAIN_FILE = 'cars_top_view_train.record'
OUTPUT_VALIDATE_FILE = 'cars_top_view_validate.record'


class BBox:
    def __init__(self, xmin, ymin, xmax, ymax):
        self.xmin = xmin
        self.xmax = xmax
        self.ymin = ymin
        self.ymax = ymax


class ImageInfo:
    def __init__(self, filename):
        self.filename = filename
        self.bboxes = []

    def add_bbox(self, xmin, ymin, xmax, ymax):
        self.bboxes.append(BBox(xmin=xmin, ymin=ymin, xmax=xmax, ymax=ymax))


def create_tf(image_info, image_dir, labels_dictionary):
    image_path = os.path.join(image_dir, image_info.filename)
    with tf.gfile.GFile(image_path, 'rb') as fid:
        encoded_png = fid.read()

    image = PIL.Image.open(io.BytesIO(encoded_png))

    if image.format != 'PNG':
        raise ValueError('Image format not png! but: ' + image.format)

    key = hashlib.sha256(encoded_png).hexdigest()
    width = image.width
    height = image.height

    xmins = []
    xmaxs = []
    ymins = []
    ymaxs = []

    classes_text = []
    classes = []

    class_name = '/m/0k4j'  # display_name = 'car' in labels file
    class_name_encoded = class_name.encode('utf8')
    class_id = labels_dictionary[class_name]

    for bbox in image_info.bboxes:
        xmins.append(max(0, float(bbox.xmin) / width))
        xmaxs.append(min(1, float(bbox.xmax) / width))
        ymins.append(max(0, float(bbox.ymin) / height))
        ymaxs.append(min(1, float(bbox.ymax) / height))

        classes_text.append(class_name_encoded)
        classes.append(class_id)

    logging.info('add image %s with bboxes count %d' % (image_info.filename, len(image_info.bboxes)))

    tf_record = tf.train.Example(features=tf.train.Features(feature={
        'image/height': dataset_util.int64_feature(height),
        'image/width': dataset_util.int64_feature(width),

        'image/filename': dataset_util.bytes_feature(image_info.filename.encode('utf8')),
        'image/source_id': dataset_util.bytes_feature(image_info.filename.encode('utf8')),

        'image/key/sha256': dataset_util.bytes_feature(key.encode('utf8')),
        'image/encoded': dataset_util.bytes_feature(encoded_png),
        'image/format': dataset_util.bytes_feature('png'.encode('utf8')),

        'image/object/bbox/xmin': dataset_util.float_list_feature(xmins),
        'image/object/bbox/xmax': dataset_util.float_list_feature(xmaxs),
        'image/object/bbox/ymin': dataset_util.float_list_feature(ymins),
        'image/object/bbox/ymax': dataset_util.float_list_feature(ymaxs),

        'image/object/class/text': dataset_util.bytes_list_feature(classes_text),
        'image/object/class/label': dataset_util.int64_list_feature(classes),
    }))

    return tf_record


def write_tf(output_file, image_infos, image_dir, labels_dictionary):
    writer = tf.python_io.TFRecordWriter(output_file)

    for info in image_infos:
        tf_record = create_tf(info, image_dir, labels_dictionary)
        writer.write(tf_record.SerializeToString())

    writer.close()


def read_images_list(path):
    with tf.gfile.GFile(path) as fid:
        lines = fid.readlines()

    # map in format {filename:ImageInfo}
    image_infos = {}

    for line in lines:
        splitted_line = line.strip().split(' ')
        if len(splitted_line) != 5:
            logging.warning("bad string in images_list! skipping")
            continue

        name = splitted_line[0]
        xmin = splitted_line[1]
        ymin = splitted_line[2]
        xmax = splitted_line[3]
        ymax = splitted_line[4]

        info = image_infos.setdefault(name, ImageInfo(filename=name))
        info.add_bbox(xmin=xmin, ymin=ymin, xmax=xmax, ymax=ymax)

    return list(image_infos.values())


def main(_):
    input_dir = FLAGS.input_dir
    image_dir = os.path.join(input_dir, IMAGES_SUBDIR)
    image_list_file = os.path.join(input_dir, IMAGES_LIST_FILE)
    labels_path = os.path.join(input_dir, LABELS_FILE)
    train_output_path = os.path.join(FLAGS.output_dir, OUTPUT_TRAIN_FILE)
    validate_output_path = os.path.join(FLAGS.output_dir, OUTPUT_VALIDATE_FILE)

    logging.info('Reading input data from directory \'%s\'...' % input_dir)
    labels_dictionary = label_map_util.get_label_map_dict(labels_path)
    image_infos = read_images_list(image_list_file)

    logging.info("count of training images: %d" % len(image_infos))

    if len(image_infos) <= 0:
        logging.error("Bad images_list data! break")
        return

    # perform split for training and validation images
    random.seed(42)
    random.shuffle(image_infos)
    num_images = len(image_infos)
    num_train = int(0.9 * num_images)
    train_images = image_infos[:num_train]
    validate_images = image_infos[num_train:]
    logging.info('%d training and %d validation examples.', len(train_images), len(validate_images))

    logging.info('Creating tfs...')
    write_tf(train_output_path, train_images, image_dir, labels_dictionary)
    write_tf(validate_output_path, validate_images, image_dir, labels_dictionary)

    logging.info('Finished')


if __name__ == '__main__':
    tf.app.run()
