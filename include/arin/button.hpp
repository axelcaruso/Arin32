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

#ifndef ARIN32_BUTTON_HPP
#define ARIN32_BUTTON_HPP

#include "types.hpp"
#include "theme.hpp"
#include "input.hpp"
#include "renderer.hpp"
#include "widget.hpp"
#include "icon.hpp"
#include <string>
#include <functional>

namespace arin {

/**
 * @brief Current interactive state of a Button.
 */
enum class ButtonState : uint8_t {
    Normal,   ///< Idle, unhovered state
    Hovered,  ///< Mouse cursor is inside button bounds
    Pressed,  ///< Left mouse button is currently held down on the button
    Disabled  ///< Button is inactive and ignores all user inputs
};

/**
 * @brief Ultra-simple, high-performance interactive Push Button widget.
 *
 * Features:
 * - Fluid, absurdly simple builder-style API.
 * - Anti-aliased rounded corners and customizable styles (primary, success, danger, outline).
 * - Automatic centered typography with embedded zero-dependency font.
 * - Robust event state machine (hover, press, release, click dispatch).
 *
 * Example:
 * @code
 * auto btn = arin::Button("Click Me!", 100, 100, 200, 50);
 * btn.on_click([]() {
 *     std::cout << "Button was clicked!\n";
 * });
 * @endcode
 */
class Button : public IWidget {
public:
    using ClickCallback = std::function<void()>;
    using DetailedClickCallback = std::function<void(Button&)>;
    using HoverCallback = std::function<void(bool)>;

    /**
     * @brief Constructs an empty button at (0, 0, 85, 32).
     */
    Button();

    /**
     * @brief Constructs a button with a text label and default dimensions.
     * @param label Text to display on the button.
     */
    explicit Button(std::string label);

    /**
     * @brief Constructs a button with explicit position and size.
     * @param label Text to display.
     * @param x Top-left horizontal coordinate.
     * @param y Top-left vertical coordinate.
     * @param width Width in pixels.
     * @param height Height in pixels.
     */
    Button(std::string label, float x, float y, float width, float height);

    /**
     * @brief Constructs a button with explicit bounding box.
     * @param label Text to display.
     * @param bounds Bounding rectangle.
     */
    Button(std::string label, const Rect& bounds);

    /**
     * @brief Constructs a button with custom bounds and style preset.
     * @param label Text to display.
     * @param bounds Bounding rectangle.
     * @param style Button visual style preset.
     */
    Button(std::string label, const Rect& bounds, const ButtonStyle& style);

    virtual ~Button() = default;

    // --- Fluent Property Setters ---

    /**
     * @brief Sets the button's display text.
     * @param text New label string.
     * @return Reference to this for chaining.
     */
    Button& set_text(std::string text);

    /**
     * @brief Sets the top-left screen position.
     * @param x Horizontal pixel position.
     * @param y Vertical pixel position.
     * @return Reference to this for chaining.
     */
    Button& set_position(float x, float y) override;

    /**
     * @brief Sets the top-left position using a Vec2.
     * @param pos New position.
     * @return Reference to this for chaining.
     */
    Button& set_position(const Vec2& pos);

    /**
     * @brief Sets the dimensions of the button.
     * @param width Width in pixels.
     * @param height Height in pixels.
     * @return Reference to this for chaining.
     */
    Button& set_size(float width, float height) override;

    /**
     * @brief Sets the full bounding box of the button.
     * @param bounds Bounding rectangle.
     * @return Reference to this for chaining.
     */
    Button& set_bounds(const Rect& bounds) override;

    /**
     * @brief Convenience overload to set bounds with scalar coordinates.
     * @param x Left coordinate.
     * @param y Top coordinate.
     * @param width Width in pixels.
     * @param height Height in pixels.
     * @return Reference to this for chaining.
     */
    Button& set_bounds(float x, float y, float width, float height);

    /**
     * @brief Sets the visual style configuration.
     * @param style ButtonStyle specifying colors, border, radius, and fonts.
     * @return Reference to this for chaining.
     */
    Button& set_style(const ButtonStyle& style);

    /**
     * @brief Enables or disables the button.
     * @param enabled true to enable user interaction, false to disable.
     * @return Reference to this for chaining.
     */
    Button& set_enabled(bool enabled);

    /**
     * @brief Sets the corner radius of the button.
     * @param radius Curvature radius in pixels (0.0 = sharp rectangular).
     * @return Reference to this for chaining.
     */
    Button& set_corner_radius(float radius);

    /**
     * @brief Enables or disables automatic width/height expansion to fit content text.
     *
     * When enabled (default: true), the button will automatically expand its dimensions
     * to guarantee that text never overflows or touches borders.
     *
     * @param enable true to auto-expand to fit text, false to keep fixed bounds.
     * @return Reference to this for chaining.
     */
    Button& set_auto_resize(bool enable);

    /// @brief Checks if auto-resize is enabled.
    bool is_auto_resize() const { return m_auto_resize; }

    /**
     * @brief Attaches a crisp vector icon to the button, rendered preceding the text.
     * @param icon Vector glyph identifier.
     * @param size Uniform dimension of the icon in pixels (default: 16.0f).
     * @param spacing Pixel gap between icon and text (default: 6.0f).
     * @return Reference to this for chaining.
     */
    Button& set_icon(IconType icon, float size = 16.0f, float spacing = 6.0f);

    /// @brief Gets active icon type.
    IconType icon() const { return m_icon; }

    /// @brief Checks if a valid icon is attached to this button.
    bool has_icon() const { return m_icon != IconType::None; }

    /**
     * @brief Computes required width based on text length and horizontal padding.
     *
     * Automatically accounts for attached icon and icon-to-text spacing if present.
     *
     * @param font Font engine to measure text with.
     * @param horizontal_padding Padding on left and right sides.
     * @return Reference to this for chaining.
     */
    Button& fit_to_text(const Font& font, float horizontal_padding = 14.0f);

    // --- Event Callbacks ---

    /**
     * @brief Registers a simple parameterless click callback.
     *
     * Invoked when the user completes a click (mouse press AND release inside bounds).
     *
     * @param callback std::function to invoke.
     * @return Reference to this for chaining.
     */
    Button& on_click(ClickCallback callback);

    /**
     * @brief Registers a click callback that receives a reference to the button.
     *
     * Ideal when the callback needs to modify the button itself (e.g. change text or style).
     *
     * @param callback Callback taking (Button&).
     * @return Reference to this for chaining.
     */
    Button& on_click(DetailedClickCallback callback);

    /**
     * @brief Registers a callback triggered when cursor enters or leaves the button.
     * @param callback Callback receiving (bool is_hovered).
     * @return Reference to this for chaining.
     */
    Button& on_hover(HoverCallback callback);

    // --- State Queries ---

    /// @brief Gets the current text label.
    const std::string& text() const { return m_text; }

    /// @brief Gets the bounding box in screen pixels.
    const Rect& bounds() const override { return m_bounds; }

    /// @brief Gets the current interactive state.
    ButtonState state() const { return m_state; }

    /// @brief Checks if the cursor is currently over the button.
    bool is_hovered() const { return m_state == ButtonState::Hovered || m_state == ButtonState::Pressed; }

    /// @brief Checks if the button is currently held down.
    bool is_pressed() const { return m_state == ButtonState::Pressed; }

    /// @brief Checks if the button is active for user interactions.
    bool is_enabled() const override { return m_state != ButtonState::Disabled; }

    /// @brief Gets the active button style.
    const ButtonStyle& style() const { return m_style; }
    ButtonStyle& style() { return m_style; }

    // --- Processing & Rendering ---

    /**
     * @brief Injects a mouse event into the button state machine.
     *
     * Dispatches hover transitions and click callbacks accordingly.
     *
     * @param event The mouse event to evaluate.
     * @return true if the event was captured or consumed by this button, false otherwise.
     */
    bool handle_mouse(const MouseEvent& event) override;

    /**
     * @brief Renders the button onto the active screen frame.
     *
     * Automatically handles state-dependent colors (hover, pressed, disabled),
     * smooth anti-aliased corners, borders, and centered text label.
     *
     * @param renderer The 2D renderer to draw with.
     */
    void render(Renderer2D& renderer) override;

private:
    std::string m_text{"Button"};
    Rect m_bounds{0.0f, 0.0f, 85.0f, 32.0f};
    ButtonStyle m_style{ButtonStyle::primary()};
    ButtonState m_state{ButtonState::Normal};

    bool m_mouse_inside{false};
    bool m_pressed_inside{false};
    bool m_auto_resize{true};

    ClickCallback m_click_cb;
    DetailedClickCallback m_detailed_click_cb;
    HoverCallback m_hover_cb;

    IconType m_icon{IconType::None};
    float m_icon_size{16.0f};
    float m_icon_spacing{6.0f};
};

} // namespace arin

#endif // ARIN32_BUTTON_HPP
