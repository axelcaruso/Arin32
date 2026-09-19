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

#include "image_pipeline.hpp"
#include "shader_util.hpp"
#include <GL/glew.h>
#include <algorithm>

namespace arin {
namespace renderer {

namespace {

// -----------------------------------------------------------------------------
// GLSL Shaders for Textured 2D Quads with Signed Distance Field Corner Rounding
// -----------------------------------------------------------------------------

/**
 * Vertex Shader:
 * Transforms 2D pixel positions to normalized device coordinates (NDC) via
 * orthographic projection and forwards UV texture coordinates to fragment shader.
 */
const char* IMAGE_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

uniform mat4 u_projection;

out vec2 v_frag_pos;
out vec2 v_uv;

void main() {
    v_frag_pos = a_pos;
    v_uv = a_uv;
    gl_Position = u_projection * vec4(a_pos, 0.0, 1.0);
}
)";

/**
 * Fragment Shader:
 * Samples the bound 2D texture, modulates with tint color, and evaluates an
 * exact Signed Distance Field (SDF) rounded rectangle to cleanly anti-alias
 * the borders when corner_radius > 0.
 */
const char* IMAGE_FRAGMENT_SHADER = R"(
#version 330 core
in vec2 v_frag_pos;
in vec2 v_uv;
out vec4 FragColor;

uniform sampler2D u_texture;
uniform vec4 u_rect;        // (x, y, width, height) in screen pixel coordinates
uniform float u_radius;     // Corner curvature radius in pixels
uniform vec4 u_tint;        // Color modulation and alpha tint

// Signed distance to a 2D rounded rectangle
float sd_rounded_box(vec2 p, vec2 half_size, float r) {
    vec2 d = abs(p) - (half_size - vec2(r));
    return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0) - r;
}

void main() {
    // Sample texture pixel
    vec4 tex_color = texture(u_texture, v_uv);
    
    // Apply tint color modulation
    vec4 result = tex_color * u_tint;

    // Apply corner rounding if requested
    if (u_radius > 0.0) {
        vec2 rect_center = u_rect.xy + u_rect.zw * 0.5;
        vec2 half_size = u_rect.zw * 0.5;
        vec2 p = v_frag_pos - rect_center;

        float dist = sd_rounded_box(p, half_size, u_radius);

        // Hardware screen-space derivative anti-aliasing (fwidth)
        float fw = fwidth(dist);
        if (fw < 0.0001) fw = 1.0;
        float alpha = 1.0 - smoothstep(-0.5 * fw, 0.5 * fw, dist);

        result.a *= alpha;
    }

    // Discard completely transparent fragments to minimize framebuffer overdraw
    if (result.a <= 0.001) {
        discard;
    }

    FragColor = result;
}
)";

} // anonymous namespace

ImagePipeline::ImagePipeline() = default;

ImagePipeline::~ImagePipeline() {
    shutdown();
}

bool ImagePipeline::init() {
    // 1. Compile GLSL shaders into linked program
    m_program = create_program(IMAGE_VERTEX_SHADER, IMAGE_FRAGMENT_SHADER);
    if (m_program == 0) {
        return false;
    }

    // 2. Query uniform locations
    m_u_proj = glGetUniformLocation(m_program, "u_projection");
    m_u_texture = glGetUniformLocation(m_program, "u_texture");
    m_u_rect = glGetUniformLocation(m_program, "u_rect");
    m_u_radius = glGetUniformLocation(m_program, "u_radius");
    m_u_tint = glGetUniformLocation(m_program, "u_tint");

    // 3. Generate dynamic VAO and VBO for textured quads (position: 2 floats, uv: 2 floats)
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // 6 vertices per quad * 4 floats per vertex (x, y, u, v) = 24 floats total
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, nullptr, GL_DYNAMIC_DRAW);

    // Attribute 0: vec2 a_pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, reinterpret_cast<void*>(0));

    // Attribute 1: vec2 a_uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, reinterpret_cast<void*>(sizeof(float) * 2));

    // Unbind to preserve clean OpenGL state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

void ImagePipeline::shutdown() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

void ImagePipeline::draw_image(
    int viewport_width,
    int viewport_height,
    uint32_t texture_handle,
    const Rect& dest,
    const Rect& src_uv,
    const Color& tint,
    float corner_radius
) {
    // Early exit on invalid dimensions, missing texture, or completely transparent tint
    if (dest.width <= 0.0f || dest.height <= 0.0f || texture_handle == 0 || tint.a <= 0.0f) {
        return;
    }

    // Clamp corner radius to half the shortest edge to avoid visual inversion artifacts
    float max_r = std::min(dest.width, dest.height) * 0.5f;
    float clamped_radius = std::clamp(corner_radius, 0.0f, max_r);

    // Activate shader program
    glUseProgram(m_program);

    // Compute and upload orthographic projection matrix
    float ortho[16];
    make_ortho_projection(viewport_width, viewport_height, ortho);
    glUniformMatrix4fv(m_u_proj, 1, GL_FALSE, ortho);

    // Bind texture to texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glUniform1i(m_u_texture, 0);

    // Upload destination bounding box and corner curvature
    glUniform4f(m_u_rect, dest.x, dest.y, dest.width, dest.height);
    glUniform1f(m_u_radius, clamped_radius);

    // Upload color tint
    glUniform4f(m_u_tint, tint.r, tint.g, tint.b, tint.a);

    // Build 2 triangles (6 vertices) with position and UV coordinates
    float x0 = dest.x;
    float y0 = dest.y;
    float x1 = dest.x + dest.width;
    float y1 = dest.y + dest.height;

    float u0 = src_uv.x;
    float v0 = src_uv.y;
    float u1 = src_uv.x + src_uv.width;
    float v1 = src_uv.y + src_uv.height;

    // Quad vertex layout: {x, y, u, v}
    float vertices[24] = {
        x0, y0, u0, v0, // Top-left
        x1, y0, u1, v0, // Top-right
        x1, y1, u1, v1, // Bottom-right

        x0, y0, u0, v0, // Top-left
        x1, y1, u1, v1, // Bottom-right
        x0, y1, u0, v1  // Bottom-left
    };

    // Upload geometry to dynamic vertex buffer and dispatch draw call
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Restore clean state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

} // namespace renderer
} // namespace arin
