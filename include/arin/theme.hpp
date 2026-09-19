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

#ifndef ARIN32_THEME_HPP
#define ARIN32_THEME_HPP

#include "metrics.hpp"
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
    Color background_color{Color::from_hex(palette::kAccentBlue)}; ///< Default idle background.
    Color hover_color{Color::from_hex(palette::kAccentBlueHover)}; ///< Background when cursor is over the button.
    Color active_color{Color::from_hex(palette::kAccentBlueActive)}; ///< Background while pressed.
    Color disabled_color{Color::from_hex(0xE5E7EB)}; ///< Background when button is disabled.

    // Text Colors
    Color text_color{Color::white()};                        ///< Label color in normal state.
    Color text_hover_color{Color::white()};                  ///< Label color in hover state.
    Color text_active_color{Color::from_hex(0xEEF2F6)};      ///< Label color in active state.
    Color text_disabled_color{Color::from_hex(0x9CA3AF)};    ///< Label color in disabled state.

    // Border Configuration
    Color border_color{Color::transparent()};                ///< Idle border stroke color.
    Color border_hover_color{Color::transparent()};          ///< Hover border stroke color.
    Color border_active_color{Color::transparent()};         ///< Active border stroke color.
    float border_width{0.0f};                                ///< Border thickness in pixels.

    // Geometry and typography.
    static constexpr float kCornerRadius = 4.5f; ///< Standard button corner radius.
    static constexpr float kHorizontalPadding = 14.0f; ///< Standard horizontal content padding.
    static constexpr float kVerticalPadding = 6.0f; ///< Standard vertical content padding.
    float corner_radius{kCornerRadius}; ///< Corner radius in pixels.
    float text_scale{UiMetrics::kButtonTextScale}; ///< Multiplier for font size.
    Padding padding{kHorizontalPadding, kVerticalPadding}; ///< Content margins.

    // Shadow Configuration - Subtle modern soft elevation
    Color shadow_color{Color(0.0f, 0.0f, 0.0f, 0.06f)};      ///< Subtle drop shadow
    Vec2 shadow_offset{0.0f, 1.0f};                          ///< Drop shadow displacement (x, y)
    float shadow_blur{2.0f};                                 ///< Drop shadow blur radius

    /**
     * @brief Creates a Primary button style using the shared accent palette.
     */
    static ButtonStyle primary() {
        ButtonStyle s;
        s.background_color    = Color::from_hex(palette::kAccentBlue);
        s.hover_color         = Color::from_hex(palette::kAccentBlueHover);
        s.active_color        = Color::from_hex(palette::kAccentBlueActive);
        s.text_color          = Color::white();
        s.text_hover_color    = Color::white();
        s.text_active_color   = Color::from_hex(0xEEF2F6);
        s.border_width        = 0.0f;
        s.corner_radius       = ButtonStyle::kCornerRadius;
        return s;
    }

    /**
     * @brief Creates a Secondary button style with a light surface and subtle border.
     */
    static ButtonStyle secondary() {
        ButtonStyle s;
        s.background_color    = Color::from_hex(0xFFFFFF);
        s.hover_color         = Color::from_hex(0xF4F4F5);
        s.active_color        = Color::from_hex(0xE4E4E7);
        s.disabled_color      = Color::from_hex(0xF8FAFC);
        s.text_color          = Color::from_hex(0x18181B);
        s.text_hover_color    = Color::from_hex(0x18181B);
        s.text_active_color   = Color::from_hex(0x18181B);
        s.text_disabled_color = Color::from_hex(0x94A3B8);
        s.border_color        = Color::from_hex(0xD4D4D8);
        s.border_hover_color  = Color::from_hex(0xA1A1AA);
        s.border_active_color = Color::from_hex(0x71717A);
        s.border_width        = 1.0f;
        s.corner_radius       = ButtonStyle::kCornerRadius;
        return s;
    }

    /**
     * @brief Creates a Success button style (positive action, emerald green).
     */
    static ButtonStyle success() {
        ButtonStyle s;
        s.background_color    = Color::from_hex(0x16A34A);
        s.hover_color         = Color::from_hex(0x22C55E);
        s.active_color        = Color::from_hex(0x15803D);
        s.text_color          = Color::white();
        s.border_width        = 0.0f;
        s.corner_radius       = ButtonStyle::kCornerRadius;
        return s;
    }

    /**
     * @brief Creates a Danger button style (destructive action, vibrant red).
     */
    static ButtonStyle danger() {
        ButtonStyle s;
        s.background_color    = Color::from_hex(0xDC2626);
        s.hover_color         = Color::from_hex(0xEF4444);
        s.active_color        = Color::from_hex(0xB91C1C);
        s.text_color          = Color::white();
        s.border_width        = 0.0f;
        s.corner_radius       = ButtonStyle::kCornerRadius;
        return s;
    }

    /**
     * @brief Creates an Outline button style (transparent fill with sleek border).
     * @param accent_color The color for border and text.
     */
    static ButtonStyle outline(Color accent_color = Color::from_hex(palette::kAccentBlue)) {
        ButtonStyle s;
        s.background_color    = Color::transparent();
        s.hover_color         = accent_color.with_alpha(0.10f);
        s.active_color        = accent_color.with_alpha(0.25f);
        s.text_color          = accent_color;
        s.text_hover_color    = accent_color;
        s.border_color        = accent_color;
        s.border_hover_color  = accent_color;
        s.border_active_color = accent_color;
        s.border_width        = 1.0f;
        s.corner_radius       = ButtonStyle::kCornerRadius;
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
