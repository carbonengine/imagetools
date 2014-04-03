__author__ = 'filipp'
import os
import imageutils

RESOURCE_DIR = os.path.join(os.path.dirname(__file__), 'res')


def load_test_image(name):
    return imageutils.load(os.path.join(RESOURCE_DIR, name))


def get_test_image_paths(subdir=''):
    for root, _, files in os.walk(os.path.join(RESOURCE_DIR, subdir)):
        for name in files:
            yield os.path.join(root, name)
