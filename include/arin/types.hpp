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

#ifndef ARIN32_TYPES_HPP
#define ARIN32_TYPES_HPP

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <string>

namespace arin {

/**
 * @brief Represents a two-dimensional floating-point vector or point in 2D space.
 *
 * In Arin32, the standard window coordinate system has its origin (0, 0)
 * at the top-left corner of the window. The X axis increases to the right,
 * and the Y axis increases downwards.
 */
struct Vec2 {
    float x{0.0f}; ///< Horizontal coordinate (pixels from left).
    float y{0.0f}; ///< Vertical coordinate (pixels from top).

    /**
     * @brief Constructs a Vec2 at (0, 0).
     */
    constexpr Vec2() = default;

    /**
     * @brief Constructs a Vec2 with explicit coordinates.
     * @param in_x Horizontal position.
     * @param in_y Vertical position.
     */
    constexpr Vec2(float in_x, float in_y) : x(in_x), y(in_y) {}

    // Vector arithmetic operations
    constexpr Vec2 operator+(const Vec2& rhs) const { return Vec2(x + rhs.x, y + rhs.y); }
    constexpr Vec2 operator-(const Vec2& rhs) const { return Vec2(x - rhs.x, y - rhs.y); }
    constexpr Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
    constexpr Vec2 operator/(float scalar) const { return Vec2(x / scalar, y / scalar); }

    Vec2& operator+=(const Vec2& rhs) { x += rhs.x; y += rhs.y; return *this; }
    Vec2& operator-=(const Vec2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
    Vec2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
    Vec2& operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }

    constexpr bool operator==(const Vec2& rhs) const {
        return std::abs(x - rhs.x) < 1e-5f && std::abs(y - rhs.y) < 1e-5f;
    }

    constexpr bool operator!=(const Vec2& rhs) const {
        return !(*this == rhs);
    }

    /**
     * @brief Calculates the Euclidean distance between this point and another point.
     * @param other Target point.
     * @return Distance in pixels.
     */
    float distance_to(const Vec2& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    /**
     * @brief Linearly interpolates between this vector and another.
     * @param other Target vector.
     * @param t Interpolation factor (0.0 = this, 1.0 = other).
     * @return Interpolated vector.
     */
    Vec2 lerp(const Vec2& other, float t) const {
        return Vec2(x + (other.x - x) * t, y + (other.y - y) * t);
    }
};

/**
 * @brief Represents an axis-aligned 2D rectangle in screen space.
 *
 * Position (x, y) defines the top-left corner, with dimensions (width, height).
 */
struct Rect {
    float x{0.0f};      ///< X coordinate of the top-left corner.
    float y{0.0f};      ///< Y coordinate of the top-left corner.
    float width{0.0f};  ///< Width of the rectangle (must be non-negative).
    float height{0.0f}; ///< Height of the rectangle (must be non-negative).

    /**
     * @brief Default constructor creating an empty rectangle at (0, 0, 0, 0).
     */
    constexpr Rect() = default;

    /**
     * @brief Constructs a rectangle with explicit position and size.
     * @param in_x X coordinate of top-left corner.
     * @param in_y Y coordinate of top-left corner.
     * @param in_width Width in pixels.
     * @param in_height Height in pixels.
     */
    constexpr Rect(float in_x, float in_y, float in_width, float in_height)
        : x(in_x), y(in_y), width(in_width), height(in_height) {}

    /**
     * @brief Convenience constructor from position vector and dimensions vector.
     * @param pos Top-left coordinate.
     * @param size Size vector (width, height).
     */
    constexpr Rect(Vec2 pos, Vec2 size)
        : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

    /// @brief Gets the top-left position as a Vec2.
    constexpr Vec2 position() const { return Vec2(x, y); }

    /// @brief Gets the dimensions as a Vec2 (width, height).
    constexpr Vec2 size() const { return Vec2(width, height); }

    /// @brief Left boundary X coordinate.
    constexpr float left() const { return x; }

    /// @brief Right boundary X coordinate.
    constexpr float right() const { return x + width; }

    /// @brief Top boundary Y coordinate.
    constexpr float top() const { return y; }

    /// @brief Bottom boundary Y coordinate.
    constexpr float bottom() const { return y + height; }

    /// @brief Center point of the rectangle.
    constexpr Vec2 center() const { return Vec2(x + width * 0.5f, y + height * 0.5f); }

    /**
     * @brief Checks if a given 2D point lies within this rectangle.
     *
     * Boundaries are considered inclusive on left/top and exclusive or inclusive on right/bottom.
     * Used for mouse hit-testing (checking if cursor is hovering over a button).
     *
     * @param point The point to test in screen coordinates.
     * @return true if point is inside the rectangle, false otherwise.
     */
    constexpr bool contains(const Vec2& point) const {
        return point.x >= x && point.x <= (x + width) &&
               point.y >= y && point.y <= (y + height);
    }

    /**
     * @brief Creates an expanded version of this rectangle.
     * @param amount Pixels to expand on each side (negative values shrink).
     * @return New expanded Rect.
     */
    constexpr Rect expanded(float amount) const {
        return Rect(x - amount, y - amount, width + amount * 2.0f, height + amount * 2.0f);
    }

    /**
     * @brief Checks if this rectangle intersects with another rectangle.
     * @param other Rectangle to test against.
     * @return true if rectangles overlap, false otherwise.
     */
    constexpr bool intersects(const Rect& other) const {
        return left() < other.right() && right() > other.left() &&
               top() < other.bottom() && bottom() > other.top();
    }

    constexpr bool operator==(const Rect& rhs) const {
        return std::abs(x - rhs.x) < 1e-5f &&
               std::abs(y - rhs.y) < 1e-5f &&
               std::abs(width - rhs.width) < 1e-5f &&
               std::abs(height - rhs.height) < 1e-5f;
    }

    constexpr bool operator!=(const Rect& rhs) const {
        return !(*this == rhs);
    }
};

/**
 * @brief Represents an RGBA 32-bit color.
 *
 * Channels are internally stored as normalized floating-point values in the range [0.0, 1.0].
 * This makes shader uniform passing and color blending mathematically direct and precise.
 */
struct Color {
    float r{0.0f}; ///< Red channel [0.0f, 1.0f].
    float g{0.0f}; ///< Green channel [0.0f, 1.0f].
    float b{0.0f}; ///< Blue channel [0.0f, 1.0f].
    float a{1.0f}; ///< Alpha channel [0.0f = fully transparent, 1.0f = fully opaque].

    /**
     * @brief Default constructor: fully opaque black.
     */
    constexpr Color() = default;

    /**
     * @brief Constructs a color from normalized floats [0.0, 1.0].
     * @param in_r Red component.
     * @param in_g Green component.
     * @param in_b Blue component.
     * @param in_a Alpha component (defaults to 1.0 = opaque).
     */
    constexpr Color(float in_r, float in_g, float in_b, float in_a = 1.0f)
        : r(in_r), g(in_g), b(in_b), a(in_a) {}

    /**
     * @brief Creates a Color from standard 8-bit per channel integers [0, 255].
     * @param red Red component [0..255].
     * @param green Green component [0..255].
     * @param blue Blue component [0..255].
     * @param alpha Alpha component [0..255], defaults to 255.
     * @return Constructed Color.
     */
    static constexpr Color from_rgba8(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) {
        return Color(
            static_cast<float>(red) / 255.0f,
            static_cast<float>(green) / 255.0f,
            static_cast<float>(blue) / 255.0f,
            static_cast<float>(alpha) / 255.0f
        );
    }

    /**
     * @brief Creates a Color from a 24-bit RGB or 32-bit RGBA hexadecimal literal.
     *
     * Example:
     *   Color::from_hex(0x2A85FF)       // Opaque electric blue
     *   Color::from_hex(0x2A85FFFF, true) // Explicit RGBA with alpha
     *
     * @param hex Value in format 0xRRGGBB (or 0xRRGGBBAA if has_alpha=true).
     * @param has_alpha If true, the lowest 8 bits represent alpha. If false, alpha is 1.0.
     * @return Constructed Color.
     */
    static constexpr Color from_hex(uint32_t hex, bool has_alpha = false) {
        if (has_alpha) {
            uint8_t red   = static_cast<uint8_t>((hex >> 24) & 0xFF);
            uint8_t green = static_cast<uint8_t>((hex >> 16) & 0xFF);
            uint8_t blue  = static_cast<uint8_t>((hex >> 8) & 0xFF);
            uint8_t alpha = static_cast<uint8_t>(hex & 0xFF);
            return from_rgba8(red, green, blue, alpha);
        } else {
            uint8_t red   = static_cast<uint8_t>((hex >> 16) & 0xFF);
            uint8_t green = static_cast<uint8_t>((hex >> 8) & 0xFF);
            uint8_t blue  = static_cast<uint8_t>(hex & 0xFF);
            return from_rgba8(red, green, blue, 255);
        }
    }

    /**
     * @brief Returns a copy of this color with a modified alpha value.
     * @param new_alpha New opacity value [0.0, 1.0].
     * @return Altered Color.
     */
    constexpr Color with_alpha(float new_alpha) const {
        return Color(r, g, b, new_alpha);
    }

    /**
     * @brief Linearly interpolates between two colors.
     * @param other Destination color.
     * @param t Blend factor [0.0 = this, 1.0 = other].
     * @return Blended Color.
     */
    Color lerp(const Color& other, float t) const {
        t = std::clamp(t, 0.0f, 1.0f);
        return Color(
            r + (other.r - r) * t,
            g + (other.g - g) * t,
            b + (other.b - b) * t,
            a + (other.a - a) * t
        );
    }

    constexpr bool operator==(const Color& rhs) const {
        return std::abs(r - rhs.r) < 1e-4f &&
               std::abs(g - rhs.g) < 1e-4f &&
               std::abs(b - rhs.b) < 1e-4f &&
               std::abs(a - rhs.a) < 1e-4f;
    }

    constexpr bool operator!=(const Color& rhs) const {
        return !(*this == rhs);
    }

    // Common Color Presets
    static constexpr Color transparent() { return Color(0.0f, 0.0f, 0.0f, 0.0f); }
    static constexpr Color black()       { return Color(0.0f, 0.0f, 0.0f, 1.0f); }
    static constexpr Color white()       { return Color(1.0f, 1.0f, 1.0f, 1.0f); }
    static constexpr Color red()         { return Color(1.0f, 0.0f, 0.0f, 1.0f); }
    static constexpr Color green()       { return Color(0.0f, 1.0f, 0.0f, 1.0f); }
    static constexpr Color blue()        { return Color(0.0f, 0.0f, 1.0f, 1.0f); }

    // Modern UI Color Palette presets
    static constexpr Color primary()     { return from_hex(0x2563EB); } ///< Modern royal blue
    static constexpr Color secondary()   { return from_hex(0x475569); } ///< Slate gray
    static constexpr Color success()     { return from_hex(0x16A34A); } ///< Emerald green
    static constexpr Color warning()     { return from_hex(0xD97706); } ///< Amber
    static constexpr Color danger()      { return from_hex(0xDC2626); } ///< Crimson red
    static constexpr Color surface()     { return from_hex(0x1E293B); } ///< Dark surface slate
    static constexpr Color background()  { return from_hex(0x0F172A); } ///< Deep background navy
};

/**
 * @brief Inner/outer spacing specification (padding or margin) along 4 edges.
 */
struct Padding {
    float left{0.0f};
    float top{0.0f};
    float right{0.0f};
    float bottom{0.0f};

    constexpr Padding() = default;
    constexpr Padding(float uniform) : left(uniform), top(uniform), right(uniform), bottom(uniform) {}
    constexpr Padding(float horizontal, float vertical)
        : left(horizontal), top(vertical), right(horizontal), bottom(vertical) {}
    constexpr Padding(float in_left, float in_top, float in_right, float in_bottom)
        : left(in_left), top(in_top), right(in_right), bottom(in_bottom) {}

    constexpr bool operator==(const Padding& rhs) const {
        return std::abs(left - rhs.left) < 1e-5f &&
               std::abs(top - rhs.top) < 1e-5f &&
               std::abs(right - rhs.right) < 1e-5f &&
               std::abs(bottom - rhs.bottom) < 1e-5f;
    }

    constexpr bool operator!=(const Padding& rhs) const {
        return !(*this == rhs);
    }
};

} // namespace arin

#endif // ARIN32_TYPES_HPP
