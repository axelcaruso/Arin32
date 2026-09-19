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
#include "svg.hpp"
#include "texture.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <stdexcept>

namespace arin {

/**
 * @brief Represents an individual item, action, submenu indicator, or separator in a ContextMenu.
 */
struct MenuItem {
    std::string label;
    IconType icon{IconType::None};
    std::shared_ptr<SvgDocument> svg_icon{nullptr};
    std::string svg_path;
    mutable std::shared_ptr<Texture> cached_texture{nullptr};
    std::string shortcut;
    bool is_separator{false};
    bool enabled{true};
    bool has_submenu{false};
    bool is_default{false};
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

    static MenuItem action(std::string text, const std::string& svg_filepath, std::function<void()> cb = nullptr) {
        MenuItem item;
        item.label = std::move(text);
        item.svg_path = svg_filepath;
        item.callback = std::move(cb);
        return item;
    }

    static MenuItem action(std::string text, const std::string& svg_filepath, std::string sc, std::function<void()> cb = nullptr) {
        MenuItem item;
        item.label = std::move(text);
        item.svg_path = svg_filepath;
        item.shortcut = std::move(sc);
        item.callback = std::move(cb);
        return item;
    }

    static MenuItem action(std::string text, std::shared_ptr<SvgDocument> doc, std::function<void()> cb = nullptr) {
        MenuItem item;
        item.label = std::move(text);
        item.svg_icon = std::move(doc);
        item.callback = std::move(cb);
        return item;
    }

    static MenuItem action(std::string text, std::shared_ptr<SvgDocument> doc, std::string sc, std::function<void()> cb = nullptr) {
        MenuItem item;
        item.label = std::move(text);
        item.svg_icon = std::move(doc);
        item.shortcut = std::move(sc);
        item.callback = std::move(cb);
        return item;
    }

    static MenuItem submenu(std::string text, std::function<void()> cb = nullptr) {
        MenuItem item;
        item.label = std::move(text);
        item.has_submenu = true;
        item.callback = std::move(cb);
        return item;
    }

    static MenuItem submenu(std::string text, const std::string& svg_filepath, std::function<void()> cb = nullptr) {
        MenuItem item;
        item.label = std::move(text);
        item.svg_path = svg_filepath;
        item.has_submenu = true;
        item.callback = std::move(cb);
        return item;
    }

    static MenuItem submenu(std::string text, IconType ic, std::function<void()> cb = nullptr) {
        MenuItem item;
        item.label = std::move(text);
        item.icon = ic;
        item.has_submenu = true;
        item.callback = std::move(cb);
        return item;
    }

    static MenuItem submenu(std::string text, std::shared_ptr<SvgDocument> doc, std::function<void()> cb = nullptr) {
        MenuItem item;
        item.label = std::move(text);
        item.svg_icon = std::move(doc);
        item.has_submenu = true;
        item.callback = std::move(cb);
        return item;
    }

    static MenuItem separator() {
        MenuItem item;
        item.is_separator = true;
        return item;
    }

    MenuItem& set_default(bool def = true) {
        is_default = def;
        return *this;
    }

    MenuItem& set_submenu(bool sub = true) {
        has_submenu = sub;
        return *this;
    }

    MenuItem& set_enabled(bool en) {
        enabled = en;
        return *this;
    }

    MenuItem& set_svg(const std::string& path) {
        svg_path = path;
        cached_texture.reset();
        return *this;
    }

    MenuItem& set_svg(std::shared_ptr<SvgDocument> doc) {
        svg_icon = std::move(doc);
        cached_texture.reset();
        return *this;
    }

    MenuItem& set_shortcut(const std::string& sc) {
        shortcut = sc;
        return *this;
    }
};

/**
 * @brief Visual styling configuration for ContextMenu.
 */
struct ContextMenuStyle {
    Color background_color{Color::white()};
    Color border_color{Color::from_hex(0xCCCCCC)};                    ///< Windows 10 #CCCCCC crisp 1px border
    Color hover_color{Color::from_hex(0xE5E5E5)};                     ///< Windows 10 soft light grey hover #E5E5E5
    Color hover_border_color{Color::transparent()};                   ///< Hover row border outline
    Color hover_text_color{Color::from_hex(0x000000)};                ///< In Windows 10 text stays dark on hover!
    Color text_color{Color::from_hex(0x000000)};                      ///< Windows 10 text #000000
    Color text_disabled_color{Color::from_hex(0x8D8D8D)};
    Color shortcut_color{Color::from_hex(0x666666)};
    Color shortcut_hover_color{Color::from_hex(0x000000)};
    Color separator_color{Color::from_hex(0xE4E4E4)};                 ///< Windows 10 1px separator #E4E4E4
    Color icon_color{Color::from_hex(0x1E1E1E)};
    Color icon_hover_color{Color::from_hex(0x000000)};
    Color chevron_color{Color::from_hex(0x333333)};                   ///< Submenu chevron right indicator
    Color chevron_hover_color{Color::from_hex(0x000000)};
    Color shadow_color{Color(0.0f, 0.0f, 0.0f, 0.20f)};
    Vec2 shadow_offset{1.0f, 2.0f};
    float shadow_blur{8.0f};
    float corner_radius{0.0f};                                        ///< Windows 10 crisp square corners (0px)
    float hover_corner_radius{0.0f};                                  ///< Windows 10 hover highlight corner radius (0px)
    static constexpr float kDefaultItemHeight = 22.0f;                ///< Windows 10 desktop menu item height
    float label_scale{UiMetrics::kMenuLabelScale};
    float shortcut_scale{UiMetrics::kMenuShortcutScale};
    float icon_size{16.0f};                                           ///< Windows 10 standard 16x16 icon size
    float icon_column_width{32.0f};                                   ///< Space allocated for icon column
    float item_height{kDefaultItemHeight};
    float separator_height{7.0f};
    float min_width{180.0f};
    Padding padding{2.0f, 2.0f};                                      ///< Windows 10 2px padding
    bool always_align_labels{true};                                   ///< Align all labels in column when icons are used

    static ContextMenuStyle windows10() {
        return ContextMenuStyle{};
    }

    static ContextMenuStyle dark() {
        ContextMenuStyle s;
        s.background_color = Color::from_hex(0x2B2B2B);
        s.border_color = Color::from_hex(0x404040);
        s.hover_color = Color::from_hex(0x414141);
        s.hover_text_color = Color::white();
        s.text_color = Color::white();
        s.text_disabled_color = Color::from_hex(0x787878);
        s.shortcut_color = Color::from_hex(0x9E9E9E);
        s.shortcut_hover_color = Color::white();
        s.separator_color = Color::from_hex(0x3E3E3E);
        s.icon_color = Color::white();
        s.icon_hover_color = Color::white();
        s.chevron_color = Color::from_hex(0x9E9E9E);
        s.chevron_hover_color = Color::white();
        s.shadow_color = Color(0.0f, 0.0f, 0.0f, 0.40f);
        return s;
    }

    static ContextMenuStyle rounded() {
        ContextMenuStyle s;
        s.corner_radius = 6.0f;
        s.hover_corner_radius = 4.0f;
        s.padding = Padding(4.0f, 4.0f);
        s.item_height = 26.0f;
        return s;
    }
};

/**
 * @brief Floating popup context menu triggered by right click or programmatic action.
 *
 * Features:
 * - 1:1 authentic Windows 10 desktop styling with crisp 1px border, soft drop shadow, and light grey hover.
 * - Native SVG icon support and vector icon badges.
 * - Submenu chevrons (>) and keyboard shortcut text (e.g. "Ctrl+C", "Ctrl+V").
 * - Bold rendering for default actions (e.g. "Abrir").
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

    ContextMenu& add_item(MenuItem item);
    ContextMenu& add_item(const std::string& label, std::function<void()> callback = nullptr);
    ContextMenu& add_item(const std::string& label, IconType icon, std::function<void()> callback = nullptr);
    ContextMenu& add_item(const std::string& label, IconType icon, const std::string& shortcut, std::function<void()> callback = nullptr);

    // SVG icon overloads
    ContextMenu& add_item(const std::string& label, const std::string& svg_path, std::function<void()> callback = nullptr);
    ContextMenu& add_item(const std::string& label, const std::string& svg_path, const std::string& shortcut, std::function<void()> callback = nullptr);
    ContextMenu& add_item(const std::string& label, std::shared_ptr<SvgDocument> svg_doc, std::function<void()> callback = nullptr);
    ContextMenu& add_item(const std::string& label, std::shared_ptr<SvgDocument> svg_doc, const std::string& shortcut, std::function<void()> callback = nullptr);

    // Submenu helpers
    ContextMenu& add_submenu(const std::string& label, std::function<void()> callback = nullptr);
    ContextMenu& add_submenu(const std::string& label, const std::string& svg_path, std::function<void()> callback = nullptr);
    ContextMenu& add_submenu(const std::string& label, IconType icon, std::function<void()> callback = nullptr);
    ContextMenu& add_submenu(const std::string& label, std::shared_ptr<SvgDocument> svg_doc, std::function<void()> callback = nullptr);

    // Default action helpers (rendered in bold)
    ContextMenu& add_default_item(const std::string& label, std::function<void()> callback = nullptr);
    ContextMenu& add_default_item(const std::string& label, const std::string& svg_path, std::function<void()> callback = nullptr);
    ContextMenu& add_default_item(const std::string& label, IconType icon, std::function<void()> callback = nullptr);
    ContextMenu& add_default_item(const std::string& label, std::shared_ptr<SvgDocument> svg_doc, std::function<void()> callback = nullptr);

    ContextMenu& add_separator();
    ContextMenu& clear();

    size_t item_count() const { return m_items.size(); }
    MenuItem& item_at(size_t index) { return m_items.at(index); }
    const MenuItem& item_at(size_t index) const { return m_items.at(index); }
    MenuItem& last_item();

    /// @brief Programmatically gets or sets the hovered item index (supports keyboard navigation).
    int hovered_index() const { return m_hovered_index; }
    void set_hovered_index(int index) { m_hovered_index = index; }

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
