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
#include <vector>

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

void RectPipeline::draw_checkmark(
    int viewport_width,
    int viewport_height,
    const Rect& box,
    const Color& color,
    float thickness
) {
    if (box.width <= 0.0f || box.height <= 0.0f || color.a <= 0.0f) return;

    // Checkmark vertex keypoints calibrated to checkbox proportions
    Vec2 p0(box.x + box.width * 0.22f, box.y + box.height * 0.50f);
    Vec2 p1(box.x + box.width * 0.42f, box.y + box.height * 0.72f);
    Vec2 p2(box.x + box.width * 0.78f, box.y + box.height * 0.28f);

    glUseProgram(m_program);

    float ortho[16];
    make_ortho_projection(viewport_width, viewport_height, ortho);
    glUniformMatrix4fv(m_u_proj, 1, GL_FALSE, ortho);

    glUniform4f(m_u_box, box.x - 10.0f, box.y - 10.0f, box.width + 20.0f, box.height + 20.0f);
    glUniform1f(m_u_radius, 0.0f);
    glUniform4f(m_u_fill, color.r, color.g, color.b, color.a);
    glUniform4f(m_u_border_color, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform1f(m_u_border_width, 0.0f);
    glUniform1i(m_u_is_shadow, 0);

    auto build_segment = [](Vec2 a, Vec2 b, float th, float* out) {
        Vec2 d(b.x - a.x, b.y - a.y);
        float len = std::sqrt(d.x * d.x + d.y * d.y);
        if (len < 1e-4f) return;
        Vec2 n(-d.y / len * (th * 0.5f), d.x / len * (th * 0.5f));

        out[0]  = a.x - n.x; out[1]  = a.y - n.y;
        out[2]  = a.x + n.x; out[3]  = a.y + n.y;
        out[4]  = b.x + n.x; out[5]  = b.y + n.y;
        out[6]  = a.x - n.x; out[7]  = a.y - n.y;
        out[8]  = b.x + n.x; out[9]  = b.y + n.y;
        out[10] = b.x - n.x; out[11] = b.y - n.y;
    };

    float verts[24];
    build_segment(p0, p1, thickness, &verts[0]);
    build_segment(p1, p2, thickness, &verts[12]);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 12);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void RectPipeline::draw_icon(
    int viewport_width,
    int viewport_height,
    IconType icon,
    const Rect& bounds,
    const Color& color
) {
    if (bounds.width <= 0.0f || bounds.height <= 0.0f || color.a <= 0.0f || icon == IconType::None) {
        return;
    }

    float bx = bounds.x;
    float by = bounds.y;
    float bw = bounds.width;
    float bh = bounds.height;
    float th = std::max(1.5f, bw * 0.10f); // Default stroke thickness scaled to icon dimensions

    // Lambda to emit thick 2D line segments
    auto build_segment = [](Vec2 a, Vec2 b, float thickness, std::vector<float>& out) {
        Vec2 d(b.x - a.x, b.y - a.y);
        float len = std::sqrt(d.x * d.x + d.y * d.y);
        if (len < 1e-4f) return;
        Vec2 n(-d.y / len * (thickness * 0.5f), d.x / len * (thickness * 0.5f));

        out.push_back(a.x - n.x); out.push_back(a.y - n.y);
        out.push_back(a.x + n.x); out.push_back(a.y + n.y);
        out.push_back(b.x + n.x); out.push_back(b.y + n.y);

        out.push_back(a.x - n.x); out.push_back(a.y - n.y);
        out.push_back(b.x + n.x); out.push_back(b.y + n.y);
        out.push_back(b.x - n.x); out.push_back(b.y - n.y);
    };

    std::vector<float> line_verts;

    switch (icon) {
        case IconType::Check: {
            draw_checkmark(viewport_width, viewport_height, bounds, color, th);
            return;
        }

        case IconType::Close: {
            Vec2 p0(bx + bw * 0.25f, by + bh * 0.25f);
            Vec2 p1(bx + bw * 0.75f, by + bh * 0.75f);
            Vec2 p2(bx + bw * 0.75f, by + bh * 0.25f);
            Vec2 p3(bx + bw * 0.25f, by + bh * 0.75f);
            build_segment(p0, p1, th, line_verts);
            build_segment(p2, p3, th, line_verts);
            break;
        }

        case IconType::ChevronRight: {
            Vec2 p0(bx + bw * 0.35f, by + bh * 0.25f);
            Vec2 p1(bx + bw * 0.65f, by + bh * 0.50f);
            Vec2 p2(bx + bw * 0.35f, by + bh * 0.75f);
            build_segment(p0, p1, th, line_verts);
            build_segment(p1, p2, th, line_verts);
            break;
        }

        case IconType::ChevronDown: {
            Vec2 p0(bx + bw * 0.25f, by + bh * 0.35f);
            Vec2 p1(bx + bw * 0.50f, by + bh * 0.65f);
            Vec2 p2(bx + bw * 0.75f, by + bh * 0.35f);
            build_segment(p0, p1, th, line_verts);
            build_segment(p1, p2, th, line_verts);
            break;
        }

        case IconType::ChevronUp: {
            Vec2 p0(bx + bw * 0.25f, by + bh * 0.65f);
            Vec2 p1(bx + bw * 0.50f, by + bh * 0.35f);
            Vec2 p2(bx + bw * 0.75f, by + bh * 0.65f);
            build_segment(p0, p1, th, line_verts);
            build_segment(p1, p2, th, line_verts);
            break;
        }

        case IconType::ChevronLeft: {
            Vec2 p0(bx + bw * 0.65f, by + bh * 0.25f);
            Vec2 p1(bx + bw * 0.35f, by + bh * 0.50f);
            Vec2 p2(bx + bw * 0.65f, by + bh * 0.75f);
            build_segment(p0, p1, th, line_verts);
            build_segment(p1, p2, th, line_verts);
            break;
        }

        case IconType::ArrowRight: {
            Vec2 p0(bx + bw * 0.18f, by + bh * 0.50f);
            Vec2 p1(bx + bw * 0.76f, by + bh * 0.50f);
            Vec2 p2(bx + bw * 0.52f, by + bh * 0.26f);
            Vec2 p3(bx + bw * 0.52f, by + bh * 0.74f);
            build_segment(p0, p1, th, line_verts);
            build_segment(p2, p1, th, line_verts);
            build_segment(p3, p1, th, line_verts);
            break;
        }

        case IconType::ArrowLeft: {
            Vec2 p0(bx + bw * 0.82f, by + bh * 0.50f);
            Vec2 p1(bx + bw * 0.24f, by + bh * 0.50f);
            Vec2 p2(bx + bw * 0.48f, by + bh * 0.26f);
            Vec2 p3(bx + bw * 0.48f, by + bh * 0.74f);
            build_segment(p0, p1, th, line_verts);
            build_segment(p2, p1, th, line_verts);
            build_segment(p3, p1, th, line_verts);
            break;
        }

        case IconType::Folder: {
            // Folder top tab
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.12f, by + bh * 0.18f, bw * 0.36f, bh * 0.22f),
                2.0f, color, Color::transparent(), 0.0f);
            // Folder front body
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.12f, by + bh * 0.32f, bw * 0.76f, bh * 0.52f),
                2.0f, color, Color::transparent(), 0.0f);
            return;
        }

        case IconType::File: {
            // Document sheet
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.22f, by + bh * 0.14f, bw * 0.56f, bh * 0.72f),
                2.0f, Color::transparent(), color, th);
            // Document text lines inside
            build_segment(Vec2(bx + bw * 0.34f, by + bh * 0.40f), Vec2(bx + bw * 0.66f, by + bh * 0.40f), th * 0.8f, line_verts);
            build_segment(Vec2(bx + bw * 0.34f, by + bh * 0.55f), Vec2(bx + bw * 0.66f, by + bh * 0.55f), th * 0.8f, line_verts);
            build_segment(Vec2(bx + bw * 0.34f, by + bh * 0.70f), Vec2(bx + bw * 0.52f, by + bh * 0.70f), th * 0.8f, line_verts);
            break;
        }

        case IconType::Search: {
            // Magnifier lens ring
            float r = bw * 0.24f;
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.16f, by + bh * 0.16f, r * 2.0f, r * 2.0f),
                r, Color::transparent(), color, th);
            // Diagonal handle
            Vec2 p0(bx + bw * 0.52f, by + bh * 0.52f);
            Vec2 p1(bx + bw * 0.82f, by + bh * 0.82f);
            build_segment(p0, p1, th * 1.3f, line_verts);
            break;
        }

        case IconType::Settings: {
            // Central cog ring
            float r = bw * 0.24f;
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.5f - r, by + bh * 0.5f - r, r * 2.0f, r * 2.0f),
                r, Color::transparent(), color, th);
            // Radiating cog teeth notches
            build_segment(Vec2(bx + bw * 0.5f, by + bh * 0.12f), Vec2(bx + bw * 0.5f, by + bh * 0.28f), th * 1.2f, line_verts);
            build_segment(Vec2(bx + bw * 0.5f, by + bh * 0.72f), Vec2(bx + bw * 0.5f, by + bh * 0.88f), th * 1.2f, line_verts);
            build_segment(Vec2(bx + bw * 0.12f, by + bh * 0.5f), Vec2(bx + bw * 0.28f, by + bh * 0.5f), th * 1.2f, line_verts);
            build_segment(Vec2(bx + bw * 0.72f, by + bh * 0.5f), Vec2(bx + bw * 0.88f, by + bh * 0.5f), th * 1.2f, line_verts);
            break;
        }

        case IconType::Cut: {
            // Scissors blades
            Vec2 p0(bx + bw * 0.32f, by + bh * 0.32f);
            Vec2 p1(bx + bw * 0.82f, by + bh * 0.78f);
            Vec2 p2(bx + bw * 0.32f, by + bh * 0.68f);
            Vec2 p3(bx + bw * 0.82f, by + bh * 0.22f);
            build_segment(p0, p1, th, line_verts);
            build_segment(p2, p3, th, line_verts);
            // Finger loops
            float lr = bw * 0.12f;
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.14f, by + bh * 0.24f, lr * 2.0f, lr * 2.0f),
                lr, Color::transparent(), color, th * 0.8f);
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.14f, by + bh * 0.56f, lr * 2.0f, lr * 2.0f),
                lr, Color::transparent(), color, th * 0.8f);
            break;
        }

        case IconType::Copy: {
            // Back sheet outline
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.16f, by + bh * 0.14f, bw * 0.48f, bh * 0.58f),
                1.5f, Color::transparent(), color, th * 0.9f);
            // Front sheet filled with background tint and outline
            Color front_fill(color.r, color.g, color.b, color.a * 0.25f);
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.34f, by + bh * 0.28f, bw * 0.48f, bh * 0.58f),
                1.5f, front_fill, color, th * 0.9f);
            return;
        }

        case IconType::Paste: {
            // Clipboard backing board
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.20f, by + bh * 0.20f, bw * 0.60f, bh * 0.68f),
                2.0f, Color::transparent(), color, th);
            // Top clip
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.36f, by + bh * 0.12f, bw * 0.28f, bh * 0.14f),
                1.5f, color, Color::transparent(), 0.0f);
            // Content lines
            build_segment(Vec2(bx + bw * 0.32f, by + bh * 0.44f), Vec2(bx + bw * 0.68f, by + bh * 0.44f), th * 0.8f, line_verts);
            build_segment(Vec2(bx + bw * 0.32f, by + bh * 0.58f), Vec2(bx + bw * 0.68f, by + bh * 0.58f), th * 0.8f, line_verts);
            build_segment(Vec2(bx + bw * 0.32f, by + bh * 0.72f), Vec2(bx + bw * 0.54f, by + bh * 0.72f), th * 0.8f, line_verts);
            break;
        }

        case IconType::Trash: {
            // Wastebasket can
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.26f, by + bh * 0.34f, bw * 0.48f, bh * 0.52f),
                1.5f, Color::transparent(), color, th);
            // Lid bar
            build_segment(Vec2(bx + bw * 0.18f, by + bh * 0.32f), Vec2(bx + bw * 0.82f, by + bh * 0.32f), th * 1.1f, line_verts);
            // Lid handle
            build_segment(Vec2(bx + bw * 0.40f, by + bh * 0.22f), Vec2(bx + bw * 0.60f, by + bh * 0.22f), th * 0.9f, line_verts);
            // Vertical ribs
            build_segment(Vec2(bx + bw * 0.42f, by + bh * 0.44f), Vec2(bx + bw * 0.42f, by + bh * 0.74f), th * 0.8f, line_verts);
            build_segment(Vec2(bx + bw * 0.58f, by + bh * 0.44f), Vec2(bx + bw * 0.58f, by + bh * 0.74f), th * 0.8f, line_verts);
            break;
        }

        case IconType::Edit: {
            // Diagonal pencil stem
            Vec2 p0(bx + bw * 0.32f, by + bh * 0.68f);
            Vec2 p1(bx + bw * 0.74f, by + bh * 0.26f);
            build_segment(p0, p1, th * 1.3f, line_verts);
            // Pencil tip
            build_segment(Vec2(bx + bw * 0.22f, by + bh * 0.78f), Vec2(bx + bw * 0.32f, by + bh * 0.68f), th, line_verts);
            break;
        }

        case IconType::More: {
            // Horizontal ellipsis dots (...)
            float dot_size = std::max(2.5f, bw * 0.14f);
            float r = dot_size * 0.5f;
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.22f - r, by + bh * 0.5f - r, dot_size, dot_size),
                r, color, Color::transparent(), 0.0f);
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.50f - r, by + bh * 0.5f - r, dot_size, dot_size),
                r, color, Color::transparent(), 0.0f);
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.78f - r, by + bh * 0.5f - r, dot_size, dot_size),
                r, color, Color::transparent(), 0.0f);
            return;
        }

        case IconType::Info: {
            // Circular boundary
            float r = bw * 0.38f;
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.5f - r, by + bh * 0.5f - r, r * 2.0f, r * 2.0f),
                r, Color::transparent(), color, th);
            // 'i' dot
            float dot_size = std::max(2.0f, th * 1.1f);
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.5f - dot_size * 0.5f, by + bh * 0.30f, dot_size, dot_size),
                dot_size * 0.5f, color, Color::transparent(), 0.0f);
            // 'i' stem
            build_segment(Vec2(bx + bw * 0.5f, by + bh * 0.44f), Vec2(bx + bw * 0.5f, by + bh * 0.72f), th, line_verts);
            break;
        }

        case IconType::Warning: {
            // Cautionary triangle
            Vec2 p0(bx + bw * 0.50f, by + bh * 0.16f);
            Vec2 p1(bx + bw * 0.16f, by + bh * 0.82f);
            Vec2 p2(bx + bw * 0.84f, by + bh * 0.82f);
            build_segment(p0, p1, th, line_verts);
            build_segment(p1, p2, th, line_verts);
            build_segment(p2, p0, th, line_verts);
            // Exclamation stem
            build_segment(Vec2(bx + bw * 0.5f, by + bh * 0.40f), Vec2(bx + bw * 0.5f, by + bh * 0.62f), th * 0.9f, line_verts);
            // Exclamation dot
            float dot_size = std::max(2.0f, th * 0.9f);
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.5f - dot_size * 0.5f, by + bh * 0.70f, dot_size, dot_size),
                dot_size * 0.5f, color, Color::transparent(), 0.0f);
            break;
        }

        case IconType::Error: {
            // Circular badge with white/color cross
            float r = bw * 0.38f;
            draw_rounded_rect(viewport_width, viewport_height,
                Rect(bx + bw * 0.5f - r, by + bh * 0.5f - r, r * 2.0f, r * 2.0f),
                r, color, Color::transparent(), 0.0f);
            // Internal sharp white X
            Vec2 p0(bx + bw * 0.34f, by + bh * 0.34f);
            Vec2 p1(bx + bw * 0.66f, by + bh * 0.66f);
            Vec2 p2(bx + bw * 0.66f, by + bh * 0.34f);
            Vec2 p3(bx + bw * 0.34f, by + bh * 0.66f);
            std::vector<float> cross_verts;
            build_segment(p0, p1, th * 0.9f, cross_verts);
            build_segment(p2, p3, th * 0.9f, cross_verts);

            // Render inner cross with white color
            glUseProgram(m_program);
            float ortho[16];
            make_ortho_projection(viewport_width, viewport_height, ortho);
            glUniformMatrix4fv(m_u_proj, 1, GL_FALSE, ortho);
            glUniform4f(m_u_box, bx - 10.0f, by - 10.0f, bw + 20.0f, bh + 20.0f);
            glUniform1f(m_u_radius, 0.0f);
            glUniform4f(m_u_fill, 1.0f, 1.0f, 1.0f, 1.0f);
            glUniform4f(m_u_border_color, 0.0f, 0.0f, 0.0f, 0.0f);
            glUniform1f(m_u_border_width, 0.0f);
            glUniform1i(m_u_is_shadow, 0);

            glBindVertexArray(m_vao);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            glBufferData(GL_ARRAY_BUFFER, cross_verts.size() * sizeof(float), cross_verts.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cross_verts.size() / 2));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glUseProgram(0);
            return;
        }

        default:
            break;
    }

    // Render queued vector segments
    if (!line_verts.empty()) {
        glUseProgram(m_program);

        float ortho[16];
        make_ortho_projection(viewport_width, viewport_height, ortho);
        glUniformMatrix4fv(m_u_proj, 1, GL_FALSE, ortho);

        glUniform4f(m_u_box, bx - 10.0f, by - 10.0f, bw + 20.0f, bh + 20.0f);
        glUniform1f(m_u_radius, 0.0f);
        glUniform4f(m_u_fill, color.r, color.g, color.b, color.a);
        glUniform4f(m_u_border_color, 0.0f, 0.0f, 0.0f, 0.0f);
        glUniform1f(m_u_border_width, 0.0f);
        glUniform1i(m_u_is_shadow, 0);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        glBufferData(GL_ARRAY_BUFFER, line_verts.size() * sizeof(float), line_verts.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(line_verts.size() / 2));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

} // namespace renderer
} // namespace arin
