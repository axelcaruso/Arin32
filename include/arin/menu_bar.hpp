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

#ifndef ARIN32_MENU_BAR_HPP
#define ARIN32_MENU_BAR_HPP

#include "widget.hpp"
#include "types.hpp"
#include "context_menu.hpp"
#include "font.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace arin {

/**
 * @brief Visual styling configuration for an application MenuBar.
 */
struct MenuBarStyle {
    Color background_color{0.18f, 0.18f, 0.19f, 1.0f};
    Color border_color{0.24f, 0.24f, 0.26f, 1.0f};
    Color text_color{0.92f, 0.92f, 0.94f, 1.0f};
    Color text_disabled_color{0.50f, 0.50f, 0.52f, 1.0f};
    Color hover_color{0.26f, 0.26f, 0.28f, 1.0f};
    Color active_color{0.30f, 0.30f, 0.33f, 1.0f};
    float height{28.0f};
    float item_padding_x{12.0f};
    float item_padding_y{4.0f};
    float corner_radius{3.0f};
    float border_width{1.0f};
    float font_size{13.0f};
};

/**
 * @brief Represents a top-level menu entry in a MenuBar.
 */
struct MenuBarItem {
    std::string title;
    std::shared_ptr<ContextMenu> menu;
    bool enabled{true};
    mutable Rect computed_bounds{0.0f, 0.0f, 0.0f, 0.0f};
    mutable bool is_hovered{false};
};

/**
 * @brief Top-level application MenuBar widget.
 *
 * Hosts a horizontal row of top-level menus ("File", "Edit", "View", etc.)
 * that anchor and reveal cascading ContextMenus upon interaction.
 */
class MenuBar : public IWidget {
public:
    MenuBar();
    explicit MenuBar(const Rect& bounds);
    ~MenuBar() override = default;

    // Menu creation and access
    ContextMenu& add_menu(std::string title);
    bool remove_menu(size_t index);
    const std::string& menu_title(size_t index) const;
    ContextMenu* get_menu(size_t index) noexcept;
    const ContextMenu* get_menu(size_t index) const noexcept;
    ContextMenu* find_menu(const std::string& title) noexcept;
    const ContextMenu* find_menu(const std::string& title) const noexcept;
    size_t menu_count() const noexcept;
    void clear() noexcept;

    // Active menu state
    bool is_open() const noexcept;
    void open_menu(size_t index);
    void close_active_menu() noexcept;
    int active_menu_index() const noexcept;

    // Styling
    void set_style(const MenuBarStyle& style) noexcept;
    const MenuBarStyle& style() const noexcept;

    // Custom font
    void set_font(const Font* font) noexcept;
    const Font* font() const noexcept;

    // IWidget geometry overrides
    const Rect& bounds() const override { return bounds_; }
    IWidget& set_bounds(const Rect& bounds) override { bounds_ = bounds; style_.height = bounds.height; return *this; }
    IWidget& set_position(float x, float y) override { bounds_.x = x; bounds_.y = y; return *this; }
    IWidget& set_size(float width, float height) override { bounds_.width = width; bounds_.height = height; style_.height = height; return *this; }
    bool is_visible() const override { return visible_; }
    bool is_enabled() const override { return enabled_; }
    void set_visible(bool v) noexcept { visible_ = v; }
    void set_enabled(bool e) noexcept { enabled_ = e; }

    // IWidget event overrides
    void render(Renderer2D& renderer) override;
    bool handle_mouse(const MouseEvent& ev) override;
    bool handle_key(const KeyEvent& ev) override;
    bool handle_text(const TextEvent& ev) override { (void)ev; return false; }

    // Convenience event helpers
    bool handle_mouse_move(const Vec2& mouse_pos);
    bool handle_mouse_button(MouseButton button, InputAction action, const Vec2& mouse_pos);
    bool handle_key_event(KeyCode key, InputAction action, KeyModifier mods);

    // Submenu dismiss listener
    void set_on_menu_action(std::function<void(const std::string& menu_title, const std::string& action_id)> cb);

private:
    void update_layout() const;

    Rect bounds_{0.0f, 0.0f, 800.0f, 28.0f};
    bool visible_{true};
    bool enabled_{true};
    std::vector<MenuBarItem> items_;
    MenuBarStyle style_;
    const Font* font_{nullptr};
    int active_index_{-1};
    int hovered_index_{-1};
    std::function<void(const std::string&, const std::string&)> on_action_cb_;
};

} // namespace arin

#endif // ARIN32_MENU_BAR_HPP
