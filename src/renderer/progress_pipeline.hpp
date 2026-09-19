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

#ifndef ARIN32_RENDERER_PROGRESS_PIPELINE_HPP
#define ARIN32_RENDERER_PROGRESS_PIPELINE_HPP

#include "arin/types.hpp"
#include <cstdint>

namespace arin {
namespace renderer {

/**
 * @brief Dedicated OpenGL pipeline for rendering Windows 10 modern progress bars.
 *
 * Supports both Determinate mode (with continuous white cosine shimmer sweep)
 * and Indeterminate mode (traveling marquee chunk with smooth specular highlights).
 */
class ProgressPipeline {
public:
    ProgressPipeline();
    ~ProgressPipeline();

    // Non-copyable
    ProgressPipeline(const ProgressPipeline&) = delete;
    ProgressPipeline& operator=(const ProgressPipeline&) = delete;

    /**
     * @brief Compiles progress GLSL shaders and creates dedicated quad buffers.
     * @return true on success, false on error.
     */
    bool init();

    /**
     * @brief Releases OpenGL resources.
     */
    void shutdown();

    /**
     * @brief Draws a progress bar with SDF anti-aliased track, border, and animations.
     */
    void draw_progress_bar(
        int viewport_width,
        int viewport_height,
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

private:
    uint32_t m_program{0};
    uint32_t m_vao{0};
    uint32_t m_vbo{0};

    // Uniform locations
    int32_t m_u_proj{-1};
    int32_t m_u_box{-1};
    int32_t m_u_radius{-1};
    int32_t m_u_track_color{-1};
    int32_t m_u_fill_color{-1};
    int32_t m_u_border_color{-1};
    int32_t m_u_border_width{-1};
    int32_t m_u_fill_fraction{-1};
    int32_t m_u_anim_phase{-1};
    int32_t m_u_is_indeterminate{-1};
};

} // namespace renderer
} // namespace arin

#endif // ARIN32_RENDERER_PROGRESS_PIPELINE_HPP
