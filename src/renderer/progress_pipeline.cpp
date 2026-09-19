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

#include "progress_pipeline.hpp"
#include "shader_util.hpp"
#include <GL/glew.h>
#include <algorithm>

namespace arin {
namespace renderer {

namespace {

// -------------------------------------------------------------------------
// GLSL Shaders for Modern Windows 10 Progress Bar with Shimmer Animation
// -------------------------------------------------------------------------

const char* PROGRESS_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec2 a_pos;

uniform mat4 u_projection;
out vec2 v_frag_pos;

void main() {
    v_frag_pos = a_pos;
    gl_Position = u_projection * vec4(a_pos, 0.0, 1.0);
}
)";

const char* PROGRESS_FRAGMENT_SHADER = R"(
#version 330 core
in vec2 v_frag_pos;
out vec4 FragColor;

uniform vec4 u_rect;            // (x, y, width, height) in pixels
uniform float u_radius;         // corner curvature radius
uniform vec4 u_track_color;     // background track color (RGBA)
uniform vec4 u_fill_color;      // active progress fill color (RGBA)
uniform vec4 u_border_color;    // border stroke color (RGBA)
uniform float u_border_width;   // border thickness in pixels
uniform float u_fill_fraction;  // determinate fraction [0.0..1.0]
uniform float u_anim_phase;     // continuous animation phase
uniform int u_is_indeterminate; // 1 = indeterminate traveling chunk, 0 = determinate

float sd_rounded_rect(vec2 p, vec2 half_size, float r) {
    vec2 d = abs(p) - (half_size - vec2(r));
    return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0) - r;
}

void main() {
    vec2 rect_center = u_rect.xy + u_rect.zw * 0.5;
    vec2 half_size = u_rect.zw * 0.5;
    vec2 p = v_frag_pos - rect_center;

    float dist = sd_rounded_rect(p, half_size, u_radius);

    // Screen-space derivative anti-aliasing
    float fw = fwidth(dist);
    if (fw < 0.0001) fw = 1.0;
    float alpha = 1.0 - smoothstep(-0.5 * fw, 0.5 * fw, dist);

    if (alpha <= 0.0) {
        discard;
    }

    vec4 out_color;

    if (u_is_indeterminate == 1) {
        // Indeterminate mode: traveling chunk gliding smoothly left to right
        float chunk_w = max(u_rect.z * 0.32, 40.0);
        float travel = u_rect.z + chunk_w * 2.0;
        float cycle = fract(u_anim_phase);
        float current_center_x = u_rect.x - chunk_w + cycle * travel;

        float d_chunk = abs(v_frag_pos.x - current_center_x);
        if (d_chunk <= chunk_w * 0.5) {
            // Inside traveling chunk: add smooth center highlight
            float highlight = cos((d_chunk / (chunk_w * 0.5)) * 1.5707963);
            out_color = mix(u_fill_color, vec4(1.0, 1.0, 1.0, 1.0), highlight * 0.35);
        } else {
            out_color = u_track_color;
        }
    } else {
        // Determinate mode (0% to 100%)
        float fill_w = u_rect.z * clamp(u_fill_fraction, 0.0, 1.0);
        float fill_end_x = u_rect.x + fill_w;

        if (v_frag_pos.x <= fill_end_x) {
            // Inside progress fill
            // Animated white shimmer sweep
            float shimmer_w = min(max(u_rect.z * 0.35, 45.0), 80.0);
            float cycle = fract(u_anim_phase);
            float shimmer_center_x = u_rect.x - shimmer_w * 0.5 + cycle * (fill_w + shimmer_w * 1.5);

            float d_shimmer = abs(v_frag_pos.x - shimmer_center_x);
            float highlight = 0.0;
            if (d_shimmer <= shimmer_w * 0.5) {
                highlight = cos((d_shimmer / (shimmer_w * 0.5)) * 1.5707963);
            }

            out_color = mix(u_fill_color, vec4(1.0, 1.0, 1.0, 1.0), highlight * 0.42);
        } else {
            out_color = u_track_color;
        }
    }

    // Apply border outline stroke
    if (u_border_width > 0.0) {
        float inner_d = dist + u_border_width;
        float inner_alpha = 1.0 - smoothstep(-0.5 * fw, 0.5 * fw, inner_d);
        out_color = mix(u_border_color, out_color, inner_alpha);
    }

    FragColor = vec4(out_color.rgb, out_color.a * alpha);
}
)";

} // anonymous namespace

ProgressPipeline::ProgressPipeline() = default;

ProgressPipeline::~ProgressPipeline() {
    shutdown();
}

bool ProgressPipeline::init() {
    m_program = create_program(PROGRESS_VERTEX_SHADER, PROGRESS_FRAGMENT_SHADER);
    if (!m_program) return false;

    m_u_proj             = glGetUniformLocation(m_program, "u_projection");
    m_u_box              = glGetUniformLocation(m_program, "u_rect");
    m_u_radius           = glGetUniformLocation(m_program, "u_radius");
    m_u_track_color      = glGetUniformLocation(m_program, "u_track_color");
    m_u_fill_color       = glGetUniformLocation(m_program, "u_fill_color");
    m_u_border_color     = glGetUniformLocation(m_program, "u_border_color");
    m_u_border_width     = glGetUniformLocation(m_program, "u_border_width");
    m_u_fill_fraction    = glGetUniformLocation(m_program, "u_fill_fraction");
    m_u_anim_phase       = glGetUniformLocation(m_program, "u_anim_phase");
    m_u_is_indeterminate = glGetUniformLocation(m_program, "u_is_indeterminate");

    // Dynamic quad VAO/VBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // Allocate buffer for 6 vertices (2 triangles per quad)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 6, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

void ProgressPipeline::shutdown() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_program) { glDeleteProgram(m_program); m_program = 0; }
}

void ProgressPipeline::draw_progress_bar(
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
) {
    if (rect.width <= 0.0f || rect.height <= 0.0f) return;

    glUseProgram(m_program);

    float ortho[16];
    make_ortho_projection(viewport_width, viewport_height, ortho);
    glUniformMatrix4fv(m_u_proj, 1, GL_FALSE, ortho);

    // Clamp corner radius so it does not exceed half the shortest side
    float max_r = std::min(rect.width, rect.height) * 0.5f;
    float clamped_radius = std::max(0.0f, std::min(corner_radius, max_r));

    glUniform4f(m_u_box, rect.x, rect.y, rect.width, rect.height);
    glUniform1f(m_u_radius, clamped_radius);
    glUniform4f(m_u_track_color, track_color.r, track_color.g, track_color.b, track_color.a);
    glUniform4f(m_u_fill_color, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
    glUniform4f(m_u_border_color, border_color.r, border_color.g, border_color.b, border_color.a);
    glUniform1f(m_u_border_width, border_width);
    glUniform1f(m_u_fill_fraction, fill_fraction);
    glUniform1f(m_u_anim_phase, anim_phase);
    glUniform1i(m_u_is_indeterminate, is_indeterminate ? 1 : 0);

    // Build quad covering the bounding box with 1px padding for anti-aliasing
    float x0 = rect.x - 1.0f;
    float y0 = rect.y - 1.0f;
    float x1 = rect.x + rect.width + 1.0f;
    float y1 = rect.y + rect.height + 1.0f;

    float vertices[12] = {
        x0, y0,
        x1, y0,
        x1, y1,
        x0, y0,
        x1, y1,
        x0, y1
    };

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

} // namespace renderer
} // namespace arin
