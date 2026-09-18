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

#ifndef ARIN32_FONT_HPP
#define ARIN32_FONT_HPP

#include "types.hpp"
#include <string>
#include <cstdint>

namespace arin {

/**
 * @brief Glyph layout and texture mapping data for a single character.
 */
struct GlyphInfo {
    float advance{0.0f}; ///< Horizontal step to the next character in pixels.
    float u0{0.0f};      ///< Minimum horizontal texture coordinate in atlas [0..1].
    float v0{0.0f};      ///< Minimum vertical texture coordinate in atlas [0..1].
    float u1{0.0f};      ///< Maximum horizontal texture coordinate in atlas [0..1].
    float v1{0.0f};      ///< Maximum vertical texture coordinate in atlas [0..1].
    float width{0.0f};   ///< Quad width in pixels.
    float height{0.0f};  ///< Quad height in pixels.
};

/**
 * @brief Manages typography, glyph metrics, and texture atlases.
 *
 * Arin32 embeds a high-quality anti-aliased font atlas directly into the binary,
 * guaranteeing zero external file dependencies. This is critical for early operating
 * system environments or recovery modes where disks or filesystems are not yet mounted.
 */
class Font {
public:
    Font();
    ~Font();

    // Disable copying to prevent double GPU texture deletion
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    // Allow moving
    Font(Font&& other) noexcept;
    Font& operator=(Font&& other) noexcept;

    /**
     * @brief Allocates and uploads the font atlas to an OpenGL 2D texture.
     *
     * Must be called while an active OpenGL context is current on the calling thread.
     * Safe to call multiple times (idempotent).
     *
     * @return true if texture upload succeeded, false on OpenGL error.
     */
    bool init_gl();

    /**
     * @brief Releases the OpenGL texture resource from GPU memory.
     */
    void cleanup_gl();

    /**
     * @brief Returns true if the OpenGL texture is loaded and ready to render.
     */
    bool is_gl_initialized() const { return m_texture_id != 0; }

    /**
     * @brief Calculates the exact bounding dimensions (width and height) of a text string.
     *
     * Used by widgets like Button to automatically center or align their labels.
     *
     * @param text String to measure.
     * @param scale Text scale multiplier (1.0 = standard font size).
     * @return Vec2 containing width (x) and height (y) in pixels.
     */
    Vec2 measure_text(const std::string& text, float scale = 1.0f) const;

    /**
     * @brief Retrieves glyph rendering and UV mapping metrics for a given character.
     * @param c ASCII character (supported: 32 ' ' through 126 '~').
     * @return GlyphInfo struct. Fallback space glyph is returned for unmapped characters.
     */
    GlyphInfo get_glyph(char c) const;

    /**
     * @brief Base font line height in pixels at scale 1.0.
     */
    float line_height() const;

    /**
     * @brief Raw OpenGL texture object ID.
     */
    uint32_t texture_id() const { return m_texture_id; }

private:
    uint32_t m_texture_id{0}; ///< OpenGL texture name (0 if uninitialized).
};

} // namespace arin

#endif // ARIN32_FONT_HPP
