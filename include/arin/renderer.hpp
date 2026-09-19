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

#ifndef ARIN32_RENDERER_HPP
#define ARIN32_RENDERER_HPP

#include "types.hpp"
#include "font.hpp"
#include <string>
#include <vector>
#include <memory>

namespace arin {

/**
 * @brief High-performance, modern OpenGL 2D graphic renderer.
 *
 * Provides GPU-accelerated drawing routines designed specifically for UI controls:
 * - Anti-aliased rounded rectangles with arbitrary corner radius
 * - Configurable border strokes
 * - Soft drop shadows
 * - GPU font rendering with texture atlas
 *
 * Uses modern OpenGL 3.3 Core profile shaders, VAOs, and VBOs.
 * Coordinate space is window-relative with (0,0) at the top-left corner.
 */
class Renderer2D {
public:
    Renderer2D();
    ~Renderer2D();

    // Non-copyable due to GPU resources
    Renderer2D(const Renderer2D&) = delete;
    Renderer2D& operator=(const Renderer2D&) = delete;

    /**
     * @brief Compiles shaders and initializes OpenGL buffers.
     *
     * Must be called after an active OpenGL context has been created and bound.
     * Safe to call on Linux, FreeBSD, or future OS platforms that provide OpenGL 3.0+.
     *
     * @return true on success, false if shader compilation or buffer creation failed.
     */
    bool init();

    /**
     * @brief Releases all allocated OpenGL objects (shaders, VAOs, VBOs, textures).
     */
    void shutdown();

    /**
     * @brief Prepares the renderer for a new frame.
     *
     * Configures OpenGL state (alpha blending, viewport, disabling depth test)
     * and updates the 2D orthographic projection matrix.
     *
     * @param viewport_width Width of the render target in pixels.
     * @param viewport_height Height of the render target in pixels.
     */
    void begin_frame(int viewport_width, int viewport_height);

    /**
     * @brief Concludes the frame and flushes any pending draw commands.
     */
    void end_frame();

    /**
     * @brief Clears the screen buffer with a solid background color.
     * @param color Background clear color.
     */
    void clear(const Color& color);

    /**
     * @brief Draws a flat solid rectangle.
     * @param rect Bounding box.
     * @param color Fill color.
     */
    void draw_rect(const Rect& rect, const Color& color);

    /**
     * @brief Draws an anti-aliased rounded rectangle with optional border stroke.
     *
     * Evaluated entirely in the GPU fragment shader via Signed Distance Field (SDF),
     * producing razor-sharp, perfectly anti-aliased corners at any resolution.
     *
     * @param rect Bounding rectangle.
     * @param corner_radius Radius of curvature for corners in pixels.
     * @param fill_color Background fill color.
     * @param border_color Color of the border outline (default: transparent).
     * @param border_width Thickness of the border in pixels (default: 0.0f).
     */
    void draw_rounded_rect(
        const Rect& rect,
        float corner_radius,
        const Color& fill_color,
        const Color& border_color = Color::transparent(),
        float border_width = 0.0f
    );

    /**
     * @brief Draws a soft drop shadow beneath a rectangle.
     * @param rect Bounding box of the casting element.
     * @param corner_radius Corner radius of the element.
     * @param shadow_color Color and opacity of the shadow.
     * @param offset Displacement vector (x = right, y = down).
     * @param blur Blur softness radius in pixels.
     */
    void draw_shadow(
        const Rect& rect,
        float corner_radius,
        const Color& shadow_color,
        const Vec2& offset,
        float blur
    );

    /**
     * @brief Renders a text string at specified coordinates.
     * @param text The ASCII text string to render.
     * @param position Top-left position of the text in pixels.
     * @param color Text color.
     * @param scale Text scale multiplier (1.0 = standard font size).
     */
    void draw_text(
        const std::string& text,
        const Vec2& position,
        const Color& color,
        float scale = 1.0f
    );

    /**
     * @brief Renders text centered horizontally and vertically within a bounding rectangle.
     *
     * Automatically computes text geometry via Font::measure_text() and aligns it.
     *
     * @param text Text string to center.
     * @param bounds Bounding rectangle to center inside (e.g. Button bounds).
     * @param color Text color.
     * @param scale Text scale multiplier.
     */
    void draw_text_centered(
        const std::string& text,
        const Rect& bounds,
        const Color& color,
        float scale = 1.0f
    );

    /**
     * @brief Renders centered text with guaranteed hardware scissor clipping to container bounds.
     *
     * Prevents text pixels from ever bleeding outside the target container.
     *
     * @param text Text string.
     * @param bounds Bounding rectangle to center inside and clip against.
     * @param color Text color.
     * @param scale Text scale multiplier.
     */
    void draw_text_centered_clipped(
        const std::string& text,
        const Rect& bounds,
        const Color& color,
        float scale = 1.0f
    );

    /**
     * @brief Draws a modern Windows 10 style progress bar with hardware shimmer animation.
     *
     * @param rect Bounding box of the progress bar track.
     * @param corner_radius Corner radius (typically 2.0f - 4.5f).
     * @param track_color Background color of the unfilled track.
     * @param fill_color Progress fill color (e.g. Windows 10 green #06B025 or blue #0067C0).
     * @param border_color Subtle boundary stroke color.
     * @param border_width Border thickness (default: 1.0f).
     * @param fill_fraction Progress percentage [0.0f, 1.0f] for Determinate mode.
     * @param anim_phase Continuous animation phase for shimmer sweeps and marquee chunk travel.
     * @param is_indeterminate true for continuous traveling chunk, false for 0-100% determinate.
     */
    void draw_progress_bar(
        const Rect& rect,
        float corner_radius,
        const Color& track_color,
        const Color& fill_color,
        const Color& border_color,
        float border_width,
        float fill_fraction,
        float anim_phase,
        bool is_indeterminate
    );

    /**
     * @brief Access the embedded Font instance for measurement and metrics.
     */
    Font& font() { return m_font; }
    const Font& font() const { return m_font; }

    /// @brief Current viewport width.
    int viewport_width() const { return m_viewport_width; }
    /// @brief Current viewport height.
    int viewport_height() const { return m_viewport_height; }

private:
    struct TextVertex {
        float x, y;       // Position in screen pixels
        float u, v;       // Atlas UV coordinates
        float r, g, b, a; // Color
    };

    void init_rect_pipeline();
    void init_text_pipeline();
    void init_progress_pipeline();
    void flush_text_batch();

    int m_viewport_width{0};
    int m_viewport_height{0};

    Font m_font;

    // Shader Programs
    uint32_t m_rect_program{0};
    uint32_t m_text_program{0};
    uint32_t m_progress_program{0};

    // Uniform locations for Progress Bar Shader
    int32_t m_u_pb_proj{-1};
    int32_t m_u_pb_box{-1};
    int32_t m_u_pb_radius{-1};
    int32_t m_u_pb_track_color{-1};
    int32_t m_u_pb_fill_color{-1};
    int32_t m_u_pb_border_color{-1};
    int32_t m_u_pb_border_width{-1};
    int32_t m_u_pb_fill_fraction{-1};
    int32_t m_u_pb_anim_phase{-1};
    int32_t m_u_pb_is_indeterminate{-1};

    // Uniform locations for Rounded Rect Shader
    int32_t m_u_rect_proj{-1};
    int32_t m_u_rect_box{-1};
    int32_t m_u_rect_radius{-1};
    int32_t m_u_rect_fill{-1};
    int32_t m_u_rect_border_color{-1};
    int32_t m_u_rect_border_width{-1};
    int32_t m_u_rect_shadow_color{-1};
    int32_t m_u_rect_shadow_blur{-1};
    int32_t m_u_rect_is_shadow{-1};

    // OpenGL Buffers for Rects
    uint32_t m_rect_vao{0};
    uint32_t m_rect_vbo{0};

    // Uniform locations for Text Shader
    int32_t m_u_text_proj{-1};
    int32_t m_u_text_sampler{-1};

    // OpenGL Buffers for Dynamic Text Batching
    uint32_t m_text_vao{0};
    uint32_t m_text_vbo{0};
    std::vector<TextVertex> m_text_batch;
};

} // namespace arin

#endif // ARIN32_RENDERER_HPP
