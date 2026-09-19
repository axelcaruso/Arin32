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

#ifndef ARIN32_RENDERER_IMAGE_PIPELINE_HPP
#define ARIN32_RENDERER_IMAGE_PIPELINE_HPP

#include "arin/types.hpp"
#include <cstdint>

namespace arin {
namespace renderer {

/**
 * @brief Dedicated OpenGL pipeline for GPU-accelerated rendering of 2D textures and images.
 *
 * Features:
 * - Anti-aliased rounded rectangle corner clipping via Signed Distance Fields (SDF) in the fragment shader
 * - UV coordinate mapping for full textures or sub-region spritesheets
 * - Color tinting and alpha transparency modulation
 * - Seamless integration with the Arin32 2D orthographic coordinate system
 */
class ImagePipeline {
public:
    ImagePipeline();
    ~ImagePipeline();

    // Non-copyable due to OpenGL GPU state
    ImagePipeline(const ImagePipeline&) = delete;
    ImagePipeline& operator=(const ImagePipeline&) = delete;

    /**
     * @brief Compiles GLSL shaders and creates dynamic quad VAO/VBO.
     * @return true on success, false if shader compilation or buffer creation fails.
     */
    bool init();

    /**
     * @brief Releases OpenGL program and buffer objects.
     */
    void shutdown();

    /**
     * @brief Draws a textured quad with optional corner rounding and color tint.
     *
     * @param viewport_width Width of the render target in pixels.
     * @param viewport_height Height of the render target in pixels.
     * @param texture_handle Raw OpenGL texture identifier (glGenTextures).
     * @param dest Target bounding box on screen in pixels.
     * @param src_uv Normalized UV texture coordinates (x=u0, y=v0, width=u1-u0, height=v1-v0).
     * @param tint Color multiplier (RGBA), where white (1,1,1,1) preserves natural colors.
     * @param corner_radius Radius of curvature for corners in pixels (SDF rounded corners).
     */
    void draw_image(
        int viewport_width,
        int viewport_height,
        uint32_t texture_handle,
        const Rect& dest,
        const Rect& src_uv,
        const Color& tint,
        float corner_radius
    );

private:
    uint32_t m_program{0};
    uint32_t m_vao{0};
    uint32_t m_vbo{0};

    // Uniform locations
    int32_t m_u_proj{-1};
    int32_t m_u_texture{-1};
    int32_t m_u_rect{-1};
    int32_t m_u_radius{-1};
    int32_t m_u_tint{-1};
};

} // namespace renderer
} // namespace arin

#endif // ARIN32_RENDERER_IMAGE_PIPELINE_HPP
