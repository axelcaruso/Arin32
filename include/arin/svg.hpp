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

#ifndef ARIN32_SVG_HPP
#define ARIN32_SVG_HPP

#include "types.hpp"
#include "metrics.hpp"
#include "widget.hpp"
#include "texture.hpp"
#include "image.hpp"
#include "renderer.hpp"
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace arin {

/**
 * @brief Represents a parsed Scalable Vector Graphics (SVG) document.
 *
 * Uses the lightweight, zero-dependency NanoSVG engine to parse SVG paths,
 * shapes, strokes, and fills. Supports rasterizing directly to 32-bit RGBA
 * pixel arrays or OpenGL 2D textures at arbitrary display scales.
 */
class SvgDocument {
public:
    SvgDocument();
    ~SvgDocument();

    // Disable copying to manage internal parsed structures safely
    SvgDocument(const SvgDocument&) = delete;
    SvgDocument& operator=(const SvgDocument&) = delete;

    // Support move semantics
    SvgDocument(SvgDocument&& other) noexcept;
    SvgDocument& operator=(SvgDocument&& other) noexcept;

    /**
     * @brief Loads and parses an SVG file from the filesystem.
     * @param filepath Absolute or relative path to the .svg file.
     * @param dpi Target display resolution in dots per inch (default: 96.0f).
     * @return Shared pointer to SvgDocument, or nullptr on failure.
     */
    static std::shared_ptr<SvgDocument> load_from_file(
        const std::string& filepath,
        float dpi = 96.0f
    );

    /**
     * @brief Parses an SVG document from a UTF-8 XML string in memory.
     * @param svg_xml String containing SVG XML markup.
     * @param dpi Target display resolution in dots per inch (default: 96.0f).
     * @return Shared pointer to SvgDocument, or nullptr on failure.
     */
    static std::shared_ptr<SvgDocument> load_from_memory(
        const std::string& svg_xml,
        float dpi = 96.0f
    );

    /**
     * @brief Parses an SVG document from a raw byte buffer.
     * @param data Pointer to raw SVG data bytes.
     * @param size_bytes Number of bytes in buffer.
     * @param dpi Target display resolution in dots per inch (default: 96.0f).
     * @return Shared pointer to SvgDocument, or nullptr on failure.
     */
    static std::shared_ptr<SvgDocument> load_from_memory(
        const uint8_t* data,
        size_t size_bytes,
        float dpi = 96.0f
    );

    /// @brief Returns true if an SVG document is successfully parsed and ready.
    bool is_valid() const;

    /// @brief Native width defined by the SVG viewBox or width attribute.
    float width() const { return m_width; }

    /// @brief Native height defined by the SVG viewBox or height attribute.
    float height() const { return m_height; }

    /// @brief Native aspect ratio (width / height).
    float aspect_ratio() const {
        return (m_height > 0.0f) ? (m_width / m_height) : 1.0f;
    }

    /// @brief Releases internal parsed SVG structures.
    void release();

    /**
     * @brief Rasterizes the vector graphic into a 32-bit RGBA pixel array.
     *
     * @param target_width Requested pixel width (0 for native width * scale).
     * @param target_height Requested pixel height (0 for native height * scale).
     * @param scale Scaling factor applied when target dimensions are 0 (default: 1.0f).
     * @param out_width Optional output pointer receiving the final raster width.
     * @param out_height Optional output pointer receiving the final raster height.
     * @return Vector of RGBA bytes (size = out_width * out_height * 4), or empty on error.
     */
    std::vector<uint8_t> rasterize_rgba(
        int target_width = 0,
        int target_height = 0,
        float scale = 1.0f,
        int* out_width = nullptr,
        int* out_height = nullptr
    ) const;

    /**
     * @brief Directly creates an OpenGL 2D Texture from this vector graphic.
     *
     * @param target_width Target texture pixel width (0 for native width * scale).
     * @param target_height Target texture pixel height (0 for native height * scale).
     * @param scale Scaling factor applied when target dimensions are 0.
     * @param filter Texture filtering mode (default: Linear).
     * @return Shared pointer to created Texture, or nullptr on failure.
     */
    std::shared_ptr<Texture> create_texture(
        int target_width = 0,
        int target_height = 0,
        float scale = 1.0f,
        TextureFilter filter = TextureFilter::Linear
    ) const;

private:
    void* m_image{nullptr}; // NSVGimage* opaque pointer
    float m_width{0.0f};
    float m_height{0.0f};
};

/**
 * @brief High-level Scalable Vector Graphics widget conforming to IWidget.
 *
 * Automatically manages resolution-independent crisp rendering by rasterizing
 * the underlying SVG vector graphic to match its displayed bounding box.
 */
class SvgImage : public IWidget {
public:
    /**
     * @brief Constructs an empty SvgImage widget at (0, 0).
     */
    SvgImage();

    /**
     * @brief Constructs an SvgImage from an existing SvgDocument.
     * @param document Shared pointer to parsed SvgDocument.
     * @param x Horizontal coordinate in pixels.
     * @param y Vertical coordinate in pixels.
     * @param width Display width in pixels.
     * @param height Display height in pixels.
     * @param scale_mode Content scaling policy (default: Fit).
     */
    SvgImage(
        std::shared_ptr<SvgDocument> document,
        float x = 0.0f,
        float y = 0.0f,
        float width = 32.0f,
        float height = 32.0f,
        ImageScaleMode scale_mode = ImageScaleMode::Fit
    );

    /**
     * @brief Constructs an SvgImage by loading an SVG file from disk.
     * @param filepath Path to SVG file.
     * @param x Horizontal coordinate in pixels.
     * @param y Vertical coordinate in pixels.
     * @param width Display width in pixels.
     * @param height Display height in pixels.
     * @param scale_mode Content scaling policy (default: Fit).
     */
    SvgImage(
        const std::string& filepath,
        float x = 0.0f,
        float y = 0.0f,
        float width = 32.0f,
        float height = 32.0f,
        ImageScaleMode scale_mode = ImageScaleMode::Fit
    );

    ~SvgImage() override = default;

    // --- IWidget Interface Implementation ---

    const Rect& bounds() const override { return m_bounds; }

    SvgImage& set_bounds(const Rect& bounds) override;
    SvgImage& set_position(float x, float y) override;
    SvgImage& set_size(float width, float height) override;

    bool handle_mouse(const MouseEvent& ev) override;
    void update(float dt) override;
    void render(Renderer2D& renderer) override;
    bool is_visible() const override { return m_visible; }
    bool is_enabled() const override { return m_enabled; }

    // --- SvgImage Properties & Chaining ---

    /// @brief Gets reference to the active SvgDocument.
    std::shared_ptr<SvgDocument> document() const { return m_document; }

    /// @brief Assigns a new SvgDocument to display.
    SvgImage& set_document(std::shared_ptr<SvgDocument> document);

    /// @brief Loads a new SVG file from disk.
    bool load_from_file(const std::string& filepath);

    /// @brief Loads an SVG from an in-memory XML string.
    bool load_from_memory(const std::string& svg_xml);

    /// @brief Gets active scaling mode.
    ImageScaleMode scale_mode() const { return m_scale_mode; }

    /// @brief Sets scaling algorithm (Fit, Fill, Stretch, Center).
    SvgImage& set_scale_mode(ImageScaleMode mode);

    /// @brief Gets corner radius in pixels.
    float corner_radius() const { return m_corner_radius; }

    /// @brief Sets corner radius for GPU-accelerated rounded edges.
    SvgImage& set_corner_radius(float radius) {
        m_corner_radius = std::max(0.0f, radius);
        return *this;
    }

    /// @brief Gets color tint multiplier.
    const Color& tint() const { return m_tint; }

    /// @brief Sets color tint and opacity modulation.
    SvgImage& set_tint(const Color& tint) {
        m_tint = tint;
        return *this;
    }

    /// @brief Sets widget visibility.
    SvgImage& set_visible(bool visible) {
        m_visible = visible;
        return *this;
    }

    /// @brief Sets whether the widget is interactive.
    SvgImage& set_enabled(bool enabled) {
        m_enabled = enabled;
        return *this;
    }

    /// @brief Forces re-rasterization of the texture at current target dimensions.
    void invalidate_texture();

    /// @brief Gets current cached rasterized texture.
    std::shared_ptr<Texture> texture() const { return m_texture; }

    /**
     * @brief Computes destination rectangle inside bounds according to scale_mode.
     */
    Rect compute_content_rect() const;

private:
    void ensure_texture();

    Rect m_bounds{0.0f, 0.0f, 32.0f, 32.0f};
    std::shared_ptr<SvgDocument> m_document{nullptr};
    std::shared_ptr<Texture> m_texture{nullptr};
    ImageScaleMode m_scale_mode{ImageScaleMode::Fit};
    float m_corner_radius{0.0f};
    Color m_tint{Color::white()};
    bool m_visible{true};
    bool m_enabled{true};
    bool m_needs_rasterize{true};
    int m_last_raster_w{0};
    int m_last_raster_h{0};
};

} // namespace arin

#endif // ARIN32_SVG_HPP
