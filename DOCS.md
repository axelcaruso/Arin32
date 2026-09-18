# Arin32 Complete API Documentation & Architecture Manual

Welcome to the official developer documentation for **Arin32**, a lightweight, high-performance, and OS-agnostic C++ graphical user interface library powered by OpenGL.

---

## Table of Contents

1. [Architectural Overview & Design Philosophy](#1-architectural-overview--design-philosophy)
2. [Operating System Portability & Future OS Integration](#2-operating-system-portability--future-os-integration)
3. [Quick Start (5-Line Button Application)](#3-quick-start-5-line-button-application)
4. [Core API Reference](#4-core-api-reference)
   - [4.1 `arin::App`](#41-arinapp)
   - [4.2 `arin::Button`](#42-arinbutton)
   - [4.3 `arin::ButtonStyle`](#43-arinbuttonstyle)
   - [4.4 `arin::Theme`](#44-arintheme)
   - [4.5 `arin::Renderer2D`](#45-arinrenderer2d)
   - [4.6 `arin::Font`](#46-arinfont)
   - [4.7 `arin::Window` & `arin::IPlatformBackend`](#47-arinwindow--ariniplatformbackend)
   - [4.8 Geometric & Color Types (`Vec2`, `Rect`, `Color`, `Padding`)](#48-geometric--color-types)
   - [4.9 Input System (`InputState`, `MouseEvent`, `MouseButton`, `InputAction`)](#49-input-system)
5. [Building, Running, and Testing](#5.building-running-and-testing)
   - [Linux Build](#linux-build)
   - [FreeBSD Build](#freebsd-build)
   - [Running the Unit Test Suite](#running-the-unit-test-suite)
   - [Running the Interactive Demo](#running-the-interactive-demo)
6. [License (BSD 2-Clause)](#6-license-bsd-2-clause)

---

## 1. Architectural Overview & Design Philosophy

Arin32 is architected from the ground up for **extreme ease of use**, **zero external asset dependencies**, and **deep operating system portability**.

```
+-------------------------------------------------------------------------+
|                           User Application                              |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                        High-Level API (arin::App)                       |
|     * Automatic event routing to widgets                                |
|     * Continuous render loop with VSync synchronization                 |
|     * Embedded theme management                                         |
+-------------------------------------------------------------------------+
          |                                                   |
          v                                                   v
+-----------------------+                           +---------------------+
|   Widgets & UI Core   |                           |    Platform Layer   |
|   * arin::Button      |                           |    (IPlatform-      |
|   * Event handling    |                           |     Backend)        |
|   * ButtonStyle/Theme |                           |    * GlfwBackend    |
+-----------------------+                           |      (Linux/FreeBSD)|
          |                                         |    * Custom OS      |
          |                                         |      Backend        |
          v                                         +---------------------+
+------------------------------------+                         |
|      Rendering Engine (Renderer2D) |                         |
|   * OpenGL 3.3 Core Shaders        |                         |
|   * Signed Distance Field (SDF) AA |                         |
|   * Anti-aliased rounded corners   |                         |
|   * Embedded zero-file Font Atlas  |<------------------------+
+------------------------------------+
```

### Key Highlights

- **Absurdly Simple API**: Creating interactive buttons, styling them, and binding lambda event handlers takes under 5 lines of readable code.
- **Embedded Font Engine**: Built-in 256x128 proportional typography atlas. Renders crisp, anti-aliased text labels with zero runtime filesystem or TTF file dependencies. Works in early kernel boot / barebones environments.
- **GPU-Accelerated Signed Distance Field (SDF) Rendering**: Rounded corners and borders are rendered in fragment shaders using screen-space derivatives (`fwidth`), ensuring razor-sharp edges with zero jagged artifacts at any scale.
- **Clean OS Abstraction**: Windowing and event polling are isolated behind `arin::IPlatformBackend`. On Linux and FreeBSD, GLFW is used. On a custom OS, implementing 8 virtual methods brings the entire GUI stack up.

---

## 2. Operating System Portability & Future OS Integration

Arin32 is strictly platform-neutral:

1. **Host OS (Linux & FreeBSD)**:
   Uses `arin::GlfwPlatformBackend` which works identically on X11, Wayland, and FreeBSD KMS/DRM displays.
2. **Target / Custom Future Operating System**:
   To port Arin32 to a custom OS, implement the `arin::IPlatformBackend` interface:

```cpp
#include <arin/window.hpp>

class CustomOSBackend : public arin::IPlatformBackend {
public:
    bool create_window(const std::string& title, int width, int height) override {
        // 1. Create native OS surface or initialize framebuffer / EGL context
        return true;
    }
    void destroy_window() override { /* release OS context */ }
    bool should_close() const override { return m_should_exit; }
    void poll_events() override {
        // 2. Read events from OS input driver (/dev/input/mice or OS IPC)
        // 3. Construct arin::MouseEvent and call m_mouse_cb(ev);
    }
    void swap_buffers() override {
        // 4. Present framebuffer (e.g. eglSwapBuffers or OS flip ioctl)
    }
    arin::Vec2 get_window_size() const override { return arin::Vec2(1024, 768); }
    arin::Vec2 get_framebuffer_size() const override { return arin::Vec2(1024, 768); }
    void set_mouse_callback(MouseCallback cb) override { m_mouse_cb = std::move(cb); }
    void set_resize_callback(ResizeCallback cb) override { m_resize_cb = std::move(cb); }

private:
    MouseCallback m_mouse_cb;
    ResizeCallback m_resize_cb;
    bool m_should_exit{false};
};
```

Pass this backend directly to `arin::Window` or `arin::App`:
```cpp
auto backend = std::make_unique<CustomOSBackend>();
arin::Window window("Custom OS Window", 1024, 768, std::move(backend));
```

---

## 3. Quick Start (5-Line Button Application)

```cpp
#include <arin/arin.hpp>
#include <iostream>

int main() {
    // 1. Initialize application
    arin::App app("My First Arin32 App", 800, 600);

    // 2. Add button with label, position (x, y) and size (w, h)
    auto btn = app.add_button("Click Me!", 300, 250, 200, 50);

    // 3. Attach a lambda callback
    btn->on_click([]() {
        std::cout << "Button was clicked!\n";
    });

    // 4. Run interactive event loop
    app.run();
    return 0;
}
```

---

## 4. Core API Reference

### 4.1 `arin::App`

The central coordinator managing window lifecycle, OpenGL context initialization, input dispatching, and rendering loops.

```cpp
#include <arin/app.hpp>
```

#### Constructors & Destructor
- `App(const std::string& title = "Arin32 Application", int width = 800, int height = 600)`:
  Creates window, initializes OpenGL 3.3 Core profile context, builds shaders and font atlases.

#### Methods
- `std::shared_ptr<Button> add_button(const std::string& label, float x, float y, float width, float height)`:
  Creates and registers a new button with given label and geometry.
- `std::shared_ptr<Button> add_button(Button button)`:
  Adds a copy of an existing button.
- `std::shared_ptr<Button> add_button(std::shared_ptr<Button> button)`:
  Adds a pre-allocated button shared pointer.
- `void on_frame(FrameCallback cb)`:
  Registers a user rendering callback executed every frame before buttons are drawn (ideal for titles, backgrounds, panels).
- `void on_after_frame(FrameCallback cb)`:
  Registers a callback executed after all buttons have rendered (ideal for overlays, tooltips, or screenshots).
- `void run()`:
  Enters the blocking interactive event loop until window is closed.
- `void close()`:
  Terminates the run loop gracefully.
- `Window& window()`: Returns reference to underlying `Window`.
- `Renderer2D& renderer()`: Returns reference to 2D OpenGL renderer.
- `Theme& theme()`: Returns reference to the active theme.

---

### 4.2 `arin::Button`

The primary interactive Push Button widget with fluid builder syntax, automatic text centering, and customizable states.

```cpp
#include <arin/button.hpp>
```

#### Interactive States
```cpp
enum class ButtonState : uint8_t {
    Normal,   // Idle state
    Hovered,  // Mouse cursor is within button bounds
    Pressed,  // Mouse left button is pressed inside bounds
    Disabled  // Button is greyed out and rejects all inputs
};
```

#### Constructors
- `Button()`: Creates default button at `(0, 0, 120, 40)`.
- `Button(std::string label)`: Creates button with label at default size.
- `Button(std::string label, float x, float y, float width, float height)`: Explicit bounds.
- `Button(std::string label, const Rect& bounds)`: Rect bounds.
- `Button(std::string label, const Rect& bounds, const ButtonStyle& style)`: Custom style preset.

#### Fluent Setters (Return `Button&` for method chaining)
- `Button& set_text(std::string text)`: Updates label text.
- `Button& set_position(float x, float y)`: Moves button.
- `Button& set_position(const Vec2& pos)`: Moves button with vector.
- `Button& set_size(float width, float height)`: Resizes button.
- `Button& set_bounds(const Rect& bounds)`: Sets bounding box.
- `Button& set_bounds(float x, float y, float width, float height)`: Sets bounds with scalars.
- `Button& set_style(const ButtonStyle& style)`: Applies visual style.
- `Button& set_enabled(bool enabled)`: Enables or disables user interaction.
- `Button& set_corner_radius(float radius)`: Sets corner radius in pixels.

#### Callbacks
- `Button& on_click(ClickCallback callback)`: Registers `void()` callback fired on complete click.
- `Button& on_click(DetailedClickCallback callback)`: Registers `void(Button&)` callback allowing modification of the clicked button.
- `Button& on_hover(HoverCallback callback)`: Registers `void(bool is_hovered)` callback.

#### State Queries
- `const std::string& text() const`: Current label.
- `const Rect& bounds() const`: Current bounding rectangle.
- `ButtonState state() const`: Current interactive state.
- `bool is_hovered() const`: True if hovered or pressed.
- `bool is_pressed() const`: True if left mouse button is held down inside.
- `bool is_enabled() const`: True if not disabled.
- `const ButtonStyle& style() const`: Reference to active style.

#### Event & Render Pipeline
- `bool handle_mouse(const MouseEvent& event)`: Injects mouse event; returns true if handled.
- `void render(Renderer2D& renderer)`: Draws drop shadow, rounded body, border stroke, and centered label.

---

### 4.3 `arin::ButtonStyle`

Defines colors, borders, and dimensions across all button states.

```cpp
#include <arin/theme.hpp>
```

#### Fields
- `Color background_color`: Idle background.
- `Color hover_color`: Background when cursor hovers over button.
- `Color active_color`: Background when button is pressed.
- `Color disabled_color`: Background when button is inactive.
- `Color text_color`: Label color in idle state.
- `Color text_hover_color`: Label color on hover.
- `Color text_active_color`: Label color when pressed.
- `Color text_disabled_color`: Label color when disabled.
- `Color border_color`: Outline stroke color.
- `float border_width`: Border stroke width in pixels (default: 0.0f).
- `float corner_radius`: Corner curvature radius in pixels (default: 8.0f).
- `float text_scale`: Multiplier for font size (default: 1.0f).
- `Color shadow_color`: Soft drop shadow color and opacity.
- `Vec2 shadow_offset`: Shadow displacement (x, y) (default: `(0, 2)`).
- `float shadow_blur`: Gaussian blur radius of shadow (default: 4.0f).

#### Preset Generators
- `static ButtonStyle primary()`: Modern royal blue (`#2563EB`).
- `static ButtonStyle secondary()`: Elegant slate gray (`#334155`).
- `static ButtonStyle success()`: Emerald green (`#16A34A`).
- `static ButtonStyle danger()`: Vibrant crimson red (`#DC2626`).
- `static ButtonStyle outline(Color accent)`: Transparent interior with colored border.

---

### 4.4 `arin::Theme`

Global theme definition holding window background clear colors and default button styles.

- `static Theme& get()`: Global singleton theme.
- `static Theme dark()`: Deep navy background (`#0F172A`) with primary buttons.
- `static Theme light()`: Clean light slate background (`#F8FAFC`).

---

### 4.5 `arin::Renderer2D`

Hardware-accelerated 2D rendering pipeline using OpenGL 3.3 Core profile shaders.

```cpp
#include <arin/renderer.hpp>
```

#### Core Methods
- `bool init()`: Compiles GLSL shaders, initializes VAOs/VBOs, and uploads font atlas.
- `void shutdown()`: Frees OpenGL resources.
- `void begin_frame(int viewport_width, int viewport_height)`: Sets orthographic projection matrix and blending modes.
- `void end_frame()`: Flushes all batched geometry.
- `void clear(const Color& color)`: Clears frame with specified color.
- `void draw_rect(const Rect& rect, const Color& color)`: Draws flat rectangle.
- `void draw_rounded_rect(const Rect& rect, float corner_radius, const Color& fill_color, const Color& border_color = Color::transparent(), float border_width = 0.0f)`:
  Draws anti-aliased rounded rectangle via fragment shader Signed Distance Field.
- `void draw_shadow(const Rect& rect, float corner_radius, const Color& shadow_color, const Vec2& offset, float blur)`:
  Draws soft Gaussian drop shadow.
- `void draw_text(const std::string& text, const Vec2& position, const Color& color, float scale = 1.0f)`:
  Batches and draws text at coordinates.
- `void draw_text_centered(const std::string& text, const Rect& bounds, const Color& color, float scale = 1.0f)`:
  Measures string and renders centered within bounding box.
- `Font& font()`: Access embedded font engine.

---

### 4.6 `arin::Font`

Zero-dependency embedded typography engine.

```cpp
#include <arin/font.hpp>
```

- `bool init_gl()`: Allocates and uploads the embedded 256x128 font atlas texture (`GL_RED`).
- `Vec2 measure_text(const std::string& text, float scale = 1.0f) const`:
  Calculates exact width and height of rendered string in pixels.
- `GlyphInfo get_glyph(char c) const`:
  Retrieves advance, UV texture coordinates, and dimensions for character `c` (ASCII 32 to 126).
- `float line_height() const`: Base font line height in pixels.

---

### 4.7 `arin::Window` & `arin::IPlatformBackend`

OS abstraction isolating window creation, swap buffers, and event polling.

- `class IPlatformBackend`: Abstract interface required for custom OS ports.
- `class GlfwPlatformBackend`: Production backend for Linux (Wayland / X11) and FreeBSD.
- `class Window`: High-level wrapper wrapping `IPlatformBackend`.

---

### 4.8 Geometric & Color Types

```cpp
#include <arin/types.hpp>
```

#### `Vec2`
- `float x, y`
- Operators: `+`, `-`, `*`, `/`, `+=`, `-=`, `*=`, `/=`, `==`, `!=`
- `float distance_to(const Vec2& other) const`
- `Vec2 lerp(const Vec2& other, float t) const`

#### `Rect`
- `float x, y, width, height`
- `Vec2 position() const`, `Vec2 size() const`, `Vec2 center() const`
- `float left() const`, `float right() const`, `float top() const`, `float bottom() const`
- `bool contains(const Vec2& point) const`: Point hit-testing.
- `Rect expanded(float amount) const`

#### `Color`
- `float r, g, b, a` in normalized range `[0.0, 1.0]`.
- `static Color from_rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)`
- `static Color from_hex(uint32_t hex, bool has_alpha = false)`
- `Color with_alpha(float new_alpha) const`
- `Color lerp(const Color& other, float t) const`
- Presets: `white()`, `black()`, `transparent()`, `primary()`, `secondary()`, `success()`, `danger()`, `surface()`, `background()`.

---

### 4.9 Input System

```cpp
#include <arin/input.hpp>
```

- `enum class MouseButton : uint8_t { Left, Right, Middle };`
- `enum class InputAction : uint8_t { Release, Press, Repeat };`
- `enum class MouseEventType : uint8_t { Move, ButtonDown, ButtonUp, Scroll };`
- `struct MouseEvent`: Holds position, button, action, scroll delta.
- `class InputState`: Tracks current cursor position and pressed button state array.

---

## 5. Building, Running, and Testing

### Prerequisites
- C++17 compatible compiler (`g++` >= 9 or `clang++` >= 10)
- CMake >= 3.16
- OpenGL >= 3.3 development libraries
- GLFW >= 3.3
- GLEW >= 2.0
- GoogleTest (for running test suite)

### Linux Build
```bash
# Ubuntu / Debian
sudo apt-get install build-essential cmake libgl1-mesa-dev libglew-dev libglfw3-dev libgtest-dev

# Arch / CachyOS / Fedora
sudo pacman -S base-devel cmake glew glfw-x11 gtest # or dnf install ...

# Build Arin32
git clone https://github.com/arin32/arin32.git
cd arin32
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### FreeBSD Build
```bash
# Install dependencies via pkg
pkg install cmake gmake glfw glew googletest

# Build with CMake and gmake
mkdir build && cd build
cmake ..
gmake -j$(sysctl -n hw.ncpu)
```

### Running the Unit Test Suite
```bash
cd build
ctest --output-on-failure
# Or run binary directly:
./tests/arin_tests
```

### Running the Interactive Demo
```bash
./build/examples/button_demo
```

To take a headless/automated screenshot:
```bash
./build/examples/button_demo --screenshot preview.ppm
```

---

## 6. License (BSD 2-Clause)

```
BSD 2-Clause License

Copyright (c) 2026, Arin32 Contributors
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
