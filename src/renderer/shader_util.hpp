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

#ifndef ARIN32_RENDERER_SHADER_UTIL_HPP
#define ARIN32_RENDERER_SHADER_UTIL_HPP

#include <cstdint>
#include <GL/glew.h>

namespace arin {
namespace renderer {

/**
 * @brief Compiles a GLSL shader object of the specified stage.
 *
 * @param shader_type GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
 * @param source Raw GLSL source code string.
 * @return Compiled OpenGL shader handle, or 0 on error.
 */
uint32_t compile_shader(GLenum shader_type, const char* source);

/**
 * @brief Links vertex and fragment shaders into a complete GPU program.
 *
 * Automatically detaches and flags shaders for deletion upon link completion.
 *
 * @param vertex_source Raw GLSL vertex shader code.
 * @param fragment_source Raw GLSL fragment shader code.
 * @return Linked OpenGL program handle, or 0 on error.
 */
uint32_t create_program(const char* vertex_source, const char* fragment_source);

/**
 * @brief Computes a standard 2D orthographic projection matrix for top-left (0,0) UI coordinates.
 *
 * @param viewport_width Width in screen pixels.
 * @param viewport_height Height in screen pixels.
 * @param out_matrix Output 16-element column-major float array.
 */
void make_ortho_projection(int viewport_width, int viewport_height, float* out_matrix);

} // namespace renderer
} // namespace arin

#endif // ARIN32_RENDERER_SHADER_UTIL_HPP
