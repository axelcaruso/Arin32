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

#include "shader_util.hpp"
#include <iostream>
#include <vector>

namespace arin {
namespace renderer {

uint32_t compile_shader(GLenum shader_type, const char* source) {
    uint32_t shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        int log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> info_log(log_length + 1, '\0');
        glGetShaderInfoLog(shader, log_length, nullptr, info_log.data());

        const char* stage_name = (shader_type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment";
        std::cerr << "[Arin32::Renderer] Shader compilation error ("
                  << stage_name << " Shader):\n"
                  << info_log.data() << std::endl;

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

uint32_t create_program(const char* vertex_source, const char* fragment_source) {
    uint32_t vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
    if (!vertex_shader) return 0;

    uint32_t fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);
    if (!fragment_shader) {
        glDeleteShader(vertex_shader);
        return 0;
    }

    uint32_t program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        int log_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> info_log(log_length + 1, '\0');
        glGetProgramInfoLog(program, log_length, nullptr, info_log.data());

        std::cerr << "[Arin32::Renderer] Program linking error:\n"
                  << info_log.data() << std::endl;

        glDeleteProgram(program);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return 0;
    }

    // Flag individual shaders for deletion once detached
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

void make_ortho_projection(int viewport_width, int viewport_height, float* out_matrix) {
    // 2D Orthographic Projection Matrix (column-major array)
    // Maps [0, width] -> [-1, 1], [0, height] -> [1, -1] (top-left origin)
    float w = static_cast<float>(viewport_width);
    float h = static_cast<float>(viewport_height);

    out_matrix[0]  =  2.0f / w;  out_matrix[4]  =  0.0f;      out_matrix[8]  =  0.0f;  out_matrix[12] = -1.0f;
    out_matrix[1]  =  0.0f;      out_matrix[5]  = -2.0f / h;  out_matrix[9]  =  0.0f;  out_matrix[13] =  1.0f;
    out_matrix[2]  =  0.0f;      out_matrix[6]  =  0.0f;      out_matrix[10] = -1.0f;  out_matrix[14] =  0.0f;
    out_matrix[3]  =  0.0f;      out_matrix[7]  =  0.0f;      out_matrix[11] =  0.0f;  out_matrix[15] =  1.0f;
}

} // namespace renderer
} // namespace arin
