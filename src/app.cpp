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
#include "arin/metrics.hpp"
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

    // Set up event routing to managed widgets
    m_window.on_mouse_event([this](const MouseEvent& ev) {
        // 1. Give active context menu modal priority
        if (m_active_context_menu && m_active_context_menu->is_visible()) {
            if (ev.type == MouseEventType::ButtonDown) {
                if (!m_active_context_menu->bounds().contains(ev.position)) {
                    close_context_menu();
                    // Fall through so click can activate target widget
                } else {
                    m_active_context_menu->handle_mouse(ev);
                    return;
                }
            } else {
                if (m_active_context_menu->handle_mouse(ev)) {
                    return;
                }
            }
        }

        // 2. Right-click context menu trigger
        if (ev.type == MouseEventType::ButtonDown && ev.button == MouseButton::Right) {
            if (m_context_menu_cb) {
                m_context_menu_cb(ev.position.x, ev.position.y);
                return;
            } else if (m_default_context_menu) {
                show_context_menu(m_default_context_menu, ev.position.x, ev.position.y);
                return;
            }
        }

        // Update focus on left mouse click
        if (ev.type == MouseEventType::ButtonDown && ev.button == MouseButton::Left) {
            std::shared_ptr<IWidget> clicked_focusable = nullptr;
            for (auto it = m_widgets.rbegin(); it != m_widgets.rend(); ++it) {
                if ((*it)->is_visible() && (*it)->bounds().contains(ev.position)) {
                    if ((*it)->is_focusable()) {
                        clicked_focusable = *it;
                    }
                    break;
                }
            }
            set_focus(clicked_focusable);
        }

        bool captured = false;

        // Traverse in reverse z-order (top-most widget gets first priority)
        for (auto it = m_widgets.rbegin(); it != m_widgets.rend(); ++it) {
            if ((*it)->handle_mouse(ev)) {
                captured = true;

                // When cursor is inside a top widget, ensure all other widgets unhover
                if (ev.type == MouseEventType::Move) {
                    MouseEvent offscreen_ev = ev;
                    offscreen_ev.position = Vec2(UiMetrics::kOffscreenCoordinate, UiMetrics::kOffscreenCoordinate);
                    for (auto& other : m_widgets) {
                        if (other != *it) {
                            other->handle_mouse(offscreen_ev);
                        }
                    }
                }
                break;
            }
        }

        // If not captured during a move event, notify all widgets of cursor position
        if (!captured && ev.type == MouseEventType::Move) {
            for (auto& w : m_widgets) {
                w->handle_mouse(ev);
            }
        }
    });

    // Set up keyboard key event dispatching to context menu or focused widget
    m_window.on_key_event([this](const KeyEvent& ev) {
        if (m_active_context_menu && m_active_context_menu->is_visible()) {
            if (m_active_context_menu->handle_key(ev)) {
                return;
            }
        }
        if (m_focused_widget && m_focused_widget->is_enabled() && m_focused_widget->is_visible()) {
            m_focused_widget->handle_key(ev);
        }
    });

    // Set up text input event dispatching to the focused widget
    m_window.on_char_event([this](const TextEvent& ev) {
        if (m_focused_widget && m_focused_widget->is_enabled() && m_focused_widget->is_visible()) {
            m_focused_widget->handle_text(ev);
        }
    });
}

std::shared_ptr<IWidget> App::add_widget(std::shared_ptr<IWidget> widget) {
    m_widgets.push_back(widget);
    return widget;
}

std::shared_ptr<Button> App::add_button(
    const std::string& label,
    float x,
    float y,
    float width,
    float height
) {
    auto btn = std::make_shared<Button>(label, x, y, width, height);
    m_widgets.push_back(btn);
    return btn;
}

std::shared_ptr<Button> App::add_button(Button button) {
    auto btn = std::make_shared<Button>(std::move(button));
    m_widgets.push_back(btn);
    return btn;
}

std::shared_ptr<Button> App::add_button(std::shared_ptr<Button> button) {
    m_widgets.push_back(button);
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
    m_widgets.push_back(bar);
    return bar;
}

std::shared_ptr<ProgressBar> App::add_progress_bar(ProgressBar bar) {
    auto ptr = std::make_shared<ProgressBar>(std::move(bar));
    m_widgets.push_back(ptr);
    return ptr;
}

std::shared_ptr<ProgressBar> App::add_progress_bar(std::shared_ptr<ProgressBar> bar) {
    m_widgets.push_back(bar);
    return bar;
}

std::shared_ptr<ListBox> App::add_list_box(
    float x,
    float y,
    float width,
    float height,
    ListBoxMode mode
) {
    auto lb = std::make_shared<ListBox>(x, y, width, height, mode);
    m_widgets.push_back(lb);
    return lb;
}

std::shared_ptr<ListBox> App::add_list_box(ListBox list_box) {
    auto ptr = std::make_shared<ListBox>(std::move(list_box));
    m_widgets.push_back(ptr);
    return ptr;
}

std::shared_ptr<ListBox> App::add_list_box(std::shared_ptr<ListBox> list_box) {
    m_widgets.push_back(list_box);
    return list_box;
}

std::shared_ptr<CheckListBox> App::add_check_list_box(
    float x,
    float y,
    float width,
    float height
) {
    auto clb = std::make_shared<CheckListBox>(x, y, width, height);
    m_widgets.push_back(clb);
    return clb;
}

std::shared_ptr<VBox> App::add_vbox(float x, float y, float spacing) {
    auto vbox = std::make_shared<VBox>(x, y, spacing);
    m_widgets.push_back(vbox);
    return vbox;
}

std::shared_ptr<HBox> App::add_hbox(float x, float y, float spacing) {
    auto hbox = std::make_shared<HBox>(x, y, spacing);
    m_widgets.push_back(hbox);
    return hbox;
}

std::shared_ptr<Layout> App::add_layout(std::shared_ptr<Layout> layout) {
    m_widgets.push_back(layout);
    return layout;
}

std::shared_ptr<Image> App::add_image(
    std::shared_ptr<Texture> texture,
    float x,
    float y,
    float width,
    float height,
    ImageScaleMode scale_mode
) {
    auto img = std::make_shared<Image>(std::move(texture), x, y, width, height, scale_mode);
    m_widgets.push_back(img);
    return img;
}

std::shared_ptr<Image> App::add_image(
    const std::string& filepath,
    float x,
    float y,
    float width,
    float height,
    ImageScaleMode scale_mode
) {
    auto img = std::make_shared<Image>(filepath, x, y, width, height, scale_mode);
    m_widgets.push_back(img);
    return img;
}

std::shared_ptr<Icon> App::add_icon(
    IconType icon,
    float x,
    float y,
    float size,
    const Color& color
) {
    auto ic = std::make_shared<Icon>(icon, x, y, size, color);
    m_widgets.push_back(ic);
    return ic;
}

std::shared_ptr<CheckBox> App::add_checkbox(
    const std::string& label,
    float x,
    float y,
    bool checked
) {
    auto cb = std::make_shared<CheckBox>(label, x, y, checked);
    cb->fit_to_content(m_renderer.font());
    m_widgets.push_back(cb);
    return cb;
}

std::shared_ptr<CheckBox> App::add_checkbox(CheckBox checkbox) {
    auto cb = std::make_shared<CheckBox>(std::move(checkbox));
    if (cb->is_auto_resize()) {
        cb->fit_to_content(m_renderer.font());
    }
    m_widgets.push_back(cb);
    return cb;
}

std::shared_ptr<CheckBox> App::add_checkbox(std::shared_ptr<CheckBox> checkbox) {
    if (checkbox) {
        if (checkbox->is_auto_resize()) {
            checkbox->fit_to_content(m_renderer.font());
        }
        m_widgets.push_back(checkbox);
    }
    return checkbox;
}

std::shared_ptr<TextInput> App::add_text_input(
    const std::string& initial_text,
    float x,
    float y,
    float width,
    float height
) {
    auto input = std::make_shared<TextInput>(initial_text, x, y, width, height);
    input->set_clipboard_provider(
        [this]() { return m_window.get_clipboard_text(); },
        [this](const std::string& text) { m_window.set_clipboard_text(text); }
    );
    m_widgets.push_back(input);
    return input;
}

std::shared_ptr<TextInput> App::add_text_input(TextInput input) {
    auto ptr = std::make_shared<TextInput>(std::move(input));
    ptr->set_clipboard_provider(
        [this]() { return m_window.get_clipboard_text(); },
        [this](const std::string& text) { m_window.set_clipboard_text(text); }
    );
    m_widgets.push_back(ptr);
    return ptr;
}

std::shared_ptr<TextInput> App::add_text_input(std::shared_ptr<TextInput> input) {
    if (input) {
        input->set_clipboard_provider(
            [this]() { return m_window.get_clipboard_text(); },
            [this](const std::string& text) { m_window.set_clipboard_text(text); }
        );
        m_widgets.push_back(input);
    }
    return input;
}

std::shared_ptr<ContextMenu> App::create_context_menu() {
    auto menu = std::make_shared<ContextMenu>();
    m_context_menus.push_back(menu);
    return menu;
}

void App::show_context_menu(std::shared_ptr<ContextMenu> menu, float x, float y) {
    if (m_active_context_menu && m_active_context_menu != menu) {
        m_active_context_menu->hide();
    }
    m_active_context_menu = menu;
    if (m_active_context_menu) {
        // Measure with the live renderer font so the popup width matches the
        // typography actually drawn on screen.
        const Vec2 fb = m_window.framebuffer_size();
        m_active_context_menu->show(x, y, m_renderer.font(), fb.x, fb.y);
    }
}

void App::close_context_menu() {
    if (m_active_context_menu) {
        m_active_context_menu->hide();
        m_active_context_menu = nullptr;
    }
}

void App::on_context_menu(std::function<void(float x, float y)> cb) {
    m_context_menu_cb = std::move(cb);
}

void App::set_default_context_menu(std::shared_ptr<ContextMenu> menu) {
    m_default_context_menu = menu;
}

void App::set_focus(std::shared_ptr<IWidget> widget) {
    if (m_focused_widget == widget) return;

    if (m_focused_widget) {
        m_focused_widget->on_focus(false);
    }
    m_focused_widget = widget;
    if (m_focused_widget) {
        m_focused_widget->on_focus(true);
    }
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
 * - Animation updates for all managed widgets
 * - Screen clearing
 * - 2D projection matrix sizing
 * - Custom frame hooks
 * - Rendering all widgets in z-order
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
        if (dt > UiMetrics::kMaxFrameDeltaTime) {
            dt = UiMetrics::kMaxFrameDeltaTime;
        }

        // 1. Process OS window and input events
        m_window.poll_events();

        // 2. Advance animations/state for all managed widgets
        for (auto& w : m_widgets) {
            w->update(dt);
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

            // 7. Render all UI widgets
            for (auto& w : m_widgets) {
                w->render(m_renderer);
            }

            // 8. Render active floating context menu on top of all widgets
            if (m_active_context_menu && m_active_context_menu->is_visible()) {
                m_active_context_menu->render(m_renderer);
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
