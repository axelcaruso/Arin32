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

#ifndef ARIN32_INPUT_HPP
#define ARIN32_INPUT_HPP

#include "types.hpp"
#include <array>

namespace arin {

/**
 * @brief Identifies mouse buttons.
 */
enum class MouseButton : uint8_t {
    Left = 0,   ///< Primary mouse button (usually left)
    Right = 1,  ///< Secondary mouse button (usually right)
    Middle = 2, ///< Scroll wheel button
    Count = 3
};

/**
 * @brief Identifies button or key actions.
 */
enum class InputAction : uint8_t {
    Release = 0, ///< Button was released
    Press = 1,   ///< Button was pressed down
    Repeat = 2   ///< Button is held down (key repeat)
};

/**
 * @brief Type classification of mouse-related events.
 */
enum class MouseEventType : uint8_t {
    Move,       ///< Cursor position changed
    ButtonDown, ///< Mouse button pressed
    ButtonUp,   ///< Mouse button released
    Scroll      ///< Wheel scrolled
};

/**
 * @brief Represents a mouse event dispatched by the platform or window backend.
 */
struct MouseEvent {
    MouseEventType type{MouseEventType::Move}; ///< Specific event category
    Vec2 position{0.0f, 0.0f};                 ///< Cursor position in window coordinates
    MouseButton button{MouseButton::Left};     ///< Affected button (if ButtonDown/ButtonUp)
    InputAction action{InputAction::Release};  ///< Press or Release
    Vec2 scroll_delta{0.0f, 0.0f};             ///< Scroll offset (x=horizontal, y=vertical)
};

/**
 * @brief Tracks the current state of user inputs (cursor, pressed buttons).
 *
 * Completely platform-independent, enabling easy event simulation for unit tests
 * or injection from custom operating system drivers.
 */
class InputState {
public:
    InputState() = default;

    /// @brief Gets the current mouse cursor coordinates.
    Vec2 mouse_pos() const { return m_cursor_pos; }

    /// @brief Checks if a specific mouse button is currently held down.
    bool is_mouse_down(MouseButton button) const {
        auto idx = static_cast<size_t>(button);
        return idx < m_mouse_buttons.size() && m_mouse_buttons[idx];
    }

    /// @brief Updates cursor position and returns a move event.
    MouseEvent on_mouse_move(float x, float y) {
        m_cursor_pos = Vec2(x, y);
        MouseEvent ev;
        ev.type = MouseEventType::Move;
        ev.position = m_cursor_pos;
        return ev;
    }

    /// @brief Updates mouse button state and returns a button event.
    MouseEvent on_mouse_button(MouseButton button, InputAction action) {
        auto idx = static_cast<size_t>(button);
        if (idx < m_mouse_buttons.size()) {
            m_mouse_buttons[idx] = (action == InputAction::Press);
        }
        MouseEvent ev;
        ev.type = (action == InputAction::Press) ? MouseEventType::ButtonDown : MouseEventType::ButtonUp;
        ev.position = m_cursor_pos;
        ev.button = button;
        ev.action = action;
        return ev;
    }

    /// @brief Generates a scroll event.
    MouseEvent on_mouse_scroll(float dx, float dy) {
        MouseEvent ev;
        ev.type = MouseEventType::Scroll;
        ev.position = m_cursor_pos;
        ev.scroll_delta = Vec2(dx, dy);
        return ev;
    }

private:
    Vec2 m_cursor_pos{0.0f, 0.0f};
    std::array<bool, static_cast<size_t>(MouseButton::Count)> m_mouse_buttons{false, false, false};
};

} // namespace arin

#endif // ARIN32_INPUT_HPP
