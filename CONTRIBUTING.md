# Contributing to Arin32

Thank you for your interest in contributing to Arin32. As an open-source, high-performance C++ graphical user interface library targeting Linux, FreeBSD, and future operating systems (such as ArinOS), we uphold strict engineering standards to ensure portability, stability, and zero runtime bloat.

---

## 1. Code of Conduct

All contributors and maintainers are expected to adhere to our [Code of Conduct](CODE_OF_CONDUCT.md). Please read it before participating in discussions or submitting contributions.

---

## 2. Core Architecture & Design Philosophy

When submitting patches or implementing new features, keep the following foundational principles in mind:

1. **Operating System Portability**:
   - Never couple UI widgets or rendering pipelines directly to OS-specific APIs.
   - All windowing, events, and display interactions must reside behind the `arin::IPlatformBackend` interface.
   - Desktop platforms utilize the reference GLFW backend; custom operating system kernels implement their own backend without altering widget logic.

2. **Zero External Runtime Bloat**:
   - The core library must remain lightweight and self-contained.
   - Default fonts and vital vector icons must function out of the box without requiring filesystem access or external asset loaders at runtime.
   - Vector graphics parsing (SVG) and rasterization must avoid heavy external toolkits.

3. **Rendering Efficiency**:
   - All rendering must utilize GPU hardware acceleration via modern OpenGL 3.3 Core Profile pipelines.
   - Rounded rectangles, outlines, shadows, and smooth badges must leverage Signed Distance Field (SDF) fragment shaders to preserve crisp fidelity at arbitrary display scales.

---

## 3. Coding Standards & Conventions

### Language Standard
* Code must strictly conform to **ISO C++17** (`-std=c++17`).
* Do not introduce language extensions (`set(CMAKE_CXX_EXTENSIONS OFF)` is enforced).

### Compiler Warnings
* All code must compile cleanly with **zero warnings** under strict compiler flags:
  ```
  -Wall -Wextra -Wpedantic
  ```
* Warnings are treated as errors in continuous integration.

### Code Style & Formatting
* **Indentation**: 4 spaces (no tabs).
* **Naming Conventions**:
  - Types, classes, and structs: `PascalCase` (e.g., `Button`, `ContextMenu`, `SvgDocument`).
  - Functions and methods: `snake_case` (e.g., `set_bounds()`, `render()`, `handle_event()`).
  - Variables and parameters: `snake_case` (e.g., `line_height`, `is_hovered`).
  - Member variables: trailing underscore or clear member prefix (e.g., `bounds_`, `mode_`, `label_`).
  - Constants and Enums: `PascalCase` or `UPPER_SNAKE_CASE` (e.g., `ButtonState::Hovered`).
* **Header Inclusions**:
  - Prefer specific standard library headers over monolithic headers.
  - Group includes: Arin32 internal headers first, third-party libraries second, standard library headers last.
* **Documentation & Comments**:
  - Maintain documentation integrity across the codebase.
  - Zero emojis in source code comments or documentation files.
  - Write clear, concise docstrings for all public classes and methods in the `include/arin/` tree.

### Licensing Header
Every new C++ source file (`.cpp`), header (`.hpp`), or script (`.py`) must begin with the project BSD 2-Clause license header:

```cpp
// Copyright (c) 2026, Arin32 & ArinOS Contributors
// Use of this source code is governed by a BSD 2-Clause License
// that can be found in the LICENSE file.
```

---

## 4. Development Workflow

### Building the Project

```bash
# Clone the repository
git clone https://github.com/arin32/arin32.git
cd arin32

# Configure CMake in Release mode
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build core libraries, examples, and test suite
cmake --build build -j$(nproc)
```

### Running Tests

Any contribution modifying existing behavior or introducing new functionality must include unit tests. Arin32 maintains test suites for both static (`libarin32.a`) and dynamic (`libarin32.so`) targets:

```bash
# Execute test suite via CTest
ctest --test-dir build --output-on-failure

# Or execute binaries directly
./build/tests/arin_tests          # Static library tests
./build/tests/arin_tests_shared   # Shared library tests
```

### Packaging & Distribution Verification

Before submitting a Pull Request, verify that the distribution pipeline packages and validates cleanly:

```bash
# Run packaging script to verify both targets and SHA256 manifest
python3 scripts/package.py

# Verify generated checksums
cd dist && sha256sum -c SHA256
```

---

## 5. Submitting Pull Requests

1. **Branch Naming**: Use descriptive branch names:
   - `feature/my-new-widget`
   - `fix/text-input-selection`
   - `docs/clarify-platform-backend`
2. **Commit Messages**:
   - Use clear, imperative commit messages (e.g., `Add cascading submenu support to ContextMenu`).
   - Keep the first line under 72 characters, followed by an optional explanatory body.
3. **Pull Request Checklist**:
   - [ ] Code strictly complies with ISO C++17.
   - [ ] Compiles with zero warnings under `-Wall -Wextra -Wpedantic`.
   - [ ] Unit tests added in `tests/` covering new logic or bug fixes.
   - [ ] All 216+ tests pass on both static and shared libraries.
   - [ ] Public API methods documented in `include/arin/` and [DOCS.md](DOCS.md).
   - [ ] BSD 2-Clause license header included in all new files.
   - [ ] No emojis in comments or documentation.
