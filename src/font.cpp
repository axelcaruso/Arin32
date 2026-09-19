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

#include "arin/font.hpp"
#include <GL/glew.h>
#include <iostream>
#include "font_atlas_data.inl"

namespace arin {

/**
 * @brief Default constructor for Font.
 * Note: Does not allocate OpenGL textures immediately; call init_gl() once
 * an OpenGL context has been made current.
 */
Font::Font() : m_texture_id(0) {}

/**
 * @brief Destructor: cleans up any allocated OpenGL texture resources.
 */
Font::~Font() {
    cleanup_gl();
}

/**
 * @brief Move constructor transferring texture ownership.
 */
Font::Font(Font&& other) noexcept : m_texture_id(other.m_texture_id) {
    other.m_texture_id = 0;
}

/**
 * @brief Move assignment transferring texture ownership.
 */
Font& Font::operator=(Font&& other) noexcept {
    if (this != &other) {
        cleanup_gl();
        m_texture_id = other.m_texture_id;
        other.m_texture_id = 0;
    }
    return *this;
}

/**
 * @brief Uploads the embedded font atlas to the GPU as an OpenGL 2D texture.
 *
 * We upload the single-channel luminance data to a GL_RED (or GL_R8) texture.
 * Bilinear filtering (GL_LINEAR) provides smooth text scaling.
 */
bool Font::init_gl() {
    if (m_texture_id != 0) {
        return true; // Already initialized
    }

    // Generate texture handle
    glGenTextures(1, &m_texture_id);
    if (m_texture_id == 0) {
        std::cerr << "[Arin32::Font] Failed to generate OpenGL texture name." << std::endl;
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    // Set linear filtering for smooth scaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Clamp coordinates to edge so glyph borders don't wrap or bleed
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Ensure 1-byte alignment for uncompressed single-channel data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Upload the font atlas byte array to GPU VRAM
    glTexImage2D(
        GL_TEXTURE_2D,
        0,                  // Mipmap level 0
        GL_RED,             // Internal format: 1 byte per pixel
        internal::FONT_ATLAS_WIDTH,
        internal::FONT_ATLAS_HEIGHT,
        0,                  // Border (must be 0 in OpenGL)
        GL_RED,             // Format of input data
        GL_UNSIGNED_BYTE,   // Data type
        internal::s_font_atlas_pixels
    );

    // Reset unpack alignment back to default 4
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

/**
 * @brief Deletes the font texture if allocated.
 */
void Font::cleanup_gl() {
    if (m_texture_id != 0) {
        glDeleteTextures(1, &m_texture_id);
        m_texture_id = 0;
    }
}

/**
 * @brief Computes text dimensions by summing glyph advances and returning line height.
 */
Vec2 Font::measure_text(const std::string& text, float scale) const {
    float total_width = 0.0f;
    for (char c : text) {
        GlyphInfo g = get_glyph(c);
        total_width += g.advance * scale;
    }
    return Vec2(total_width, line_height() * scale);
}

/**
 * @brief Looks up glyph info from the embedded table.
 */
GlyphInfo Font::get_glyph(char c) const {
    // If character is outside printable ASCII range, fallback to space
    if (static_cast<unsigned char>(c) < 32 || static_cast<unsigned char>(c) > 126) {
        c = ' ';
    }

    const auto& metric = internal::s_glyph_metrics[static_cast<unsigned char>(c) - 32];
    GlyphInfo g;
    g.advance = metric.advance;
    g.u0 = metric.u0;
    g.v0 = metric.v0;
    g.u1 = metric.u1;
    g.v1 = metric.v1;
    g.width = metric.width;
    g.height = metric.height;
    return g;
}

/**
 * @brief Returns base line height.
 */
float Font::line_height() const {
    return internal::FONT_LINE_HEIGHT;
}

/**
 * @brief Returns the baseline offset used for optical vertical centering.
 *
 * The atlas glyph quads reserve generous line spacing. Measurements of the
 * embedded Open Sans atlas show the visible cap height occupying roughly
 * seventy percent of the reported line height, so centering the cap height
 * keeps labels optically centered in buttons, rows, and inputs.
 */
float Font::baseline_offset(float scale) const {
    constexpr float kAscentProportion = 0.35f;
    return line_height() * kAscentProportion * scale;
}

/**
 * @brief Computes the drawing origin for aligned text inside a rectangle.
 */
Vec2 Font::layout_text_in_rect(
    const std::string& text,
    const Rect& bounds,
    float scale,
    TextAlignH align_h,
    TextAlignV align_v
) const {
    const Vec2 text_size = measure_text(text, scale);
    float origin_x = bounds.x;
    float origin_y = bounds.y;

    // Step 1: Resolve the horizontal origin from the alignment policy.
    switch (align_h) {
        case TextAlignH::Left:
            origin_x = bounds.x;
            break;
        case TextAlignH::Right:
            origin_x = bounds.x + bounds.width - text_size.x;
            break;
        case TextAlignH::Center:
        default:
            origin_x = bounds.x + (bounds.width - text_size.x) * detail::kHalf;
            break;
    }

    // Step 2: Resolve the vertical origin. Center uses the visible glyph mass
    // rather than the full line box so text does not appear shifted down.
    const float visible_height = text_size.y * 0.72f;
    switch (align_v) {
        case TextAlignV::Top:
            origin_y = bounds.y;
            break;
        case TextAlignV::Bottom:
            origin_y = bounds.y + bounds.height - text_size.y;
            break;
        case TextAlignV::Center:
        default: {
            const float centered_line_top = bounds.y + (bounds.height - text_size.y) * detail::kHalf;
            const float optical_correction = (text_size.y - visible_height) * detail::kHalf;
            origin_y = centered_line_top - optical_correction;
            break;
        }
    }

    return Vec2(origin_x, origin_y);
}

} // namespace arin
