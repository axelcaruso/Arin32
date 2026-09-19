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

#ifndef ARIN32_PROGRESS_BAR_HPP
#define ARIN32_PROGRESS_BAR_HPP

#include "types.hpp"
#include "renderer.hpp"
#include <algorithm>

namespace arin {

/**
 * @brief Operating modes for the ProgressBar widget.
 */
enum class ProgressBarMode : uint8_t {
    /**
     * @brief Quantified progress mode from 0% to 100% (or min to max range).
     * The filled section grows proportionally and displays an animated white
     * shimmer sweep highlight moving across the active progress.
     */
    Determinate,

    /**
     * @brief Continuous loading mode with unknown total duration.
     * An animated slice ("un cachito") of the accent color glides smoothly
     * from left to right across the track continuously.
     */
    Indeterminate
};

/**
 * @brief Visual styling configuration for ProgressBar.
 *
 * Designed to match the clean, flat modern Windows 10 aesthetic with subtle
 * borders, smooth 2.5px corner radius, and hardware-accelerated shimmer animation.
 */
struct ProgressBarStyle {
    Color track_color{Color::from_hex(0xE5E7EB)};     ///< Background track color (Light Slate Grey)
    Color fill_color{Color::from_hex(0x06B025)};      ///< Windows 10 Accent Green (#06B025)
    Color border_color{Color::from_hex(0xD1D5DB)};    ///< Subtle boundary border stroke
    float border_width{1.0f};                         ///< Border thickness in pixels (1.0 = subtle)
    float corner_radius{2.5f};                        ///< Sleek flat modern corner radius
    float animation_speed{0.85f};                     ///< Speed multiplier for shimmer and marquee sweeps

    /**
     * @brief Windows 10 classic green progress style (default).
     */
    static ProgressBarStyle green() {
        ProgressBarStyle s;
        s.track_color   = Color::from_hex(0xE5E7EB);
        s.fill_color    = Color::from_hex(0x06B025); // Windows 10 progress green
        s.border_color  = Color::from_hex(0xD1D5DB);
        s.border_width  = 1.0f;
        s.corner_radius = 2.5f;
        return s;
    }

    /**
     * @brief Windows 10 accent blue progress style (#0067C0 / #0078D7).
     */
    static ProgressBarStyle blue() {
        ProgressBarStyle s;
        s.track_color   = Color::from_hex(0xE5E7EB);
        s.fill_color    = Color::from_hex(0x0067C0); // ArinOS / Windows 10 Accent Blue
        s.border_color  = Color::from_hex(0xD1D5DB);
        s.border_width  = 1.0f;
        s.corner_radius = 2.5f;
        return s;
    }

    /**
     * @brief Dark mode progress style for dark surfaces.
     */
    static ProgressBarStyle dark() {
        ProgressBarStyle s;
        s.track_color   = Color::from_hex(0x27272A); // Dark slate track
        s.fill_color    = Color::from_hex(0x06B025); // Bright green fill
        s.border_color  = Color::from_hex(0x3F3F46); // Dark border
        s.border_width  = 1.0f;
        s.corner_radius = 2.5f;
        return s;
    }
};

/**
 * @brief High-performance OpenGL progress bar with Determinate and Indeterminate modes.
 *
 * Features:
 * - Ultra-simple, fluid builder API.
 * - Determinate mode (0% to 100%) with animated white shimmer sweep ("la cosita blanca").
 * - Indeterminate mode with smooth continuous traveling chunk ("un cachito").
 * - 100% GPU-accelerated with Signed Distance Fields (SDF) and screen-space derivative anti-aliasing.
 *
 * Example:
 * @code
 * auto pbar = app.add_progress_bar(100.0f, 200.0f, 300.0f, 20.0f);
 * pbar->set_value(65.0f); // 65%
 * @endcode
 */
class ProgressBar {
public:
    /**
     * @brief Constructs a default 260x20 progress bar at (0, 0).
     */
    ProgressBar();

    /**
     * @brief Constructs a progress bar with explicit position and size.
     * @param x Left coordinate in pixels.
     * @param y Top coordinate in pixels.
     * @param width Width in pixels (default: 260).
     * @param height Height in pixels (default: 20).
     */
    ProgressBar(float x, float y, float width = 260.0f, float height = 20.0f);

    /**
     * @brief Constructs a progress bar with position, size, initial value, and range.
     * @param x Left coordinate.
     * @param y Top coordinate.
     * @param width Width in pixels.
     * @param height Height in pixels.
     * @param value Initial progress value.
     * @param min Minimum range value (default: 0.0f).
     * @param max Maximum range value (default: 100.0f).
     */
    ProgressBar(float x, float y, float width, float height, float value, float min = 0.0f, float max = 100.0f);

    /**
     * @brief Constructs a progress bar with explicit bounding rectangle.
     * @param bounds Bounding box.
     */
    explicit ProgressBar(const Rect& bounds);

    /**
     * @brief Constructs a progress bar with bounding box and mode.
     * @param bounds Bounding box.
     * @param mode Determinate or Indeterminate.
     */
    ProgressBar(const Rect& bounds, ProgressBarMode mode);

    virtual ~ProgressBar() = default;

    // --- Fluent Configuration Setters ---

    /**
     * @brief Sets the operating mode.
     * @param mode Determinate (0-100%) or Indeterminate (traveling chunk).
     * @return Reference to this for chaining.
     */
    ProgressBar& set_mode(ProgressBarMode mode);

    /**
     * @brief Convenience method to toggle between Indeterminate and Determinate modes.
     * @param indeterminate true for indeterminate marquee mode, false for 0-100% determinate.
     * @return Reference to this for chaining.
     */
    ProgressBar& set_indeterminate(bool indeterminate);

    /**
     * @brief Sets the current progress value.
     * Automatically clamped between min_value() and max_value().
     * @param value Progress value.
     * @return Reference to this for chaining.
     */
    ProgressBar& set_value(float value);

    /**
     * @brief Sets the numerical range for determinate progress.
     * @param min_val Minimum progress value (default: 0.0f).
     * @param max_val Maximum progress value (default: 100.0f).
     * @return Reference to this for chaining.
     */
    ProgressBar& set_range(float min_val, float max_val);

    /**
     * @brief Sets bounding rectangle.
     * @param bounds Bounding box.
     * @return Reference to this for chaining.
     */
    ProgressBar& set_bounds(const Rect& bounds);

    /**
     * @brief Sets bounding rectangle with scalar coordinates.
     * @return Reference to this for chaining.
     */
    ProgressBar& set_bounds(float x, float y, float width, float height);

    /**
     * @brief Sets screen position.
     * @return Reference to this for chaining.
     */
    ProgressBar& set_position(float x, float y);

    /**
     * @brief Sets dimensions.
     * @return Reference to this for chaining.
     */
    ProgressBar& set_size(float width, float height);

    /**
     * @brief Sets visual style configuration.
     * @param style ProgressBarStyle.
     * @return Reference to this for chaining.
     */
    ProgressBar& set_style(const ProgressBarStyle& style);

    /**
     * @brief Quick setter for the fill / accent color.
     * @param color New progress fill color.
     * @return Reference to this for chaining.
     */
    ProgressBar& set_fill_color(const Color& color);

    /**
     * @brief Quick setter for track background color.
     * @param color New track color.
     * @return Reference to this for chaining.
     */
    ProgressBar& set_track_color(const Color& color);

    /**
     * @brief Quick setter for corner radius.
     * @param radius Corner radius in pixels.
     * @return Reference to this for chaining.
     */
    ProgressBar& set_corner_radius(float radius) {
        m_style.corner_radius = radius;
        return *this;
    }

    /**
     * @brief Sets animation speed multiplier.
     * @param speed Speed factor (1.0 = normal).
     * @return Reference to this for chaining.
     */
    ProgressBar& set_speed(float speed);

    /**
     * @brief Manually overrides the animation phase (useful for previews and tests).
     * @param phase Phase value [0.0..infinity).
     * @return Reference to this for chaining.
     */
    ProgressBar& set_anim_phase(float phase) {
        m_anim_phase = phase;
        return *this;
    }

    // --- State Queries ---

    /// @brief Gets the current value.
    float value() const { return m_value; }

    /// @brief Gets the minimum value.
    float min_value() const { return m_min_value; }

    /// @brief Gets the maximum value.
    float max_value() const { return m_max_value; }

    /// @brief Gets the normalized progress fraction in the range [0.0, 1.0].
    float percentage() const;

    /// @brief Gets current continuous animation phase.
    float anim_phase() const { return m_anim_phase; }

    /// @brief Gets the current operating mode.
    ProgressBarMode mode() const { return m_mode; }

    /// @brief Returns true if in Indeterminate mode.
    bool is_indeterminate() const { return m_mode == ProgressBarMode::Indeterminate; }

    /// @brief Gets bounding rectangle.
    const Rect& bounds() const { return m_bounds; }

    /// @brief Gets active visual style.
    const ProgressBarStyle& style() const { return m_style; }
    ProgressBarStyle& style() { return m_style; }

    // --- Animation & Rendering ---

    /**
     * @brief Advances internal animation phase for shimmer / indeterminate sweeps.
     * @param dt Delta time in seconds elapsed since previous frame.
     */
    void update(float dt);

    /**
     * @brief Renders the progress bar using the 2D OpenGL renderer.
     * @param renderer The 2D renderer to draw with.
     */
    void render(Renderer2D& renderer);

private:
    Rect m_bounds{0.0f, 0.0f, 260.0f, 20.0f};
    ProgressBarMode m_mode{ProgressBarMode::Determinate};
    ProgressBarStyle m_style{ProgressBarStyle::green()};

    float m_value{0.0f};
    float m_min_value{0.0f};
    float m_max_value{100.0f};

    float m_anim_phase{0.0f}; // Continuous [0.0..1.0] cycle for shimmer sweeps
};

} // namespace arin

#endif // ARIN32_PROGRESS_BAR_HPP
