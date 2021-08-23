import os
from PIL import Image
import unittest
import imageutils

import helputils


RESOURCE_DIR = os.path.join(os.path.dirname(__file__), 'res')


def _is_file_supported_by_pil(filename):
    return os.path.splitext(filename)[1].lower() not in ['.jpg', '.psd']


def load_pil_image(filename):
    if not _is_file_supported_by_pil(filename):
        raise IOError("unsupported PIL format")
    img = Image.open(helputils.get_image_path(filename))
    img.load()
    return img


class TestPilInterface(unittest.TestCase):
    def test_image_created_from_pil_is_the_same_as_loaded_image(self):
        for filename in helputils.get_image_paths('uncompressed'):
            try:
                img = load_pil_image(filename)
            except IOError:
                continue
            bmp1 = helputils.load_image(filename)
            bmp2 = imageutils.from_pil(img)
            self.assertEquals(bmp1.format, bmp2.format)
            self.assertEquals(bmp1.width, bmp2.width)
            self.assertEquals(bmp1.height, bmp2.height)
            self.assertEquals(bmp1.get_pixel_data(), bmp2.get_pixel_data())

    def test_pil_image_created_from_imageutils_is_the_same_as_loaded_image(self):
        for filename in helputils.get_image_paths('uncompressed'):
            try:
                bmp1 = load_pil_image(filename)
            except IOError:
                continue
            bmp2 = imageutils.to_pil(helputils.load_image(filename))
            self.assertEquals(bmp1.mode, bmp2.mode)
            self.assertEquals(bmp1.size, bmp2.size)
            self.assertEquals(bmp1.tostring(), bmp2.tostring())

    def _decompress(self, image):
        uncompressed_format = imageutils.PIXEL_FORMAT.B8G8R8A8_UNORM
        if image.format == imageutils.PIXEL_FORMAT.BC1_UNORM:
            uncompressed_format = imageutils.PIXEL_FORMAT.B8G8R8X8_UNORM
        image.convert_format(uncompressed_format)

    def test_pil_image_created_from_compressed_is_the_same_as_loaded_image(self):
        for filename in helputils.get_image_paths('compressed'):
            bmp1 = helputils.load_image(filename)
            bmp2 = imageutils.from_pil(imageutils.to_pil(bmp1))
            self._decompress(bmp1)
            self.assertEquals(bmp1.format, bmp2.format)
            self.assertEquals(bmp1.width, bmp2.width)
            self.assertEquals(bmp1.height, bmp2.height)

    def test_load_from_path_or_pil_with_path_returns_image(self):
        path = 'uncompressed/bgr8.png'
        img = imageutils.load_image_from_path_or_pil(
            helputils.get_image_path(path))
        self.assertEquals(img.format, imageutils.PIXEL_FORMAT.B8G8R8X8_UNORM)

    def test_load_from_path_or_pil_with_pil_image_returns_image(self):
        path = 'uncompressed/bgr8.png'
        img = imageutils.load_image_from_path_or_pil(load_pil_image(path))
        self.assertEquals(img.format, imageutils.PIXEL_FORMAT.B8G8R8X8_UNORM)


