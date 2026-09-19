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
   - [4.4 `arin::CheckBox` & `arin::CheckBoxStyle`](#44-arincheckbox--arincheckboxstyle)
   - [4.5 `arin::TextInput` (`arin::TextBox`) & `arin::TextInputStyle`](#45-arintextinput-arintextbox--arintextinputstyle)
   - [4.6 `arin::ProgressBar` & `arin::ProgressBarMode`](#46-arinprogressbar--arinprogressbarmode)
   - [4.7 `arin::ProgressBarStyle`](#47-arinprogressbarstyle)
   - [4.8 `arin::IWidget` (Polymorphic Widget Base)](#48-ariniwidget-polymorphic-widget-base)
   - [4.9 Automatic Layout Containers (`arin::Layout`, `arin::VBox`, `arin::HBox`, `arin::LayoutJustify`)](#49-automatic-layout-containers-arinlayout-arinvbox-arinhbox-arinlayoutjustify)
   - [4.10 List Box Widgets (`arin::ListBox`, `arin::CheckListBox`, `arin::ListBoxMode`, `arin::ListBoxStyle`)](#410-list-box-widgets-arinlistbox-arinchecklistbox-arinlistboxmode-arinlistboxstyle)
   - [4.11 Image & Texture Support (`arin::Texture`, `arin::Image`, `arin::TextureFilter`, `arin::ImageScaleMode`)](#411-image--texture-support-arintexture-arinimage-arintexturefilter-arinimagescalemode)
   - [4.11b Scalable Vector Graphics (`arin::SvgDocument`, `arin::SvgImage`, and Icon Arsenal)](#411b-scalable-vector-graphics-arinsvgdocument-arinsvgimage-and-icon-arsenal)
   - [4.12 Vector Icons (`arin::Icon`, `arin::IconType`)](#412-vector-icons-arinicon-arinicontype)
   - [4.13 Context Menu System (`arin::ContextMenu`, `arin::MenuItem`, `arin::ContextMenuStyle`)](#413-context-menu-system-arincontextmenu-arinmenuitem-arincontextmenustyle)
   - [4.14 `arin::Theme`](#414-arintheme)
   - [4.15 `arin::Renderer2D`](#415-arinrenderer2d)
   - [4.16 `arin::Font`](#416-arinfont)
   - [4.17 `arin::Window` & `arin::IPlatformBackend`](#417-arinwindow--ariniplatformbackend)
   - [4.18 Geometric & Color Types (`Vec2`, `Rect`, `Color`, `Padding`)](#418-geometric--color-types)
   - [4.19 Input System (`InputState`, `MouseEvent`, `KeyEvent`, `TextEvent`, `KeyCode`, `KeyModifier`)](#419-input-system)
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

- **Concise API**: Creating interactive buttons, styling them, and binding lambda event handlers takes only a few lines of readable code.
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
- `std::shared_ptr<ProgressBar> add_progress_bar(float x, float y, float width, float height, float value = 0.0f, float min = 0.0f, float max = 100.0f)`:
  Creates, registers, and automatically manages frame rendering and animation updates for a new progress bar.
- `std::shared_ptr<ProgressBar> add_progress_bar(ProgressBar bar)`:
  Adds a copy of an existing progress bar.
- `std::shared_ptr<ProgressBar> add_progress_bar(std::shared_ptr<ProgressBar> bar)`:
  Adds a pre-allocated progress bar shared pointer.
- `std::shared_ptr<IWidget> add_widget(std::shared_ptr<IWidget> widget)`:
  Registers an arbitrary polymorphic `IWidget` into the application's event loop and render list.
- `std::shared_ptr<ListBox> add_list_box(float x, float y, float width = 220.0f, float height = 180.0f, ListBoxMode mode = ListBoxMode::Standard)`:
  Creates, registers, and returns an interactive single-selection or checkbox list box.
- `std::shared_ptr<CheckListBox> add_check_list_box(float x, float y, float width = 220.0f, float height = 180.0f)`:
  Creates, registers, and returns a specialized check box list with independent item toggling.
- `std::shared_ptr<VBox> add_vbox(float x = 0.0f, float y = 0.0f, float spacing = 8.0f)`:
  Creates and registers a vertical automatic layout container (stacks child widgets top-to-bottom).
- `std::shared_ptr<HBox> add_hbox(float x = 0.0f, float y = 0.0f, float spacing = 8.0f)`:
  Creates and registers a horizontal automatic layout container (stacks child widgets left-to-right).
- `std::shared_ptr<Layout> add_layout(std::shared_ptr<Layout> layout)`:
  Registers a pre-allocated layout container.
- `std::shared_ptr<Image> add_image(std::shared_ptr<Texture> texture, float x, float y, float width, float height, ImageScaleMode scale_mode = ImageScaleMode::Fit)`:
  Creates, registers, and returns an Image widget bound to an existing texture.
- `std::shared_ptr<Image> add_image(const std::string& filepath, float x, float y, float width, float height, ImageScaleMode scale_mode = ImageScaleMode::Fit)`:
  Creates, registers, and returns an Image widget loaded directly from an image file (PNG, JPG, BMP, TGA).
- `std::shared_ptr<Icon> add_icon(IconType icon, float x, float y, float size = 16.0f, const Color& color = Color(240, 240, 240))`:
  Creates, registers, and returns a crisp vector system icon widget.
- `std::shared_ptr<CheckBox> add_checkbox(const std::string& label, float x, float y, bool checked = false)`:
  Creates, registers, and returns an interactive two-state CheckBox widget.
- `std::shared_ptr<TextInput> add_text_input(const std::string& initial_text, float x, float y, float width = 200.0f, float height = 32.0f)`:
  Creates, registers, and returns an editable TextInput (TextBox) widget with keyboard focus routing and clipboard integration.
- `void set_focus(std::shared_ptr<IWidget> widget)`:
  Sets active keyboard input focus to the specified widget (or clears focus if passed `nullptr`).
- `std::shared_ptr<IWidget> focused_widget() const`:
  Gets the currently focused widget, or `nullptr`.
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
- `Button()`: Creates default button at `(0, 0, 85, 32)`.
- `Button(std::string label)`: Creates button with label at default size (32px height).
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
- `Button& set_corner_radius(float radius)`: Sets corner radius in pixels (default: 4.5px).
- `Button& set_icon(IconType icon, float size = 16.0f, float spacing = 6.0f)`: Attaches a crisp vector icon preceding the button label.
- `Button& set_auto_resize(bool enable)`: Enables/disables automatic expansion to fit text and icon content (default: true).
- `Button& fit_to_text(const Font& font, float horizontal_padding = 14.0f)`: Explicitly resizes the button to fit its text and icon with padding.

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
- `bool is_auto_resize() const`: True if auto-resize is active.
- `const ButtonStyle& style() const`: Reference to active style.

#### Strict Overflow Prevention Guarantee
In Arin32, text is mathematically and physically guaranteed **never to overflow or bleed out of its button or container**:
1. **Auto-Resize**: Enabled by default, automatically enlarges button bounds if label requires more room than current bounds.
2. **Dynamic Scale Fallback**: If fixed dimensions are enforced (`auto_resize=false`), text scale automatically downscales so that it remains fully contained within the inner padding.
3. **Hardware Scissor Clipping**: All button text rendering is wrapped in `glScissor` set to the button's exact bounding box, preventing any pixel from drawing outside.

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

### 4.4 `arin::CheckBox` & `arin::CheckBoxStyle`

A standalone, fully interactive two-state checkbox widget featuring crisp GPU-drawn vector check marks, fluid hover animations, automatic content measurement, and seamless integration with layouts.

```cpp
#include <arin/checkbox.hpp>
```

#### Constructors
- `CheckBox(std::string label = "CheckBox", float x = 0.0f, float y = 0.0f, bool checked = false)`:
  Constructs a checkbox with accompanying text label, coordinates, and initial toggle state.

#### Methods & Chaining
- `bool is_checked() const`: Queries whether the box is checked.
- `CheckBox& set_checked(bool checked)`: Sets checked state and fires `on_toggled` if state changed.
- `CheckBox& toggle()`: Inverts current check state.
- `CheckBox& on_toggled(ToggleCallback cb)`: Registers callback `std::function<void(bool is_checked)>` fired whenever state toggles.
- `const std::string& label() const`: Gets label text.
- `CheckBox& set_label(std::string label)`: Sets label text and refits bounds if auto-resize is enabled.
- `CheckBox& set_auto_resize(bool enable)`: Enables or disables automatic content-driven sizing (enabled by default).
- `bool is_auto_resize() const`: Checks whether auto-resize is enabled.
- `CheckBox& fit_to_content()`: Automatically resizes widget bounding box to tightly enclose square box and measured label text using default font metrics.
- `CheckBox& fit_to_content(const Font& font)`: Automatically resizes widget bounding box to tightly enclose square box and measured label text using the specified font.
- `CheckBox& ensure_containment(const Font& font)`: Expands bounding box if needed so text and box are fully contained.
- `CheckBox& set_style(const CheckBoxStyle& style)`: Applies visual style configuration.
- `CheckBox& set_enabled(bool enabled)`: Sets interactive state.
- `CheckBox& set_visible(bool visible)`: Sets visibility.

#### Visual Styling (`CheckBoxStyle`)
- `float box_size{16.0f}`: Width and height of checkable box.
- `float corner_radius{3.0f}`: Box corner rounding radius.
- `float text_spacing{8.0f}`: Gap between box and text.
- `float text_scale{0.92f}`: Typography scale factor.
- `Color box_background`: Fill color when unchecked (`#FFFFFF`).
- `Color box_border`: Border color when unchecked (`#9CA3AF`).
- `Color checked_fill`: Accent fill when checked (`#0067C0`).
- `Color checkmark_color`: Stroke color of the checkmark (`#FFFFFF`).
- `Color hover_border`: Border outline highlight on hover (`#4B5563`).
- `Color disabled_color`: Dimmed border and text color when disabled.

---

### 4.5 `arin::TextInput` (`arin::TextBox`) & `arin::TextInputStyle`

A complete, modern editable text field widget built with deep keyboard event dispatching, focus management, hardware scissor clipping, and clipboard integration.

```cpp
#include <arin/text_input.hpp>
```

#### Key Capabilities
1. **Focus & Caret**: Clicking gives keyboard focus; renders an animated 500ms blinking vertical insertion caret that resets to visible during typing or navigation.
2. **Text Containment & Scrolling**: Dynamic horizontal scroll offset (`m_scroll_offset`) and hardware scissor clipping (`Renderer2D::push_clip_rect`) guarantee typed text **never bleeds outside the input box**.
3. **Cursor Navigation**: Left / Right arrow navigation, word navigation via `Ctrl + Left / Right`, and `Home` / `End` key jumping.
4. **Text Selection**: Mouse click-and-drag selection, `Shift + Arrow / Home / End`, or `Ctrl + A` to select all with accent blue highlight.
5. **Editing**: Backspace and Delete keys (operates on single characters or selected ranges), Enter key submission (`on_submit`).
6. **Clipboard Integration**: Native `Ctrl+C` (copy), `Ctrl+X` (cut), and `Ctrl+V` (paste) linked automatically to system clipboard through `Window`.
7. **Placeholder & Masking**: Custom placeholder text rendered when empty; optional password masking (`set_password(true)`).

#### Constructors
- `TextInput(std::string initial_text = "", float x = 0.0f, float y = 0.0f, float width = 200.0f, float height = 32.0f)`

#### Methods & Chaining
- `const std::string& text() const` / `TextInput& set_text(std::string text)`: Text accessors.
- `const std::string& placeholder() const` / `TextInput& set_placeholder(std::string placeholder)`: Placeholder text.
- `size_t cursor_position() const` / `TextInput& set_cursor_position(size_t pos)`: Caret index.
- `bool has_selection() const` / `std::string selected_text() const`: Selection queries.
- `TextInput& select_all()` / `TextInput& clear_selection()` / `bool delete_selection()`: Selection actions.
- `TextInput& insert_text(const std::string& str)`: Inserts string at cursor (replacing selection if active).
- `TextInput& set_read_only(bool read_only)` / `bool is_read_only() const`: Read-only toggle.
- `TextInput& set_password(bool password, char mask = '*')`: Password masking mode.
- `bool is_focused() const` / `TextInput& set_focused(bool focused)`: Focus state.
- `TextInput& on_text_changed(TextChangeCallback cb)`: Callback `std::function<void(const std::string& text)>`.
- `TextInput& on_submit(SubmitCallback cb)`: Callback `std::function<void(const std::string& text)>` fired on Enter key.
- `TextInput& set_clipboard_provider(ClipboardGetCallback get_cb, ClipboardSetCallback set_cb)`: Custom clipboard hook.

#### Visual Styling (`TextInputStyle`)
- `float corner_radius{4.0f}`, `float border_width{1.0f}`, `float focus_border_width{2.0f}`.
- `float padding_x{10.0f}`, `float text_scale{0.92f}`, `float cursor_width{1.5f}`.
- `Color background_color`, `Color border_color`, `Color hover_border_color`.
- `Color focus_border_color`: Windows 10 Accent Blue (`#0067C0`).
- `Color selection_color`: Semi-transparent blue fill (`rgba(0, 103, 192, 75)`).

---

### 4.6 `arin::ProgressBar` & `arin::ProgressBarMode`

A modern, hardware-accelerated progress bar widget designed to strictly match the flat aesthetic of Windows 10 with 1:1 visual concordance, subtle 1.0px borders, and GPU-driven animations.

```cpp
#include <arin/progress_bar.hpp>
```

#### Operating Modes (`ProgressBarMode`)
- `ProgressBarMode::Determinate`:
  Quantified progress mode (default: 0 to 100%). Renders an active fill segment proportional to current progress, traversed by an animated, continuous soft-white shimmer sweep.
- `ProgressBarMode::Indeterminate`:
  Continuous activity / marquee mode with unknown duration. A smooth accent slice with a soft central specular highlight glides continuously across the track.

#### Constructors
- `ProgressBar()`: Constructs default 260x20 progress bar at (0, 0).
- `ProgressBar(float x, float y, float width = 260.0f, float height = 20.0f)`: Constructs progress bar at position with size.
- `ProgressBar(float x, float y, float width, float height, float value, float min = 0.0f, float max = 100.0f)`: Constructs progress bar with position, size, initial value, and range.
- `ProgressBar(const Rect& bounds)`: Constructs with bounding box.
- `ProgressBar(const Rect& bounds, ProgressBarMode mode)`: Constructs with bounding box and mode.

#### Fluent Setters
- `set_mode(ProgressBarMode mode)`: Switches between Determinate and Indeterminate.
- `set_indeterminate(bool indeterminate)`: Quick helper to toggle marquee mode.
- `set_value(float value)`: Sets current progress value (automatically clamped to `[min, max]`).
- `set_range(float min_val, float max_val)`: Configures lower and upper bounds.
- `set_bounds(const Rect& bounds)` / `set_bounds(float x, float y, float w, float h)`: Updates geometry.
- `set_position(float x, float y)` / `set_size(float width, float height)`: Updates position or size.
- `set_style(const ProgressBarStyle& style)`: Applies visual style.
- `set_fill_color(const Color& color)`: Quick setter for active accent fill color.
- `set_track_color(const Color& color)`: Quick setter for background track color.
- `set_corner_radius(float radius)`: Quick setter for corner curvature.
- `set_speed(float speed)`: Multiplies animation speed (1.0 = standard rate).
- `set_anim_phase(float phase)`: Manually sets animation phase (ideal for deterministic tests or screenshots).

#### Queries
- `float value() const`: Current value.
- `float min_value() const`: Lower bound.
- `float max_value() const`: Upper bound.
- `float percentage() const`: Normalized fraction in range `[0.0, 1.0]`.
- `float anim_phase() const`: Continuous phase timer.
- `ProgressBarMode mode() const`: Current mode.
- `bool is_indeterminate() const`: True if in marquee mode.
- `const Rect& bounds() const`: Bounding box.
- `const ProgressBarStyle& style() const`: Active styling parameters.

#### Lifecycle & Rendering
- `void update(float dt)`: Advances animation phase based on delta time and style speed. Automatically called by `App::run()`.
- `void render(Renderer2D& renderer)`: Draws track, border, fill, and animated sweeps in a single anti-aliased fragment shader pass.

---

### 4.7 `arin::ProgressBarStyle`

Visual configuration for `ProgressBar` matching Windows 10 flat modern design specifications.

```cpp
#include <arin/progress_bar.hpp>
```

#### Fields
- `Color track_color`: Background track color (default: light slate grey `#E5E7EB`).
- `Color fill_color`: Active progress color (default: Windows 10 Green `#06B025`).
- `Color border_color`: Boundary border stroke (default: subtle grey `#D1D5DB`).
- `float border_width`: Border thickness in pixels (default: 1.0f).
- `float corner_radius`: Corner curvature radius in pixels (default: 2.5f).
- `float animation_speed`: Speed multiplier for shimmer and marquee sweeps (default: 0.85f).

#### Preset Generators
- `static ProgressBarStyle green()`: Windows 10 classic green progress (`#06B025`).
- `static ProgressBarStyle blue()`: Windows 10 / ArinOS accent blue (`#0067C0`).
- `static ProgressBarStyle dark()`: Dark mode theme with dark slate track (`#27272A`) and green fill.

---

### 4.8 `arin::IWidget` (Polymorphic Widget Base)

Base interface for all UI elements in Arin32 (`Button`, `CheckBox`, `TextInput`, `ProgressBar`, `ListBox`, `CheckListBox`, `Image`, `Icon`, `Layout`). Enables uniform lifecycle management, event dispatching, and container nesting.

```cpp
#include <arin/widget.hpp>
```

#### Core Virtual Methods
- `virtual const Rect& bounds() const = 0`: Returns current bounding box.
- `virtual IWidget& set_bounds(const Rect& bounds)`: Updates bounding box.
- `virtual IWidget& set_position(float x, float y)`: Moves widget.
- `virtual IWidget& set_size(float width, float height)`: Resizes widget.
- `virtual bool handle_mouse(const MouseEvent& ev)`: Dispatches mouse move, click, or scroll events. Returns `true` if consumed.
- `virtual bool handle_key(const KeyEvent& ev)`: Processes keyboard key press, repeat, or release events. Returns `true` if consumed.
- `virtual bool handle_text(const TextEvent& ev)`: Processes character text input events (Unicode codepoint and UTF-8 string).
- `virtual void on_focus(bool focused)`: Invoked when the widget gains or loses keyboard input focus.
- `virtual bool is_focusable() const`: Indicates whether this widget can accept keyboard input focus (default: `false`).
- `virtual void update(float dt)`: Advances widget animations and timers.
- `virtual void render(Renderer2D& renderer) = 0`: Draws widget to OpenGL frame.
- `virtual bool is_visible() const`: Returns `true` if widget is visible.
- `virtual bool is_enabled() const`: Returns `true` if widget is interactive.

---

### 4.9 Automatic Layout Containers (`arin::Layout`, `arin::VBox`, `arin::HBox`, `arin::LayoutJustify`)

Eliminates manual pixel coordinate calculations by automatically arranging child widgets linearly with customizable spacing, padding, cross-axis alignment, main-axis justification, equal distribution, and container boundary validation. Reaches Flutter- and Qt-level ergonomics.

```cpp
#include <arin/layout.hpp>
```

#### Layout Orientations, Alignments & Justification
- `enum class LayoutOrientation : uint8_t { Vertical, Horizontal };`
- `enum class LayoutAlignment : uint8_t { Start, Center, End, Stretch };`
- `enum class LayoutJustify : uint8_t { Start, Center, End, SpaceBetween, SpaceAround, SpaceEvenly };`

#### Container Classes
- **`arin::Layout`**: General linear layout container.
- **`arin::VBox`**: Vertical container stacking children top-to-bottom.
- **`arin::HBox`**: Horizontal container placing children left-to-right.

#### Fluent Configuration
- `set_spacing(float spacing)`: Sets pixel gap between adjacent child widgets.
- `set_padding(const Padding& padding)` / `set_padding(float uniform)`: Sets inner padding offsets.
- `set_alignment(LayoutAlignment alignment)`: Configures cross-axis alignment (`Start`, `Center`, `End`, `Stretch`).
- `set_justify(LayoutJustify justify)`: Configures main-axis distribution (`Start`, `Center`, `End`, `SpaceBetween`, `SpaceAround`, `SpaceEvenly`).
- `set_auto_size(bool enable)`: Automatically resizes layout bounding box to wrap its children (default: true).
- `distribute_children_equally()`: Automatically resizes all children along the main axis to uniformly share available space within a fixed container size.
- `set_validation_logging(bool enable)`: Enables automatic console error/warning logging whenever a layout collision or boundary overflow occurs during layout updates.

#### Layout Validation & Diagnostics
- `LayoutValidationResult validate() const`: Analyzes the container and children geometry to ensure zero overlaps between sibling widgets and zero overflow beyond container boundaries.
- `const LayoutValidationResult& last_validation_result() const`: Returns the result from the most recent validation pass.
- `struct LayoutIssue`: Contains severity (`Warning`, `Error`) and descriptive diagnostics.
- `struct LayoutValidationResult`: Holds `bool is_valid`, `has_errors()`, and vector of `LayoutIssue` diagnostics.

#### Child Management
- `add_widget(std::shared_ptr<IWidget> widget)`: Appends an existing polymorphic widget.
- `add_button(const std::string& label, float width = 85.0f, float height = 32.0f)`: Creates and appends a `Button`.
- `add_progress_bar(float width = 260.0f, float height = 18.0f, float value = 0.0f)`: Creates and appends a `ProgressBar`.
- `add_list_box(float width = 220.0f, float height = 180.0f, ListBoxMode mode = ListBoxMode::Standard)`: Creates and appends a `ListBox`.
- `add_check_list_box(float width = 220.0f, float height = 180.0f)`: Creates and appends a `CheckListBox`.
- `add_vbox(float spacing = 8.0f)` / `add_hbox(float spacing = 8.0f)`: Appends a nested layout container (supports arbitrary recursion).
- `add_spacer(float size)`: Inserts a fixed empty space along the layout axis.
- `clear()`: Removes all managed children.
- `update_layout()`: Recalculates all child positions and sizes, and runs validation if logging is enabled. Automatically called before rendering and input dispatching.

---

### 4.10 List Box Widgets (`arin::ListBox`, `arin::CheckListBox`, `arin::ListBoxMode`, `arin::ListBoxStyle`)

High-performance, scrollable single-selection and checkbox lists designed after Windows 7/10 dialog lists.

```cpp
#include <arin/list_box.hpp>
```

#### Operating Modes
- `ListBoxMode::Standard`: Single-item selection with Accent Blue (`#0067C0`) background highlight and white typography.
- `ListBoxMode::CheckBox`: Check box list where each row contains an independent, interactive checkbox with crisp vector check marks.

#### Constructors
- `ListBox()`: Default list box.
- `ListBox(float x, float y, float width = 220.0f, float height = 180.0f, ListBoxMode mode = ListBoxMode::Standard)`
- `CheckListBox(float x, float y, float width = 220.0f, float height = 180.0f)`: Dedicated CheckBox list subclass.

#### Item Management
- `add_item(const std::string& text, bool checked = false)`: Appends a row item.
- `add_items(const std::vector<std::string>& items)`: Appends multiple items.
- `set_items(const std::vector<std::string>& items)`: Replaces entire item collection.
- `clear_items()`: Clears all items.
- `size_t item_count() const`: Number of items.
- `const std::string& item_text(size_t index) const`: Text of item at index.

#### Selection & Check State
- `int selected_index() const`: Current highlighted row index (-1 if none).
- `std::string selected_item() const`: Text of current selected item.
- `set_selected_index(int index)`: Selects specified row index.
- `bool is_checked(size_t index) const`: Check state of row.
- `set_checked(size_t index, bool checked)`: Sets checked state.
- `toggle_checked(size_t index)`: Inverts checked state.
- `std::vector<int> checked_indices() const`: List of all checked row indices.
- `std::vector<std::string> checked_items() const`: List of all checked item labels.

#### Callbacks
- `on_selection_changed(std::function<void(int index, const std::string& text)> cb)`: Fired on row selection.
- `on_item_toggled(std::function<void(int index, bool checked)> cb)`: Fired on checkbox toggle.

#### Visual Styling & Colors (`ListBoxStyle`)
Full visual customization of card borders, row highlights, text colors, checkbox states, and scrollbar appearance:
- `set_style(const ListBoxStyle& style)`: Replaces full style configuration.
- `set_selection_color(const Color& color)`: Changes highlight color for selected row.
- `set_text_color(const Color& color)`: Changes standard row text color.
- `set_background_color(const Color& color)`: Changes container card background.
- `set_border_color(const Color& color)`: Changes container border outline.
- Built-in presets:
  - `ListBoxStyle::light()`: Default clean Windows 10 look (white card, `#D1D5DB` border, `#0067C0` accent).
  - `ListBoxStyle::dark()`: Deep dark mode (`#1F2937` card, `#374151` border, `#2563EB` accent).
  - `ListBoxStyle::accent(const Color& color)`: Customizes primary accent color for selection and checked boxes.

#### Scrolling & Geometry
- `bool needs_scrollbar() const`: True if items overflow visible container height.
- `float scroll_offset() const` / `float max_scroll_offset() const`: Scroll pixel positions.
- Smooth mouse wheel scrolling and interactive thumb dragging.
- **Symmetrical Geometry**: The gap between the row selection highlight and the scrollbar exactly matches the scrollbar's margin to the right border (`m_style.item_padding = 4.0f`), guaranteeing perfect visual symmetry.
- Content rendering strictly clipped to card boundaries via `glScissor`.

---

### 4.11 Image & Texture Support (`arin::Texture`, `arin::Image`, `arin::TextureFilter`, `arin::ImageScaleMode`)

Arin32 includes a built-in, OS-agnostic texture loading engine and GPU image pipeline. Using the embedded `stb_image` decoder, images in standard formats (PNG, JPEG, BMP, TGA) are loaded directly into OpenGL 2D textures without dynamic library dependencies.

```cpp
#include <arin/texture.hpp>
#include <arin/image.hpp>
```

#### Texture Class (`arin::Texture`)
- `create_from_file(const std::string& path, TextureFilter filter = TextureFilter::Linear)`:
  Loads and uploads an image from the filesystem.
- `create_from_memory(const uint8_t* data, size_t size, TextureFilter filter = TextureFilter::Linear)`:
  Decodes and uploads an image from an in-memory byte buffer (useful for embedded assets or virtual filesystems).
- `create_from_rgba(int width, int height, const uint8_t* rgba, TextureFilter filter = TextureFilter::Linear)`:
  Creates an OpenGL texture from uncompressed 32-bit RGBA pixel buffers.
- `create_empty(int width, int height, TextureFilter filter = TextureFilter::Linear)`:
  Allocates uninitialized GPU texture storage.
- `update_sub_rect(int x, int y, int w, int h, const uint8_t* rgba)`:
  Uploads modified pixels to a specific rectangular sub-region.
- `bool is_valid() const`: Checks if GPU texture handle is valid.
- `int width() const`, `int height() const`, `float aspect_ratio() const`: Image geometry.

#### Texture Filtering (`TextureFilter`)
- `TextureFilter::Linear`: Bilinear interpolation for smooth scaling (photos, icons, UI graphics).
- `TextureFilter::Nearest`: Point sampling for pixel art and pixel-exact graphics.

#### Image Widget (`arin::Image`)
Conforms to the `IWidget` interface, allowing direct integration into layouts (`VBox`, `HBox`) or absolute positioning:
- `set_texture(std::shared_ptr<Texture> texture)`: Binds texture to widget.
- `load_from_file(const std::string& filepath)`: Loads and sets an image file directly.
- `set_scale_mode(ImageScaleMode mode)`: Configures aspect-ratio scaling behavior:
  - `ImageScaleMode::Fit`: Scales uniformly to fit entirely within bounds (letterbox/pillarbox).
  - `ImageScaleMode::Stretch`: Stretches non-uniformly to exactly fill container bounds.
  - `ImageScaleMode::Fill`: Scales uniformly to completely cover bounds, clipping any excess.
  - `ImageScaleMode::Center`: Keeps native pixel dimensions centered in bounds.
- `set_corner_radius(float radius)`: GPU Signed Distance Field (SDF) rounded corners, rendering razor-sharp curved image borders with zero clipping masks.
- `set_tint(const Color& tint)`: Modulates image color and alpha transparency.

---

### 4.11b Scalable Vector Graphics (`arin::SvgDocument`, `arin::SvgImage`, and Icon Arsenal)

Arin32 includes native, resolution-independent Scalable Vector Graphics (SVG) support powered by the embedded, lightweight NanoSVG engine (`src/thirdparty/nanosvg.h`, `src/thirdparty/nanosvgrast.h`). The implementation has zero dynamic library dependencies and renders crisp, anti-aliased vector artwork at arbitrary DPI settings and window scales.

```cpp
#include <arin/svg.hpp>
```

#### Key Architecture & Capabilities

- **Lightweight & Header-Only Engine**: SVG parsing and software rasterization are handled entirely in-engine with no external dynamic dependencies.
- **Dynamic Resolution-Independent Rasterization**: Instead of statically scaling fixed bitmap textures, `SvgImage` automatically rasterizes vector paths at the exact physical pixel dimensions required by its bounding box, avoiding pixelation, blurriness, or scaling artifacts.
- **Full Layout & Widget Integration**: `SvgImage` conforms to `arin::IWidget`, meaning it integrates seamlessly into `arin::VBox`, `arin::HBox`, and absolute positioning systems.
- **GPU Accelerated Post-Processing**: Rasterized vector textures support fragment-shader Signed Distance Field (SDF) rounded corners (`set_corner_radius`) and dynamic color tinting (`set_tint`).
- **Standard Scale Modes**: Supports `ImageScaleMode::Fit`, `Fill`, `Stretch`, and `Center` preserving or adapting native aspect ratios.

#### SVG Document Class (`arin::SvgDocument`)

Represents parsed SVG vector paths and shapes:

- `static std::shared_ptr<SvgDocument> load_from_file(const std::string& filepath, float dpi = 96.0f)`:
  Loads and parses an SVG file from the filesystem.
- `static std::shared_ptr<SvgDocument> load_from_memory(const std::string& svg_xml, float dpi = 96.0f)`:
  Parses an SVG document from a UTF-8 XML string in memory.
- `static std::shared_ptr<SvgDocument> load_from_memory(const uint8_t* data, size_t size_bytes, float dpi = 96.0f)`:
  Parses an SVG document from a raw contiguous byte buffer.
- `bool is_valid() const`:
  Returns `true` if vector shapes were parsed successfully.
- `float width() const`, `float height() const`, `float aspect_ratio() const`:
  Native canvas dimensions defined by SVG attributes or `viewBox`.
- `std::vector<uint8_t> rasterize_rgba(int target_width = 0, int target_height = 0, float scale = 1.0f, int* out_width = nullptr, int* out_height = nullptr) const`:
  Rasterizes vector paths into 32-bit RGBA pixel bytes at the requested pixel resolution.
- `std::shared_ptr<Texture> create_texture(int target_width = 0, int target_height = 0, float scale = 1.0f, TextureFilter filter = TextureFilter::Linear) const`:
  Rasterizes and uploads the vector graphic directly to an OpenGL 2D texture.

#### SVG Image Widget (`arin::SvgImage`)

High-level widget for displaying vector graphics within user interfaces:

- **Constructors**:
  - `SvgImage()`: Empty vector image widget.
  - `SvgImage(std::shared_ptr<SvgDocument> doc, float x, float y, float width, float height, ImageScaleMode scale_mode = ImageScaleMode::Fit)`
  - `SvgImage(const std::string& filepath, float x, float y, float width, float height, ImageScaleMode scale_mode = ImageScaleMode::Fit)`
- **Configuration & Chaining**:
  - `set_document(std::shared_ptr<SvgDocument> doc)`: Replaces active vector document.
  - `load_from_file(const std::string& filepath)`: Loads and sets an SVG file from disk.
  - `load_from_memory(const std::string& svg_xml)`: Loads an SVG from an XML string.
  - `set_scale_mode(ImageScaleMode mode)`: Sets scaling algorithm (`Fit`, `Fill`, `Stretch`, `Center`).
  - `set_corner_radius(float radius)`: Applies fragment-shader SDF rounded corners.
  - `set_tint(const Color& tint)`: Modulates color channels and transparency.
  - `invalidate_texture()`: Clears the cached rasterization and triggers a redraw on next frame.
  - `texture()`: Returns current cached GPU texture.
  - `compute_content_rect()`: Computes destination rectangle inside widget bounds according to active scale mode.

#### Texture & App Helpers

- `Texture::create_from_svg_file(const std::string& filepath, int target_w = 0, int target_h = 0, float scale = 1.0f)`:
  Directly decodes and rasterizes an SVG file into a GPU texture.
- `Texture::create_from_svg_memory(const uint8_t* svg_data, size_t size, int target_w = 0, int target_h = 0, float scale = 1.0f)`:
  Decodes and rasterizes an in-memory SVG into a GPU texture.
- `Texture::create_from_file(path)`:
  Automatically checks for `.svg` file extensions and delegates to SVG rasterization transparently.
- `app.add_svg_image(filepath, x, y, width, height, scale_mode)`:
  Instantiates and registers an `SvgImage` widget into the application.
- `app.add_svg_image(document, x, y, width, height, scale_mode)`:
  Instantiates an `SvgImage` widget from an existing parsed `SvgDocument`.

#### Icon Asset Arsenal (`assets/icons/`)

Arin32 includes a curated collection of 38 handcrafted, production-ready SVG vector icons designed for constructing file managers, navigation sidebars, and desktop explorer interfaces (modeled after modern desktop operating system file explorers):

- **Folders & Locations**:
  - `folder.svg`: Standard closed folder.
  - `folder-open.svg`: Expanded directory folder.
  - `folder-downloads.svg`: Downloads folder with directional badge.
  - `folder-documents.svg`: Documents directory with sheet badge.
  - `folder-pictures.svg`: Pictures directory with image frame badge.
  - `folder-music.svg`: Music folder with eighth note badge.
  - `folder-videos.svg`: Videos folder with film reel badge.
  - `folder-desktop.svg`: Desktop directory with monitor display badge.
  - `folder-work.svg`: Work directory with briefcase badge.
  - `quick-access.svg`: Blue Quick Access pin navigation glyph.
  - `this-pc.svg`: Computer workstation system drive entry.
  - `drive-harddisk.svg`: Storage drive disk indicator.
  - `cloud.svg`: Cloud storage synchronization location.
  - `network.svg`: Local network share and workstation node.
  - `trash.svg`: Recycle bin and file deletion container.
- **File Formats**:
  - `file.svg`: Generic file document sheet.
  - `file-text.svg`: Plain text document (`.txt`, `.log`).
  - `file-doc.svg`: Rich text document (`.doc`, `.docx`, `.odt`).
  - `file-sheet.svg`: Spreadsheet table (`.xlsx`, `.csv`).
  - `file-image.svg`: Bitmap / vector graphic file (`.png`, `.jpg`).
  - `file-audio.svg`: Sound / audio recording (`.mp3`, `.wav`).
  - `file-video.svg`: Video recording (`.mp4`, `.mkv`).
  - `file-archive.svg`: Compressed archive (`.zip`, `.tar.gz`, `.7z`).
  - `file-code.svg`: Source code file (`.cpp`, `.hpp`, `.py`).
  - `file-pdf.svg`: Portable Document Format file (`.pdf`).
- **Navigation, Badges & View Controls**:
  - `arrow-left.svg`, `arrow-right.svg`, `arrow-up.svg`: Navigation toolbar direction arrows.
  - `refresh.svg`: Address bar refresh and reload button.
  - `search.svg`: Search input box magnifying lens.
  - `pin.svg`, `pin-filled.svg`: Pinning items to Quick Access.
  - `check-badge.svg`: Green synchronized / verified status badge.
  - `sync-badge.svg`: Blue pending cloud synchronization status badge.
  - `chevron-down.svg`, `chevron-right.svg`: Directory treeview expandable chevrons.
  - `view-grid.svg`, `view-list.svg`: Explorer layout presentation toggle icons.
- **Desktop Actions & Context Menu**:
  - `cut.svg`: Scissors icon for cut action.
  - `copy.svg`: Overlapping document sheets for copy action.
  - `paste.svg`: Clipboard with paper sheet for paste action.
  - `rename.svg`: Selection box with text cursor and pencil for file renaming.
  - `properties.svg`: Document sheet with adjustment sliders for item properties.
  - `shortcut.svg`: File sheet with bottom-left curved arrow badge for shortcut creation.
  - `shield.svg`: Green security shield with checkmark for antivirus and security actions.

#### Example: Building a File Explorer Row

```cpp
#include <arin/arin.hpp>

// Create a horizontal row layout for a file explorer item
auto row = app.add_hbox(20.0f, 40.0f, 10.0f);
row->set_size(320.0f, 28.0f);

// Add SVG folder icon
auto icon = std::make_shared<arin::SvgImage>(
    "assets/icons/folder-documents.svg",
    0.0f, 0.0f, 20.0f, 20.0f
);
row->add_widget(icon);

// Add action button for the folder name
auto btn = row->add_button("Documents", 240.0f, 24.0f);
btn->on_click([]() {
    // Open directory
});
```

---

### 4.12 Vector Icons (`arin::Icon`, `arin::IconType`)

Arin32 features a mathematically defined vector icon system. Icons are rendered via hardware-accelerated Signed Distance Fields and thick anti-aliased geometric primitives. They scale losslessly to any resolution or DPI and adapt instantly to any color tint.

```cpp
#include <arin/icon.hpp>
```

#### Supported Icon Glyphs (`arin::IconType`)
- `IconType::Folder`: Desktop file folder glyph.
- `IconType::File`: Document sheet with content lines.
- `IconType::Check`: Crisp confirmation checkmark.
- `IconType::Close`: Dismiss / cancel cross (X).
- `IconType::Search`: Magnifying glass with lens and diagonal handle.
- `IconType::Settings`: Gear cog wheel with radiating teeth.
- `IconType::Cut`: Scissors icon.
- `IconType::Copy`: Overlapping document sheets.
- `IconType::Paste`: Clipboard with document sheet.
- `IconType::Trash`: Wastebasket delete icon.
- `IconType::Edit`: Pencil icon.
- `IconType::More`: Horizontal ellipsis (...).
- `IconType::ChevronRight`, `ChevronDown`, `ChevronUp`, `ChevronLeft`: Directional sub-menu chevrons.
- `IconType::ArrowRight`, `ArrowLeft`: Navigational arrows with stems.
- `IconType::Info`, `Warning`, `Error`: Status badge glyphs.

#### Icon Widget (`arin::Icon`)
- `set_type(IconType type)`: Sets the vector icon to display.
- `set_color(const Color& color)`: Changes stroke and fill color.
- `set_size(float width, float height)` / `set_position(float x, float y)`: Configures geometry.

---

### 4.12a Shared Geometry, Metrics, and Text Alignment

All widgets share the same centering and spacing foundation. Custom code should
prefer these helpers instead of manual `* 0.5f` arithmetic or literal offsets.

- `arin::Rect::centered(Vec2 size)`: returns a rectangle of the requested size
  centered inside the current rectangle. Used for icons, checkbox squares, and
  button content rows.
- `arin::Rect::inset(Padding)` and `Rect::shrunk(margin)`: remove padding from
  every edge with clamping to a non-negative size. Used for content clipping.
- `arin::UiMetrics`: named defaults for widget sizes, menu and list geometry,
  label scales, and behavior limits (`kMaxFrameDeltaTime`, `kOffscreenCoordinate`,
  `kValidationTolerance`). Replaces scattered floating point literals.
- `arin::palette`: shared accent colors (`kAccentBlue`, `kProgressGreen`) so
  style factories do not duplicate hexadecimal literals.
- `arin::Font::layout_text_in_rect(text, bounds, scale, align_h, align_v)`: computes
  the drawing origin with optical vertical centering. The visible glyph mass is
  centered instead of the full line box, so labels do not appear shifted down.
- `Renderer2D::draw_text_centered` and `Renderer2D::draw_text_in_rect`: render
  through `Font::layout_text_in_rect`. Buttons, list rows, menu entries, and
  checkbox labels all use this path.
- `Layout::clear_hover()`: releases hover state on all children without
  synthetic magic coordinates at call sites.
- `ListBox::row_rect_at(index)` and `ContextMenu::row_rect_at(index)`: pure row
  geometry used by both rendering and hit testing.

```cpp
#include <arin/arin.hpp>

arin::Rect button(0.0f, 0.0f, 200.0f, 48.0f);
arin::Rect icon = button.centered(arin::Vec2(16.0f, 16.0f));
arin::Rect inner = button.inset(arin::Padding(14.0f, 6.0f));

arin::Font font;
arin::Vec2 origin = font.layout_text_in_rect("Save", button, 1.0f);
```

#### Fixed-size widgets and layout stability

Rendering never mutates widget bounds. A `Button` grows only through the explicit
sizing helpers `fit_to_text()` and `ensure_containment()`, which callers invoke
before the layout pass. Consequences:

- Layout containers keep the geometry they computed, so `SpaceBetween` gaps and
  `distribute_children_equally()` widths stay exact on every frame.
- Multi-button rows cannot overflow their parent when a label is longer than the
  space reserved for it; the label is scaled down to fit inside the button.
- `Button::set_auto_resize(false)` documents that a button must keep its assigned
  bounds. It is optional, because render does not resize either way.

```cpp
// Equal-width rows: reserve exactly the container width across all children.
auto row = app.add_hbox(40.0f, 120.0f, 10.0f);
row->set_size(440.0f, 32.0f);
row->add_button("Save", 0.0f, 32.0f)->set_auto_resize(false);
row->add_button("Don't Save", 0.0f, 32.0f)->set_auto_resize(false);
row->add_button("Cancel", 0.0f, 32.0f)->set_auto_resize(false);
row->set_justify(arin::LayoutJustify::Start);
row->distribute_children_equally();
```

#### Layout and alignment rules

- `LayoutAlignment` controls the cross axis (horizontal in `VBox`, vertical in `HBox`).
- `LayoutJustify` controls the main axis (`Start`, `Center`, `End`, `SpaceBetween`,
  `SpaceAround`, `SpaceEvenly`).
- `Stretch` is resolved before justification so distributed space uses final sizes.
- `validate()` reports overlaps and container overflows with a fixed tolerance
  (`UiMetrics::kValidationTolerance`).

---

### 4.13 Context Menu System (`arin::ContextMenu`, `arin::MenuItem`, `arin::ContextMenuStyle`)

Desktop-grade floating popup context menu triggered on right-click or programmatic invocation. Modeled 1:1 after authentic Windows 10 desktop menus with 1px border, soft drop shadow, 22px row height, soft light grey hover highlight, full SVG icon support, bold default actions, and submenu indicators.

```cpp
#include <arin/context_menu.hpp>
```

#### Key Capabilities

- **1:1 Windows 10 Styling**: Square 0px corners, crisp 1px `#CCCCCC` outline, 22px item height, `#E5E5E5` hover highlight where text remains black `#000000`, and subtle 1px inset horizontal separators.
- **Multi-Level Cascading Submenus**: Context menus support child submenus (`std::shared_ptr<ContextMenu>`). Hovering over a submenu row automatically cascades and displays the child menu adjacent to the parent row. Submenus flip horizontally to the left if space on the right is constrained.
- **Native SVG Icon Support**: Menu items accept SVG filepaths (e.g., `"assets/icons/folder-open.svg"`) or `std::shared_ptr<SvgDocument>`. SVGs are automatically rasterized at HiDPI resolution and render with their full multicolor appearance.
- **Bold Default Action**: Items marked as default (`is_default = true` or `add_default_item`) render with prominent bold text (modeled after "Abrir" / Open in Windows).
- **Submenu Indicators**: Items marked with `has_submenu = true` or `add_submenu` render a right-aligned directional chevron (`>`) indicating an expandable submenu.
- **Uniform Gutter Alignment**: When any item in the menu displays an icon, all labels are cleanly aligned to the icon column, matching operating system menus.
- **Keyboard Navigation**: Supports full keyboard traversal. Arrow Down / Arrow Up navigate items, Arrow Right expands and enters child submenus, Arrow Left closes child submenus, Enter executes the highlighted action, and Escape dismisses.
- **Elevation and Drop Shadow**: Rendered on the topmost overlay layer with an elevated Gaussian blur drop shadow.
- **Screen Boundary Clamping**: Automatically adjusts popup placement coordinates when triggered near display edges, preventing the menu from clipping outside visible window bounds.
- **Automatic Dismissal**: Dismisses automatically on outside mouse clicks, action execution, or when pressing the `Escape` key.
- **Z-Order Modal Priority**: Intercepts mouse and keyboard events before standard widgets when visible.

#### Core Classes & Types

##### `MenuItem`
Represents an individual action entry, submenu, or visual separator:
- `static MenuItem action(std::string label, std::function<void()> callback = nullptr)`
- `static MenuItem action(std::string label, const std::string& svg_path, std::function<void()> callback = nullptr)`
- `static MenuItem action(std::string label, const std::string& svg_path, std::string shortcut, std::function<void()> callback = nullptr)`
- `static MenuItem action(std::string label, std::shared_ptr<SvgDocument> doc, std::function<void()> callback = nullptr)`
- `static MenuItem action(std::string label, IconType icon, std::function<void()> callback = nullptr)`
- `static MenuItem action(std::string label, IconType icon, std::string shortcut, std::function<void()> callback = nullptr)`
- `static MenuItem submenu(std::string label, std::function<void()> callback = nullptr)`
- `static MenuItem submenu(std::string label, const std::string& svg_path, std::function<void()> callback = nullptr)`
- `static MenuItem submenu(std::string label, std::shared_ptr<ContextMenu> child, std::function<void()> callback = nullptr)`
- `static MenuItem submenu(std::string label, std::shared_ptr<ContextMenu> child, const std::string& svg_path, std::function<void()> callback = nullptr)`
- `static MenuItem submenu(std::string label, std::shared_ptr<ContextMenu> child, IconType icon, std::function<void()> callback = nullptr)`
- `static MenuItem separator()`: Horizontal divider line.
- `set_id(std::string identifier)`: Assigns a unique ID for programmatic lookups and toggles.
- `set_visible(bool visible)`: Shows or hides the individual menu entry.
- `set_default(bool def)`: Marks the item as the bold default action.
- `set_submenu(bool sub)`: Toggles the right-pointing submenu chevron.
- `set_submenu(std::shared_ptr<ContextMenu> child)`: Attaches a child submenu.
- `set_enabled(bool en)`: Enables or disables the item (disabled items render in muted grey).

##### `ContextMenu`
The floating menu widget itself:
- `add_item(...)`: Appends actions (plain, vector icon, SVG icon, shortcut text).
- `add_default_item(...)`: Appends default bold action.
- `add_submenu(...)`: Appends cascading submenu.
- `add_separator()`: Inserts divider between action groups.
- `insert_item(size_t index, MenuItem item)`: Inserts custom action at specific zero-indexed position.
- `insert_item_before(const std::string& target, MenuItem item)`: Inserts action before target item (by label or ID).
- `insert_item_after(const std::string& target, MenuItem item)`: Inserts action after target item (by label or ID).
- `insert_submenu(size_t index, const std::string& label, std::shared_ptr<ContextMenu> child)`: Inserts child submenu at index.
- `insert_separator(size_t index)`: Inserts divider line at index.
- `remove_item(size_t index)`: Removes item at index.
- `remove_item(const std::string& label)`: Removes item matching label.
- `remove_item_by_id(const std::string& id)`: Removes item matching unique identifier.
- `find_item(const std::string& label)` / `find_item_by_id(const std::string& id)`: Returns pointer to item for in-place modification.
- `has_item(const std::string& label)` / `has_item_by_id(const std::string& id)`: Checks presence of item.
- `set_item_visible(index / label / id, bool visible)`: Dynamically shows or hides items. Hidden items do not consume vertical space, cannot be hovered, and are skipped during keyboard navigation.
- `set_item_enabled(index / label / id, bool enabled)`: Enables or disables items.
- `visible_item_count()`: Returns the number of currently visible items.
- `on_before_show(std::function<void(ContextMenu&)> cb)`: Dynamic hook invoked immediately prior to popup presentation to modify or filter items based on runtime state.
- `clear()`: Removes all items.
- `last_item()`: Returns reference to most recently appended item for fluent chaining.
- `open_child_menu(int index)`: Opens cascading child submenu for item at index.
- `close_active_child()`: Closes currently active child submenu.
- `active_child_menu() const`: Returns pointer to active child submenu or nullptr.
- `show(float x, float y, float screen_width = 0.0f, float screen_height = 0.0f)`: Opens and positions popup with automatic screen clamping.
- `show(float x, float y, const Font& font, float screen_width = 0.0f, float screen_height = 0.0f)`: Preferred overload that measures labels with real font metrics.
- `row_rect_at(size_t index) const`: Returns the row rectangle used for rendering and hit testing.
- `hide()`: Closes and dismisses the context menu.
- `is_visible() const`: Checks if popup is active.
- `set_hovered_index(int index)` / `hovered_index() const`: Gets or sets hovered item index.
- `on_dismiss(std::function<void()> cb)`: Callback fired on menu closure.
- `set_style(const ContextMenuStyle& style)`: Customizes colors, corner radius, padding, and drop shadow.

##### `ContextMenuStyle`
Preset visual configurations:
- `ContextMenuStyle::windows10()`: 1:1 authentic Windows 10 desktop menu (white background, `#CCCCCC` border, `#E5E5E5` hover, black text, 0px corners, 22px item height).
- `ContextMenuStyle::dark()`: Deep dark mode (`#2B2B2B` card, `#404040` border, `#414141` hover, white text).
- `ContextMenuStyle::rounded()`: Modern rounded variant with 6px corner radius and 26px item height.

##### Application & Widget Integration
- `widget->set_context_menu(menu)`: Attaches a dedicated custom context menu to any `IWidget` (Button, ListBox, Image, TextInput, etc.).
- `widget->context_menu()`: Retrieves the widget-specific context menu.
- `auto menu = app.create_context_menu()`: Allocates and registers a context menu managed by the app.
- `app.set_default_context_menu(menu)`: Sets the fallback menu opened automatically on right-clicks anywhere in the window.
- Right-click dispatch: When right-clicking inside a widget with its own context menu, the widget-specific menu is opened. Otherwise, the window default context menu is shown.
- `app.show_context_menu(menu, x, y)`: Opens a context menu programmatically at (x, y).
- `app.close_context_menu()`: Dismisses currently open context menu.
- `app.on_context_menu(cb)`: Registers a custom callback invoked with cursor coordinates `(x, y)` on right-clicks.

---

### 4.14 `arin::Theme`

Global theme definition holding window background clear colors and default button styles.

- `static Theme& get()`: Global singleton theme.
- `static Theme dark()`: Deep navy background (`#0F172A`) with primary buttons.
- `static Theme light()`: Clean light slate background (`#F8FAFC`).

---

### 4.15 `arin::Renderer2D`

Hardware-accelerated 2D rendering engine powered by OpenGL 3.3 Core profile shaders. Internally organized into modular, decoupled sub-pipelines located under `src/renderer/`:
- **`shader_util`**: Centralized shader compilation, program linking, and 2D orthographic projection matrix calculation.
- **`rect_pipeline`**: Signed Distance Field (SDF) evaluation for anti-aliased rectangles, rounded corners, soft drop shadows, vector icons, and checkmarks.
- **`text_pipeline`**: Dynamic vertex streaming and batching for proportional typography with embedded Open Sans font atlas.
- **`progress_pipeline`**: Windows 10 modern progress bars with animated cosine shimmer sweeps and traveling marquee chunks.
- **`image_pipeline`**: Textured 2D quads with fragment-shader SDF rounded corners, UV mapping, and color modulation.
- **`renderer`**: High-level orchestrator managing viewport state, frame lifecycles (`begin_frame`, `end_frame`, `flush`), hardware scissor stack, and drawing delegation.

```cpp
#include <arin/renderer.hpp>
```

#### Core Methods
- `bool init()`: Compiles GLSL shaders, initializes VAOs/VBOs across all pipelines, and uploads font atlas.
- `void shutdown()`: Frees OpenGL resources.
- `void begin_frame(int viewport_width, int viewport_height)`: Sets orthographic projection matrix, blending modes, and resets scissor clip stack.
- `void end_frame()`: Flushes all batched geometry.
- `void flush()`: Flushes pending geometry (e.g. text quads) without ending frame.
- `void push_clip_rect(const Rect& rect)`: Pushes a scissor clipping rectangle onto the clip stack, strictly constraining all subsequent draw calls.
- `void pop_clip_rect()`: Restores previous scissor clipping boundary from the stack.
- `void clear(const Color& color)`: Clears frame with specified color.
- `void draw_rect(const Rect& rect, const Color& color)`: Draws flat rectangle.
- `void draw_rounded_rect(const Rect& rect, float corner_radius, const Color& fill_color, const Color& border_color = Color::transparent(), float border_width = 0.0f)`:
  Draws anti-aliased rounded rectangle via fragment shader Signed Distance Field.
- `void draw_shadow(const Rect& rect, float corner_radius, const Color& shadow_color, const Vec2& offset, float blur)`:
  Draws soft Gaussian drop shadow.
- `void draw_image(const Texture& texture, const Rect& dest, const Color& tint = Color::white(), float corner_radius = 0.0f)`:
  Renders a 2D texture with optional SDF anti-aliased rounded corners and color tinting.
- `void draw_icon(IconType icon, const Rect& bounds, const Color& color)`:
  Renders a crisp vector system icon at specified bounding box.
- `void draw_checkmark(const Rect& box, const Color& color, float thickness = 2.0f)`:
  Draws crisp anti-aliased checkmark inside a checkbox boundary.
- `void draw_progress_bar(const Rect& rect, float corner_radius, const Color& track_color, const Color& fill_color, const Color& border_color, float border_width, float fill_fraction, float anim_phase, bool is_indeterminate)`:
  Draws modern Windows 10 style progress bar with 1px border, determinate fill with animated cosine shimmer sweep, or indeterminate traveling marquee chunk.
- `void draw_text(const std::string& text, const Vec2& position, const Color& color, float scale = 1.0f)`:
  Batches and draws text at coordinates.
- `void draw_text_centered(const std::string& text, const Rect& bounds, const Color& color, float scale = 1.0f)`:
  Measures string and renders centered within bounding box.
- `void draw_text_centered_clipped(const std::string& text, const Rect& bounds, const Color& color, float scale = 1.0f)`:
  Renders centered text strictly clipped to bounding rectangle using `glScissor`.
- `Font& font()`: Access embedded font engine.

---

### 4.16 `arin::Font`

Zero-dependency embedded typography engine powered by **Open Sans**.

```cpp
#include <arin/font.hpp>
```

- **Embedded Typeface**: Uses Open Sans SemiBold rasterized at standard 12px UI size.
- **Offline & Standalone**: Requires no external TTF files or disk access at runtime.
- **Font Extractor Tool**: Developers can re-bake any font using `tools/font_extractor.py` (see `tools/README.md`).
- **Attribution**: Licensed under Apache 2.0; credited in `THIRDPARTY`.

- `bool init_gl()`: Allocates and uploads the embedded 256x128 font atlas texture (`GL_RED`).
- `Vec2 measure_text(const std::string& text, float scale = 1.0f) const`:
  Calculates exact width and height of rendered string in pixels.
- `GlyphInfo get_glyph(char c) const`:
  Retrieves advance, UV texture coordinates, and dimensions for character `c` (ASCII 32 to 126).
- `float line_height() const`: Base font line height in pixels (14.0px).

---

### 4.17 `arin::Window` & `arin::IPlatformBackend`

OS abstraction isolating window creation, swap buffers, clipboard, and input event polling.

- `class IPlatformBackend`: Abstract interface required for custom OS ports.
  - `set_mouse_callback(MouseCallback cb)`
  - `set_key_callback(KeyCallback cb)`
  - `set_char_callback(CharCallback cb)`
  - `set_resize_callback(ResizeCallback cb)`
  - `set_clipboard_text(const std::string& text)`
  - `get_clipboard_text() const`
- `class GlfwPlatformBackend`: Production backend for Linux (Wayland / X11) and FreeBSD.
- `class Window`: High-level wrapper wrapping `IPlatformBackend`.
  - `on_mouse_event(MouseCallback cb)`
  - `on_key_event(KeyCallback cb)`
  - `on_char_event(CharCallback cb)`
  - `on_resize(ResizeCallback cb)`
  - `set_clipboard_text(const std::string& text)`
  - `get_clipboard_text() const`

---

### 4.18 Geometric & Color Types

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
- `bool intersects(const Rect& other) const`: Geometric collision and overlap detection.
- `Rect expanded(float amount) const`

#### `Padding`
- `float left, top, right, bottom`
- Constructors: `Padding(uniform)`, `Padding(horizontal, vertical)`, `Padding(left, top, right, bottom)`

#### `Color`
- `float r, g, b, a` in normalized range `[0.0, 1.0]`.
- `static Color from_rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)`
- `static Color from_hex(uint32_t hex, bool has_alpha = false)`
- `Color with_alpha(float new_alpha) const`
- `Color lerp(const Color& other, float t) const`
- Presets: `white()`, `black()`, `transparent()`, `primary()`, `secondary()`, `success()`, `danger()`, `surface()`, `background()`.

---

### 4.19 Input System

```cpp
#include <arin/input.hpp>
```

- `enum class MouseButton : uint8_t { Left, Right, Middle };`
- `enum class InputAction : uint8_t { Release, Press, Repeat };`
- `enum class MouseEventType : uint8_t { Move, ButtonDown, ButtonUp, Scroll };`
- `struct MouseEvent`: Holds position, button, action, scroll delta, with factory helpers (`make_move`, `make_button_down`, `make_button_up`, `make_scroll`).
- `class InputState`: Tracks current cursor position and pressed button state array.
- `enum class KeyCode : uint16_t`: Platform-agnostic scan codes for alphanumeric, functional, and navigation keys.
- `enum class KeyModifier : uint8_t { None = 0, Shift = 1, Control = 2, Alt = 4, Super = 8 };`
- `struct KeyEvent`: Discrete key event with `key`, `action`, and `modifiers` (`has_shift()`, `has_ctrl()`, `has_alt()`, `has_super()`).
- `struct TextEvent`: Character input event containing decoded Unicode `codepoint` and UTF-8 `text`.

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

Copyright (c) 2026, Arin32 & ArinOS Contributors
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
