# Arin32

**Arin32** is a lightweight, modern, OS-agnostic C++ graphical user interface library powered by OpenGL. Designed for desktop operating systems (Linux, FreeBSD) and engineered to be easily ported to custom, future operating systems.

Licensed under the **BSD 2-Clause License**.

---

## Features

- **Absurdly Simple API**: Create interactive buttons, style them, and hook lambda callbacks in 5 lines of code.
- **Embedded Font Engine**: Zero external asset dependencies. Embedded crisp font atlas allows buttons to render text out of the box in barebones environments.
- **Hardware-Accelerated Anti-Aliasing**: Rounded corners, borders, and drop shadows rendered via GPU fragment shaders with Signed Distance Fields (SDF).
- **OS-Agnostic Architecture**: Platform backend decoupled behind `arin::IPlatformBackend`. Production-ready GLFW backend for Linux & FreeBSD; easily swappable for custom OS kernel framebuffers.
- **Thoroughly Documented & Commented**: Built for readability, self-explanatory architecture, and zero guessing.
- **Automated Unit Testing**: Complete GoogleTest test suite covering geometry, math, inputs, and button state machines.

---

## 5-Line Quick Start

```cpp
#include <arin/arin.hpp>
#include <iostream>

int main() {
    arin::App app("Arin32 Window", 800, 600);

    auto btn = app.add_button("Click Me!", 300, 250, 200, 50);
    btn->on_click([]() {
        std::cout << "Button clicked!\n";
    });

    app.run();
    return 0;
}
```

---

## Building & Testing

### Build with CMake
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run Unit Tests
```bash
ctest --output-on-failure
```

### Run the Interactive Button Showcase
```bash
./examples/button_demo
```

---

## Documentation

Full architectural guides, OS porting instructions, and complete API specifications are documented in English in:
- [**DOCS.md**](DOCS.md)

---

## License

Arin32 is distributed under the **BSD 2-Clause License**. See [LICENSE](LICENSE) for full details.
