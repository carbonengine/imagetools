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

## 🤝 Contributing

Contributions are welcome. Please read the Carbon Engine [contributing guide](https://github.com/carbonengine/.github/blob/main/CONTRIBUTING.md) before opening an issue or pull request. It covers the workflow, the CLA and the pull request template, and applies to every `carbonengine` repository. Please also follow the [Code of Conduct](https://github.com/carbonengine/.github/blob/main/CODE_OF_CONDUCT.md), and report security issues privately as described in the [Security Policy](https://github.com/carbonengine/.github/blob/main/SECURITY.md) rather than in a public issue.

By submitting a pull request or otherwise contributing to this project, you agree to license your contribution under the [MIT License](LICENSE.md), and you confirm that you have the right to do so.

## 📄 License and Legal Notices

© 2026 Fenris Creations 

This software is provided by Fenris Creations and does not include or distribute any third-party libraries or frameworks. 

This software provides Python bindings for image processing and compression functionality.

Trademark Notice: Fenris Creations is a trademark of CCP ehf. 

This project is licensed under the [MIT License](LICENSE.md). Nothing in the [MIT License](LICENSE.md) grants any rights to Fenris Creations' trademarks or game content.
