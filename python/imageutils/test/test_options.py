__author__ = 'filipp'

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


class TestCompressionOptions(unittest.TestCase):
    def test_can_create_compression_options(self):
        opt = imageutils.CompressionOptions()
        self.assertEquals(opt.format, imageutils.PIXEL_FORMAT.BC1_UNORM)
        self.assertEquals(opt.alpha_for_bc1, False)
        self.assertEquals(opt.quality, imageutils.COMPRESSION_QUALITY.PRODUCTION)

    def test_can_create_compression_options_with_format(self):
        opt = imageutils.CompressionOptions(imageutils.PIXEL_FORMAT.BC3_UNORM)
        self.assertEquals(opt.format, imageutils.PIXEL_FORMAT.BC3_UNORM)
        self.assertEquals(opt.alpha_for_bc1, False)
        self.assertEquals(opt.quality, imageutils.COMPRESSION_QUALITY.PRODUCTION)

    def test_can_create_compression_options_with_format_and_quality(self):
        opt = imageutils.CompressionOptions(imageutils.PIXEL_FORMAT.BC3_UNORM,
                                            imageutils.COMPRESSION_QUALITY.HIGHEST)
        self.assertEquals(opt.format, imageutils.PIXEL_FORMAT.BC3_UNORM)
        self.assertEquals(opt.alpha_for_bc1, False)
        self.assertEquals(opt.quality, imageutils.COMPRESSION_QUALITY.HIGHEST)

    def test_can_create_compression_options_with_format_quality_and_mips(self):
        opt = imageutils.CompressionOptions(imageutils.PIXEL_FORMAT.BC1_UNORM,
                                            imageutils.COMPRESSION_QUALITY.FASTEST, True)
        self.assertEquals(opt.format, imageutils.PIXEL_FORMAT.BC1_UNORM)
        self.assertEquals(opt.quality, imageutils.COMPRESSION_QUALITY.FASTEST)
        self.assertEquals(opt.generate_mips, True)

    def test_assigning_uncompressed_format_to_compression_format_raises(self):
        opt = imageutils.CompressionOptions()
        with self.assertRaises(RuntimeError):
            opt.format = imageutils.PIXEL_FORMAT.A8_UNORM
        with self.assertRaises(RuntimeError):
            opt.format = imageutils.PIXEL_FORMAT.B8G8R8A8_UNORM

    def test_can_assign_compressed_format_to_compression_format(self):
        opt = imageutils.CompressionOptions()
        opt.format = imageutils.PIXEL_FORMAT.BC1_UNORM
        self.assertEquals(opt.format, imageutils.PIXEL_FORMAT.BC1_UNORM)

