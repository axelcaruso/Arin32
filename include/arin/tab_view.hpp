/*
 * Arin32 - Modern OpenGL Graphical User Interface Library
 *
 * Copyright (c) 2026, Arin32 & ArinOS Contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions@ are met:
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

#ifndef ARIN32_TAB_VIEW_HPP
#define ARIN32_TAB_VIEW_HPP

#include "widget.hpp"
#include "types.hpp"
#include "font.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace arin {

/**
 * @brief Visual styling configuration for TabBar and TabView.
 */
struct TabStyle {
    Color bar_background{0.14f, 0.14f, 0.15f, 1.0f};
    Color tab_inactive_background{0.18f, 0.18f, 0.20f, 1.0f};
    Color tab_active_background{0.24f, 0.24f, 0.26f, 1.0f};
    Color tab_hover_background{0.21f, 0.21f, 0.23f, 1.0f};
    Color text_active_color{1.0f, 1.0f, 1.0f, 1.0f};
    Color text_inactive_color{0.72f, 0.72f, 0.75f, 1.0f};
    Color accent_indicator_color{0.0f, 0.48f, 0.85f, 1.0f};
    Color content_background{0.20f, 0.20f, 0.22f, 1.0f};
    Color border_color{0.12f, 0.12f, 0.13f, 1.0f};
    float tab_height{32.0f};
    float tab_padding_x{14.0f};
    float indicator_height{2.5f};
    float font_size{13.0f};
    float corner_radius{3.0f};
    float border_width{1.0f};
};

/**
 * @brief Represents an individual tab within a TabView.
 */
struct TabItem {
    std::string title;
    std::shared_ptr<IWidget> content{nullptr};
    bool closable{false};
    bool enabled{true};
    mutable Rect computed_bounds{0.0f, 0.0f, 0.0f, 0.0f};
    mutable Rect computed_close_bounds{0.0f, 0.0f, 0.0f, 0.0f};
    mutable bool is_hovered{false};
    mutable bool is_close_hovered{false};
};

/**
 * @brief Tabbed container widget managing multiple pages with interactive tab headers.
 */
class TabView : public IWidget {
public:
    TabView();
    explicit TabView(const Rect& bounds);
    ~TabView() override = default;

    // Tab manipulation
    size_t add_tab(std::string title, std::shared_ptr<IWidget> content = nullptr, bool closable = false);
    bool remove_tab(size_t index);
    void clear_tabs() noexcept;
    size_t tab_count() const noexcept;

    // Active tab selection
    void set_active_tab(size_t index);
    int active_tab_index() const noexcept;
    std::shared_ptr<IWidget> active_content() const noexcept;

    // Tab accessors
    const std::string& tab_title(size_t index) const;
    void set_tab_title(size_t index, std::string title);
    std::shared_ptr<IWidget> tab_content(size_t index) const;
    void set_tab_content(size_t index, std::shared_ptr<IWidget> content);
    bool is_tab_closable(size_t index) const;
    void set_tab_closable(size_t index, bool closable);

    // Callbacks
    void on_tab_changed(std::function<void(size_t index)> cb);
    void on_tab_close_requested(std::function<void(size_t index)> cb);

    // Styling & Layout
    void set_style(const TabStyle& style) noexcept;
    const TabStyle& style() const noexcept;
    void set_font(const Font* font) noexcept;
    const Font* font() const noexcept;
    Rect content_area_bounds() const noexcept;

    // IWidget geometry overrides
    const Rect& bounds() const override { return bounds_; }
    IWidget& set_bounds(const Rect& bounds) override { bounds_ = bounds; return *this; }
    IWidget& set_position(float x, float y) override { bounds_.x = x; bounds_.y = y; return *this; }
    IWidget& set_size(float width, float height) override { bounds_.width = width; bounds_.height = height; return *this; }
    bool is_visible() const override { return visible_; }
    bool is_enabled() const override { return enabled_; }
    void set_visible(bool v) noexcept { visible_ = v; }
    void set_enabled(bool e) noexcept { enabled_ = e; }

    // IWidget event overrides
    void render(Renderer2D& renderer) override;
    bool handle_mouse(const MouseEvent& ev) override;
    bool handle_key(const KeyEvent& ev) override;
    bool handle_text(const TextEvent& ev) override;

    // Convenience event helpers
    bool handle_mouse_move(const Vec2& mouse_pos);
    bool handle_mouse_button(MouseButton button, InputAction action, const Vec2& mouse_pos);
    bool handle_key_event(KeyCode key, InputAction action, KeyModifier mods);

private:
    void update_layout() const;

    Rect bounds_{0.0f, 0.0f, 600.0f, 400.0f};
    bool visible_{true};
    bool enabled_{true};
    std::vector<TabItem> tabs_;
    int active_tab_{-1};
    int hovered_tab_{-1};
    TabStyle style_;
    const Font* font_{nullptr};
    std::function<void(size_t)> tab_changed_cb_;
    std::function<void(size_t)> tab_close_cb_;
};

} // namespace arin

#endif // ARIN32_TAB_VIEW_HPP
