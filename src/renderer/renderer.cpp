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
#include "arin/texture.hpp"
#include "arin/icon.hpp"
#include "rect_pipeline.hpp"
#include "text_pipeline.hpp"
#include "progress_pipeline.hpp"
#include "image_pipeline.hpp"

#include <GL/glew.h>
#include <iostream>
#include <algorithm>

namespace arin {

Renderer2D::Renderer2D()
    : m_rect_pipeline(std::make_unique<renderer::RectPipeline>()),
      m_text_pipeline(std::make_unique<renderer::TextPipeline>()),
      m_progress_pipeline(std::make_unique<renderer::ProgressPipeline>()),
      m_image_pipeline(std::make_unique<renderer::ImagePipeline>()) {}

Renderer2D::~Renderer2D() {
    shutdown();
}

Renderer2D::Renderer2D(Renderer2D&&) noexcept = default;
Renderer2D& Renderer2D::operator=(Renderer2D&&) noexcept = default;

/**
 * @brief Sets up OpenGL shader programs, vertex array objects, and font atlas.
 */
bool Renderer2D::init() {
    // 1. Initialize Rect Pipeline
    if (!m_rect_pipeline->init()) {
        std::cerr << "[Arin32::Renderer2D] Rect pipeline initialization failed." << std::endl;
        return false;
    }

    // 2. Initialize Text Pipeline
    if (!m_text_pipeline->init()) {
        std::cerr << "[Arin32::Renderer2D] Text pipeline initialization failed." << std::endl;
        return false;
    }

    // 3. Initialize Progress Bar Pipeline
    if (!m_progress_pipeline->init()) {
        std::cerr << "[Arin32::Renderer2D] Progress pipeline initialization failed." << std::endl;
        return false;
    }

    // 4. Initialize Image Pipeline
    if (!m_image_pipeline->init()) {
        std::cerr << "[Arin32::Renderer2D] Image pipeline initialization failed." << std::endl;
        return false;
    }

    // 5. Upload Embedded Font Atlas Texture
    if (!m_font.init_gl()) {
        std::cerr << "[Arin32::Renderer2D] Font atlas texture initialization failed." << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Releases GPU memory handles across all pipelines.
 */
void Renderer2D::shutdown() {
    m_font.cleanup_gl();

    if (m_rect_pipeline) m_rect_pipeline->shutdown();
    if (m_text_pipeline) m_text_pipeline->shutdown();
    if (m_progress_pipeline) m_progress_pipeline->shutdown();
    if (m_image_pipeline) m_image_pipeline->shutdown();
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

    m_clip_stack.clear();
    glDisable(GL_SCISSOR_TEST);

    m_text_pipeline->clear();
}

void Renderer2D::flush() {
    m_text_pipeline->flush(m_viewport_width, m_viewport_height, m_font.texture_id());
}

/**
 * @brief Flushes batched primitives at the end of the frame.
 */
void Renderer2D::end_frame() {
    flush();
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
    m_text_pipeline->flush(m_viewport_width, m_viewport_height, m_font.texture_id());

    m_rect_pipeline->draw_rounded_rect(
        m_viewport_width,
        m_viewport_height,
        rect,
        corner_radius,
        fill_color,
        border_color,
        border_width
    );
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
    // Flush pending text so ordering is correct
    m_text_pipeline->flush(m_viewport_width, m_viewport_height, m_font.texture_id());

    m_rect_pipeline->draw_shadow(
        m_viewport_width,
        m_viewport_height,
        rect,
        corner_radius,
        shadow_color,
        offset,
        blur
    );
}

/**
 * @brief Draws a crisp checkmark inside a checkbox boundary.
 */
void Renderer2D::draw_checkmark(const Rect& box, const Color& color, float thickness) {
    m_text_pipeline->flush(m_viewport_width, m_viewport_height, m_font.texture_id());
    m_rect_pipeline->draw_checkmark(m_viewport_width, m_viewport_height, box, color, thickness);
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
    // Flush pending text before drawing the progress bar
    m_text_pipeline->flush(m_viewport_width, m_viewport_height, m_font.texture_id());

    m_progress_pipeline->draw_progress_bar(
        m_viewport_width,
        m_viewport_height,
        rect,
        corner_radius,
        track_color,
        fill_color,
        border_color,
        border_width,
        fill_fraction,
        anim_phase,
        is_indeterminate
    );
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
    m_text_pipeline->draw_text(m_font, text, position, color, scale);
}

/**
 * @brief Centers text optically within a bounding rectangle.
 *
 * Delegates origin computation to Font::layout_text_in_rect so every widget
 * shares the same baseline-aware vertical centering behavior.
 */
void Renderer2D::draw_text_centered(
    const std::string& text,
    const Rect& bounds,
    const Color& color,
    float scale
) {
    draw_text_in_rect(text, bounds, color, scale, TextAlignH::Center, TextAlignV::Center);
}

/**
 * @brief Renders aligned text inside a rectangle with optical vertical centering.
 */
void Renderer2D::draw_text_in_rect(
    const std::string& text,
    const Rect& bounds,
    const Color& color,
    float scale,
    TextAlignH align_h,
    TextAlignV align_v
) {
    if (text.empty() || bounds.width <= 0.0f || bounds.height <= 0.0f) {
        return;
    }
    const Vec2 origin = m_font.layout_text_in_rect(text, bounds, scale, align_h, align_v);
    draw_text(text, origin, color, scale);
}

void Renderer2D::push_clip_rect(const Rect& rect) {
    flush();

    Rect current = rect;
    if (!m_clip_stack.empty()) {
        const Rect& top = m_clip_stack.back();
        float x1 = std::max(top.x, rect.x);
        float y1 = std::max(top.y, rect.y);
        float x2 = std::min(top.x + top.width, rect.x + rect.width);
        float y2 = std::min(top.y + top.height, rect.y + rect.height);
        current = Rect(x1, y1, std::max(0.0f, x2 - x1), std::max(0.0f, y2 - y1));
    }
    m_clip_stack.push_back(current);

    int scissor_x = static_cast<int>(std::max(0.0f, current.x));
    int scissor_y = m_viewport_height - static_cast<int>(current.y + current.height);
    int scissor_w = static_cast<int>(std::max(0.0f, current.width));
    int scissor_h = static_cast<int>(std::max(0.0f, current.height));

    glEnable(GL_SCISSOR_TEST);
    glScissor(scissor_x, std::max(0, scissor_y), scissor_w, scissor_h);
}

void Renderer2D::pop_clip_rect() {
    flush();

    if (!m_clip_stack.empty()) {
        m_clip_stack.pop_back();
    }

    if (m_clip_stack.empty()) {
        glDisable(GL_SCISSOR_TEST);
    } else {
        const Rect& current = m_clip_stack.back();
        int scissor_x = static_cast<int>(std::max(0.0f, current.x));
        int scissor_y = m_viewport_height - static_cast<int>(current.y + current.height);
        int scissor_w = static_cast<int>(std::max(0.0f, current.width));
        int scissor_h = static_cast<int>(std::max(0.0f, current.height));

        glEnable(GL_SCISSOR_TEST);
        glScissor(scissor_x, std::max(0, scissor_y), scissor_w, scissor_h);
    }
}

/**
 * @brief Centers text within a bounding box strictly clipped via glScissor.
 */
void Renderer2D::draw_text_centered_clipped(
    const std::string& text,
    const Rect& bounds,
    const Color& color,
    float scale
) {
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) return;

    push_clip_rect(bounds);
    draw_text_centered(text, bounds, color, scale);
    pop_clip_rect();
}

void Renderer2D::draw_image(
    const Texture& texture,
    const Rect& dest,
    const Color& tint,
    float corner_radius
) {
    if (!m_image_pipeline || !texture.is_valid()) return;

    // Flush any pending batched typography before switching textures
    flush();

    // Default UV covers the full texture [0.0, 1.0] in both axes
    Rect full_uv(0.0f, 0.0f, 1.0f, 1.0f);
    m_image_pipeline->draw_image(
        m_viewport_width,
        m_viewport_height,
        texture.handle(),
        dest,
        full_uv,
        tint,
        corner_radius
    );
}

void Renderer2D::draw_image(
    const Texture& texture,
    const Rect& src_uv,
    const Rect& dest,
    const Color& tint,
    float corner_radius
) {
    if (!m_image_pipeline || !texture.is_valid()) return;

    // Flush pending text before drawing texture quad
    flush();

    m_image_pipeline->draw_image(
        m_viewport_width,
        m_viewport_height,
        texture.handle(),
        dest,
        src_uv,
        tint,
        corner_radius
    );
}

void Renderer2D::draw_icon(
    IconType icon,
    const Rect& bounds,
    const Color& color
) {
    if (!m_rect_pipeline || icon == IconType::None) return;

    // Flush pending batched text before rendering vector icon
    flush();

    m_rect_pipeline->draw_icon(
        m_viewport_width,
        m_viewport_height,
        icon,
        bounds,
        color
    );
}

} // namespace arin
