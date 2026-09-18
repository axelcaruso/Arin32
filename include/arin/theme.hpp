/*
 * Arin32 - Modern OpenGL Graphical User Interface Library
 *
 * Copyright (c) 2026, Arin32 Contributors
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

#ifndef ARIN32_THEME_HPP
#define ARIN32_THEME_HPP

#include "types.hpp"

namespace arin {

/**
 * @brief Complete styling configuration for a Button widget.
 *
 * Holds visual properties for all button interactive states:
 * Normal, Hovered, Active (Pressed down), and Disabled.
 * Supports rounded corners, smooth borders, and text colors.
 */
struct ButtonStyle {
    // Background Colors
    Color background_color{Color::from_hex(0x2563EB)};       ///< Default idle background (Modern Blue)
    Color hover_color{Color::from_hex(0x3B82F6)};            ///< Background when cursor is over the button
    Color active_color{Color::from_hex(0x1D4ED8)};           ///< Background when mouse button is held down
    Color disabled_color{Color::from_hex(0x334155)};         ///< Background when button is disabled

    // Text Colors
    Color text_color{Color::white()};                        ///< Label color in normal state
    Color text_hover_color{Color::white()};                  ///< Label color in hover state
    Color text_active_color{Color::from_hex(0xE2E8F0)};      ///< Label color in active state
    Color text_disabled_color{Color::from_hex(0x64748B)};    ///< Label color in disabled state

    // Border Configuration
    Color border_color{Color::transparent()};                ///< Idle border stroke color
    Color border_hover_color{Color::transparent()};          ///< Hover border stroke color
    Color border_active_color{Color::transparent()};         ///< Active border stroke color
    float border_width{0.0f};                                ///< Border thickness in pixels (0.0 = no border)

    // Geometry & Typography
    float corner_radius{8.0f};                               ///< Corner curvature in pixels (0 = rectangle)
    float text_scale{1.0f};                                  ///< Multiplier for font size (1.0 = standard)
    Padding padding{16.0f, 10.0f};                           ///< Content margins (horizontal, vertical)

    // Shadow Configuration
    Color shadow_color{Color(0.0f, 0.0f, 0.0f, 0.25f)};      ///< Drop shadow tint
    Vec2 shadow_offset{0.0f, 2.0f};                          ///< Drop shadow displacement (x, y)
    float shadow_blur{4.0f};                                 ///< Drop shadow blur radius

    /**
     * @brief Creates a Primary button style (high emphasis, modern royal blue).
     */
    static ButtonStyle primary() {
        ButtonStyle s;
        s.background_color = Color::from_hex(0x2563EB);
        s.hover_color      = Color::from_hex(0x3B82F6);
        s.active_color     = Color::from_hex(0x1D4ED8);
        s.text_color       = Color::white();
        s.corner_radius    = 8.0f;
        return s;
    }

    /**
     * @brief Creates a Secondary button style (subtle dark slate tone).
     */
    static ButtonStyle secondary() {
        ButtonStyle s;
        s.background_color = Color::from_hex(0x334155);
        s.hover_color      = Color::from_hex(0x475569);
        s.active_color     = Color::from_hex(0x1E293B);
        s.text_color       = Color::from_hex(0xF8FAFC);
        s.corner_radius    = 8.0f;
        return s;
    }

    /**
     * @brief Creates a Success button style (positive action, emerald green).
     */
    static ButtonStyle success() {
        ButtonStyle s;
        s.background_color = Color::from_hex(0x16A34A);
        s.hover_color      = Color::from_hex(0x22C55E);
        s.active_color     = Color::from_hex(0x15803D);
        s.text_color       = Color::white();
        s.corner_radius    = 8.0f;
        return s;
    }

    /**
     * @brief Creates a Danger button style (destructive action, vibrant red).
     */
    static ButtonStyle danger() {
        ButtonStyle s;
        s.background_color = Color::from_hex(0xDC2626);
        s.hover_color      = Color::from_hex(0xEF4444);
        s.active_color     = Color::from_hex(0xB91C1C);
        s.text_color       = Color::white();
        s.corner_radius    = 8.0f;
        return s;
    }

    /**
     * @brief Creates an Outline button style (transparent fill with sleek border).
     * @param accent_color The color for border and text.
     */
    static ButtonStyle outline(Color accent_color = Color::from_hex(0x38BDF8)) {
        ButtonStyle s;
        s.background_color    = Color::transparent();
        s.hover_color         = accent_color.with_alpha(0.15f);
        s.active_color        = accent_color.with_alpha(0.30f);
        s.text_color          = accent_color;
        s.text_hover_color    = Color::white();
        s.border_color        = accent_color;
        s.border_hover_color  = accent_color;
        s.border_active_color = accent_color;
        s.border_width        = 1.5f;
        s.corner_radius       = 8.0f;
        return s;
    }
};

/**
 * @brief Global UI Theme container defining default styles and color schemes.
 */
class Theme {
public:
    Color background_color{Color::from_hex(0x0F172A)}; ///< Window clear color (Deep slate navy)
    Color text_color{Color::from_hex(0xF8FAFC)};       ///< Default font color
    ButtonStyle button_default{ButtonStyle::primary()}; ///< Default button style

    /**
     * @brief Returns a reference to the global active theme instance.
     */
    static Theme& get() {
        static Theme global_theme;
        return global_theme;
    }

    /**
     * @brief Generates a modern Dark Theme (default).
     */
    static Theme dark() {
        Theme t;
        t.background_color = Color::from_hex(0x0F172A);
        t.text_color       = Color::from_hex(0xF8FAFC);
        t.button_default   = ButtonStyle::primary();
        return t;
    }

    /**
     * @brief Generates a clean Light Theme.
     */
    static Theme light() {
        Theme t;
        t.background_color = Color::from_hex(0xF8FAFC);
        t.text_color       = Color::from_hex(0x0F172A);
        t.button_default   = ButtonStyle::primary();
        return t;
    }
};

} // namespace arin

#endif // ARIN32_THEME_HPP
