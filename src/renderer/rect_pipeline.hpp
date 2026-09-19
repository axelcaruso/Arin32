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

#ifndef ARIN32_RENDERER_RECT_PIPELINE_HPP
#define ARIN32_RENDERER_RECT_PIPELINE_HPP

#include "arin/types.hpp"
#include <cstdint>

namespace arin {
namespace renderer {

/**
 * @brief Dedicated OpenGL pipeline for rendering anti-aliased rectangles,
 *        rounded rectangles via GPU Signed Distance Fields (SDF), and soft drop shadows.
 */
class RectPipeline {
public:
    RectPipeline();
    ~RectPipeline();

    // Non-copyable
    RectPipeline(const RectPipeline&) = delete;
    RectPipeline& operator=(const RectPipeline&) = delete;

    /**
     * @brief Compiles GLSL shaders and creates quad VAO/VBO.
     * @return true on success, false on error.
     */
    bool init();

    /**
     * @brief Releases OpenGL programs and vertex buffers.
     */
    void shutdown();

    /**
     * @brief Draws a rounded rectangle with Signed Distance Field anti-aliasing.
     */
    void draw_rounded_rect(
        int viewport_width,
        int viewport_height,
        const Rect& rect,
        float corner_radius,
        const Color& fill_color,
        const Color& border_color,
        float border_width
    );

    /**
     * @brief Draws a soft drop shadow beneath a rectangle.
     */
    void draw_shadow(
        int viewport_width,
        int viewport_height,
        const Rect& rect,
        float corner_radius,
        const Color& shadow_color,
        const Vec2& offset,
        float blur
    );

    /**
     * @brief Draws a crisp checkmark inside a checkbox box.
     */
    void draw_checkmark(
        int viewport_width,
        int viewport_height,
        const Rect& box,
        const Color& color,
        float thickness = 2.0f
    );

private:
    uint32_t m_program{0};
    uint32_t m_vao{0};
    uint32_t m_vbo{0};

    // Uniform locations
    int32_t m_u_proj{-1};
    int32_t m_u_box{-1};
    int32_t m_u_radius{-1};
    int32_t m_u_fill{-1};
    int32_t m_u_border_color{-1};
    int32_t m_u_border_width{-1};
    int32_t m_u_shadow_color{-1};
    int32_t m_u_shadow_blur{-1};
    int32_t m_u_is_shadow{-1};
};

} // namespace renderer
} // namespace arin

#endif // ARIN32_RENDERER_RECT_PIPELINE_HPP
