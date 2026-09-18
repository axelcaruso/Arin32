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

namespace arin {

/**
 * @brief Default constructor creating a standard 85x32 button.
 */
Button::Button()
    : m_text("Button"), m_bounds(0.0f, 0.0f, 85.0f, 32.0f), m_style(ButtonStyle::primary()) {}

/**
 * @brief Constructs a button with a given label.
 */
Button::Button(std::string label)
    : m_text(std::move(label)), m_bounds(0.0f, 0.0f, 85.0f, 32.0f), m_style(ButtonStyle::primary()) {}

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

/**
 * @brief Renders the button with appropriate colors and typography.
 */
void Button::render(Renderer2D& renderer) {
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

    // Render centered button label
    renderer.draw_text_centered(
        m_text,
        m_bounds,
        text_color,
        m_style.text_scale
    );
}

} // namespace arin
