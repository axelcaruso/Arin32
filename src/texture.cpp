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

#include "arin/texture.hpp"
#include <GL/glew.h>
#include <iostream>

// Suppress third-party warnings in stb_image to maintain zero compiler warnings (-Wall -Wextra -Wpedantic)
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO_FAIL_WARNING
#include "thirdparty/stb_image.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace arin {

namespace {

/**
 * @brief Helper function to configure standard OpenGL 2D texture filtering and clamping.
 *
 * Forces GL_CLAMP_TO_EDGE to avoid seam bleeding at borders and sets min/mag filters.
 */
void configure_texture_parameters(TextureFilter filter) {
    GLenum gl_filter = (filter == TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

} // anonymous namespace

Texture::Texture() = default;

Texture::~Texture() {
    release();
}

Texture::Texture(Texture&& other) noexcept
    : m_handle(other.m_handle),
      m_width(other.m_width),
      m_height(other.m_height),
      m_channels(other.m_channels),
      m_filter(other.m_filter) {
    other.m_handle = 0;
    other.m_width = 0;
    other.m_height = 0;
    other.m_channels = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        release();
        m_handle = other.m_handle;
        m_width = other.m_width;
        m_height = other.m_height;
        m_channels = other.m_channels;
        m_filter = other.m_filter;

        other.m_handle = 0;
        other.m_width = 0;
        other.m_height = 0;
        other.m_channels = 0;
    }
    return *this;
}

void Texture::release() {
    if (m_handle != 0) {
        glDeleteTextures(1, &m_handle);
        m_handle = 0;
    }
    m_width = 0;
    m_height = 0;
    m_channels = 0;
}

std::shared_ptr<Texture> Texture::create_from_file(
    const std::string& filepath,
    TextureFilter filter
) {
    int width = 0;
    int height = 0;
    int channels = 0;

    // Force 4 channels (RGBA) so shaders always receive standard 32-bit pixel data
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "[Arin32::Texture] Failed to load image from file: " << filepath
                  << " (" << stbi_failure_reason() << ")" << std::endl;
        return nullptr;
    }

    auto texture = create_from_rgba(width, height, data, filter);

    // Free the decoded buffer allocated by stb_image
    stbi_image_free(data);

    return texture;
}

std::shared_ptr<Texture> Texture::create_from_memory(
    const uint8_t* data,
    size_t size_bytes,
    TextureFilter filter
) {
    if (!data || size_bytes == 0) {
        return nullptr;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* decoded = stbi_load_from_memory(
        data,
        static_cast<int>(size_bytes),
        &width,
        &height,
        &channels,
        4
    );

    if (!decoded) {
        std::cerr << "[Arin32::Texture] Failed to decode image from memory buffer: "
                  << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    auto texture = create_from_rgba(width, height, decoded, filter);
    stbi_image_free(decoded);

    return texture;
}

std::shared_ptr<Texture> Texture::create_from_rgba(
    int width,
    int height,
    const uint8_t* rgba_pixels,
    TextureFilter filter
) {
    if (width <= 0 || height <= 0) {
        return nullptr;
    }

    auto texture = std::make_shared<Texture>();
    texture->m_width = width;
    texture->m_height = height;
    texture->m_channels = 4;
    texture->m_filter = filter;

    // Generate OpenGL 2D texture
    glGenTextures(1, &texture->m_handle);
    glBindTexture(GL_TEXTURE_2D, texture->m_handle);

    // Ensure 1-byte pixel unpack alignment for safety
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Configure texture filtering and boundary wrapping
    configure_texture_parameters(filter);

    // Upload pixel data to GPU
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        rgba_pixels
    );

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

std::shared_ptr<Texture> Texture::create_empty(
    int width,
    int height,
    TextureFilter filter
) {
    return create_from_rgba(width, height, nullptr, filter);
}

void Texture::update_sub_rect(int x, int y, int width, int height, const uint8_t* rgba_pixels) {
    if (m_handle == 0 || !rgba_pixels || width <= 0 || height <= 0) {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, m_handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        x,
        y,
        width,
        height,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        rgba_pixels
    );

    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace arin
