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

#ifndef ARIN32_TEXT_INPUT_HPP
#define ARIN32_TEXT_INPUT_HPP

#include "types.hpp"
#include "widget.hpp"
#include "renderer.hpp"
#include "input.hpp"
#include <string>
#include <functional>

namespace arin {

/**
 * @brief Visual styling configuration for the TextInput widget.
 */
struct TextInputStyle {
    float corner_radius{4.0f};                      ///< Corner radius of the input field in pixels
    float border_width{1.0f};                       ///< Standard border thickness
    float focus_border_width{2.0f};                 ///< Border thickness when keyboard focused
    float padding_x{10.0f};                         ///< Horizontal internal padding from box edge
    float text_scale{0.92f};                        ///< Typography scale factor
    float cursor_width{1.5f};                       ///< Width of the blinking insertion caret
    float blink_interval{0.5f};                     ///< Caret blink cycle duration in seconds

    Color background_color{Color::white()};         ///< Fill color when active and enabled
    Color disabled_background{Color::from_hex(0xF3F4F6)};///< Fill color when disabled
    Color border_color{Color::from_hex(0xD1D5DB)};  ///< Inactive subtle border color
    Color hover_border_color{Color::from_hex(0x9CA3AF)};///< Border color when cursor hovers
    Color focus_border_color{Color::from_hex(0x0067C0)};///< Windows 10 Accent Blue focus stroke
    Color text_color{Color::from_hex(0x111827)};    ///< Normal user input text color
    Color disabled_text_color{Color::from_hex(0x9CA3AF)};///< Text color when disabled
    Color placeholder_color{Color::from_hex(0x9CA3AF)};///< Hint text color when empty
    Color selection_color{Color::from_rgba8(0, 103, 192, 75)};///< Highlight fill behind selected range
    Color cursor_color{Color::from_hex(0x111827)};  ///< Blinking insertion bar color
};

/**
 * @brief Modern editable text field widget (TextBox / TextInput).
 *
 * Features:
 * - Real-time keyboard input routing with focus management
 * - Animated blinking caret with automatic blink timer reset on navigation/typing
 * - Cursor navigation via Left/Right arrows, Home, End, Backspace, and Delete
 * - Word navigation with Ctrl + Left / Right
 * - Character and text selection via mouse click-and-drag, Shift + navigation, or Ctrl+A
 * - Integrated clipboard cut/copy/paste (Ctrl+X, Ctrl+C, Ctrl+V)
 * - Automatic horizontal auto-scrolling guaranteeing cursor visibility
 * - Hardware scissor clipping (`glScissor`) ensuring text never bleeds outside bounds
 * - Placeholder support and optional password masking
 * - Event callbacks for value changes (`on_text_changed`) and submission (`on_submit`)
 */
class TextInput : public IWidget {
public:
    using TextChangeCallback = std::function<void(const std::string& text)>;
    using SubmitCallback = std::function<void(const std::string& text)>;
    using ClipboardGetCallback = std::function<std::string()>;
    using ClipboardSetCallback = std::function<void(const std::string& text)>;

    /**
     * @brief Constructs a TextInput widget.
     * @param initial_text Initial text string contained in the field.
     * @param x Top-left horizontal coordinate in pixels.
     * @param y Top-left vertical coordinate in pixels.
     * @param width Width in pixels.
     * @param height Height in pixels (default: 32.0f).
     */
    explicit TextInput(
        std::string initial_text = "",
        float x = 0.0f,
        float y = 0.0f,
        float width = 200.0f,
        float height = 32.0f
    );

    ~TextInput() override = default;

    // --- IWidget Interface Implementation ---

    const Rect& bounds() const override { return m_bounds; }

    TextInput& set_bounds(const Rect& bounds) override {
        m_bounds = bounds;
        return *this;
    }

    TextInput& set_position(float x, float y) override {
        m_bounds.x = x;
        m_bounds.y = y;
        return *this;
    }

    TextInput& set_size(float width, float height) override {
        m_bounds.width = width;
        m_bounds.height = height;
        return *this;
    }

    bool handle_mouse(const MouseEvent& ev) override;
    bool handle_key(const KeyEvent& ev) override;
    bool handle_text(const TextEvent& ev) override;
    void on_focus(bool focused) override;
    bool is_focusable() const override { return m_enabled && m_visible; }
    void update(float dt) override;
    void render(Renderer2D& renderer) override;
    bool is_visible() const override { return m_visible; }
    bool is_enabled() const override { return m_enabled; }

    // --- TextInput Specific Methods & Chaining ---

    /// @brief Gets the current text content.
    const std::string& text() const { return m_text; }

    /// @brief Sets text content and repositions cursor to the end.
    TextInput& set_text(std::string text);

    /// @brief Gets placeholder text displayed when empty.
    const std::string& placeholder() const { return m_placeholder; }

    /// @brief Sets placeholder text displayed when empty.
    TextInput& set_placeholder(std::string placeholder) {
        m_placeholder = std::move(placeholder);
        return *this;
    }

    /// @brief Gets current insertion cursor index [0, text.size()].
    size_t cursor_position() const { return m_cursor_pos; }

    /// @brief Moves insertion cursor to the specified index.
    TextInput& set_cursor_position(size_t pos);

    /// @brief Checks if there is an active text selection.
    bool has_selection() const { return m_has_selection && m_sel_start != m_sel_end; }

    /// @brief Returns the selected substring.
    std::string selected_text() const;

    /// @brief Selects all text inside the input.
    TextInput& select_all();

    /// @brief Clears active selection range.
    TextInput& clear_selection();

    /// @brief Deletes currently selected text and collapses cursor.
    bool delete_selection();

    /// @brief Inserts text at the current cursor position (replacing selection if active).
    TextInput& insert_text(const std::string& str);

    /// @brief Checks whether the widget is read-only.
    bool is_read_only() const { return m_read_only; }

    /// @brief Sets read-only state.
    TextInput& set_read_only(bool read_only) {
        m_read_only = read_only;
        return *this;
    }

    /// @brief Checks whether password masking is enabled.
    bool is_password() const { return m_is_password; }

    /// @brief Configures password masking mode.
    TextInput& set_password(bool password, char mask = '*') {
        m_is_password = password;
        m_password_mask = mask;
        return *this;
    }

    /// @brief Checks whether the widget currently possesses keyboard focus.
    bool is_focused() const { return m_focused; }

    /// @brief Sets keyboard focus state.
    TextInput& set_focused(bool focused);

    /// @brief Sets widget visibility.
    TextInput& set_visible(bool visible) {
        m_visible = visible;
        return *this;
    }

    /// @brief Sets enabled state.
    TextInput& set_enabled(bool enabled) {
        m_enabled = enabled;
        return *this;
    }

    /// @brief Gets active visual styling configuration.
    const TextInputStyle& style() const { return m_style; }

    /// @brief Sets active visual styling configuration.
    TextInput& set_style(const TextInputStyle& style) {
        m_style = style;
        return *this;
    }

    /// @brief Registers callback invoked whenever the text changes.
    TextInput& on_text_changed(TextChangeCallback cb) {
        m_change_cb = std::move(cb);
        return *this;
    }

    /// @brief Registers callback invoked when Enter is pressed.
    TextInput& on_submit(SubmitCallback cb) {
        m_submit_cb = std::move(cb);
        return *this;
    }

    /// @brief Sets external clipboard get/set handlers (e.g. from Window).
    TextInput& set_clipboard_provider(ClipboardGetCallback get_cb, ClipboardSetCallback set_cb) {
        m_clip_get_cb = std::move(get_cb);
        m_clip_set_cb = std::move(set_cb);
        return *this;
    }

private:
    void ensure_cursor_visible(const Font& font);
    size_t index_from_x_offset(float local_x, const Font& font) const;
    float x_offset_from_index(size_t index, const Font& font) const;
    std::string get_display_text() const;
    void reset_blink();

    std::string m_text;
    std::string m_placeholder{"Enter text..."};
    Rect m_bounds{0.0f, 0.0f, 200.0f, 32.0f};

    size_t m_cursor_pos{0};
    size_t m_sel_start{0};
    size_t m_sel_end{0};
    bool m_has_selection{false};

    float m_scroll_offset{0.0f};
    float m_blink_timer{0.0f};
    bool m_cursor_visible{true};

    bool m_focused{false};
    bool m_hovered{false};
    bool m_pressed{false};
    bool m_enabled{true};
    bool m_visible{true};
    bool m_read_only{false};
    bool m_is_password{false};
    char m_password_mask{'*'};

    TextInputStyle m_style;
    TextChangeCallback m_change_cb;
    SubmitCallback m_submit_cb;
    ClipboardGetCallback m_clip_get_cb;
    ClipboardSetCallback m_clip_set_cb;

    static std::string s_internal_clipboard;
};

/// @brief Convenient alias for TextInput
using TextBox = TextInput;

} // namespace arin

#endif // ARIN32_TEXT_INPUT_HPP
