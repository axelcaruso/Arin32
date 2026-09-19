/*
 * Arin32 - Modern OpenGL Graphical User Interface Library
 *
 * Copyright (c) 2026, Arin32 & ArinOS Contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ARIN32_APP_HPP
#define ARIN32_APP_HPP

#include "types.hpp"
#include "metrics.hpp"
#include "theme.hpp"
#include "window.hpp"
#include "renderer.hpp"
#include "widget.hpp"
#include "button.hpp"
#include "progress_bar.hpp"
#include "list_box.hpp"
#include "layout.hpp"
#include "texture.hpp"
#include "image.hpp"
#include "icon.hpp"
#include "checkbox.hpp"
#include "text_input.hpp"
#include "context_menu.hpp"
#include <memory>
#include <vector>
#include <functional>

namespace arin {

/**
 * @brief High-level application orchestrator for Arin32.
 *
 * Encapsulates window lifecycle, OpenGL context initialization, rendering loop,
 * and automatic widget event dispatching into a concise API requiring only a few lines of setup code.
 *
 * Example:
 * @code
 * #include <arin/arin.hpp>
 *
 * int main() {
 *     arin::App app("Arin32 Demo", 800, 600);
 *     auto btn = app.add_button("Click Me!", 300, 260, 200, 50);
 *     btn->on_click([]() {
 *         std::cout << "Clicked!\n";
 *     });
 *     app.run();
 * }
 * @endcode
 */
class App {
public:
    using FrameCallback = std::function<void(Renderer2D&)>;

    /**
     * @brief Creates a windowed application.
     * @param title Title text for the window.
     * @param width Initial window width in pixels (default: 800).
     * @param height Initial window height in pixels (default: 600).
     */
    App(const std::string& title = "Arin32 Application", int width = 800, int height = 600);

    ~App() = default;

    // Non-copyable
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    /**
     * @brief Adds a new button with specified label and geometry.
     *
     * Automatically registers the button for mouse events and frame rendering.
     *
     * @param label Text displayed on the button.
     * @param x Left coordinate in pixels.
     * @param y Top coordinate in pixels.
     * @param width Width in pixels.
     * @param height Height in pixels.
     * @return Shared pointer to the created Button for chaining or manipulation.
     */
    std::shared_ptr<Button> add_button(
        const std::string& label,
        float x,
        float y,
        float width,
        float height
    );

    /**
     * @brief Adds an existing button instance to the application.
     * @param button Button object to manage.
     * @return Shared pointer to the added Button.
     */
    std::shared_ptr<Button> add_button(Button button);

    /**
     * @brief Adds a pre-allocated button shared pointer.
     * @param button Shared pointer to button.
     * @return The same shared pointer.
     */
    std::shared_ptr<Button> add_button(std::shared_ptr<Button> button);

    /**
     * @brief Adds a new progress bar with specified geometry and initial value.
     *
     * Automatically registers the progress bar for frame rendering and animation updates.
     *
     * @param x Left coordinate in pixels.
     * @param y Top coordinate in pixels.
     * @param width Width in pixels.
     * @param height Height in pixels.
     * @param value Initial value (default: 0.0f).
     * @param min Minimum range value (default: 0.0f).
     * @param max Maximum range value (default: 100.0f).
     * @return Shared pointer to the created ProgressBar.
     */
    std::shared_ptr<ProgressBar> add_progress_bar(
        float x,
        float y,
        float width,
        float height,
        float value = 0.0f,
        float min = 0.0f,
        float max = 100.0f
    );

    /**
     * @brief Adds an existing progress bar instance to the application.
     * @param bar ProgressBar object to manage.
     * @return Shared pointer to the added ProgressBar.
     */
    std::shared_ptr<ProgressBar> add_progress_bar(ProgressBar bar);

    /**
     * @brief Adds a pre-allocated progress bar shared pointer.
     * @param bar Shared pointer to progress bar.
     * @return The same shared pointer.
     */
    std::shared_ptr<ProgressBar> add_progress_bar(std::shared_ptr<ProgressBar> bar);

    /**
     * @brief Adds an existing polymorphic widget to the application.
     * @param widget Widget shared pointer.
     * @return The same shared pointer.
     */
    std::shared_ptr<IWidget> add_widget(std::shared_ptr<IWidget> widget);

    /**
     * @brief Creates and adds a ListBox with specified geometry.
     */
    std::shared_ptr<ListBox> add_list_box(
        float x,
        float y,
        float width = UiMetrics::kDefaultListBoxSize.x,
        float height = UiMetrics::kDefaultListBoxSize.y,
        ListBoxMode mode = ListBoxMode::Standard
    );

    std::shared_ptr<ListBox> add_list_box(ListBox list_box);
    std::shared_ptr<ListBox> add_list_box(std::shared_ptr<ListBox> list_box);

    /**
     * @brief Creates and adds a CheckListBox (list with interactive checkboxes on each row).
     */
    std::shared_ptr<CheckListBox> add_check_list_box(
        float x,
        float y,
        float width = UiMetrics::kDefaultListBoxSize.x,
        float height = UiMetrics::kDefaultListBoxSize.y
    );

    /**
     * @brief Creates and adds an automatic vertical layout container (VBox).
     */
    std::shared_ptr<VBox> add_vbox(float x = 0.0f, float y = 0.0f, float spacing = 8.0f);

    /**
     * @brief Creates and adds an automatic horizontal layout container (HBox).
     */
    std::shared_ptr<HBox> add_hbox(float x = 0.0f, float y = 0.0f, float spacing = 8.0f);

    /**
     * @brief Adds an automatic layout container to the application.
     */
    std::shared_ptr<Layout> add_layout(std::shared_ptr<Layout> layout);

    /**
     * @brief Creates and adds an Image widget bound to a texture.
     */
    std::shared_ptr<Image> add_image(
        std::shared_ptr<Texture> texture,
        float x,
        float y,
        float width,
        float height,
        ImageScaleMode scale_mode = ImageScaleMode::Fit
    );

    /**
     * @brief Creates and adds an Image widget by loading an image file from disk.
     */
    std::shared_ptr<Image> add_image(
        const std::string& filepath,
        float x,
        float y,
        float width,
        float height,
        ImageScaleMode scale_mode = ImageScaleMode::Fit
    );

    /**
     * @brief Creates and adds a vector Icon widget.
     */
    std::shared_ptr<Icon> add_icon(
        IconType icon,
        float x,
        float y,
        float size = 16.0f,
        const Color& color = Color(240, 240, 240)
    );

    /**
     * @brief Creates and adds an interactive CheckBox widget.
     */
    std::shared_ptr<CheckBox> add_checkbox(
        const std::string& label,
        float x,
        float y,
        bool checked = false
    );
    std::shared_ptr<CheckBox> add_checkbox(CheckBox checkbox);
    std::shared_ptr<CheckBox> add_checkbox(std::shared_ptr<CheckBox> checkbox);

    /**
     * @brief Creates and adds an editable TextInput field widget.
     */
    std::shared_ptr<TextInput> add_text_input(
        const std::string& initial_text,
        float x,
        float y,
        float width = UiMetrics::kDefaultTextInputSize.x,
        float height = 32.0f
    );
    std::shared_ptr<TextInput> add_text_input(TextInput input);
    std::shared_ptr<TextInput> add_text_input(std::shared_ptr<TextInput> input);

    /**
     * @brief Creates and registers a floating context menu managed by the application.
     * @return Shared pointer to the created ContextMenu.
     */
    std::shared_ptr<ContextMenu> create_context_menu();

    /**
     * @brief Displays an active context menu at (x, y).
     */
    void show_context_menu(std::shared_ptr<ContextMenu> menu, float x, float y);

    /**
     * @brief Dismisses any currently visible context menu.
     */
    void close_context_menu();

    /// @brief Gets the currently active context menu, or nullptr if none is open.
    std::shared_ptr<ContextMenu> active_context_menu() const { return m_active_context_menu; }

    /**
     * @brief Registers a callback invoked on right click with cursor position (x, y).
     */
    void on_context_menu(std::function<void(float x, float y)> cb);

    /**
     * @brief Configures a default context menu automatically opened on right clicks.
     */
    void set_default_context_menu(std::shared_ptr<ContextMenu> menu);

    /**
     * @brief Sets keyboard focus to a specific widget.
     */
    void set_focus(std::shared_ptr<IWidget> widget);

    /// @brief Gets currently focused widget, or nullptr.
    std::shared_ptr<IWidget> focused_widget() const { return m_focused_widget; }

    /**
     * @brief Registers an optional custom rendering hook called every frame.
     *
     * Allows drawing custom shapes, text, or background elements alongside buttons.
     *
     * @param cb Callback function accepting (Renderer2D&).
     */
    void on_frame(FrameCallback cb);

    /**
     * @brief Registers an optional callback executed after all buttons have rendered.
     *
     * Ideal for tooltips, debug overlays, or automated screenshot capture.
     *
     * @param cb Callback function accepting (Renderer2D&).
     */
    void on_after_frame(FrameCallback cb);

    /**
     * @brief Runs the main loop until the window is closed.
     *
     * Handles polling inputs, clearing background, rendering buttons, and swapping buffers.
     */
    void run();

    /**
     * @brief Requests termination of the application run loop.
     */
    void close();

    /// @brief Gets reference to the application's window wrapper.
    Window& window() { return m_window; }

    /// @brief Gets reference to the 2D OpenGL renderer.
    Renderer2D& renderer() { return m_renderer; }

    /// @brief Gets reference to the active theme.
    Theme& theme() { return m_theme; }

private:
    Window m_window;
    Renderer2D m_renderer;
    Theme m_theme{Theme::dark()};
    std::vector<std::shared_ptr<IWidget>> m_widgets;
    std::shared_ptr<IWidget> m_focused_widget{nullptr};
    std::shared_ptr<ContextMenu> m_active_context_menu{nullptr};
    std::shared_ptr<ContextMenu> m_default_context_menu{nullptr};
    std::vector<std::shared_ptr<ContextMenu>> m_context_menus;
    std::function<void(float x, float y)> m_context_menu_cb;
    FrameCallback m_custom_frame_cb;
    FrameCallback m_after_frame_cb;
    bool m_running{true};
};

} // namespace arin

#endif // ARIN32_APP_HPP
