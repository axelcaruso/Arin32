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

#include "arin/context_menu.hpp"
#include "arin/metrics.hpp"
#include "arin/font.hpp"
#include <algorithm>

namespace arin {

ContextMenu::ContextMenu() {
    m_bounds = Rect(0.0f, 0.0f, m_style.min_width, 0.0f);
}

ContextMenu& ContextMenu::set_style(const ContextMenuStyle& style) {
    m_style = style;
    return *this;
}

ContextMenu& ContextMenu::add_item(const std::string& label, std::function<void()> callback) {
    m_items.push_back(MenuItem::action(label, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_item(const std::string& label, IconType icon, std::function<void()> callback) {
    m_items.push_back(MenuItem::action(label, icon, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_item(
    const std::string& label,
    IconType icon,
    const std::string& shortcut,
    std::function<void()> callback
) {
    m_items.push_back(MenuItem::action(label, icon, shortcut, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_separator() {
    m_items.push_back(MenuItem::separator());
    return *this;
}

ContextMenu& ContextMenu::clear() {
    m_items.clear();
    m_hovered_index = -1;
    return *this;
}

void ContextMenu::calculate_dimensions(const Font& font) {
    float max_label_w = 0.0f;
    float max_shortcut_w = 0.0f;
    bool has_any_icons = false;

    float total_h = m_style.padding.top + m_style.padding.bottom;

    for (const auto& item : m_items) {
        if (item.is_separator) {
            total_h += m_style.separator_height;
        } else {
            total_h += m_style.item_height;
            if (item.icon != IconType::None) {
                has_any_icons = true;
            }
            Vec2 lbl_sz = font.measure_text(item.label, m_style.label_scale);
            max_label_w = std::max(max_label_w, lbl_sz.x);

            if (!item.shortcut.empty()) {
                Vec2 sc_sz = font.measure_text(item.shortcut, m_style.shortcut_scale);
                max_shortcut_w = std::max(max_shortcut_w, sc_sz.x);
            }
        }
    }

    float icon_gap = has_any_icons ? 26.0f : 0.0f;
    float shortcut_gap = (max_shortcut_w > 0.0f) ? (24.0f + max_shortcut_w) : 0.0f;
    float required_w = m_style.padding.left + m_style.padding.right + 16.0f +
                       icon_gap + max_label_w + shortcut_gap + 12.0f;

    m_bounds.width = std::max(m_style.min_width, required_w);
    m_bounds.height = total_h;
}

/**
 * @brief Returns the row rectangle for an item index (separator or action).
 */
Rect ContextMenu::row_rect_at(size_t index) const {
    // Step 1: Walk previous rows to find the vertical offset.
    float cursor_y = m_bounds.y + m_style.padding.top;
    for (size_t i = 0; i < index && i < m_items.size(); ++i) {
        cursor_y += m_items[i].is_separator ? m_style.separator_height : m_style.item_height;
    }
    // Step 2: Build the full-width row inside horizontal padding.
    const float content_w = m_bounds.width - m_style.padding.left - m_style.padding.right;
    const float row_h = (index < m_items.size() && m_items[index].is_separator)
        ? m_style.separator_height
        : m_style.item_height;
    return Rect(m_bounds.x + m_style.padding.left, cursor_y, content_w, row_h);
}

void ContextMenu::show(float x, float y, float screen_width, float screen_height) {
    // Step 1: Compatibility path uses a neutral scale for measurement.
    // Prefer the font-aware overload whenever the renderer font is available.
    show(x, y, Font(), screen_width, screen_height);
}

void ContextMenu::show(float x, float y, const Font& font, float screen_width, float screen_height) {
    m_visible = true;
    m_hovered_index = -1;
    m_screen_w = screen_width;
    m_screen_h = screen_height;

    // Step 2: Measure with real font metrics when available.
    calculate_dimensions(font);

    // Step 3: Clamp the popup so it never leaves the visible window surface.
    if (screen_width > 0.0f) {
        if (x + m_bounds.width > screen_width - UiMetrics::kMenuScreenMargin) {
            x = screen_width - m_bounds.width - UiMetrics::kMenuScreenMargin;
        }
        if (x < UiMetrics::kMenuScreenMargin) {
            x = UiMetrics::kMenuScreenMargin;
        }
    }

    if (screen_height > 0.0f) {
        if (y + m_bounds.height > screen_height - UiMetrics::kMenuScreenMargin) {
            y = screen_height - m_bounds.height - UiMetrics::kMenuScreenMargin;
        }
        if (y < UiMetrics::kMenuScreenMargin) {
            y = UiMetrics::kMenuScreenMargin;
        }
    }

    m_bounds.x = x;
    m_bounds.y = y;
}

void ContextMenu::hide() {
    if (m_visible) {
        m_visible = false;
        m_hovered_index = -1;
        if (m_dismiss_cb) {
            m_dismiss_cb();
        }
    }
}

bool ContextMenu::handle_mouse(const MouseEvent& ev) {
    if (!m_visible) return false;

    // 1. Mouse move: track hovered menu item
    if (ev.type == MouseEventType::Move) {
        if (!m_bounds.contains(ev.position)) {
            m_hovered_index = -1;
            return false;
        }

        float cur_y = m_bounds.y + m_style.padding.top;
        m_hovered_index = -1;
        for (size_t i = 0; i < m_items.size(); ++i) {
            const auto& item = m_items[i];
            float item_h = item.is_separator ? m_style.separator_height : m_style.item_height;
            Rect item_rect(m_bounds.x, cur_y, m_bounds.width, item_h);

            if (!item.is_separator && item.enabled && item_rect.contains(ev.position)) {
                m_hovered_index = static_cast<int>(i);
                break;
            }
            cur_y += item_h;
        }
        return true;
    }

    // 2. Mouse button down: click outside dismisses menu
    if (ev.type == MouseEventType::ButtonDown) {
        if (!m_bounds.contains(ev.position)) {
            hide();
            return false; // Allow click to pass through to underlying widget
        }
        return true; // Click inside menu is consumed
    }

    // 3. Mouse button up: item selection
    if (ev.type == MouseEventType::ButtonUp && ev.button == MouseButton::Left) {
        if (!m_bounds.contains(ev.position)) {
            hide();
            return false;
        }

        if (m_hovered_index >= 0 && m_hovered_index < static_cast<int>(m_items.size())) {
            const auto& item = m_items[m_hovered_index];
            if (!item.is_separator && item.enabled && item.callback) {
                auto cb = item.callback;
                hide();
                cb();
                return true;
            }
        }
        hide();
        return true;
    }

    return m_bounds.contains(ev.position);
}

bool ContextMenu::handle_key(const KeyEvent& ev) {
    if (!m_visible) return false;

    // Dismiss context menu on Escape key
    if (ev.key == KeyCode::Escape && ev.action == InputAction::Press) {
        hide();
        return true;
    }

    return false;
}

void ContextMenu::update(float dt) {
    (void)dt;
}

void ContextMenu::render(Renderer2D& renderer) {
    if (!m_visible || m_items.empty()) return;

    // 1. Elevated Floating Drop Shadow
    renderer.draw_shadow(
        m_bounds,
        m_style.corner_radius,
        m_style.shadow_color,
        m_style.shadow_offset,
        m_style.shadow_blur
    );

    // 2. Card Surface and 1px Border
    renderer.draw_rounded_rect(
        m_bounds,
        m_style.corner_radius,
        m_style.background_color,
        m_style.border_color,
        1.0f
    );

    bool has_any_icons = false;
    for (const auto& item : m_items) {
        if (item.icon != IconType::None) {
            has_any_icons = true;
            break;
        }
    }

    // 3. Render Items
    float cur_y = m_bounds.y + m_style.padding.top;
    float content_w = m_bounds.width - m_style.padding.left - m_style.padding.right;

    for (size_t i = 0; i < m_items.size(); ++i) {
        const auto& item = m_items[i];

        if (item.is_separator) {
            // Step 3a: Draw the subtle separator rule with shared insets.
            const float line_y = cur_y + m_style.separator_height * detail::kHalf;
            renderer.draw_rect(
                Rect(m_bounds.x + UiMetrics::kMenuSeparatorInset, line_y, m_bounds.width - 2.0f * UiMetrics::kMenuSeparatorInset, UiMetrics::kMenuSeparatorThickness),
                m_style.separator_color
            );
            cur_y += m_style.separator_height;
        } else {
            Rect item_rect(m_bounds.x + m_style.padding.left, cur_y, content_w, m_style.item_height);
            bool is_hovered = (static_cast<int>(i) == m_hovered_index) && item.enabled;

            Color text_col;
            Color icon_col;
            Color sc_col;

            if (is_hovered) {
                // Accent Blue selection highlight
                renderer.draw_rounded_rect(item_rect, UiMetrics::kMenuItemCornerRadius, m_style.hover_color);
                text_col = m_style.hover_text_color;
                icon_col = m_style.icon_hover_color;
                sc_col   = m_style.shortcut_hover_color;
            } else {
                text_col = item.enabled ? m_style.text_color : m_style.text_disabled_color;
                icon_col = item.enabled ? m_style.icon_color : m_style.text_disabled_color;
                sc_col   = item.enabled ? m_style.shortcut_color : m_style.text_disabled_color;
            }

            // Step 3b: Draw the optional vector icon centered in its column.
            if (item.icon != IconType::None) {
                const float icon_sz = m_style.icon_size;
                const float icon_x = item_rect.x + UiMetrics::kMenuIconInset;
                const Rect icon_bounds = item_rect.centered(Vec2(icon_sz, icon_sz));
                renderer.draw_icon(item.icon, Rect(icon_x, icon_bounds.y, icon_sz, icon_sz), icon_col);
            }

            // Step 3c: Draw the label with optical vertical centering.
            const float text_x = item_rect.x + (has_any_icons ? m_style.icon_column_width : UiMetrics::kMenuTextInsetNoIcon);
            const Rect label_bounds(text_x, item_rect.y, item_rect.right() - text_x, item_rect.height);
            renderer.draw_text_in_rect(item.label, label_bounds, text_col, m_style.label_scale, TextAlignH::Left, TextAlignV::Center);

            // Step 3d: Draw the optional shortcut right-aligned with the same centering.
            if (!item.shortcut.empty()) {
                const Vec2 sc_sz = renderer.font().measure_text(item.shortcut, m_style.shortcut_scale);
                const float sc_x = item_rect.right() - sc_sz.x - UiMetrics::kMenuShortcutTrailingInset;
                const Rect shortcut_bounds(sc_x, item_rect.y, sc_sz.x, item_rect.height);
                renderer.draw_text_in_rect(item.shortcut, shortcut_bounds, sc_col, m_style.shortcut_scale, TextAlignH::Left, TextAlignV::Center);
            }

            cur_y += m_style.item_height;
        }
    }
}

} // namespace arin
