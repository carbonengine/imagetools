# ImageTools

ImageTools is a Python extension and C++ image processing library used for loading, manipulating, and compressing image data.

## Build

Initialize dependencies:

```sh
git submodule update --init --recursive
```

Configure and build with CMake:

```sh
cmake --preset x64-windows-release
cmake --build .cmake-build-x64-windows-release --config Release
```

On macOS, use one of the macOS presets, such as `arm64-osx-release` or `x64-osx-release`.

## 📄 License and Legal Notices

© 2026 CCP Games 

This software is provided by CCP Games and does not include or distribute any third-party libraries or frameworks. 

This software provides Python bindings for image processing and compression functionality.

Trademark Notice: CCP Games is a trademark of CCP ehf. 

This project is licensed under the [MIT License](LICENSE.md). Nothing in the [MIT License](LICENSE.md) grants any rights to CCP Games' trademarks or game content.
