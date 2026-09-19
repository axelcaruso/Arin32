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

#ifndef ARIN32_IMAGE_HPP
#define ARIN32_IMAGE_HPP

#include "types.hpp"
#include "widget.hpp"
#include "texture.hpp"
#include "renderer.hpp"
#include <memory>
#include <string>

namespace arin {

/**
 * @brief High-level Image display widget conforming to the Arin32 IWidget architecture.
 *
 * Supports:
 * - Direct texture binding or path-based lazy loading
 * - Multiple layout scaling strategies (Fit, Fill, Stretch, Center)
 * - GPU Signed Distance Field anti-aliased corner rounding (avatars, thumbnails)
 * - Color tinting and opacity blending
 * - Integration with automatic layout containers (VBox, HBox)
 */
class Image : public IWidget {
public:
    /**
     * @brief Constructs an empty Image widget at (0, 0).
     */
    Image();

    /**
     * @brief Constructs an Image widget bound to a texture.
     * @param texture Shared pointer to an initialized Texture.
     * @param x Horizontal position in window coordinates.
     * @param y Vertical position in window coordinates.
     * @param width Display width in pixels.
     * @param height Display height in pixels.
     * @param scale_mode Image scaling algorithm (default: Fit).
     */
    Image(
        std::shared_ptr<Texture> texture,
        float x = 0.0f,
        float y = 0.0f,
        float width = 100.0f,
        float height = 100.0f,
        ImageScaleMode scale_mode = ImageScaleMode::Fit
    );

    /**
     * @brief Constructs an Image widget by loading an image file from disk.
     * @param filepath Path to the image file on disk.
     * @param x Horizontal position in window coordinates.
     * @param y Vertical position in window coordinates.
     * @param width Display width in pixels.
     * @param height Display height in pixels.
     * @param scale_mode Image scaling algorithm (default: Fit).
     */
    Image(
        const std::string& filepath,
        float x = 0.0f,
        float y = 0.0f,
        float width = 100.0f,
        float height = 100.0f,
        ImageScaleMode scale_mode = ImageScaleMode::Fit
    );

    ~Image() override = default;

    // --- IWidget Interface Implementation ---

    const Rect& bounds() const override { return m_bounds; }

    Image& set_bounds(const Rect& bounds) override {
        m_bounds = bounds;
        return *this;
    }

    Image& set_position(float x, float y) override {
        m_bounds.x = x;
        m_bounds.y = y;
        return *this;
    }

    Image& set_size(float width, float height) override {
        m_bounds.width = width;
        m_bounds.height = height;
        return *this;
    }

    bool handle_mouse(const MouseEvent& ev) override;
    void update(float dt) override;
    void render(Renderer2D& renderer) override;
    bool is_visible() const override { return m_visible; }
    bool is_enabled() const override { return m_enabled; }

    // --- Image Attributes & Chaining ---

    /// @brief Gets reference to the active texture.
    std::shared_ptr<Texture> texture() const { return m_texture; }

    /// @brief Assigns a new texture to display.
    Image& set_texture(std::shared_ptr<Texture> texture) {
        m_texture = std::move(texture);
        return *this;
    }

    /// @brief Loads a new image from disk into this widget.
    bool load_from_file(const std::string& filepath);

    /// @brief Gets active scaling mode.
    ImageScaleMode scale_mode() const { return m_scale_mode; }

    /// @brief Sets scaling algorithm.
    Image& set_scale_mode(ImageScaleMode mode) {
        m_scale_mode = mode;
        return *this;
    }

    /// @brief Gets corner radius in pixels.
    float corner_radius() const { return m_corner_radius; }

    /// @brief Sets corner radius for GPU-accelerated rounded edges.
    Image& set_corner_radius(float radius) {
        m_corner_radius = std::max(0.0f, radius);
        return *this;
    }

    /// @brief Gets color tint multiplier.
    const Color& tint() const { return m_tint; }

    /// @brief Sets color tint and opacity modulation.
    Image& set_tint(const Color& tint) {
        m_tint = tint;
        return *this;
    }

    /// @brief Sets widget visibility.
    Image& set_visible(bool visible) {
        m_visible = visible;
        return *this;
    }

    /// @brief Sets whether the image widget is enabled.
    Image& set_enabled(bool enabled) {
        m_enabled = enabled;
        return *this;
    }

    /**
     * @brief Computes the destination rectangle after applying the active scale mode.
     *
     * In Fit and Center modes, the image may be smaller than the total bounding box.
     *
     * @return Position and size of the drawn image pixels.
     */
    Rect compute_content_rect() const;

private:
    Rect m_bounds{0.0f, 0.0f, 100.0f, 100.0f};
    std::shared_ptr<Texture> m_texture{nullptr};
    ImageScaleMode m_scale_mode{ImageScaleMode::Fit};
    float m_corner_radius{0.0f};
    Color m_tint{Color::white()};
    bool m_visible{true};
    bool m_enabled{true};
};

} // namespace arin

#endif // ARIN32_IMAGE_HPP
