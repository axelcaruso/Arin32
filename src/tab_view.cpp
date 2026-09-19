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

#include "arin/tab_view.hpp"
#include "arin/renderer.hpp"
#include <algorithm>
#include <stdexcept>

namespace arin {

TabView::TabView()
    : bounds_{0.0f, 0.0f, 600.0f, 400.0f} {
}

TabView::TabView(const Rect& bounds)
    : bounds_(bounds) {
}

size_t TabView::add_tab(std::string title, std::shared_ptr<IWidget> content, bool closable) {
    TabItem item;
    item.title = std::move(title);
    item.content = std::move(content);
    item.closable = closable;
    tabs_.push_back(std::move(item));

    if (active_tab_ < 0) {
        active_tab_ = 0;
    }
    return tabs_.size() - 1;
}

bool TabView::remove_tab(size_t index) {
    if (index >= tabs_.size()) {
        return false;
    }

    tabs_.erase(tabs_.begin() + static_cast<std::ptrdiff_t>(index));

    if (tabs_.empty()) {
        active_tab_ = -1;
    } else if (active_tab_ >= static_cast<int>(tabs_.size())) {
        active_tab_ = static_cast<int>(tabs_.size()) - 1;
        if (tab_changed_cb_) {
            tab_changed_cb_(static_cast<size_t>(active_tab_));
        }
    } else if (active_tab_ == static_cast<int>(index)) {
        if (tab_changed_cb_) {
            tab_changed_cb_(static_cast<size_t>(active_tab_));
        }
    }

    return true;
}

void TabView::clear_tabs() noexcept {
    tabs_.clear();
    active_tab_ = -1;
    hovered_tab_ = -1;
}

size_t TabView::tab_count() const noexcept {
    return tabs_.size();
}

void TabView::set_active_tab(size_t index) {
    if (index >= tabs_.size() || static_cast<int>(index) == active_tab_) {
        return;
    }
    active_tab_ = static_cast<int>(index);
    if (tab_changed_cb_) {
        tab_changed_cb_(index);
    }
}

int TabView::active_tab_index() const noexcept {
    return active_tab_;
}

std::shared_ptr<IWidget> TabView::active_content() const noexcept {
    if (active_tab_ >= 0 && static_cast<size_t>(active_tab_) < tabs_.size()) {
        return tabs_[static_cast<size_t>(active_tab_)].content;
    }
    return nullptr;
}

const std::string& TabView::tab_title(size_t index) const {
    if (index >= tabs_.size()) {
        throw std::out_of_range("TabView::tab_title index out of range");
    }
    return tabs_[index].title;
}

void TabView::set_tab_title(size_t index, std::string title) {
    if (index >= tabs_.size()) {
        throw std::out_of_range("TabView::set_tab_title index out of range");
    }
    tabs_[index].title = std::move(title);
}

std::shared_ptr<IWidget> TabView::tab_content(size_t index) const {
    if (index >= tabs_.size()) {
        throw std::out_of_range("TabView::tab_content index out of range");
    }
    return tabs_[index].content;
}

void TabView::set_tab_content(size_t index, std::shared_ptr<IWidget> content) {
    if (index >= tabs_.size()) {
        throw std::out_of_range("TabView::set_tab_content index out of range");
    }
    tabs_[index].content = std::move(content);
}

bool TabView::is_tab_closable(size_t index) const {
    if (index >= tabs_.size()) {
        throw std::out_of_range("TabView::is_tab_closable index out of range");
    }
    return tabs_[index].closable;
}

void TabView::set_tab_closable(size_t index, bool closable) {
    if (index >= tabs_.size()) {
        throw std::out_of_range("TabView::set_tab_closable index out of range");
    }
    tabs_[index].closable = closable;
}

void TabView::on_tab_changed(std::function<void(size_t)> cb) {
    tab_changed_cb_ = std::move(cb);
}

void TabView::on_tab_close_requested(std::function<void(size_t)> cb) {
    tab_close_cb_ = std::move(cb);
}

void TabView::set_style(const TabStyle& style) noexcept {
    style_ = style;
}

const TabStyle& TabView::style() const noexcept {
    return style_;
}

void TabView::set_font(const Font* font) noexcept {
    font_ = font;
}

const Font* TabView::font() const noexcept {
    return font_;
}

Rect TabView::content_area_bounds() const noexcept {
    return Rect{
        bounds_.x,
        bounds_.y + style_.tab_height,
        bounds_.width,
        std::max(0.0f, bounds_.height - style_.tab_height)
    };
}

void TabView::update_layout() const {
    float current_x = bounds_.x + 4.0f;
    const float y = bounds_.y + 2.0f;
    const float h = style_.tab_height - 2.0f;

    for (const auto& tab : tabs_) {
        float text_width = 0.0f;
        if (font_) {
            text_width = font_->measure_text(tab.title, style_.font_size).x;
        } else {
            text_width = static_cast<float>(tab.title.length()) * (style_.font_size * 0.58f);
        }

        const float close_btn_w = tab.closable ? 18.0f : 0.0f;
        const float tab_w = text_width + (style_.tab_padding_x * 2.0f) + close_btn_w;

        tab.computed_bounds = Rect{current_x, y, tab_w, h};

        if (tab.closable) {
            tab.computed_close_bounds = Rect{
                current_x + tab_w - style_.tab_padding_x - 14.0f,
                y + (h - 14.0f) * 0.5f,
                14.0f,
                14.0f
            };
        } else {
            tab.computed_close_bounds = Rect{0.0f, 0.0f, 0.0f, 0.0f};
        }

        current_x += tab_w + 3.0f;
    }
}

void TabView::render(Renderer2D& renderer) {
    if (!visible_) {
        return;
    }

    update_layout();

    // Render tab bar header background
    Rect bar_rect{bounds_.x, bounds_.y, bounds_.width, style_.tab_height};
    renderer.draw_rounded_rect(bar_rect, 0.0f, style_.bar_background, style_.border_color, style_.border_width);

    // Render individual tab headers
    for (size_t i = 0; i < tabs_.size(); ++i) {
        const auto& tab = tabs_[i];
        const bool is_active = (static_cast<int>(i) == active_tab_);
        const bool is_hov = (static_cast<int>(i) == hovered_tab_) || tab.is_hovered;

        Color bg = style_.tab_inactive_background;
        Color text_col = style_.text_inactive_color;

        if (is_active) {
            bg = style_.tab_active_background;
            text_col = style_.text_active_color;
        } else if (is_hov) {
            bg = style_.tab_hover_background;
            text_col = style_.text_active_color;
        }

        // Draw tab background
        renderer.draw_rounded_rect(tab.computed_bounds, style_.corner_radius, bg);

        // Draw active bottom accent line indicator
        if (is_active) {
            Rect indicator{
                tab.computed_bounds.x,
                tab.computed_bounds.y + tab.computed_bounds.height - style_.indicator_height,
                tab.computed_bounds.width,
                style_.indicator_height
            };
            renderer.draw_rect(indicator, style_.accent_indicator_color);
        }

        // Render tab label text
        const float text_x = tab.computed_bounds.x + style_.tab_padding_x;
        const float text_y = tab.computed_bounds.y + (tab.computed_bounds.height - style_.font_size) * 0.5f;

        renderer.draw_text(tab.title, Vec2{text_x, text_y}, text_col, 1.0f);

        // Render optional close 'x' button
        if (tab.closable) {
            Color close_col = tab.is_close_hovered ? Color{0.95f, 0.35f, 0.35f, 1.0f} : Color{0.60f, 0.60f, 0.65f, 1.0f};
            const float cx = tab.computed_close_bounds.x + 2.0f;
            const float cy = tab.computed_close_bounds.y;
            renderer.draw_text("x", Vec2{cx, cy}, close_col, 0.9f);
        }
    }

    // Render content area background and border
    const Rect content_rect = content_area_bounds();
    renderer.draw_rounded_rect(content_rect, 0.0f, style_.content_background, style_.border_color, style_.border_width);

    // Render active tab widget inside content area
    auto active_wid = active_content();
    if (active_wid && active_wid->is_visible()) {
        active_wid->set_bounds(content_rect);
        active_wid->render(renderer);
    }
}

bool TabView::handle_mouse(const MouseEvent& ev) {
    if (!visible_ || !enabled_) {
        return false;
    }
    if (ev.type == MouseEventType::Move) {
        return handle_mouse_move(ev.position);
    } else if (ev.type == MouseEventType::ButtonDown) {
        return handle_mouse_button(ev.button, InputAction::Press, ev.position);
    } else if (ev.type == MouseEventType::ButtonUp) {
        return handle_mouse_button(ev.button, InputAction::Release, ev.position);
    } else if (ev.type == MouseEventType::Scroll) {
        auto active_wid = active_content();
        if (active_wid && active_wid->is_enabled() && active_wid->is_visible()) {
            return active_wid->handle_mouse(ev);
        }
    }
    return false;
}

bool TabView::handle_mouse_move(const Vec2& mouse_pos) {
    if (!visible_ || !enabled_) {
        return false;
    }

    update_layout();

    int prev_hover = hovered_tab_;
    hovered_tab_ = -1;

    for (size_t i = 0; i < tabs_.size(); ++i) {
        if (tabs_[i].computed_bounds.contains(mouse_pos)) {
            hovered_tab_ = static_cast<int>(i);
            tabs_[i].is_hovered = true;
            tabs_[i].is_close_hovered = tabs_[i].closable && tabs_[i].computed_close_bounds.contains(mouse_pos);
            return true;
        } else {
            tabs_[i].is_hovered = false;
            tabs_[i].is_close_hovered = false;
        }
    }

    // If mouse is inside content area, forward to active content
    const Rect content_rect = content_area_bounds();
    if (content_rect.contains(mouse_pos)) {
        auto active_wid = active_content();
        if (active_wid && active_wid->is_enabled() && active_wid->is_visible()) {
            MouseEvent ev = MouseEvent::make_move(mouse_pos);
            return active_wid->handle_mouse(ev);
        }
    }

    return (hovered_tab_ != prev_hover);
}

bool TabView::handle_mouse_button(MouseButton button, InputAction action, const Vec2& mouse_pos) {
    if (!visible_ || !enabled_) {
        return false;
    }

    update_layout();

    if (button == MouseButton::Left && action == InputAction::Press) {
        for (size_t i = 0; i < tabs_.size(); ++i) {
            if (tabs_[i].computed_bounds.contains(mouse_pos)) {
                // Check if clicked the close button
                if (tabs_[i].closable && tabs_[i].computed_close_bounds.contains(mouse_pos)) {
                    if (tab_close_cb_) {
                        tab_close_cb_(i);
                    } else {
                        remove_tab(i);
                    }
                    return true;
                }

                // Switch active tab
                set_active_tab(i);
                return true;
            }
        }
    }

    // Forward click to active content widget
    const Rect content_rect = content_area_bounds();
    if (content_rect.contains(mouse_pos)) {
        auto active_wid = active_content();
        if (active_wid && active_wid->is_enabled() && active_wid->is_visible()) {
            MouseEvent ev = (action == InputAction::Press)
                ? MouseEvent::make_button_down(mouse_pos, button)
                : MouseEvent::make_button_up(mouse_pos, button);
            return active_wid->handle_mouse(ev);
        }
    }

    return false;
}

bool TabView::handle_key(const KeyEvent& ev) {
    return handle_key_event(ev.key, ev.action, static_cast<KeyModifier>(ev.modifiers));
}

bool TabView::handle_key_event(KeyCode key, InputAction action, KeyModifier mods) {
    if (!visible_ || !enabled_) {
        return false;
    }

    // Ctrl+Tab to cycle tabs
    if (action == InputAction::Press && key == KeyCode::Tab && (static_cast<uint8_t>(mods) & static_cast<uint8_t>(KeyModifier::Control))) {
        if (!tabs_.empty()) {
            size_t next = (static_cast<size_t>(active_tab_ + 1)) % tabs_.size();
            set_active_tab(next);
            return true;
        }
    }

    // Forward key events to active content
    auto active_wid = active_content();
    if (active_wid && active_wid->is_enabled() && active_wid->is_visible()) {
        KeyEvent ev;
        ev.key = key;
        ev.action = action;
        ev.modifiers = static_cast<uint8_t>(mods);
        return active_wid->handle_key(ev);
    }
    return false;
}

bool TabView::handle_text(const TextEvent& ev) {
    if (!visible_ || !enabled_) {
        return false;
    }
    auto active_wid = active_content();
    if (active_wid && active_wid->is_enabled() && active_wid->is_visible()) {
        return active_wid->handle_text(ev);
    }
    return false;
}

} // namespace arin
