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

#include "arin/button.hpp"
#include "arin/metrics.hpp"

namespace arin {

/**
 * @brief Default constructor creating a standard 85x32 button.
 */
Button::Button()
    : m_text("Button"), m_bounds(0.0f, 0.0f, UiMetrics::kDefaultButtonSize.x, UiMetrics::kDefaultButtonSize.y), m_style(ButtonStyle::primary()) {}

/**
 * @brief Constructs a button with a given label.
 */
Button::Button(std::string label)
    : m_text(std::move(label)), m_bounds(0.0f, 0.0f, UiMetrics::kDefaultButtonSize.x, UiMetrics::kDefaultButtonSize.y), m_style(ButtonStyle::primary()) {}

/**
 * @brief Constructs a button with explicit position and size.
 */
Button::Button(std::string label, float x, float y, float width, float height)
    : m_text(std::move(label)), m_bounds(x, y, width, height), m_style(ButtonStyle::primary()) {}

/**
 * @brief Constructs a button with a given bounding box.
 */
Button::Button(std::string label, const Rect& bounds)
    : m_text(std::move(label)), m_bounds(bounds), m_style(ButtonStyle::primary()) {}

/**
 * @brief Constructs a button with label, bounds, and style.
 */
Button::Button(std::string label, const Rect& bounds, const ButtonStyle& style)
    : m_text(std::move(label)), m_bounds(bounds), m_style(style) {}

Button& Button::set_text(std::string text) {
    m_text = std::move(text);
    return *this;
}

Button& Button::set_position(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
    return *this;
}

Button& Button::set_position(const Vec2& pos) {
    m_bounds.x = pos.x;
    m_bounds.y = pos.y;
    return *this;
}

Button& Button::set_size(float width, float height) {
    m_bounds.width = width;
    m_bounds.height = height;
    return *this;
}

Button& Button::set_bounds(const Rect& bounds) {
    m_bounds = bounds;
    return *this;
}

Button& Button::set_bounds(float x, float y, float width, float height) {
    m_bounds = Rect(x, y, width, height);
    return *this;
}

Button& Button::set_style(const ButtonStyle& style) {
    m_style = style;
    return *this;
}

Button& Button::set_enabled(bool enabled) {
    if (!enabled) {
        m_state = ButtonState::Disabled;
        m_mouse_inside = false;
        m_pressed_inside = false;
    } else if (m_state == ButtonState::Disabled) {
        m_state = ButtonState::Normal;
    }
    return *this;
}

Button& Button::set_corner_radius(float radius) {
    m_style.corner_radius = radius;
    return *this;
}

Button& Button::on_click(ClickCallback callback) {
    m_click_cb = std::move(callback);
    return *this;
}

Button& Button::on_click(DetailedClickCallback callback) {
    m_detailed_click_cb = std::move(callback);
    return *this;
}

Button& Button::on_hover(HoverCallback callback) {
    m_hover_cb = std::move(callback);
    return *this;
}

/**
 * @brief Evaluates an incoming mouse event against the button bounds.
 *
 * Implements standard desktop GUI button semantics:
 * - A click is registered ONLY if mouse button down occurred inside the button
 *   AND mouse button up occurs inside the button.
 * - If the user presses down inside, drags outside, and releases outside, the click is cancelled.
 */
bool Button::handle_mouse(const MouseEvent& event) {
    if (m_state == ButtonState::Disabled) {
        return false;
    }

    bool inside = m_bounds.contains(event.position);

    switch (event.type) {
        case MouseEventType::Move: {
            if (inside != m_mouse_inside) {
                m_mouse_inside = inside;
                if (m_hover_cb) {
                    m_hover_cb(inside);
                }
            }

            if (m_pressed_inside) {
                m_state = inside ? ButtonState::Pressed : ButtonState::Normal;
            } else {
                m_state = inside ? ButtonState::Hovered : ButtonState::Normal;
            }
            return inside;
        }

        case MouseEventType::ButtonDown: {
            if (event.button == MouseButton::Left && inside) {
                m_pressed_inside = true;
                m_state = ButtonState::Pressed;
                return true;
            }
            break;
        }

        case MouseEventType::ButtonUp: {
            if (event.button == MouseButton::Left && m_pressed_inside) {
                m_pressed_inside = false;
                m_state = inside ? ButtonState::Hovered : ButtonState::Normal;

                if (inside) {
                    // Successful click detected! Invoke registered callbacks
                    if (m_click_cb) {
                        m_click_cb();
                    }
                    if (m_detailed_click_cb) {
                        m_detailed_click_cb(*this);
                    }
                }
                return true;
            }
            break;
        }

        default:
            break;
    }

    return false;
}

Button& Button::set_auto_resize(bool enable) {
    m_auto_resize = enable;
    return *this;
}

Button& Button::set_icon(IconType icon, float size, float spacing) {
    m_icon = icon;
    m_icon_size = size;
    m_icon_spacing = spacing;
    return *this;
}

Button& Button::fit_to_text(const Font& font, float horizontal_padding) {
    // Step 1: Measure content once through the shared helper.
    const Vec2 content = content_size(font);
    // Step 2: Expand by padding, enforcing a minimal clickable extent.
    constexpr float kMinimumButtonExtent = 4.0f;
    m_bounds.width = std::max(kMinimumButtonExtent, content.x + horizontal_padding * 2.0f);
    m_bounds.height = std::max(m_bounds.height, content.y + m_style.padding.top + m_style.padding.bottom);
    return *this;
}

/**
 * @brief Measures button content (icon plus label) without padding.
 */
Vec2 Button::content_size(const Font& font) const {
    // Step 1: Measure the label at the configured text scale.
    const Vec2 text_size = font.measure_text(m_text, m_style.text_scale);
    // Step 2: Account for the optional leading icon and its spacing.
    const float icon_width = (m_icon != IconType::None)
        ? (m_icon_size + (m_text.empty() ? 0.0f : m_icon_spacing))
        : 0.0f;
    // Step 3: Content height is the tallest element (text, icon, or line height).
    const float content_height = std::max({text_size.y, m_icon_size, font.line_height() * m_style.text_scale});
    return Vec2(text_size.x + icon_width, content_height);
}

/**
 * @brief Returns the content rectangle centered inside the button bounds.
 */
Rect Button::content_rect(const Font& font) const {
    return m_bounds.centered(content_size(font));
}

Button& Button::ensure_containment(const Font& font) {
    if (!m_auto_resize) return *this;

    Vec2 text_size = font.measure_text(m_text, m_style.text_scale);
    float icon_w = (m_icon != IconType::None) ? (m_icon_size + (m_text.empty() ? 0.0f : m_icon_spacing)) : 0.0f;
    float total_content_w = text_size.x + icon_w;
    float max_content_h = std::max(text_size.y, (m_icon != IconType::None) ? m_icon_size : 0.0f);

    float min_w = total_content_w + m_style.padding.left + m_style.padding.right;
    float min_h = max_content_h + m_style.padding.top + m_style.padding.bottom;
    if (m_bounds.width < min_w) {
        m_bounds.width = min_w;
    }
    if (m_bounds.height < min_h) {
        m_bounds.height = min_h;
    }
    return *this;
}

/**
 * @brief Renders the button with appropriate colors, guaranteed containment, and typography.
 */
void Button::render(Renderer2D& renderer) {
    // Step 1: Measure content. Render never resizes bounds, so fixed layouts
    // (SpaceBetween, dialogs, control rows) stay exactly as computed.
    // Call fit_to_text() or ensure_containment() explicitly to grow a button.
    Vec2 text_size = renderer.font().measure_text(m_text, m_style.text_scale);
    float effective_scale = m_style.text_scale;
    float icon_w = (m_icon != IconType::None) ? (m_icon_size + (m_text.empty() ? 0.0f : m_icon_spacing)) : 0.0f;
    float total_content_w = text_size.x + icon_w;

    // Step 2: Shrink text to fit when the label exceeds the available width.
    // This applies to both fixed-size and auto-size buttons, since render
    // performs no layout growth anymore.
    {
        float avail_w = m_bounds.width - m_style.padding.left - m_style.padding.right - icon_w;
        if (avail_w > 0.0f && text_size.x > avail_w) {
            effective_scale *= (avail_w / text_size.x);
            text_size = renderer.font().measure_text(m_text, effective_scale);
            total_content_w = text_size.x + icon_w;
        }
    }

    Color bg_color;
    Color text_color;
    Color border_color;

    // Pick colors according to current button state
    switch (m_state) {
        case ButtonState::Disabled:
            bg_color     = m_style.disabled_color;
            text_color   = m_style.text_disabled_color;
            border_color = m_style.border_color;
            break;

        case ButtonState::Pressed:
            bg_color     = m_style.active_color;
            text_color   = m_style.text_active_color;
            border_color = m_style.border_active_color;
            break;

        case ButtonState::Hovered:
            bg_color     = m_style.hover_color;
            text_color   = m_style.text_hover_color;
            border_color = m_style.border_hover_color;
            break;

        case ButtonState::Normal:
        default:
            bg_color     = m_style.background_color;
            text_color   = m_style.text_color;
            border_color = m_style.border_color;
            break;
    }

    // Render soft drop shadow when not pressed or disabled
    if (m_state != ButtonState::Disabled && m_state != ButtonState::Pressed) {
        renderer.draw_shadow(
            m_bounds,
            m_style.corner_radius,
            m_style.shadow_color,
            m_style.shadow_offset,
            m_style.shadow_blur
        );
    }

    // Render anti-aliased rounded background box and border
    renderer.draw_rounded_rect(
        m_bounds,
        m_style.corner_radius,
        bg_color,
        border_color,
        m_style.border_width
    );

    // Step 5: Render button content with shared optical centering and clipping.
    // The whole icon-plus-label row is centered once, then text uses the same
    // baseline-aware helper as every other widget.
    if (m_icon != IconType::None) {
        const float total_content_h = std::max(text_size.y, m_icon_size);
        const Rect row_rect = m_bounds.centered(Vec2(total_content_w, total_content_h));
        const Rect icon_rect = Rect(row_rect.x, m_bounds.centered(Vec2(m_icon_size, m_icon_size)).y, m_icon_size, m_icon_size);
        renderer.draw_icon(m_icon, icon_rect, text_color);

        if (!m_text.empty()) {
            const Rect text_bounds(row_rect.x + m_icon_size + m_icon_spacing, m_bounds.y, text_size.x, m_bounds.height);
            renderer.push_clip_rect(m_bounds);
            renderer.draw_text_in_rect(m_text, text_bounds, text_color, effective_scale, TextAlignH::Left, TextAlignV::Center);
            renderer.pop_clip_rect();
        }
    } else {
        renderer.draw_text_centered_clipped(m_text, m_bounds, text_color, effective_scale);
    }
}

} // namespace arin
