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

#ifndef ARIN32_RENDERER_TEXT_PIPELINE_HPP
#define ARIN32_RENDERER_TEXT_PIPELINE_HPP

#include "arin/types.hpp"
#include "arin/font.hpp"
#include <vector>
#include <string>
#include <cstdint>

namespace arin {
namespace renderer {

/**
 * @brief Vertex representation for dynamic streaming text quads.
 */
struct TextVertex {
    float x, y;       // Screen pixel position
    float u, v;       // Atlas UV coordinates
    float r, g, b, a; // Glyph color
};

/**
 * @brief Dedicated OpenGL pipeline for dynamic batched typography rendering.
 */
class TextPipeline {
public:
    TextPipeline();
    ~TextPipeline();

    // Non-copyable
    TextPipeline(const TextPipeline&) = delete;
    TextPipeline& operator=(const TextPipeline&) = delete;

    /**
     * @brief Compiles typography shaders and initializes streaming dynamic VBO.
     * @return true on success, false on error.
     */
    bool init();

    /**
     * @brief Releases OpenGL objects.
     */
    void shutdown();

    /**
     * @brief Batches glyph quads for rendering.
     */
    void draw_text(
        Font& font,
        const std::string& text,
        const Vec2& position,
        const Color& color,
        float scale
    );

    /**
     * @brief Uploads batched glyphs and executes the single batched OpenGL draw call.
     */
    void flush(int viewport_width, int viewport_height, uint32_t font_texture_id);

    /**
     * @brief Returns true if there are uncommitted glyph vertices in the batch buffer.
     */
    bool has_pending() const { return !m_batch.empty(); }

    /**
     * @brief Clears pending glyphs without rendering (e.g. on new frame).
     */
    void clear() { m_batch.clear(); }

private:
    uint32_t m_program{0};
    uint32_t m_vao{0};
    uint32_t m_vbo{0};

    int32_t m_u_proj{-1};
    int32_t m_u_sampler{-1};

    std::vector<TextVertex> m_batch;
};

} // namespace renderer
} // namespace arin

#endif // ARIN32_RENDERER_TEXT_PIPELINE_HPP
