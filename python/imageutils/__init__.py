from PIL import Image as PILImage
import struct

import osutils

# noinspection PyUnresolvedReferences
from _imagetools import *


def from_pil(pil_image):
    """Converts PIL image to a imageutils image.

    :param pil_image: PIL.Image instance
    :return: imageutils image instance created from PIL image
    :rtype : imageutils.ImageToolsBitmap
    :raise RuntimeError: when PIL image cannot be converted
    """
    if pil_image.mode == 'L':
        img_format = PIXEL_FORMAT.R8_UNORM
    elif pil_image.mode == 'RGB':
        b, g, r = pil_image.split()
        pil_image = PILImage.merge("RGB", (r, g, b))
        img_format = PIXEL_FORMAT.B8G8R8X8_UNORM
        pil_image = pil_image.convert('RGBA')
    elif pil_image.mode == 'RGBA':
        b, g, r, a = pil_image.split()
        pil_image = PILImage.merge("RGBA", (r, g, b, a))
        img_format = PIXEL_FORMAT.B8G8R8A8_UNORM
    elif pil_image.mode == 'RGBX':
        b, g, r, a = pil_image.split()
        pil_image = PILImage.merge("RGBA", (r, g, b, a))
        img_format = PIXEL_FORMAT.B8G8R8X8_UNORM
    elif pil_image.mode == 'LA':
        img_format = PIXEL_FORMAT.R8G8_UNORM
    elif pil_image.mode == 'F':
        img_format = PIXEL_FORMAT.R32G32B32A32_FLOAT
    else:
        raise RuntimeError(
            'unsupported PIL image mode %s' % pil_image.mode)
    return create_2d_from_string(pil_image.size[0], pil_image.size[1], 1, img_format,
                                 pil_image.tostring())


def to_pil(image):
    """Converts imageutils image to a PIL image. If the input image is compressed
    attempts to decompress it first.

    :param image: imageutils.ImageToolsBitmap image
    :return: PIL image converted from imageutils image
    :rtype : PIL.Image
    :raise RuntimeError: if the input image cannot be converted to PIL
    """
    if image.is_compressed():
        image = image.copy()
        if image.format == PIXEL_FORMAT.BC1_UNORM:
            image.convert_format(PIXEL_FORMAT.B8G8R8X8_UNORM)
        else:
            image.convert_format(PIXEL_FORMAT.B8G8R8A8_UNORM)
    switch_rb = False
    if image.format == PIXEL_FORMAT.R8_UNORM:
        mode = 'L'
        input_mode = 'L'
    elif image.format == PIXEL_FORMAT.B8G8R8X8_UNORM:
        mode = 'RGB'
        input_mode = 'RGBX'
        switch_rb = True
    elif image.format == PIXEL_FORMAT.B8G8R8A8_UNORM:
        mode = 'RGBA'
        input_mode = 'RGBA'
        switch_rb = True
    elif image.format == PIXEL_FORMAT.R8G8_UNORM:
        mode = 'LA'
        input_mode = 'LA'
    elif image.format == PIXEL_FORMAT.R32G32B32A32_FLOAT:
        mode = 'F'
        input_mode = 'F'
    else:
        raise RuntimeError('image format %s cannot be converted to PIL' % image.format)
    pil_image = PILImage.fromstring(mode, (image.width, image.height), image.get_pixel_data(), 'raw', input_mode)
    if switch_rb:
        channels = list(pil_image.split())
        tmp = channels[0]
        channels[0] = channels[2]
        channels[2] = tmp
        pil_image = PILImage.merge(mode, tuple(channels))
    return pil_image


def load_image_from_path_or_pil(path_or_pil):
    """Loads image from specified path or PIL image.

    :param path_or_pil: either a string specifying path to image file or a PIL.Image
    instance.
    :return: imageutils image
    """
    if isinstance(path_or_pil, PILImage.Image):
        return from_pil(path_or_pil)
    return load(path_or_pil)


def convert_image(path_or_pil, out_path):
    """Saves an image to a specified path possibly converting image in the process.

    :param path_or_pil: either a string specifying path to image file or a PIL.Image
    instance.
    :param out_path: output file path.
    """
    img = load_image_from_path_or_pil(path_or_pil)
    img.save(out_path)


def convert_image_to_temp_file(path_or_pil, extension):
    """Saves image to a temporary file with specified extension possibly converting image
    in the process.

    :param path_or_pil: either a string specifying path to image file or a PIL.Image
    instance.
    :param extension: output file extension (including leading dot)
    :return: path to the output temporary file
    """
    img = load_image_from_path_or_pil(path_or_pil)
    tmp_path = osutils.MkTemp(suffix=extension)
    if extension.lower() != '.dds' and img.is_compressed():
        img.convert_format(PIXEL_FORMAT.B8G8R8A8_UNORM)
    img.save(tmp_path)
    return tmp_path


def get_dds_size(ddspath):
    """Return ``(width, height)`` of ``ddspath``."""
    with open(ddspath) as f:
        h = struct.unpack('IIIII', f.read(4*5))
        height = h[3]
        width = h[4]
    return width, height
