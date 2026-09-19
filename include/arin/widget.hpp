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

#ifndef ARIN32_WIDGET_HPP
#define ARIN32_WIDGET_HPP

#include <memory>
#include "types.hpp"
#include "input.hpp"
#include "renderer.hpp"

namespace arin {

class ContextMenu;

/**
 * @brief Abstract base interface for all visual and interactive user interface elements in Arin32.
 *
 * Unifies buttons, progress bars, list boxes, and layout containers under a polymorphic interface
 * enabling automatic nesting, layout calculation, mouse event dispatching, and frame rendering.
 */
class IWidget {
public:
    virtual ~IWidget() = default;

    /// @brief Gets the widget's bounding rectangle in screen pixels.
    virtual const Rect& bounds() const = 0;

    /// @brief Sets the widget's bounding rectangle.
    virtual IWidget& set_bounds(const Rect& bounds) = 0;

    /// @brief Sets widget position (x, y).
    virtual IWidget& set_position(float x, float y) = 0;

    /// @brief Sets widget dimensions (width, height).
    virtual IWidget& set_size(float width, float height) = 0;

    /**
     * @brief Processes a mouse interaction event.
     * @param ev Mouse event data (move, press, release, scroll).
     * @return true if the event was captured and consumed by this widget, false otherwise.
     */
    virtual bool handle_mouse(const MouseEvent& ev) {
        (void)ev;
        return false;
    }

    /**
     * @brief Processes a keyboard key press, repeat, or release event.
     * @param ev Key event data (key code, action, modifiers).
     * @return true if consumed, false otherwise.
     */
    virtual bool handle_key(const KeyEvent& ev) {
        (void)ev;
        return false;
    }

    /**
     * @brief Processes a text/character input event.
     * @param ev Text event data (Unicode codepoint and UTF-8 string).
     * @return true if consumed, false otherwise.
     */
    virtual bool handle_text(const TextEvent& ev) {
        (void)ev;
        return false;
    }

    /**
     * @brief Invoked when this widget gains or loses keyboard input focus.
     * @param focused true if focused, false if blurred.
     */
    virtual void on_focus(bool focused) {
        (void)focused;
    }

    /**
     * @brief Indicates whether this widget can accept keyboard input focus.
     */
    virtual bool is_focusable() const {
        return false;
    }

    /**
     * @brief Advances time-dependent animations (e.g. shimmer sweeps, caret blinking).
     * @param dt Elapsed delta time in seconds.
     */
    virtual void update(float dt) {
        (void)dt;
    }

    /**
     * @brief Renders the widget using the 2D graphics engine.
     * @param renderer The 2D OpenGL renderer.
     */
    virtual void render(Renderer2D& renderer) = 0;

    /// @brief Returns true if widget is visible.
    virtual bool is_visible() const { return true; }

    /// @brief Returns true if widget is interactive.
    virtual bool is_enabled() const { return true; }

    /// @brief Sets a dedicated context menu for this specific widget.
    virtual IWidget& set_context_menu(std::shared_ptr<ContextMenu> menu) {
        m_context_menu = std::move(menu);
        return *this;
    }

    /// @brief Gets the dedicated context menu attached to this widget, or nullptr.
    virtual std::shared_ptr<ContextMenu> context_menu() const {
        return m_context_menu;
    }

protected:
    std::shared_ptr<ContextMenu> m_context_menu{nullptr};
};

} // namespace arin

#endif // ARIN32_WIDGET_HPP
