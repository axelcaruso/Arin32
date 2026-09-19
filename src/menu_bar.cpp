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

#include "arin/menu_bar.hpp"
#include "arin/renderer.hpp"
#include <algorithm>

namespace arin {

MenuBar::MenuBar()
    : bounds_{0.0f, 0.0f, 800.0f, 28.0f} {
}

MenuBar::MenuBar(const Rect& bounds)
    : bounds_(bounds) {
    style_.height = bounds.height;
}

ContextMenu& MenuBar::add_menu(std::string title) {
    MenuBarItem item;
    item.title = std::move(title);
    item.menu = std::make_shared<ContextMenu>();
    
    // Wire dismiss listener to close active menu in the bar
    auto menu_ptr = item.menu.get();
    menu_ptr->on_dismiss([this, menu_ptr]() {
        if (active_index_ >= 0 && static_cast<size_t>(active_index_) < items_.size()) {
            if (items_[static_cast<size_t>(active_index_)].menu.get() == menu_ptr) {
                active_index_ = -1;
            }
        }
    });

    items_.push_back(std::move(item));
    return *items_.back().menu;
}

bool MenuBar::remove_menu(size_t index) {
    if (index >= items_.size()) {
        return false;
    }
    if (active_index_ == static_cast<int>(index)) {
        close_active_menu();
    } else if (active_index_ > static_cast<int>(index)) {
        active_index_--;
    }
    items_.erase(items_.begin() + index);
    return true;
}

const std::string& MenuBar::menu_title(size_t index) const {
    if (index >= items_.size()) {
        static const std::string empty;
        return empty;
    }
    return items_[index].title;
}

ContextMenu* MenuBar::get_menu(size_t index) noexcept {
    if (index < items_.size()) {
        return items_[index].menu.get();
    }
    return nullptr;
}

const ContextMenu* MenuBar::get_menu(size_t index) const noexcept {
    if (index < items_.size()) {
        return items_[index].menu.get();
    }
    return nullptr;
}

ContextMenu* MenuBar::find_menu(const std::string& title) noexcept {
    for (auto& item : items_) {
        if (item.title == title) {
            return item.menu.get();
        }
    }
    return nullptr;
}

const ContextMenu* MenuBar::find_menu(const std::string& title) const noexcept {
    for (const auto& item : items_) {
        if (item.title == title) {
            return item.menu.get();
        }
    }
    return nullptr;
}

size_t MenuBar::menu_count() const noexcept {
    return items_.size();
}

void MenuBar::clear() noexcept {
    close_active_menu();
    items_.clear();
}

bool MenuBar::is_open() const noexcept {
    return active_index_ >= 0;
}

void MenuBar::close_active_menu() noexcept {
    if (active_index_ >= 0 && static_cast<size_t>(active_index_) < items_.size()) {
        if (items_[static_cast<size_t>(active_index_)].menu) {
            items_[static_cast<size_t>(active_index_)].menu->hide();
        }
    }
    active_index_ = -1;
}

int MenuBar::active_menu_index() const noexcept {
    return active_index_;
}

void MenuBar::set_style(const MenuBarStyle& style) noexcept {
    style_ = style;
    bounds_.height = style_.height;
}

const MenuBarStyle& MenuBar::style() const noexcept {
    return style_;
}

void MenuBar::set_font(const Font* font) noexcept {
    font_ = font;
}

const Font* MenuBar::font() const noexcept {
    return font_;
}

void MenuBar::set_on_menu_action(std::function<void(const std::string&, const std::string&)> cb) {
    on_action_cb_ = std::move(cb);
}

void MenuBar::update_layout() const {
    float current_x = bounds_.x + 6.0f;
    const float y = bounds_.y + style_.item_padding_y;
    const float h = bounds_.height - (style_.item_padding_y * 2.0f);

    for (const auto& item : items_) {
        float text_width = 0.0f;
        if (font_) {
            text_width = font_->measure_text(item.title, style_.font_size).x;
        } else {
            text_width = static_cast<float>(item.title.length()) * (style_.font_size * 0.58f);
        }
        const float item_w = text_width + (style_.item_padding_x * 2.0f);
        item.computed_bounds = Rect{current_x, y, item_w, h};
        current_x += item_w + 2.0f;
    }
}

void MenuBar::open_menu(size_t index) {
    if (index >= items_.size()) {
        return;
    }
    update_layout();
    if (active_index_ >= 0 && static_cast<size_t>(active_index_) < items_.size()) {
        items_[static_cast<size_t>(active_index_)].menu->hide();
    }
    active_index_ = static_cast<int>(index);
    const auto& item = items_[index];
    if (item.menu) {
        Vec2 anchor{item.computed_bounds.x, bounds_.y + bounds_.height};
        if (font_) {
            item.menu->show(anchor.x, anchor.y, *font_, bounds_.width, 0.0f);
        } else {
            item.menu->show(anchor.x, anchor.y, bounds_.width, 0.0f);
        }
    }
}

void MenuBar::render(Renderer2D& renderer) {
    if (!visible_) {
        return;
    }

    update_layout();

    // Render bar background
    renderer.draw_rounded_rect(bounds_, 0.0f, style_.background_color, style_.border_color, style_.border_width);

    // Render individual top-level menu titles
    for (size_t i = 0; i < items_.size(); ++i) {
        const auto& item = items_[i];
        const bool is_active = (static_cast<int>(i) == active_index_);
        const bool is_hov = (static_cast<int>(i) == hovered_index_) || item.is_hovered;

        if (is_active) {
            renderer.draw_rounded_rect(item.computed_bounds, style_.corner_radius, style_.active_color);
        } else if (is_hov && item.enabled) {
            renderer.draw_rounded_rect(item.computed_bounds, style_.corner_radius, style_.hover_color);
        }

        const Color text_col = item.enabled ? style_.text_color : style_.text_disabled_color;
        float text_width = static_cast<float>(item.title.length()) * (style_.font_size * 0.58f);
        if (font_) {
            text_width = font_->measure_text(item.title, style_.font_size).x;
        }

        const float text_x = item.computed_bounds.x + (item.computed_bounds.width - text_width) * 0.5f;
        const float text_y = item.computed_bounds.y + (item.computed_bounds.height - style_.font_size) * 0.5f;

        renderer.draw_text(item.title, Vec2{text_x, text_y}, text_col, 1.0f);
    }

    // Render active ContextMenu overlay on top
    if (active_index_ >= 0 && static_cast<size_t>(active_index_) < items_.size()) {
        if (items_[static_cast<size_t>(active_index_)].menu) {
            items_[static_cast<size_t>(active_index_)].menu->render(renderer);
        }
    }
}

bool MenuBar::handle_mouse_move(const Vec2& mouse_pos) {
    if (!visible_ || !enabled_) {
        return false;
    }

    update_layout();

    // If an active dropdown menu is open, route events to it first
    if (active_index_ >= 0 && static_cast<size_t>(active_index_) < items_.size()) {
        auto& active_menu = items_[static_cast<size_t>(active_index_)].menu;
        MouseEvent mev = MouseEvent::make_move(mouse_pos);
        if (active_menu && active_menu->handle_mouse(mev)) {
            return true;
        }
    }

    int previous_hover = hovered_index_;
    hovered_index_ = -1;

    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].computed_bounds.contains(mouse_pos)) {
            hovered_index_ = static_cast<int>(i);
            items_[i].is_hovered = true;
            // If already opened, hovering another menu opens it immediately (desktop standard)
            if (active_index_ >= 0 && active_index_ != hovered_index_) {
                open_menu(i);
            }
            return true;
        } else {
            items_[i].is_hovered = false;
        }
    }

    return (hovered_index_ != previous_hover);
}

bool MenuBar::handle_mouse(const MouseEvent& ev) {
    if (!visible_ || !enabled_) {
        return false;
    }
    if (ev.type == MouseEventType::Move) {
        return handle_mouse_move(ev.position);
    } else if (ev.type == MouseEventType::ButtonDown) {
        return handle_mouse_button(ev.button, InputAction::Press, ev.position);
    } else if (ev.type == MouseEventType::ButtonUp) {
        return handle_mouse_button(ev.button, InputAction::Release, ev.position);
    }
    return false;
}

bool MenuBar::handle_mouse_button(MouseButton button, InputAction action, const Vec2& mouse_pos) {
    if (!visible_ || !enabled_) {
        return false;
    }

    update_layout();

    // Route click to active menu first
    if (active_index_ >= 0 && static_cast<size_t>(active_index_) < items_.size()) {
        auto& active_menu = items_[static_cast<size_t>(active_index_)].menu;
        MouseEvent mev = (action == InputAction::Press)
            ? MouseEvent::make_button_down(mouse_pos, button)
            : MouseEvent::make_button_up(mouse_pos, button);
        if (active_menu && active_menu->handle_mouse(mev)) {
            if (!active_menu->is_visible()) {
                active_index_ = -1;
            }
            return true;
        }
    }

    if (button == MouseButton::Left && action == InputAction::Press) {
        for (size_t i = 0; i < items_.size(); ++i) {
            if (items_[i].computed_bounds.contains(mouse_pos) && items_[i].enabled) {
                if (active_index_ == static_cast<int>(i)) {
                    close_active_menu();
                } else {
                    open_menu(i);
                }
                return true;
            }
        }

        // Clicking outside closes open menus
        if (active_index_ >= 0) {
            close_active_menu();
            return true;
        }
    }

    return false;
}

bool MenuBar::handle_key(const KeyEvent& ev) {
    return handle_key_event(ev.key, ev.action, static_cast<KeyModifier>(ev.modifiers));
}

bool MenuBar::handle_key_event(KeyCode key, InputAction action, KeyModifier mods) {
    if (!visible_ || !enabled_) {
        return false;
    }

    // Forward to active dropdown menu first
    if (active_index_ >= 0 && static_cast<size_t>(active_index_) < items_.size()) {
        auto& active_menu = items_[static_cast<size_t>(active_index_)].menu;
        KeyEvent kev;
        kev.key = key;
        kev.action = action;
        kev.modifiers = static_cast<uint8_t>(mods);
        if (active_menu && active_menu->handle_key(kev)) {
            if (!active_menu->is_visible()) {
                active_index_ = -1;
            }
            return true;
        }
    }

    if (action == InputAction::Press) {
        if (key == KeyCode::Escape && active_index_ >= 0) {
            close_active_menu();
            return true;
        }

        if (active_index_ >= 0 && !items_.empty()) {
            if (key == KeyCode::Left) {
                size_t prev = (active_index_ == 0) ? (items_.size() - 1) : (static_cast<size_t>(active_index_) - 1);
                open_menu(prev);
                return true;
            } else if (key == KeyCode::Right) {
                size_t next = (static_cast<size_t>(active_index_) + 1) % items_.size();
                open_menu(next);
                return true;
            }
        }
    }

    return false;
}

} // namespace arin
