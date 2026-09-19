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
#include "arin/metrics.hpp"
#include "arin/font.hpp"
#include <algorithm>
#include <cmath>

namespace arin {

CheckBox::CheckBox(
    std::string label,
    float x,
    float y,
    bool checked
) : m_label(std::move(label)),
    m_bounds(x, y, UiMetrics::kDefaultCheckBoxSize.x, UiMetrics::kDefaultCheckBoxSize.y),
    m_checked(checked),
    m_auto_resize(true) {
    fit_to_content();
}

CheckBox& CheckBox::set_bounds(const Rect& bounds) {
    m_bounds = bounds;
    m_auto_resize = false;
    return *this;
}

CheckBox& CheckBox::set_position(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
    return *this;
}

CheckBox& CheckBox::set_size(float width, float height) {
    m_bounds.width = width;
    m_bounds.height = height;
    m_auto_resize = false;
    return *this;
}

CheckBox& CheckBox::set_label(std::string label) {
    m_label = std::move(label);
    if (m_auto_resize) {
        fit_to_content();
    }
    return *this;
}

CheckBox& CheckBox::set_style(const CheckBoxStyle& style) {
    m_style = style;
    if (m_auto_resize) {
        fit_to_content();
    }
    return *this;
}

CheckBox& CheckBox::set_auto_resize(bool enable) {
    m_auto_resize = enable;
    return *this;
}

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
    float w = m_style.box_size;
    float h = std::max(UiMetrics::kDefaultCheckBoxSize.y, m_style.box_size);
    if (!m_label.empty()) {
        Vec2 text_size = font.measure_text(m_label, m_style.text_scale);
        w += m_style.text_spacing + text_size.x;
        h = std::max(h, text_size.y);
    }
    m_bounds.width = std::ceil(w);
    m_bounds.height = std::ceil(h);
    return *this;
}

CheckBox& CheckBox::fit_to_content() {
    Font font;
    return fit_to_content(font);
}

CheckBox& CheckBox::ensure_containment(const Font& font) {
    if (!m_auto_resize) return *this;

    float w = m_style.box_size;
    float h = std::max(UiMetrics::kDefaultCheckBoxSize.y, m_style.box_size);
    if (!m_label.empty()) {
        Vec2 text_size = font.measure_text(m_label, m_style.text_scale);
        w += m_style.text_spacing + text_size.x;
        h = std::max(h, text_size.y);
    }
    w = std::ceil(w);
    h = std::ceil(h);
    if (m_bounds.width < w) {
        m_bounds.width = w;
    }
    if (m_bounds.height < h) {
        m_bounds.height = h;
    }
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
    if (m_auto_resize) {
        ensure_containment(renderer.font());
    }

    if (!m_visible || m_bounds.width <= 0.0f || m_bounds.height <= 0.0f) {
        return;
    }

    // Step 1: Vertically center the box with the shared rectangle helper.
    const Rect box_rect = m_bounds.centered(Vec2(m_style.box_size, m_style.box_size));
    const Rect aligned_box_rect(m_bounds.x, box_rect.y, m_style.box_size, m_style.box_size);

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
    renderer.draw_rounded_rect(aligned_box_rect, m_style.corner_radius, fill, border, 1.0f);

    // Step 2: Draw the checkmark when checked.
    if (m_checked) {
        renderer.draw_checkmark(aligned_box_rect, m_style.checkmark_color, 2.0f);
    }

    // Step 3: Draw the label with shared optical vertical centering.
    // The text column starts after the box plus spacing and is vertically
    // centered with the same baseline-aware helper used by every widget.
    if (!m_label.empty()) {
        const float text_x = m_bounds.x + m_style.box_size + m_style.text_spacing;
        Vec2 text_size = renderer.font().measure_text(m_label, m_style.text_scale);
        float effective_scale = m_style.text_scale;
        float avail_w = m_bounds.right() - text_x;
        if (!m_auto_resize && avail_w > 0.0f && text_size.x > avail_w) {
            effective_scale *= (avail_w / text_size.x);
        }
        const Rect text_bounds(text_x, m_bounds.y, std::max(0.0f, avail_w), m_bounds.height);
        const Color label_color = m_enabled ? m_style.text_color : m_style.disabled_color;
        renderer.push_clip_rect(m_bounds);
        renderer.draw_text_in_rect(m_label, text_bounds, label_color, effective_scale, TextAlignH::Left, TextAlignV::Center);
        renderer.pop_clip_rect();
    }
}

} // namespace arin
