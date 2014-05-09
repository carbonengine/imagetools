__author__ = 'filipp'

import itertools
import os
import unittest

import binbootstrapper
binbootstrapper.update_binaries(__file__, binbootstrapper.DLL_IMAGETOOLS)

import imageutils


RESOURCE_DIR = os.path.join(os.path.dirname(__file__), 'res')


def load_image(name):
    return imageutils.load(get_image_path(name))


def get_image_path(name):
    return os.path.join(RESOURCE_DIR, name)


def get_image_paths(subdir=''):
    for root, _, files in os.walk(os.path.join(RESOURCE_DIR, subdir)):
        for name in files:
            yield os.path.join(root, name)


def is_compressed_image(filename):
    return os.path.basename(filename).lower().startswith('bc')


class TestCompression(unittest.TestCase):
    def _check_compression(self, input_image, compression_options):
        out = input_image.compress(compression_options)
        self.assertEquals(out.format, compression_options.format)
        self.assertEquals(out.type, input_image.type)
        self.assertEquals(out.width, input_image.width)
        self.assertEquals(out.height, input_image.height)
        self.assertEquals(out.mip_count, input_image.mip_count)
        return out

    def _check_compression_for_directory(self, compression_options):
        for name in itertools.ifilterfalse(is_compressed_image, get_image_paths()):
            bmp = load_image(name)
            if bmp.format == imageutils.PIXEL_FORMAT.R8_UNORM:
                bmp.convert_format(imageutils.PIXEL_FORMAT.B8G8R8A8_UNORM)
            self._check_compression(bmp, compression_options)

    def test_compress_bc1(self):
        opt = imageutils.CompressionOptions(imageutils.PIXEL_FORMAT.BC1_UNORM)
        self._check_compression_for_directory(opt)

    def test_compress_bc2(self):
        opt = imageutils.CompressionOptions(imageutils.PIXEL_FORMAT.BC2_UNORM)
        self._check_compression_for_directory(opt)

    def test_compress_bc3(self):
        opt = imageutils.CompressionOptions(imageutils.PIXEL_FORMAT.BC3_UNORM)
        self._check_compression_for_directory(opt)
