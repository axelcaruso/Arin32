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

#include "arin/svg.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <algorithm>

// Suppress compiler warnings in third-party NanoSVG header files (-Wall -Wextra -Wpedantic)
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif

#define NANOSVG_IMPLEMENTATION
#include "thirdparty/nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "thirdparty/nanosvgrast.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace arin {

// -----------------------------------------------------------------------------
// SvgDocument Implementation
// -----------------------------------------------------------------------------

SvgDocument::SvgDocument() = default;

SvgDocument::~SvgDocument() {
    release();
}

SvgDocument::SvgDocument(SvgDocument&& other) noexcept
    : m_image(other.m_image),
      m_width(other.m_width),
      m_height(other.m_height) {
    other.m_image = nullptr;
    other.m_width = 0.0f;
    other.m_height = 0.0f;
}

SvgDocument& SvgDocument::operator=(SvgDocument&& other) noexcept {
    if (this != &other) {
        release();
        m_image = other.m_image;
        m_width = other.m_width;
        m_height = other.m_height;
        other.m_image = nullptr;
        other.m_width = 0.0f;
        other.m_height = 0.0f;
    }
    return *this;
}

bool SvgDocument::is_valid() const {
    return m_image != nullptr && m_width > 0.0f && m_height > 0.0f;
}

void SvgDocument::release() {
    if (m_image != nullptr) {
        nsvgDelete(static_cast<NSVGimage*>(m_image));
        m_image = nullptr;
    }
    m_width = 0.0f;
    m_height = 0.0f;
}

std::shared_ptr<SvgDocument> SvgDocument::load_from_file(
    const std::string& filepath,
    float dpi
) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[Arin32::SvgDocument] Unable to open SVG file: " << filepath << std::endl;
        return nullptr;
    }

    std::string buffer((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return load_from_memory(buffer, dpi);
}

std::shared_ptr<SvgDocument> SvgDocument::load_from_memory(
    const std::string& svg_xml,
    float dpi
) {
    if (svg_xml.empty()) {
        return nullptr;
    }

    // NanoSVG modifies the input buffer in-place during tokenization, so pass a writable copy
    std::vector<char> copy(svg_xml.begin(), svg_xml.end());
    copy.push_back('\0');

    NSVGimage* img = nsvgParse(copy.data(), "px", dpi);
    if (!img) {
        std::cerr << "[Arin32::SvgDocument] Failed to parse SVG markup" << std::endl;
        return nullptr;
    }

    if (img->width <= 0.0f || img->height <= 0.0f || img->shapes == nullptr) {
        nsvgDelete(img);
        return nullptr;
    }

    auto doc = std::make_shared<SvgDocument>();
    doc->m_image = img;
    doc->m_width = img->width;
    doc->m_height = img->height;
    return doc;
}

std::shared_ptr<SvgDocument> SvgDocument::load_from_memory(
    const uint8_t* data,
    size_t size_bytes,
    float dpi
) {
    if (!data || size_bytes == 0) {
        return nullptr;
    }
    std::string buffer(reinterpret_cast<const char*>(data), size_bytes);
    return load_from_memory(buffer, dpi);
}

std::vector<uint8_t> SvgDocument::rasterize_rgba(
    int target_width,
    int target_height,
    float scale,
    int* out_width,
    int* out_height
) const {
    if (!is_valid()) {
        if (out_width) *out_width = 0;
        if (out_height) *out_height = 0;
        return {};
    }

    int raster_w = target_width;
    int raster_h = target_height;

    if (raster_w <= 0 && raster_h <= 0) {
        raster_w = static_cast<int>(std::ceil(m_width * scale));
        raster_h = static_cast<int>(std::ceil(m_height * scale));
    } else if (raster_w <= 0 && raster_h > 0) {
        raster_w = static_cast<int>(std::ceil(raster_h * aspect_ratio()));
    } else if (raster_h <= 0 && raster_w > 0) {
        raster_h = static_cast<int>(std::ceil(raster_w / aspect_ratio()));
    }

    raster_w = std::max(1, raster_w);
    raster_h = std::max(1, raster_h);

    if (out_width) *out_width = raster_w;
    if (out_height) *out_height = raster_h;

    NSVGrasterizer* rast = nsvgCreateRasterizer();
    if (!rast) {
        std::cerr << "[Arin32::SvgDocument] Failed to initialize NanoSVG rasterizer" << std::endl;
        return {};
    }

    std::vector<uint8_t> pixels(raster_w * raster_h * 4, 0);
    float scale_x = static_cast<float>(raster_w) / m_width;
    float scale_y = static_cast<float>(raster_h) / m_height;
    float effective_scale = std::min(scale_x, scale_y);

    float offset_x = (static_cast<float>(raster_w) - m_width * effective_scale) * 0.5f;
    float offset_y = (static_cast<float>(raster_h) - m_height * effective_scale) * 0.5f;

    nsvgRasterize(
        rast,
        static_cast<NSVGimage*>(m_image),
        offset_x,
        offset_y,
        effective_scale,
        pixels.data(),
        raster_w,
        raster_h,
        raster_w * 4
    );

    nsvgDeleteRasterizer(rast);
    return pixels;
}

std::shared_ptr<Texture> SvgDocument::create_texture(
    int target_width,
    int target_height,
    float scale,
    TextureFilter filter
) const {
    int w = 0;
    int h = 0;
    std::vector<uint8_t> pixels = rasterize_rgba(target_width, target_height, scale, &w, &h);
    if (pixels.empty() || w <= 0 || h <= 0) {
        return nullptr;
    }
    return Texture::create_from_rgba(w, h, pixels.data(), filter);
}

// -----------------------------------------------------------------------------
// SvgImage Widget Implementation
// -----------------------------------------------------------------------------

SvgImage::SvgImage() = default;

SvgImage::SvgImage(
    std::shared_ptr<SvgDocument> document,
    float x,
    float y,
    float width,
    float height,
    ImageScaleMode scale_mode
) : m_bounds(x, y, width, height),
    m_document(std::move(document)),
    m_scale_mode(scale_mode),
    m_needs_rasterize(true) {}

SvgImage::SvgImage(
    const std::string& filepath,
    float x,
    float y,
    float width,
    float height,
    ImageScaleMode scale_mode
) : m_bounds(x, y, width, height),
    m_scale_mode(scale_mode),
    m_needs_rasterize(true) {
    load_from_file(filepath);
}

SvgImage& SvgImage::set_bounds(const Rect& bounds) {
    if (bounds.width != m_bounds.width || bounds.height != m_bounds.height) {
        m_needs_rasterize = true;
    }
    m_bounds = bounds;
    return *this;
}

SvgImage& SvgImage::set_position(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
    return *this;
}

SvgImage& SvgImage::set_size(float width, float height) {
    if (width != m_bounds.width || height != m_bounds.height) {
        m_needs_rasterize = true;
    }
    m_bounds.width = width;
    m_bounds.height = height;
    return *this;
}

SvgImage& SvgImage::set_document(std::shared_ptr<SvgDocument> document) {
    m_document = std::move(document);
    invalidate_texture();
    return *this;
}

bool SvgImage::load_from_file(const std::string& filepath) {
    m_document = SvgDocument::load_from_file(filepath);
    invalidate_texture();
    return m_document != nullptr && m_document->is_valid();
}

bool SvgImage::load_from_memory(const std::string& svg_xml) {
    m_document = SvgDocument::load_from_memory(svg_xml);
    invalidate_texture();
    return m_document != nullptr && m_document->is_valid();
}

SvgImage& SvgImage::set_scale_mode(ImageScaleMode mode) {
    m_scale_mode = mode;
    return *this;
}

void SvgImage::invalidate_texture() {
    m_needs_rasterize = true;
    m_texture = nullptr;
}

void SvgImage::ensure_texture() {
    if (!m_document || !m_document->is_valid()) {
        return;
    }

    int req_w = static_cast<int>(std::ceil(m_bounds.width));
    int req_h = static_cast<int>(std::ceil(m_bounds.height));

    if (req_w <= 0 || req_h <= 0) {
        return;
    }

    if (m_needs_rasterize || !m_texture || req_w != m_last_raster_w || req_h != m_last_raster_h) {
        m_texture = m_document->create_texture(req_w, req_h, 1.0f, TextureFilter::Linear);
        m_last_raster_w = req_w;
        m_last_raster_h = req_h;
        m_needs_rasterize = false;
    }
}

Rect SvgImage::compute_content_rect() const {
    if (!m_document || !m_document->is_valid()) {
        return m_bounds;
    }

    float doc_w = m_document->width();
    float doc_h = m_document->height();
    if (doc_w <= 0.0f || doc_h <= 0.0f) {
        return m_bounds;
    }

    float box_w = m_bounds.width;
    float box_h = m_bounds.height;
    float doc_aspect = doc_w / doc_h;
    float box_aspect = (box_h > 0.0f) ? (box_w / box_h) : 1.0f;

    switch (m_scale_mode) {
        case ImageScaleMode::Stretch:
            return m_bounds;

        case ImageScaleMode::Fit: {
            float dest_w = box_w;
            float dest_h = box_h;
            if (box_aspect > doc_aspect) {
                dest_w = box_h * doc_aspect;
            } else {
                dest_h = box_w / doc_aspect;
            }
            float dest_x = m_bounds.x + (box_w - dest_w) * 0.5f;
            float dest_y = m_bounds.y + (box_h - dest_h) * 0.5f;
            return Rect(dest_x, dest_y, dest_w, dest_h);
        }

        case ImageScaleMode::Fill: {
            float dest_w = box_w;
            float dest_h = box_h;
            if (box_aspect > doc_aspect) {
                dest_h = box_w / doc_aspect;
            } else {
                dest_w = box_h * doc_aspect;
            }
            float dest_x = m_bounds.x + (box_w - dest_w) * 0.5f;
            float dest_y = m_bounds.y + (box_h - dest_h) * 0.5f;
            return Rect(dest_x, dest_y, dest_w, dest_h);
        }

        case ImageScaleMode::Center: {
            float dest_w = doc_w;
            float dest_h = doc_h;
            float dest_x = m_bounds.x + (box_w - dest_w) * 0.5f;
            float dest_y = m_bounds.y + (box_h - dest_h) * 0.5f;
            return Rect(dest_x, dest_y, dest_w, dest_h);
        }

        default:
            return m_bounds;
    }
}

bool SvgImage::handle_mouse(const MouseEvent& ev) {
    (void)ev;
    return false;
}

void SvgImage::update(float dt) {
    (void)dt;
}

void SvgImage::render(Renderer2D& renderer) {
    if (!m_visible || m_bounds.width <= 0.0f || m_bounds.height <= 0.0f) {
        return;
    }

    ensure_texture();

    if (!m_texture || !m_texture->is_valid()) {
        return;
    }

    Rect content_rect = compute_content_rect();

    renderer.draw_image(
        *m_texture,
        content_rect,
        m_tint,
        m_corner_radius
    );
}

} // namespace arin
