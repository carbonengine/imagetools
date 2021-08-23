import os
import shutil
import tempfile
import unittest
import testhelpers

import imageutils

import helputils


RESOURCE_DIR = os.path.join(os.path.dirname(__file__), 'res')


def _get_relative_path(path):
    return os.path.relpath(path, os.path.dirname(__file__))


class TestSavingBase(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.temp_dir)

    def check_same_images(self, image1, image2):
        self.assertEquals(image2.type, image1.type)
        self.assertEquals(image2.format, image1.format)
        self.assertEquals(image2.width, image1.width)
        self.assertEquals(image2.height, image1.height)

    def save_temp_image_with_extension(self, bmp, filename, extension):
        out_name = os.path.join(self.temp_dir, filename)
        try:
            os.makedirs(os.path.dirname(out_name))
        except OSError:
            pass
        out_name = os.path.splitext(out_name)[0] + extension
        if extension.lower() != '.dds' and bmp.is_compressed():
            bmp.convert_format(imageutils.PIXEL_FORMAT.B8G8R8A8_UNORM)
        bmp.save(out_name)
        return out_name


class TestSaving(TestSavingBase):
    __metaclass__ = testhelpers.TestGeneratorMetaClass
    __metatestitems__ = [(os.path.basename(fp), fp)
                         for fp in helputils.get_image_paths()]

    def check_same_images(self, image1, image2):
        super(TestSaving, self).check_same_images(image1, image2)
        self.assertEquals(image2.mip_count, image1.mip_count)

    def _test_can_save_image_to_dds(self, filename):
        bmp = helputils.load_image(filename)
        rel_name = _get_relative_path(filename)
        out_name = self.save_temp_image_with_extension(bmp, rel_name, '.dds')
        out = imageutils.load(out_name)
        self.check_same_images(bmp, out)


class TestSavingUncompressed(TestSavingBase):
    __metaclass__ = testhelpers.TestGeneratorMetaClass
    __metatestitems__ = [
        (os.path.basename(fp), fp)
        for fp in (list(helputils.get_image_paths('uncompressed')) +
                   list(helputils.get_image_paths('compressed')))]

    def _test_can_save_image_to_png(self, filename):
        bmp = helputils.load_image(filename)
        rel_name = _get_relative_path(filename)
        out_name = self.save_temp_image_with_extension(bmp, rel_name, '.png')
        out = imageutils.load(out_name)
        self.check_same_images(bmp, out)
