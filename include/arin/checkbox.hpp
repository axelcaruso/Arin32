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

#ifndef ARIN32_CHECKBOX_HPP
#define ARIN32_CHECKBOX_HPP

#include "types.hpp"
#include "widget.hpp"
#include "renderer.hpp"
#include <string>
#include <functional>

namespace arin {

/**
 * @brief Visual styling configuration for the CheckBox widget.
 */
struct CheckBoxStyle {
    float box_size{16.0f};                      ///< Width and height of the checkable square in pixels
    float corner_radius{3.0f};                 ///< Corner rounding radius of the square box
    float text_spacing{8.0f};                  ///< Gap between the checkbox square and its label text
    float text_scale{0.92f};                   ///< Typography scale factor
    Color box_background{Color::white()};      ///< Fill color when unchecked
    Color box_border{Color::from_hex(0x9CA3AF)};///< Border color when unchecked
    Color checked_fill{Color::from_hex(0x0067C0)};///< Accent fill color when checked
    Color checkmark_color{Color::white()};     ///< Stroke color of the internal checkmark
    Color text_color{Color::from_hex(0x1F2937)};///< Label text color
    Color hover_border{Color::from_hex(0x4B5563)};///< Border outline highlight when hovered
    Color disabled_color{Color::from_hex(0xD1D5DB)};///< Toned down color when disabled
};

/**
 * @brief Fully interactive, standalone two-state CheckBox widget.
 *
 * Supports:
 * - Fluid click interaction on both the box and its associated label
 * - Crisp GPU-rendered checkmark via Signed Distance Field line segments
 * - Hover state visual feedback
 * - Automatic bounding box computation based on measured text length
 * - Clean integration with automatic layout containers (VBox, HBox)
 */
class CheckBox : public IWidget {
public:
    using ToggleCallback = std::function<void(bool is_checked)>;

    /**
     * @brief Constructs a CheckBox widget.
     * @param label Accompanying descriptive label text.
     * @param x Top-left horizontal position in pixels.
     * @param y Top-left vertical position in pixels.
     * @param checked Initial toggle state (default: false).
     */
    explicit CheckBox(
        std::string label = "CheckBox",
        float x = 0.0f,
        float y = 0.0f,
        bool checked = false
    );

    ~CheckBox() override = default;

    // --- IWidget Interface Implementation ---

    const Rect& bounds() const override { return m_bounds; }

    CheckBox& set_bounds(const Rect& bounds) override {
        m_bounds = bounds;
        return *this;
    }

    CheckBox& set_position(float x, float y) override {
        m_bounds.x = x;
        m_bounds.y = y;
        return *this;
    }

    CheckBox& set_size(float width, float height) override {
        m_bounds.width = width;
        m_bounds.height = height;
        return *this;
    }

    bool handle_mouse(const MouseEvent& ev) override;
    void update(float dt) override;
    void render(Renderer2D& renderer) override;
    bool is_visible() const override { return m_visible; }
    bool is_enabled() const override { return m_enabled; }

    // --- CheckBox Properties & Chaining ---

    /// @brief Gets label text.
    const std::string& label() const { return m_label; }

    /// @brief Sets label text.
    CheckBox& set_label(std::string label) {
        m_label = std::move(label);
        return *this;
    }

    /// @brief Checks whether the checkbox is checked.
    bool is_checked() const { return m_checked; }

    /// @brief Sets the checked state and triggers callback if changed.
    CheckBox& set_checked(bool checked);

    /// @brief Toggles the checked state.
    CheckBox& toggle();

    /// @brief Registers callback invoked whenever check state toggles.
    CheckBox& on_toggled(ToggleCallback cb) {
        m_toggle_cb = std::move(cb);
        return *this;
    }

    /// @brief Sets active visual style.
    CheckBox& set_style(const CheckBoxStyle& style) {
        m_style = style;
        return *this;
    }

    /// @brief Gets active visual style.
    const CheckBoxStyle& style() const { return m_style; }

    /// @brief Sets widget visibility.
    CheckBox& set_visible(bool visible) {
        m_visible = visible;
        return *this;
    }

    /// @brief Sets interactive enabled state.
    CheckBox& set_enabled(bool enabled) {
        m_enabled = enabled;
        return *this;
    }

    /// @brief Automatically adjusts widget bounds to tightly fit the box and label.
    CheckBox& fit_to_content(const Font& font);

private:
    std::string m_label{"CheckBox"};
    Rect m_bounds{0.0f, 0.0f, 160.0f, 20.0f};
    bool m_checked{false};
    bool m_hovered{false};
    bool m_pressed{false};
    bool m_visible{true};
    bool m_enabled{true};

    CheckBoxStyle m_style;
    ToggleCallback m_toggle_cb;
};

} // namespace arin

#endif // ARIN32_CHECKBOX_HPP
