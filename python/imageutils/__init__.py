__author__ = 'filipp'

import binbootstrapper
app_root = binbootstrapper.get_approot()
if app_root is None:
    app_root = __file__
binbootstrapper.update_binaries(app_root, binbootstrapper.DLL_IMAGETOOLS)
del app_root

from _imagetools import *
