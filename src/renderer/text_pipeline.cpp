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

#include "text_pipeline.hpp"
#include "shader_util.hpp"
#include <GL/glew.h>

namespace arin {
namespace renderer {

namespace {

// -------------------------------------------------------------------------
// GLSL Shaders for Textured Batched Typography
// -------------------------------------------------------------------------

const char* TEXT_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec4 a_color;

uniform mat4 u_projection;
out vec2 v_uv;
out vec4 v_color;

void main() {
    v_uv = a_uv;
    v_color = a_color;
    gl_Position = u_projection * vec4(a_pos, 0.0, 1.0);
}
)";

const char* TEXT_FRAGMENT_SHADER = R"(
#version 330 core
in vec2 v_uv;
in vec4 v_color;
out vec4 FragColor;

uniform sampler2D u_font_texture;

void main() {
    float text_alpha = texture(u_font_texture, v_uv).r;
    FragColor = vec4(v_color.rgb, v_color.a * text_alpha);
}
)";

} // anonymous namespace

TextPipeline::TextPipeline() = default;

TextPipeline::~TextPipeline() {
    shutdown();
}

bool TextPipeline::init() {
    m_program = create_program(TEXT_VERTEX_SHADER, TEXT_FRAGMENT_SHADER);
    if (!m_program) return false;

    m_u_proj    = glGetUniformLocation(m_program, "u_projection");
    m_u_sampler = glGetUniformLocation(m_program, "u_font_texture");

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // Initial batch capacity: 1024 vertices
    glBufferData(GL_ARRAY_BUFFER, sizeof(TextVertex) * 1024, nullptr, GL_DYNAMIC_DRAW);

    // Attrib 0: vec2 pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, x));

    // Attrib 1: vec2 uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, u));

    // Attrib 2: vec4 color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, r));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_batch.reserve(1024);
    return true;
}

void TextPipeline::shutdown() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_program) { glDeleteProgram(m_program); m_program = 0; }
}

void TextPipeline::draw_text(
    Font& font,
    const std::string& text,
    const Vec2& position,
    const Color& color,
    float scale
) {
    float cursor_x = position.x;
    float cursor_y = position.y;

    for (char c : text) {
        GlyphInfo g = font.get_glyph(c);

        float qw = g.width * scale;
        float qh = g.height * scale;

        float x0 = cursor_x;
        float y0 = cursor_y;
        float x1 = cursor_x + qw;
        float y1 = cursor_y + qh;

        // Two triangles (6 vertices) per glyph quad
        TextVertex v0{ x0, y0, g.u0, g.v0, color.r, color.g, color.b, color.a };
        TextVertex v1{ x1, y0, g.u1, g.v0, color.r, color.g, color.b, color.a };
        TextVertex v2{ x1, y1, g.u1, g.v1, color.r, color.g, color.b, color.a };

        TextVertex v3{ x0, y0, g.u0, g.v0, color.r, color.g, color.b, color.a };
        TextVertex v4{ x1, y1, g.u1, g.v1, color.r, color.g, color.b, color.a };
        TextVertex v5{ x0, y1, g.u0, g.v1, color.r, color.g, color.b, color.a };

        m_batch.push_back(v0);
        m_batch.push_back(v1);
        m_batch.push_back(v2);
        m_batch.push_back(v3);
        m_batch.push_back(v4);
        m_batch.push_back(v5);

        cursor_x += g.advance * scale;
    }
}

void TextPipeline::flush(int viewport_width, int viewport_height, uint32_t font_texture_id) {
    if (m_batch.empty()) return;

    glUseProgram(m_program);

    float ortho[16];
    make_ortho_projection(viewport_width, viewport_height, ortho);
    glUniformMatrix4fv(m_u_proj, 1, GL_FALSE, ortho);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glUniform1i(m_u_sampler, 0);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // Upload batch vertex array
    glBufferData(
        GL_ARRAY_BUFFER,
        m_batch.size() * sizeof(TextVertex),
        m_batch.data(),
        GL_DYNAMIC_DRAW
    );

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_batch.size()));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    m_batch.clear();
}

} // namespace renderer
} // namespace arin
