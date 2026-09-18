/*
 * Arin32 - Modern OpenGL Graphical User Interface Library
 *
 * Copyright (c) 2026, Arin32 Contributors
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
#include "theme.hpp"
#include "window.hpp"
#include "renderer.hpp"
#include "button.hpp"
#include <memory>
#include <vector>
#include <functional>

namespace arin {

/**
 * @brief High-level application orchestrator for Arin32.
 *
 * Encapsulates window lifecycle, OpenGL context initialization, rendering loop,
 * and automatic widget event dispatching into an absurdly simple API.
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
    std::vector<std::shared_ptr<Button>> m_buttons;
    FrameCallback m_custom_frame_cb;
    FrameCallback m_after_frame_cb;
    bool m_running{true};
};

} // namespace arin

#endif // ARIN32_APP_HPP
