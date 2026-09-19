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

#include "rect_pipeline.hpp"
#include "shader_util.hpp"
#include <GL/glew.h>
#include <algorithm>

namespace arin {
namespace renderer {

namespace {

// -------------------------------------------------------------------------
// GLSL Shaders for Anti-Aliased Rounded Rectangles & Drop Shadows
// -------------------------------------------------------------------------

const char* RECT_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec2 a_pos;

uniform mat4 u_projection;
out vec2 v_frag_pos;

void main() {
    v_frag_pos = a_pos;
    gl_Position = u_projection * vec4(a_pos, 0.0, 1.0);
}
)";

const char* RECT_FRAGMENT_SHADER = R"(
#version 330 core
in vec2 v_frag_pos;
out vec4 FragColor;

uniform vec4 u_rect;            // (x, y, width, height) in screen pixels
uniform float u_radius;         // Corner curvature radius in pixels
uniform vec4 u_fill_color;      // RGBA fill
uniform vec4 u_border_color;    // RGBA border
uniform float u_border_width;   // Border stroke width in pixels
uniform vec4 u_shadow_color;    // RGBA shadow
uniform float u_shadow_blur;    // Shadow softness radius
uniform int u_is_shadow;        // 1 = render shadow pass, 0 = render solid/bordered rect

// Signed distance to a rounded box
float sd_rounded_rect(vec2 p, vec2 half_size, float r) {
    vec2 d = abs(p) - (half_size - vec2(r));
    return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0) - r;
}

void main() {
    vec2 rect_center = u_rect.xy + u_rect.zw * 0.5;
    vec2 half_size = u_rect.zw * 0.5;
    vec2 p = v_frag_pos - rect_center;

    float dist = sd_rounded_rect(p, half_size, u_radius);

    if (u_is_shadow == 1) {
        // Soft Gaussian-like exponential falloff for drop shadow
        float shadow_alpha = exp(-max(dist, 0.0) / max(u_shadow_blur * 0.5, 0.001));
        shadow_alpha *= u_shadow_color.a;

        if (shadow_alpha <= 0.001) {
            discard;
        }

        FragColor = vec4(u_shadow_color.rgb, shadow_alpha);
    } else {
        // Hardware screen-space derivative anti-aliasing
        float fw = fwidth(dist);
        if (fw < 0.0001) fw = 1.0;
        float alpha = 1.0 - smoothstep(-0.5 * fw, 0.5 * fw, dist);

        if (alpha <= 0.0) {
            discard;
        }

        vec4 col = u_fill_color;

        if (u_border_width > 0.0) {
            float inner_d = dist + u_border_width;
            float inner_alpha = 1.0 - smoothstep(-0.5 * fw, 0.5 * fw, inner_d);
            col = mix(u_border_color, col, inner_alpha);
        }

        FragColor = vec4(col.rgb, col.a * alpha);
    }
}
)";

} // anonymous namespace

RectPipeline::RectPipeline() = default;

RectPipeline::~RectPipeline() {
    shutdown();
}

bool RectPipeline::init() {
    m_program = create_program(RECT_VERTEX_SHADER, RECT_FRAGMENT_SHADER);
    if (!m_program) return false;

    // Uniform lookups
    m_u_proj         = glGetUniformLocation(m_program, "u_projection");
    m_u_box          = glGetUniformLocation(m_program, "u_rect");
    m_u_radius       = glGetUniformLocation(m_program, "u_radius");
    m_u_fill         = glGetUniformLocation(m_program, "u_fill_color");
    m_u_border_color = glGetUniformLocation(m_program, "u_border_color");
    m_u_border_width = glGetUniformLocation(m_program, "u_border_width");
    m_u_shadow_color = glGetUniformLocation(m_program, "u_shadow_color");
    m_u_shadow_blur  = glGetUniformLocation(m_program, "u_shadow_blur");
    m_u_is_shadow    = glGetUniformLocation(m_program, "u_is_shadow");

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

void RectPipeline::shutdown() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_program) { glDeleteProgram(m_program); m_program = 0; }
}

void RectPipeline::draw_rounded_rect(
    int viewport_width,
    int viewport_height,
    const Rect& rect,
    float corner_radius,
    const Color& fill_color,
    const Color& border_color,
    float border_width
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
    glUniform4f(m_u_fill, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
    glUniform4f(m_u_border_color, border_color.r, border_color.g, border_color.b, border_color.a);
    glUniform1f(m_u_border_width, border_width);
    glUniform1i(m_u_is_shadow, 0);

    // Build quad covering the bounding box with 1px padding for smooth AA
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

void RectPipeline::draw_shadow(
    int viewport_width,
    int viewport_height,
    const Rect& rect,
    float corner_radius,
    const Color& shadow_color,
    const Vec2& offset,
    float blur
) {
    if (shadow_color.a <= 0.0f || blur <= 0.0f) return;

    glUseProgram(m_program);

    float ortho[16];
    make_ortho_projection(viewport_width, viewport_height, ortho);
    glUniformMatrix4fv(m_u_proj, 1, GL_FALSE, ortho);

    Rect shadow_rect(rect.x + offset.x, rect.y + offset.y, rect.width, rect.height);
    float max_r = std::min(shadow_rect.width, shadow_rect.height) * 0.5f;
    float clamped_radius = std::max(0.0f, std::min(corner_radius, max_r));

    glUniform4f(m_u_box, shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height);
    glUniform1f(m_u_radius, clamped_radius);
    glUniform4f(m_u_shadow_color, shadow_color.r, shadow_color.g, shadow_color.b, shadow_color.a);
    glUniform1f(m_u_shadow_blur, blur);
    glUniform1i(m_u_is_shadow, 1);

    // Expand the quad geometry by 3*blur to allow the gaussian tail to fade to 0
    float pad = blur * 3.0f;
    float x0 = shadow_rect.x - pad;
    float y0 = shadow_rect.y - pad;
    float x1 = shadow_rect.x + shadow_rect.width + pad;
    float y1 = shadow_rect.y + shadow_rect.height + pad;

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
