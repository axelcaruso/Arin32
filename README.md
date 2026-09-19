# Arin32

Arin32 is a lightweight, OS-agnostic C++17 GUI library built on OpenGL 3.3 Core. It targets Linux and FreeBSD today and is designed so the same UI stack can later run on a custom operating system by implementing a single, small platform interface.

Licensed under the BSD 2-Clause License.

## Why Arin32

- **Zero asset dependencies at runtime.** The UI font (Open Sans) ships pre-rasterized as an embedded atlas, so text rendering works out of the box even in bare-metal or early-boot environments where no filesystem is available.
- **GPU-rendered visuals.** Rounded corners, borders, and shadows are drawn with Signed Distance Field fragment shaders, giving crisp anti-aliased edges at any resolution or scale.
- **Portability by design.** All windowing and input handling is isolated behind the `arin::IPlatformBackend` interface. GLFW is the reference backend; a custom OS backend replaces it without touching widget or rendering code.
- **Small, readable codebase.** The library favors clarity over abstraction layers and is covered by a GoogleTest unit suite.

## Widget Set

The current high-level API covers the core building blocks of a desktop UI: buttons, checkboxes, single-line text input, progress bars (determinate and indeterminate), list boxes and check list boxes, vector icons, images and textures, and automatic vertical/horizontal layout containers. All widgets are styleable through a theme system.

## Requirements

- C++17 compiler
- CMake 3.16 or newer
- OpenGL 3.3 Core capable GPU/driver
- GLEW and GLFW 3.3+
- GoogleTest (unit tests only)

## Build

```bash
cmake -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

Examples and tests are enabled by default and can be disabled with `-DARIN_BUILD_EXAMPLES=OFF` and `-DARIN_BUILD_TESTS=OFF`.

## Documentation

This README is intentionally a short summary. Complete API reference, architecture overview, widget-by-widget usage, and step-by-step instructions for porting Arin32 to a custom OS are in [DOCS.md](DOCS.md). Third-party licensing notices are in [THIRDPARTY](THIRDPARTY).

## License

Arin32 is distributed under the BSD 2-Clause License. See [LICENSE](LICENSE) for details.

