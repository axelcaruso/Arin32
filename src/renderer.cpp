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

#include "arin/renderer.hpp"
#include <GL/glew.h>
#include <iostream>
#include <vector>

namespace arin {

namespace {

/**
 * @brief Helper utility to compile a single GLSL shader stage.
 * @param type GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
 * @param source Null-terminated GLSL source code string.
 * @return Compiled shader ID, or 0 on failure.
 */
GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[1024];
        glGetShaderInfoLog(shader, sizeof(info_log), nullptr, info_log);
        std::cerr << "[Arin32::Renderer2D] Shader compilation error ("
                  << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
                  << "):\n" << info_log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

/**
 * @brief Links a vertex shader and fragment shader into a complete OpenGL program.
 * @param vs_src Vertex shader GLSL code.
 * @param fs_src Fragment shader GLSL code.
 * @return Linked program ID, or 0 on failure.
 */
GLuint create_program(const char* vs_src, const char* fs_src) {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_src);
    if (!vs) return 0;

    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);
    if (!fs) {
        glDeleteShader(vs);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[1024];
        glGetProgramInfoLog(program, sizeof(info_log), nullptr, info_log);
        std::cerr << "[Arin32::Renderer2D] Program linking error:\n" << info_log << std::endl;
        glDeleteProgram(program);
        program = 0;
    }

    // Shaders can be deleted once linked into the program
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

// -------------------------------------------------------------------------
// GLSL Shaders for Anti-Aliased Rounded Rectangles & Shadows
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

uniform vec4 u_rect;            // (x, y, width, height) in pixels
uniform float u_radius;         // corner curvature radius
uniform vec4 u_fill_color;      // rectangle fill color (RGBA)
uniform vec4 u_border_color;    // border stroke color (RGBA)
uniform float u_border_width;   // border thickness in pixels
uniform vec4 u_shadow_color;    // drop shadow color (RGBA)
uniform float u_shadow_blur;    // shadow blur spread
uniform int u_is_shadow;        // 1 = shadow pass, 0 = geometry pass

/**
 * 2D Signed Distance Field (SDF) of a rounded rectangle.
 * Computes exact Euclidean distance from point p to the rounded box boundary.
 * Negative values indicate interior points, positive indicate exterior.
 */
float sd_rounded_rect(vec2 p, vec2 half_size, float r) {
    vec2 d = abs(p) - (half_size - vec2(r));
    return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0) - r;
}

void main() {
    vec2 rect_center = u_rect.xy + u_rect.zw * 0.5;
    vec2 half_size = u_rect.zw * 0.5;
    vec2 p = v_frag_pos - rect_center;

    if (u_is_shadow == 1) {
        float d = sd_rounded_rect(p, half_size, u_radius);
        if (d <= 0.0) {
            FragColor = u_shadow_color;
        } else {
            float sigma = max(u_shadow_blur * 0.5, 0.1);
            float factor = exp(-(d * d) / (2.0 * sigma * sigma));
            FragColor = vec4(u_shadow_color.rgb, u_shadow_color.a * factor);
        }
        return;
    }

    float d = sd_rounded_rect(p, half_size, u_radius);

    // Screen-space derivative anti-aliasing (smooth 1-pixel transition)
    float fw = fwidth(d);
    if (fw < 0.0001) fw = 1.0;
    float alpha = 1.0 - smoothstep(-0.5 * fw, 0.5 * fw, d);

    if (alpha <= 0.0) {
        discard;
    }

    if (u_border_width > 0.0) {
        // Inner edge for the stroke
        float inner_d = d + u_border_width;
        float inner_alpha = 1.0 - smoothstep(-0.5 * fw, 0.5 * fw, inner_d);
        vec4 color = mix(u_border_color, u_fill_color, inner_alpha);
        FragColor = vec4(color.rgb, color.a * alpha);
    } else {
        FragColor = vec4(u_fill_color.rgb, u_fill_color.a * alpha);
    }
}
)";

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
        // Indeterminate mode: traveling chunk ("un cachito") gliding smoothly left to right
        float chunk_w = max(u_rect.z * 0.32, 40.0);
        float travel = u_rect.z + chunk_w * 2.0;
        float cycle = fract(u_anim_phase);
        float current_center_x = u_rect.x - chunk_w + cycle * travel;

        float d_chunk = abs(v_frag_pos.x - current_center_x);
        if (d_chunk <= chunk_w * 0.5) {
            // Inside traveling chunk: add smooth center highlight ("la cosita blanca")
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
            // Animated white shimmer sweep ("la cosita blanca que va avanzando")
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

Renderer2D::Renderer2D() = default;

Renderer2D::~Renderer2D() {
    shutdown();
}

/**
 * @brief Sets up OpenGL shader programs, vertex array objects, and font atlas.
 */
bool Renderer2D::init() {
    // 1. Initialize Rect Pipeline
    init_rect_pipeline();
    if (m_rect_program == 0) return false;

    // 2. Initialize Text Pipeline
    init_text_pipeline();
    if (m_text_program == 0) return false;

    // 3. Initialize Progress Bar Pipeline
    init_progress_pipeline();
    if (m_progress_program == 0) return false;

    // 4. Upload Font Atlas Texture
    if (!m_font.init_gl()) {
        std::cerr << "[Arin32::Renderer2D] Font atlas texture initialization failed." << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Compiles rect shader and creates rect quad VAO/VBO.
 */
void Renderer2D::init_rect_pipeline() {
    m_rect_program = create_program(RECT_VERTEX_SHADER, RECT_FRAGMENT_SHADER);
    if (!m_rect_program) return;

    // Uniform lookups
    m_u_rect_proj         = glGetUniformLocation(m_rect_program, "u_projection");
    m_u_rect_box          = glGetUniformLocation(m_rect_program, "u_rect");
    m_u_rect_radius       = glGetUniformLocation(m_rect_program, "u_radius");
    m_u_rect_fill         = glGetUniformLocation(m_rect_program, "u_fill_color");
    m_u_rect_border_color = glGetUniformLocation(m_rect_program, "u_border_color");
    m_u_rect_border_width = glGetUniformLocation(m_rect_program, "u_border_width");
    m_u_rect_shadow_color = glGetUniformLocation(m_rect_program, "u_shadow_color");
    m_u_rect_shadow_blur  = glGetUniformLocation(m_rect_program, "u_shadow_blur");
    m_u_rect_is_shadow    = glGetUniformLocation(m_rect_program, "u_is_shadow");

    // Dynamic quad VAO/VBO (updated per draw call or per batch)
    glGenVertexArrays(1, &m_rect_vao);
    glGenBuffers(1, &m_rect_vbo);

    glBindVertexArray(m_rect_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_rect_vbo);

    // Allocate buffer for 6 vertices (2 triangles per quad)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 6, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/**
 * @brief Compiles text shader and creates text dynamic streaming VAO/VBO.
 */
void Renderer2D::init_text_pipeline() {
    m_text_program = create_program(TEXT_VERTEX_SHADER, TEXT_FRAGMENT_SHADER);
    if (!m_text_program) return;

    m_u_text_proj    = glGetUniformLocation(m_text_program, "u_projection");
    m_u_text_sampler = glGetUniformLocation(m_text_program, "u_font_texture");

    glGenVertexArrays(1, &m_text_vao);
    glGenBuffers(1, &m_text_vbo);

    glBindVertexArray(m_text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_text_vbo);

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

    m_text_batch.reserve(1024);
}

void Renderer2D::init_progress_pipeline() {
    m_progress_program = create_program(PROGRESS_VERTEX_SHADER, PROGRESS_FRAGMENT_SHADER);
    if (!m_progress_program) return;

    m_u_pb_proj             = glGetUniformLocation(m_progress_program, "u_projection");
    m_u_pb_box              = glGetUniformLocation(m_progress_program, "u_rect");
    m_u_pb_radius           = glGetUniformLocation(m_progress_program, "u_radius");
    m_u_pb_track_color      = glGetUniformLocation(m_progress_program, "u_track_color");
    m_u_pb_fill_color       = glGetUniformLocation(m_progress_program, "u_fill_color");
    m_u_pb_border_color     = glGetUniformLocation(m_progress_program, "u_border_color");
    m_u_pb_border_width     = glGetUniformLocation(m_progress_program, "u_border_width");
    m_u_pb_fill_fraction    = glGetUniformLocation(m_progress_program, "u_fill_fraction");
    m_u_pb_anim_phase       = glGetUniformLocation(m_progress_program, "u_anim_phase");
    m_u_pb_is_indeterminate = glGetUniformLocation(m_progress_program, "u_is_indeterminate");
}

/**
 * @brief Releases GPU memory handles.
 */
void Renderer2D::shutdown() {
    m_font.cleanup_gl();

    if (m_rect_vao) { glDeleteVertexArrays(1, &m_rect_vao); m_rect_vao = 0; }
    if (m_rect_vbo) { glDeleteBuffers(1, &m_rect_vbo); m_rect_vbo = 0; }
    if (m_rect_program) { glDeleteProgram(m_rect_program); m_rect_program = 0; }

    if (m_text_vao) { glDeleteVertexArrays(1, &m_text_vao); m_text_vao = 0; }
    if (m_text_vbo) { glDeleteBuffers(1, &m_text_vbo); m_text_vbo = 0; }
    if (m_text_program) { glDeleteProgram(m_text_program); m_text_program = 0; }

    if (m_progress_program) { glDeleteProgram(m_progress_program); m_progress_program = 0; }
}

/**
 * @brief Configures OpenGL state machine for 2D UI rendering.
 */
void Renderer2D::begin_frame(int viewport_width, int viewport_height) {
    m_viewport_width = viewport_width;
    m_viewport_height = viewport_height;

    glViewport(0, 0, viewport_width, viewport_height);

    // Alpha blending: Premultiplied / standard alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable 3D depth test and face culling for 2D overlays
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    m_text_batch.clear();
}

/**
 * @brief Flushes batched primitives at the end of the frame.
 */
void Renderer2D::end_frame() {
    flush_text_batch();
}

/**
 * @brief Clears the color buffer.
 */
void Renderer2D::clear(const Color& color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

/**
 * @brief Draws a solid flat rectangle.
 */
void Renderer2D::draw_rect(const Rect& rect, const Color& color) {
    draw_rounded_rect(rect, 0.0f, color);
}

/**
 * @brief Draws a rounded rectangle with Signed Distance Field anti-aliasing.
 */
void Renderer2D::draw_rounded_rect(
    const Rect& rect,
    float corner_radius,
    const Color& fill_color,
    const Color& border_color,
    float border_width
) {
    // Flush pending text so ordering is correct
    flush_text_batch();

    glUseProgram(m_rect_program);

    // 2D Orthographic Projection Matrix (column-major array)
    float ortho[16] = {
        2.0f / static_cast<float>(m_viewport_width), 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / static_cast<float>(m_viewport_height), 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(m_u_rect_proj, 1, GL_FALSE, ortho);

    // Clamp corner radius so it does not exceed half the shortest side
    float max_r = std::min(rect.width, rect.height) * 0.5f;
    float clamped_radius = std::max(0.0f, std::min(corner_radius, max_r));

    glUniform4f(m_u_rect_box, rect.x, rect.y, rect.width, rect.height);
    glUniform1f(m_u_rect_radius, clamped_radius);
    glUniform4f(m_u_rect_fill, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
    glUniform4f(m_u_rect_border_color, border_color.r, border_color.g, border_color.b, border_color.a);
    glUniform1f(m_u_rect_border_width, border_width);
    glUniform1i(m_u_rect_is_shadow, 0);

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

    glBindVertexArray(m_rect_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_rect_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

/**
 * @brief Draws a soft drop shadow beneath a rectangle.
 */
void Renderer2D::draw_shadow(
    const Rect& rect,
    float corner_radius,
    const Color& shadow_color,
    const Vec2& offset,
    float blur
) {
    if (shadow_color.a <= 0.0f || blur <= 0.0f) return;

    flush_text_batch();

    glUseProgram(m_rect_program);

    float ortho[16] = {
        2.0f / static_cast<float>(m_viewport_width), 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / static_cast<float>(m_viewport_height), 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(m_u_rect_proj, 1, GL_FALSE, ortho);

    Rect shadow_rect(rect.x + offset.x, rect.y + offset.y, rect.width, rect.height);
    float max_r = std::min(shadow_rect.width, shadow_rect.height) * 0.5f;
    float clamped_radius = std::max(0.0f, std::min(corner_radius, max_r));

    glUniform4f(m_u_rect_box, shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height);
    glUniform1f(m_u_rect_radius, clamped_radius);
    glUniform4f(m_u_rect_shadow_color, shadow_color.r, shadow_color.g, shadow_color.b, shadow_color.a);
    glUniform1f(m_u_rect_shadow_blur, blur);
    glUniform1i(m_u_rect_is_shadow, 1);

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

    glBindVertexArray(m_rect_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_rect_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

/**
 * @brief Draws a modern Windows 10 style progress bar with hardware shimmer animation.
 */
void Renderer2D::draw_progress_bar(
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

    // Flush pending text before drawing the progress bar
    flush_text_batch();

    glUseProgram(m_progress_program);

    // 2D Orthographic Projection Matrix
    float ortho[16] = {
        2.0f / static_cast<float>(m_viewport_width), 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / static_cast<float>(m_viewport_height), 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(m_u_pb_proj, 1, GL_FALSE, ortho);

    // Clamp corner radius so it does not exceed half the shortest side
    float max_r = std::min(rect.width, rect.height) * 0.5f;
    float clamped_radius = std::max(0.0f, std::min(corner_radius, max_r));

    glUniform4f(m_u_pb_box, rect.x, rect.y, rect.width, rect.height);
    glUniform1f(m_u_pb_radius, clamped_radius);
    glUniform4f(m_u_pb_track_color, track_color.r, track_color.g, track_color.b, track_color.a);
    glUniform4f(m_u_pb_fill_color, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
    glUniform4f(m_u_pb_border_color, border_color.r, border_color.g, border_color.b, border_color.a);
    glUniform1f(m_u_pb_border_width, border_width);
    glUniform1f(m_u_pb_fill_fraction, fill_fraction);
    glUniform1f(m_u_pb_anim_phase, anim_phase);
    glUniform1i(m_u_pb_is_indeterminate, is_indeterminate ? 1 : 0);

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

    glBindVertexArray(m_rect_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_rect_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


/**
 * @brief Appends text quads to the dynamic vertex batch buffer.
 */
void Renderer2D::draw_text(
    const std::string& text,
    const Vec2& position,
    const Color& color,
    float scale
) {
    float cursor_x = position.x;
    float cursor_y = position.y;

    for (char c : text) {
        GlyphInfo g = m_font.get_glyph(c);

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

        m_text_batch.push_back(v0);
        m_text_batch.push_back(v1);
        m_text_batch.push_back(v2);
        m_text_batch.push_back(v3);
        m_text_batch.push_back(v4);
        m_text_batch.push_back(v5);

        cursor_x += g.advance * scale;
    }
}

/**
 * @brief Centers text within a bounding rectangle.
 */
void Renderer2D::draw_text_centered(
    const std::string& text,
    const Rect& bounds,
    const Color& color,
    float scale
) {
    Vec2 text_size = m_font.measure_text(text, scale);
    float text_x = bounds.x + (bounds.width - text_size.x) * 0.5f;
    float text_y = bounds.y + (bounds.height - text_size.y) * 0.5f;
    draw_text(text, Vec2(text_x, text_y), color, scale);
}

void Renderer2D::draw_text_centered_clipped(
    const std::string& text,
    const Rect& bounds,
    const Color& color,
    float scale
) {
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) return;

    // Flush pending text before applying scissor
    flush_text_batch();

    // Convert from top-left (Arin32) to bottom-left (OpenGL scissor coordinates)
    int scissor_x = static_cast<int>(std::max(0.0f, bounds.x));
    int scissor_y = m_viewport_height - static_cast<int>(bounds.y + bounds.height);
    int scissor_w = static_cast<int>(bounds.width);
    int scissor_h = static_cast<int>(bounds.height);

    glEnable(GL_SCISSOR_TEST);
    glScissor(scissor_x, std::max(0, scissor_y), std::max(0, scissor_w), std::max(0, scissor_h));

    draw_text_centered(text, bounds, color, scale);
    flush_text_batch();

    glDisable(GL_SCISSOR_TEST);
}

/**
 * @brief Flushes and renders all batched text quads to the screen.
 */
void Renderer2D::flush_text_batch() {
    if (m_text_batch.empty()) return;

    glUseProgram(m_text_program);

    float ortho[16] = {
        2.0f / static_cast<float>(m_viewport_width), 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / static_cast<float>(m_viewport_height), 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(m_u_text_proj, 1, GL_FALSE, ortho);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_font.texture_id());
    glUniform1i(m_u_text_sampler, 0);

    glBindVertexArray(m_text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_text_vbo);

    // Upload batch vertex array
    glBufferData(
        GL_ARRAY_BUFFER,
        m_text_batch.size() * sizeof(TextVertex),
        m_text_batch.data(),
        GL_DYNAMIC_DRAW
    );

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_text_batch.size()));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    m_text_batch.clear();
}

} // namespace arin
