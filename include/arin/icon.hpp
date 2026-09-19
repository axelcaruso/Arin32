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

#ifndef ARIN32_ICON_HPP
#define ARIN32_ICON_HPP

#include "types.hpp"
#include "metrics.hpp"
#include "widget.hpp"
#include "renderer.hpp"
#include <cstdint>

namespace arin {

/**
 * @brief Enumerates crisp, hardware-rendered vector system icons.
 *
 * All icons in Arin32 are mathematically defined vector glyphs rendered via
 * GPU-accelerated primitives and Signed Distance Fields (SDF). This ensures:
 * 1. Zero rasterization blur regardless of display scaling factor or DPI.
 * 2. Instant recoloring through tint parameters without needing multiple PNG assets.
 * 3. Zero dependency on heavyweight external font icon libraries.
 */
enum class IconType : uint8_t {
    None = 0,         ///< No icon displayed
    Folder,           ///< Desktop folder icon
    File,             ///< Standard document icon with dog-eared corner
    Check,            ///< Confirmation checkmark
    Close,            ///< Cancel/Dismiss cross (X)
    Search,           ///< Magnifying glass
    Settings,         ///< Configuration gear cog
    Cut,              ///< Scissors icon for clipboard cut
    Copy,             ///< Dual overlapping sheets for clipboard copy
    Paste,            ///< Clipboard with document sheet for clipboard paste
    Trash,            ///< Wastebasket / delete icon
    Edit,             ///< Pencil / writing utensil
    ChevronRight,     ///< Right-pointing sub-menu navigation chevron (>)
    ChevronDown,      ///< Down-pointing expandable chevron (v)
    ChevronUp,        ///< Up-pointing collapsible chevron (^)
    ChevronLeft,      ///< Left-pointing back chevron (<)
    ArrowRight,       ///< Right directional arrow with stem
    ArrowLeft,        ///< Left directional arrow with stem
    Info,             ///< Informational badge ('i' in circle)
    Warning,          ///< Cautionary triangle with exclamation point
    Error,            ///< Critical error symbol
    More,             ///< Horizontal ellipsis (...) for secondary actions
    Count
};

/**
 * @brief Lightweight vector icon widget conforming to the Arin32 IWidget contract.
 *
 * Can be placed independently into automatic layout containers (VBox, HBox)
 * or positioned explicitly at pixel coordinates.
 */
class Icon : public IWidget {
public:
    /**
     * @brief Constructs an Icon widget.
     * @param type The specific vector icon to render.
     * @param x Horizontal position in window pixels.
     * @param y Vertical position in window pixels.
     * @param size Uniform bounding width and height (default: 16.0f).
     * @param color Icon stroke and fill color (default: clean white).
     */
    Icon(
        IconType type = IconType::None,
        float x = 0.0f,
        float y = 0.0f,
        float size = UiMetrics::kDefaultIconSize,
        const Color& color = Color(240, 240, 240)
    );

    ~Icon() override = default;

    // --- IWidget Interface Implementation ---

    const Rect& bounds() const override { return m_bounds; }

    Icon& set_bounds(const Rect& bounds) override {
        m_bounds = bounds;
        return *this;
    }

    Icon& set_position(float x, float y) override {
        m_bounds.x = x;
        m_bounds.y = y;
        return *this;
    }

    Icon& set_size(float width, float height) override {
        m_bounds.width = width;
        m_bounds.height = height;
        return *this;
    }

    bool handle_mouse(const MouseEvent& ev) override;
    void update(float dt) override;
    void render(Renderer2D& renderer) override;
    bool is_visible() const override { return m_visible; }
    bool is_enabled() const override { return m_enabled; }

    // --- Icon Configuration & Ergonomics ---

    /// @brief Gets current icon type.
    IconType type() const { return m_type; }

    /// @brief Sets the icon type to display.
    Icon& set_type(IconType type) {
        m_type = type;
        return *this;
    }

    /// @brief Gets icon tint color.
    const Color& color() const { return m_color; }

    /// @brief Sets icon tint color.
    Icon& set_color(const Color& color) {
        m_color = color;
        return *this;
    }

    /// @brief Sets widget visibility.
    Icon& set_visible(bool visible) {
        m_visible = visible;
        return *this;
    }

    /// @brief Sets whether the icon is enabled.
    Icon& set_enabled(bool enabled) {
        m_enabled = enabled;
        return *this;
    }

private:
    Rect m_bounds{0.0f, 0.0f, UiMetrics::kDefaultIconSize, UiMetrics::kDefaultIconSize};
    IconType m_type{IconType::None};
    Color m_color{240, 240, 240};
    bool m_visible{true};
    bool m_enabled{true};
};

} // namespace arin

#endif // ARIN32_ICON_HPP
