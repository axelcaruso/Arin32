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

#ifndef ARIN32_CONTEXT_MENU_HPP
#define ARIN32_CONTEXT_MENU_HPP

#include "widget.hpp"
#include "types.hpp"
#include "metrics.hpp"
#include "icon.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace arin {

/**
 * @brief Represents an individual item, action, or separator in a ContextMenu.
 */
struct MenuItem {
    std::string label;
    IconType icon{IconType::None};
    std::string shortcut;
    bool is_separator{false};
    bool enabled{true};
    std::function<void()> callback;

    static MenuItem action(std::string text, std::function<void()> cb = nullptr) {
        MenuItem item;
        item.label = std::move(text);
        item.callback = std::move(cb);
        return item;
    }

    static MenuItem action(std::string text, IconType ic, std::function<void()> cb = nullptr) {
        MenuItem item;
        item.label = std::move(text);
        item.icon = ic;
        item.callback = std::move(cb);
        return item;
    }

    static MenuItem action(std::string text, IconType ic, std::string sc, std::function<void()> cb = nullptr) {
        MenuItem item;
        item.label = std::move(text);
        item.icon = ic;
        item.shortcut = std::move(sc);
        item.callback = std::move(cb);
        return item;
    }

    static MenuItem separator() {
        MenuItem item;
        item.is_separator = true;
        return item;
    }
};

/**
 * @brief Visual styling configuration for ContextMenu.
 */
struct ContextMenuStyle {
    Color background_color{Color::white()};
    Color border_color{Color::from_hex(0xD1D5DB)};
    Color hover_color{Color::from_hex(palette::kAccentBlue)};          ///< Accent Blue highlight on hover
    Color hover_text_color{Color::white()};
    Color text_color{Color::from_hex(0x1F2937)};
    Color text_disabled_color{Color::from_hex(0x9CA3AF)};
    Color shortcut_color{Color::from_hex(0x6B7280)};
    Color shortcut_hover_color{Color::from_hex(0xDBEAFE)};
    Color separator_color{Color::from_hex(0xE5E7EB)};
    Color icon_color{Color::from_hex(0x4B5563)};
    Color icon_hover_color{Color::white()};
    Color shadow_color{Color(0.0f, 0.0f, 0.0f, 0.16f)};
    Vec2 shadow_offset{0.0f, 3.0f};
    float shadow_blur{10.0f};
    float corner_radius{5.0f};
    static constexpr float kDefaultItemHeight = 26.0f;
    float label_scale{UiMetrics::kMenuLabelScale};
    float shortcut_scale{UiMetrics::kMenuShortcutScale};
    float icon_size{UiMetrics::kMenuIconSize};
    float icon_column_width{UiMetrics::kMenuIconColumnWidth};
    float item_height{kDefaultItemHeight};
    float separator_height{7.0f};
    float min_width{180.0f};
    Padding padding{4.0f, 4.0f};
};

/**
 * @brief Floating popup context menu triggered by right click or programmatic action.
 *
 * Features:
 * - Clean modern styling with GPU SDF rounded corners, subtle drop shadow, and crisp 1px border.
 * - Icon badges and keyboard shortcut text (e.g. "Ctrl+C", "Ctrl+V").
 * - Automatic screen boundary clamping so popups near display edges never bleed offscreen.
 * - Clicking outside or pressing Escape automatically dismisses the menu.
 * - Z-order elevation (rendered as top-level application overlay).
 */
class ContextMenu : public IWidget {
public:
    ContextMenu();
    ~ContextMenu() override = default;

    // --- Configuration ---

    ContextMenu& set_style(const ContextMenuStyle& style);
    const ContextMenuStyle& style() const { return m_style; }

    // --- Menu Population ---

    ContextMenu& add_item(const std::string& label, std::function<void()> callback = nullptr);
    ContextMenu& add_item(const std::string& label, IconType icon, std::function<void()> callback = nullptr);
    ContextMenu& add_item(const std::string& label, IconType icon, const std::string& shortcut, std::function<void()> callback = nullptr);
    ContextMenu& add_separator();
    ContextMenu& clear();

    size_t item_count() const { return m_items.size(); }
    const MenuItem& item_at(size_t index) const { return m_items.at(index); }

    // --- Visibility and Popup Controls ---

    /**
     * @brief Displays the popup menu at screen position (x, y), automatically clamping
     *        within screen dimensions to prevent clipping.
     */
    void show(float x, float y, float screen_width = 0.0f, float screen_height = 0.0f);

    /**
     * @brief Displays the popup using real font metrics for size calculation.
     * @param x Requested left edge in pixels.
     * @param y Requested top edge in pixels.
     * @param font Active font used for label measurement.
     * @param screen_width Available screen width for clamping (0 disables clamping).
     * @param screen_height Available screen height for clamping (0 disables clamping).
     */
    void show(float x, float y, const Font& font, float screen_width = 0.0f, float screen_height = 0.0f);

    /// @brief Returns the row rectangle for an item index (used for hit testing).
    Rect row_rect_at(size_t index) const;

    /**
     * @brief Hides and dismisses the context menu.
     */
    void hide();

    /// @brief Checks if the context menu is currently open.
    bool is_visible() const { return m_visible; }

    // --- IWidget Interface ---

    const Rect& bounds() const override { return m_bounds; }
    IWidget& set_bounds(const Rect& b) override { m_bounds = b; return *this; }
    IWidget& set_position(float x, float y) override { m_bounds.x = x; m_bounds.y = y; return *this; }
    IWidget& set_size(float w, float h) override { m_bounds.width = w; m_bounds.height = h; return *this; }

    bool handle_mouse(const MouseEvent& ev) override;
    bool handle_key(const KeyEvent& ev) override;
    void update(float dt) override;
    void render(Renderer2D& renderer) override;

    /// @brief Dismiss callback invoked when the menu is closed.
    void on_dismiss(std::function<void()> cb) { m_dismiss_cb = std::move(cb); }

private:
    void calculate_dimensions(const Font& font);

    Rect m_bounds{0.0f, 0.0f, 0.0f, 0.0f};
    ContextMenuStyle m_style;
    std::vector<MenuItem> m_items;
    bool m_visible{false};
    int m_hovered_index{-1};
    float m_screen_w{0.0f};
    float m_screen_h{0.0f};
    std::function<void()> m_dismiss_cb;
};

} // namespace arin

#endif // ARIN32_CONTEXT_MENU_HPP
