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

#include "arin/app.hpp"
#include <iostream>
#include <chrono>

namespace arin {

/**
 * @brief Initializes window, renderer, and automatic mouse event distribution.
 */
App::App(const std::string& title, int width, int height)
    : m_window(title, width, height) {

    // Initialize OpenGL shaders, buffers, and font atlas
    if (!m_renderer.init()) {
        std::cerr << "[Arin32::App] Failed to initialize OpenGL 2D renderer." << std::endl;
    }

    // Set up event routing to managed buttons
    m_window.on_mouse_event([this](const MouseEvent& ev) {
        bool captured = false;

        // Traverse in reverse z-order (top-most button gets first priority)
        for (auto it = m_buttons.rbegin(); it != m_buttons.rend(); ++it) {
            if ((*it)->handle_mouse(ev)) {
                captured = true;

                // When cursor is inside a top button, ensure all other buttons unhover
                if (ev.type == MouseEventType::Move) {
                    MouseEvent offscreen_ev = ev;
                    offscreen_ev.position = Vec2(-99999.0f, -99999.0f);
                    for (auto& other : m_buttons) {
                        if (other != *it) {
                            other->handle_mouse(offscreen_ev);
                        }
                    }
                }
                break;
            }
        }

        // If not captured during a move event, notify all buttons of cursor position
        if (!captured && ev.type == MouseEventType::Move) {
            for (auto& btn : m_buttons) {
                btn->handle_mouse(ev);
            }
        }
    });
}

std::shared_ptr<Button> App::add_button(
    const std::string& label,
    float x,
    float y,
    float width,
    float height
) {
    auto btn = std::make_shared<Button>(label, x, y, width, height);
    m_buttons.push_back(btn);
    return btn;
}

std::shared_ptr<Button> App::add_button(Button button) {
    auto btn = std::make_shared<Button>(std::move(button));
    m_buttons.push_back(btn);
    return btn;
}

std::shared_ptr<Button> App::add_button(std::shared_ptr<Button> button) {
    m_buttons.push_back(button);
    return button;
}

std::shared_ptr<ProgressBar> App::add_progress_bar(
    float x,
    float y,
    float width,
    float height,
    float value,
    float min,
    float max
) {
    auto bar = std::make_shared<ProgressBar>(x, y, width, height, value, min, max);
    m_progress_bars.push_back(bar);
    return bar;
}

std::shared_ptr<ProgressBar> App::add_progress_bar(ProgressBar bar) {
    auto ptr = std::make_shared<ProgressBar>(std::move(bar));
    m_progress_bars.push_back(ptr);
    return ptr;
}

std::shared_ptr<ProgressBar> App::add_progress_bar(std::shared_ptr<ProgressBar> bar) {
    m_progress_bars.push_back(bar);
    return bar;
}

void App::on_frame(FrameCallback cb) {
    m_custom_frame_cb = std::move(cb);
}

void App::on_after_frame(FrameCallback cb) {
    m_after_frame_cb = std::move(cb);
}

void App::close() {
    m_running = false;
}

/**
 * @brief Executes the interactive application loop.
 *
 * Runs at display refresh rate with VSync enabled. Automatically handles:
 * - Frame delta time calculation for smooth hardware animations
 * - Operating system window events
 * - Animation updates for progress bars
 * - Screen clearing
 * - 2D projection matrix sizing
 * - Custom frame hooks
 * - Rendering progress bars and button widgets
 * - OpenGL buffer swap
 */
void App::run() {
    m_running = true;

    auto last_time = std::chrono::steady_clock::now();

    while (m_running && m_window.is_open()) {
        // Frame-rate independent delta time calculation
        auto current_time = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = current_time - last_time;
        float dt = elapsed.count();
        last_time = current_time;

        // Clamp delta time against large pauses or window moves
        if (dt > 0.1f) {
            dt = 0.1f;
        }

        // 1. Process OS window and input events
        m_window.poll_events();

        // 2. Advance animations for all managed progress bars
        for (auto& bar : m_progress_bars) {
            bar->update(dt);
        }

        // 3. Fetch current physical framebuffer dimensions
        Vec2 fb = m_window.framebuffer_size();
        int fb_w = static_cast<int>(fb.x);
        int fb_h = static_cast<int>(fb.y);

        if (fb_w > 0 && fb_h > 0) {
            // 4. Clear window surface with theme background color
            m_renderer.clear(m_theme.background_color);

            // 5. Begin 2D UI render pass
            m_renderer.begin_frame(fb_w, fb_h);

            // 6. Invoke optional user drawing callback (background layer)
            if (m_custom_frame_cb) {
                m_custom_frame_cb(m_renderer);
            }

            // 7. Render all UI progress bars
            for (auto& bar : m_progress_bars) {
                bar->render(m_renderer);
            }

            // 8. Render all UI buttons
            for (auto& btn : m_buttons) {
                btn->render(m_renderer);
            }

            // 9. Flush drawing batches
            m_renderer.end_frame();

            // 10. Invoke optional after_frame callback (overlays / screenshots)
            if (m_after_frame_cb) {
                m_after_frame_cb(m_renderer);
            }

            // 11. Present rendered frame to the display
            m_window.swap_buffers();
        }
    }
}

} // namespace arin
