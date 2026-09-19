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

#include "arin/progress_bar.hpp"
#include "arin/metrics.hpp"
#include <cmath>

namespace arin {

ProgressBar::ProgressBar()
    : m_bounds(0.0f, 0.0f, UiMetrics::kDefaultProgressBarSize.x, UiMetrics::kDefaultProgressBarSize.y), m_mode(ProgressBarMode::Determinate) {}

ProgressBar::ProgressBar(float x, float y, float width, float height)
    : m_bounds(x, y, width, height), m_mode(ProgressBarMode::Determinate) {}

ProgressBar::ProgressBar(float x, float y, float width, float height, float value, float min, float max)
    : m_bounds(x, y, width, height),
      m_mode(ProgressBarMode::Determinate),
      m_style(ProgressBarStyle::green()),
      m_value(std::clamp(value, min, max > min ? max : min + 1.0f)),
      m_min_value(min),
      m_max_value(max > min ? max : min + 1.0f),
      m_anim_phase(0.0f) {}

ProgressBar::ProgressBar(const Rect& bounds)
    : m_bounds(bounds), m_mode(ProgressBarMode::Determinate) {}

ProgressBar::ProgressBar(const Rect& bounds, ProgressBarMode mode)
    : m_bounds(bounds), m_mode(mode) {}

ProgressBar& ProgressBar::set_mode(ProgressBarMode mode) {
    m_mode = mode;
    return *this;
}

ProgressBar& ProgressBar::set_indeterminate(bool indeterminate) {
    m_mode = indeterminate ? ProgressBarMode::Indeterminate : ProgressBarMode::Determinate;
    return *this;
}

ProgressBar& ProgressBar::set_value(float value) {
    m_value = std::clamp(value, m_min_value, m_max_value);
    return *this;
}

ProgressBar& ProgressBar::set_range(float min_val, float max_val) {
    m_min_value = min_val;
    m_max_value = std::max(min_val + 0.0001f, max_val);
    m_value = std::clamp(m_value, m_min_value, m_max_value);
    return *this;
}

ProgressBar& ProgressBar::set_bounds(const Rect& bounds) {
    m_bounds = bounds;
    return *this;
}

ProgressBar& ProgressBar::set_bounds(float x, float y, float width, float height) {
    m_bounds = Rect(x, y, width, height);
    return *this;
}

ProgressBar& ProgressBar::set_position(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
    return *this;
}

ProgressBar& ProgressBar::set_size(float width, float height) {
    m_bounds.width = width;
    m_bounds.height = height;
    return *this;
}

ProgressBar& ProgressBar::set_style(const ProgressBarStyle& style) {
    m_style = style;
    return *this;
}

ProgressBar& ProgressBar::set_fill_color(const Color& color) {
    m_style.fill_color = color;
    return *this;
}

ProgressBar& ProgressBar::set_track_color(const Color& color) {
    m_style.track_color = color;
    return *this;
}

ProgressBar& ProgressBar::set_speed(float speed) {
    m_style.animation_speed = speed;
    return *this;
}

float ProgressBar::percentage() const {
    if (m_max_value <= m_min_value) return 0.0f;
    float clamped = std::clamp(m_value, m_min_value, m_max_value);
    return (clamped - m_min_value) / (m_max_value - m_min_value);
}

/**
 * @brief Advances internal animation timer for shimmer sweeps and marquee chunk travel.
 */
void ProgressBar::update(float dt) {
    m_anim_phase += dt * m_style.animation_speed;
    // Keep phase bounded without precision loss
    if (m_anim_phase > UiMetrics::kProgressPhaseWrap) {
        m_anim_phase = std::fmod(m_anim_phase, 1.0f);
    }
}

/**
 * @brief Renders the progress bar track, active fill, and animated shimmer effect.
 */
void ProgressBar::render(Renderer2D& renderer) {
    renderer.draw_progress_bar(
        m_bounds,
        m_style.corner_radius,
        m_style.track_color,
        m_style.fill_color,
        m_style.border_color,
        m_style.border_width,
        percentage(),
        m_anim_phase,
        m_mode == ProgressBarMode::Indeterminate
    );
}

} // namespace arin
