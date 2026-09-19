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

    static MouseEvent make_move(const Vec2& pos) {
        MouseEvent ev;
        ev.type = MouseEventType::Move;
        ev.position = pos;
        return ev;
    }

    static MouseEvent make_button_down(const Vec2& pos, MouseButton btn = MouseButton::Left) {
        MouseEvent ev;
        ev.type = MouseEventType::ButtonDown;
        ev.position = pos;
        ev.button = btn;
        ev.action = InputAction::Press;
        return ev;
    }

    static MouseEvent make_button_up(const Vec2& pos, MouseButton btn = MouseButton::Left) {
        MouseEvent ev;
        ev.type = MouseEventType::ButtonUp;
        ev.position = pos;
        ev.button = btn;
        ev.action = InputAction::Release;
        return ev;
    }

    static MouseEvent make_scroll(const Vec2& pos, const Vec2& delta) {
        MouseEvent ev;
        ev.type = MouseEventType::Scroll;
        ev.position = pos;
        ev.scroll_delta = delta;
        return ev;
    }
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

/**
 * @brief Platform-agnostic keyboard scan codes and virtual keys.
 * Aligned with standard ASCII and desktop layout values.
 */
enum class KeyCode : uint16_t {
    Unknown = 0,
    Space = 32,
    Apostrophe = 39,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    Num0 = 48,
    Num1 = 49,
    Num2 = 50,
    Num3 = 51,
    Num4 = 52,
    Num5 = 53,
    Num6 = 54,
    Num7 = 55,
    Num8 = 56,
    Num9 = 57,
    Semicolon = 59,
    Equal = 61,
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    LeftBracket = 91,
    Backslash = 92,
    RightBracket = 93,
    GraveAccent = 96,
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347
};

/**
 * @brief Bitfield flags representing keyboard modifier keys.
 */
enum class KeyModifier : uint8_t {
    None    = 0,
    Shift   = 1 << 0,
    Control = 1 << 1,
    Alt     = 1 << 2,
    Super   = 1 << 3
};

inline KeyModifier operator|(KeyModifier a, KeyModifier b) {
    return static_cast<KeyModifier>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline bool operator&(KeyModifier a, KeyModifier b) {
    return (static_cast<uint8_t>(a) & static_cast<uint8_t>(b)) != 0;
}

/**
 * @brief Discrete keyboard event (key press, release, repeat).
 */
struct KeyEvent {
    KeyCode key{KeyCode::Unknown};
    InputAction action{InputAction::Press};
    uint8_t modifiers{0};

    bool has_shift() const { return (modifiers & static_cast<uint8_t>(KeyModifier::Shift)) != 0; }
    bool has_ctrl() const { return (modifiers & static_cast<uint8_t>(KeyModifier::Control)) != 0; }
    bool has_alt() const { return (modifiers & static_cast<uint8_t>(KeyModifier::Alt)) != 0; }
    bool has_super() const { return (modifiers & static_cast<uint8_t>(KeyModifier::Super)) != 0; }

    static KeyEvent make_press(KeyCode key, uint8_t mods = 0) {
        KeyEvent ev;
        ev.key = key;
        ev.action = InputAction::Press;
        ev.modifiers = mods;
        return ev;
    }

    static KeyEvent make_repeat(KeyCode key, uint8_t mods = 0) {
        KeyEvent ev;
        ev.key = key;
        ev.action = InputAction::Repeat;
        ev.modifiers = mods;
        return ev;
    }

    static KeyEvent make_release(KeyCode key, uint8_t mods = 0) {
        KeyEvent ev;
        ev.key = key;
        ev.action = InputAction::Release;
        ev.modifiers = mods;
        return ev;
    }
};

/**
 * @brief Character input event containing decoded Unicode codepoint and UTF-8 string.
 */
struct TextEvent {
    uint32_t codepoint{0};
    std::string text;

    static TextEvent from_char(char c) {
        TextEvent ev;
        ev.codepoint = static_cast<uint32_t>(c);
        ev.text = std::string(1, c);
        return ev;
    }

    static TextEvent from_codepoint(uint32_t cp) {
        TextEvent ev;
        ev.codepoint = cp;
        if (cp <= 0x7F) {
            ev.text += static_cast<char>(cp);
        } else if (cp <= 0x7FF) {
            ev.text += static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
            ev.text += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp <= 0xFFFF) {
            ev.text += static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
            ev.text += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            ev.text += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp <= 0x10FFFF) {
            ev.text += static_cast<char>(0xF0 | ((cp >> 18) & 0x07));
            ev.text += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            ev.text += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            ev.text += static_cast<char>(0x80 | (cp & 0x3F));
        }
        return ev;
    }
};

} // namespace arin

#endif // ARIN32_INPUT_HPP
