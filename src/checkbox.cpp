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

#include "arin/checkbox.hpp"
#include <algorithm>

namespace arin {

CheckBox::CheckBox(
    std::string label,
    float x,
    float y,
    bool checked
) : m_label(std::move(label)),
    m_bounds(x, y, 160.0f, 20.0f),
    m_checked(checked) {}

CheckBox& CheckBox::set_checked(bool checked) {
    if (m_checked != checked) {
        m_checked = checked;
        if (m_toggle_cb) {
            m_toggle_cb(m_checked);
        }
    }
    return *this;
}

CheckBox& CheckBox::toggle() {
    return set_checked(!m_checked);
}

CheckBox& CheckBox::fit_to_content(const Font& font) {
    Vec2 text_size = font.measure_text(m_label, m_style.text_scale);
    float w = m_style.box_size + m_style.text_spacing + text_size.x;
    float h = std::max(m_style.box_size, text_size.y);
    m_bounds.width = w;
    m_bounds.height = h;
    return *this;
}

bool CheckBox::handle_mouse(const MouseEvent& ev) {
    if (!m_enabled || !m_visible) {
        m_hovered = false;
        m_pressed = false;
        return false;
    }

    bool inside = m_bounds.contains(ev.position);

    switch (ev.type) {
        case MouseEventType::Move:
            m_hovered = inside;
            return inside;

        case MouseEventType::ButtonDown:
            if (ev.button == MouseButton::Left && inside) {
                m_pressed = true;
                return true;
            }
            break;

        case MouseEventType::ButtonUp:
            if (ev.button == MouseButton::Left && m_pressed) {
                m_pressed = false;
                if (inside) {
                    toggle();
                    return true;
                }
            }
            m_pressed = false;
            break;

        default:
            break;
    }

    return false;
}

void CheckBox::update(float dt) {
    (void)dt;
}

void CheckBox::render(Renderer2D& renderer) {
    if (!m_visible || m_bounds.width <= 0.0f || m_bounds.height <= 0.0f) {
        return;
    }

    // Vertically center the checkbox square inside the widget bounds
    float box_y = m_bounds.y + (m_bounds.height - m_style.box_size) * 0.5f;
    Rect box_rect(m_bounds.x, box_y, m_style.box_size, m_style.box_size);

    Color fill;
    Color border;

    if (!m_enabled) {
        fill = m_style.box_background;
        border = m_style.disabled_color;
    } else if (m_checked) {
        fill = m_style.checked_fill;
        border = m_style.checked_fill;
    } else {
        fill = m_style.box_background;
        border = m_hovered ? m_style.hover_border : m_style.box_border;
    }

    // 1. Draw rounded square container
    renderer.draw_rounded_rect(
        box_rect,
        m_style.corner_radius,
        fill,
        border,
        1.0f
    );

    // 2. Draw checkmark if checked
    if (m_checked) {
        renderer.draw_checkmark(box_rect, m_style.checkmark_color, 2.0f);
    }

    // 3. Draw accompanying text label strictly clipped to widget area
    if (!m_label.empty()) {
        float text_x = m_bounds.x + m_style.box_size + m_style.text_spacing;
        Vec2 text_size = renderer.font().measure_text(m_label, m_style.text_scale);
        float text_y = m_bounds.y + (m_bounds.height - text_size.y) * 0.5f;

        Color label_color = m_enabled ? m_style.text_color : m_style.disabled_color;

        renderer.draw_text(
            m_label,
            Vec2(text_x, text_y),
            label_color,
            m_style.text_scale
        );
    }
}

} // namespace arin
