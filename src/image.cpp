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

#include "arin/image.hpp"
#include <algorithm>

namespace arin {

Image::Image() = default;

Image::Image(
    std::shared_ptr<Texture> texture,
    float x,
    float y,
    float width,
    float height,
    ImageScaleMode scale_mode
) : m_bounds(x, y, width, height),
    m_texture(std::move(texture)),
    m_scale_mode(scale_mode) {}

Image::Image(
    const std::string& filepath,
    float x,
    float y,
    float width,
    float height,
    ImageScaleMode scale_mode
) : m_bounds(x, y, width, height),
    m_texture(Texture::create_from_file(filepath)),
    m_scale_mode(scale_mode) {}

bool Image::load_from_file(const std::string& filepath) {
    m_texture = Texture::create_from_file(filepath);
    return m_texture != nullptr;
}

Rect Image::compute_content_rect() const {
    if (!m_texture || !m_texture->is_valid() || m_bounds.width <= 0.0f || m_bounds.height <= 0.0f) {
        return m_bounds;
    }

    float tex_w = static_cast<float>(m_texture->width());
    float tex_h = static_cast<float>(m_texture->height());
    if (tex_w <= 0.0f || tex_h <= 0.0f) {
        return m_bounds;
    }

    switch (m_scale_mode) {
        case ImageScaleMode::Stretch:
            // Non-uniform fill: stretched to exactly fill the container boundary
            return m_bounds;

        case ImageScaleMode::Center: {
            // Keep native pixel size, center inside bounding box
            float cx = m_bounds.x + (m_bounds.width - tex_w) * 0.5f;
            float cy = m_bounds.y + (m_bounds.height - tex_h) * 0.5f;
            return Rect(cx, cy, tex_w, tex_h);
        }

        case ImageScaleMode::Fit: {
            // Uniform scale: maintain aspect ratio, fit entirely inside bounds (letterbox / pillarbox)
            float aspect = tex_w / tex_h;
            float target_w = m_bounds.width;
            float target_h = m_bounds.width / aspect;

            if (target_h > m_bounds.height) {
                target_h = m_bounds.height;
                target_w = m_bounds.height * aspect;
            }

            float cx = m_bounds.x + (m_bounds.width - target_w) * 0.5f;
            float cy = m_bounds.y + (m_bounds.height - target_h) * 0.5f;
            return Rect(cx, cy, target_w, target_h);
        }

        case ImageScaleMode::Fill: {
            // Uniform scale: maintain aspect ratio, completely cover bounds
            float aspect = tex_w / tex_h;
            float target_w = m_bounds.width;
            float target_h = m_bounds.width / aspect;

            if (target_h < m_bounds.height) {
                target_h = m_bounds.height;
                target_w = m_bounds.height * aspect;
            }

            float cx = m_bounds.x + (m_bounds.width - target_w) * 0.5f;
            float cy = m_bounds.y + (m_bounds.height - target_h) * 0.5f;
            return Rect(cx, cy, target_w, target_h);
        }
    }

    return m_bounds;
}

bool Image::handle_mouse(const MouseEvent& ev) {
    // Images are visual display widgets by default (pass mouse events through)
    (void)ev;
    return false;
}

void Image::update(float dt) {
    (void)dt;
}

void Image::render(Renderer2D& renderer) {
    if (!m_visible || !m_texture || !m_texture->is_valid()) {
        return;
    }

    Rect content_rect = compute_content_rect();
    renderer.draw_image(*m_texture, content_rect, m_tint, m_corner_radius);
}

} // namespace arin
