import os
import unittest

import binbootstrapper
binbootstrapper.update_binaries(__file__, binbootstrapper.DLL_IMAGETOOLS)

import imageutils

import helputils


RESOURCE_DIR = os.path.join(os.path.dirname(__file__), 'res')


class TestImageTools(unittest.TestCase):
    def test_can_create_2d_bitmap(self):
        bmp = imageutils.create_2d(123, 456, 1, imageutils.PIXEL_FORMAT.B8G8R8A8_UNORM)

        self.assertEquals(bmp.type, imageutils.BITMAP_TYPE.BITMAP_2D)
        self.assertEquals(bmp.width, 123)
        self.assertEquals(bmp.height, 456)
        self.assertEquals(bmp.mip_count, 1)
        self.assertEquals(bmp.format, imageutils.PIXEL_FORMAT.B8G8R8A8_UNORM)

    def test_can_create_cube_bitmap(self):
        bmp = imageutils.create_cube(123, 1, imageutils.PIXEL_FORMAT.B8G8R8X8_UNORM)

        self.assertEquals(bmp.type, imageutils.BITMAP_TYPE.BITMAP_CUBE)
        self.assertEquals(bmp.width, 123)
        self.assertEquals(bmp.height, 123)
        self.assertEquals(bmp.mip_count, 1)
        self.assertEquals(bmp.format, imageutils.PIXEL_FORMAT.B8G8R8X8_UNORM)

    def test_can_create_volume_bitmap(self):
        bmp = imageutils.create_volume(123, 456, 8, 1, imageutils.PIXEL_FORMAT.A8_UNORM)

        self.assertEquals(bmp.type, imageutils.BITMAP_TYPE.BITMAP_3D)
        self.assertEquals(bmp.width, 123)
        self.assertEquals(bmp.height, 456)
        self.assertEquals(bmp.depth, 8)
        self.assertEquals(bmp.mip_count, 1)
        self.assertEquals(bmp.format, imageutils.PIXEL_FORMAT.A8_UNORM)

    def test_loading_nonexistent_file_raises(self):
        with self.assertRaises(RuntimeError):
            imageutils.load('iDontExist.png')

    def test_loading_unsupported_image_type_raises(self):
        with self.assertRaises(RuntimeError):
            imageutils.load(__file__)

    def test_can_load_images(self):
        for filename in helputils.get_image_paths():
            helputils.load_image(filename)

    def test_can_get_image_data_as_string(self):
        bmp = helputils.load_image('uncompressed/bgr8.png')
        self.assertEquals(len(bmp.get_pixel_data()), bmp.width * bmp.height * 4)

    def test_can_create_image_from_string(self):
        pixel_data = '\xab\x12\x13\x15\xcd\x22\x23\x25'
        bmp = imageutils.create_2d_from_string(
            1, 2, 1, imageutils.PIXEL_FORMAT.B8G8R8A8_UNORM, pixel_data)
        self.assertEquals(bmp.type, imageutils.BITMAP_TYPE.BITMAP_2D)
        self.assertEquals(bmp.width, 1)
        self.assertEquals(bmp.height, 2)
        self.assertEquals(bmp.mip_count, 1)
        self.assertEquals(bmp.format, imageutils.PIXEL_FORMAT.B8G8R8A8_UNORM)
        self.assertEquals(bmp.get_pixel_data(), pixel_data)

    def test_get_dds_size(self):
        path = helputils.get_image_path('uncompressed/bgra8_npot.dds')
        self.assertEquals(imageutils.get_dds_size(path), (24, 32))