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

#include "arin/text_input.hpp"
#include <algorithm>

namespace arin {

std::string TextInput::s_internal_clipboard = "";

TextInput::TextInput(
    std::string initial_text,
    float x,
    float y,
    float width,
    float height
) : m_text(std::move(initial_text)),
    m_bounds(x, y, width, height),
    m_cursor_pos(m_text.size()) {}

TextInput& TextInput::set_text(std::string text) {
    if (m_text != text) {
        m_text = std::move(text);
        m_cursor_pos = std::min(m_cursor_pos, m_text.size());
        clear_selection();
        reset_blink();
        if (m_change_cb) {
            m_change_cb(m_text);
        }
    }
    return *this;
}

TextInput& TextInput::set_cursor_position(size_t pos) {
    m_cursor_pos = std::min(pos, m_text.size());
    reset_blink();
    return *this;
}

TextInput& TextInput::set_focused(bool focused) {
    if (m_focused != focused) {
        on_focus(focused);
    }
    return *this;
}

void TextInput::on_focus(bool focused) {
    m_focused = focused;
    reset_blink();
    if (!focused) {
        clear_selection();
    }
}

/**
 * @brief Returns the padded inner rectangle where text is drawn and clipped.
 */
Rect TextInput::inner_rect() const {
    // Step 1: Remove horizontal padding from both sides.
    const float inner_x = m_bounds.x + m_style.padding_x;
    const float inner_w = std::max(0.0f, m_bounds.width - 2.0f * m_style.padding_x);
    // Step 2: Keep a minimal vertical inset so text never touches the border.
    constexpr float kVerticalInset = 1.0f;
    return Rect(inner_x, m_bounds.y + kVerticalInset, inner_w, std::max(0.0f, m_bounds.height - 2.0f * kVerticalInset));
}

void TextInput::reset_blink() {
    m_blink_timer = 0.0f;
    m_cursor_visible = true;
}

std::string TextInput::get_display_text() const {
    if (m_is_password) {
        return std::string(m_text.size(), m_password_mask);
    }
    return m_text;
}

float TextInput::x_offset_from_index(size_t index, const Font& font) const {
    std::string disp = get_display_text();
    size_t limit = std::min(index, disp.size());
    float x = 0.0f;
    for (size_t i = 0; i < limit; ++i) {
        x += font.get_glyph(disp[i]).advance * m_style.text_scale;
    }
    return x;
}

size_t TextInput::index_from_x_offset(float local_x, const Font& font) const {
    // Step 1: Walk glyph advances and snap to the nearest glyph boundary.
    // A click in the left half of a glyph selects its start, otherwise its end.
    const std::string disp = get_display_text();
    float current_x = 0.0f;
    for (size_t i = 0; i < disp.size(); ++i) {
        const float adv = font.get_glyph(disp[i]).advance * m_style.text_scale;
        if (local_x < current_x + adv * detail::kHalf) {
            return i;
        }
        current_x += adv;
    }
    return disp.size();
}

void TextInput::ensure_cursor_visible(const Font& font) {
    float inner_width = std::max(0.0f, m_bounds.width - 2.0f * m_style.padding_x);
    float cursor_x = x_offset_from_index(m_cursor_pos, font);

    // Scroll right if cursor moves past visible boundary
    if (cursor_x - m_scroll_offset > inner_width - 8.0f) {
        m_scroll_offset = cursor_x - inner_width + 16.0f;
    }

    // Scroll left if cursor moves before visible boundary
    if (cursor_x - m_scroll_offset < 8.0f) {
        m_scroll_offset = std::max(0.0f, cursor_x - 16.0f);
    }

    float total_w = x_offset_from_index(m_text.size(), font);
    if (total_w <= inner_width) {
        m_scroll_offset = 0.0f;
    } else {
        float max_scroll = total_w - inner_width + 16.0f;
        m_scroll_offset = std::max(0.0f, std::min(m_scroll_offset, max_scroll));
    }
}

TextInput& TextInput::select_all() {
    m_sel_start = 0;
    m_sel_end = m_text.size();
    m_has_selection = (m_sel_start != m_sel_end);
    m_cursor_pos = m_sel_end;
    reset_blink();
    return *this;
}

TextInput& TextInput::clear_selection() {
    m_sel_start = 0;
    m_sel_end = 0;
    m_has_selection = false;
    return *this;
}

std::string TextInput::selected_text() const {
    if (!has_selection()) return "";
    size_t s1 = std::min(m_sel_start, m_sel_end);
    size_t s2 = std::max(m_sel_start, m_sel_end);
    return m_text.substr(s1, s2 - s1);
}

bool TextInput::delete_selection() {
    if (!has_selection()) return false;
    size_t s1 = std::min(m_sel_start, m_sel_end);
    size_t s2 = std::max(m_sel_start, m_sel_end);
    m_text.erase(s1, s2 - s1);
    m_cursor_pos = s1;
    clear_selection();
    reset_blink();
    if (m_change_cb) {
        m_change_cb(m_text);
    }
    return true;
}

TextInput& TextInput::insert_text(const std::string& str) {
    if (m_read_only || str.empty()) return *this;

    delete_selection();
    m_text.insert(m_cursor_pos, str);
    m_cursor_pos += str.size();
    reset_blink();

    if (m_change_cb) {
        m_change_cb(m_text);
    }
    return *this;
}

bool TextInput::handle_mouse(const MouseEvent& ev) {
    if (!m_enabled || !m_visible) {
        m_hovered = false;
        m_pressed = false;
        return false;
    }

    bool inside = m_bounds.contains(ev.position);

    Font temp_font; // In Arin32 font metrics are embedded and statically available

    switch (ev.type) {
        case MouseEventType::Move: {
            m_hovered = inside;
            if (m_pressed) {
                float local_x = ev.position.x - (m_bounds.x + m_style.padding_x) + m_scroll_offset;
                size_t idx = index_from_x_offset(local_x, temp_font);
                m_cursor_pos = idx;
                m_sel_end = idx;
                m_has_selection = (m_sel_start != m_sel_end);
                reset_blink();
                return true;
            }
            return inside;
        }

        case MouseEventType::ButtonDown: {
            if (ev.button == MouseButton::Left && inside) {
                m_pressed = true;
                set_focused(true);
                float local_x = ev.position.x - (m_bounds.x + m_style.padding_x) + m_scroll_offset;
                size_t idx = index_from_x_offset(local_x, temp_font);
                m_cursor_pos = idx;
                m_sel_start = idx;
                m_sel_end = idx;
                m_has_selection = false;
                reset_blink();
                return true;
            }
            break;
        }

        case MouseEventType::ButtonUp: {
            if (ev.button == MouseButton::Left && m_pressed) {
                m_pressed = false;
                return true;
            }
            break;
        }

        default:
            break;
    }

    return false;
}

static size_t find_word_left(const std::string& str, size_t pos) {
    if (pos == 0) return 0;
    size_t i = pos - 1;
    while (i > 0 && str[i] == ' ') --i;
    while (i > 0 && str[i - 1] != ' ') --i;
    return i;
}

static size_t find_word_right(const std::string& str, size_t pos) {
    size_t n = str.size();
    if (pos >= n) return n;
    size_t i = pos;
    while (i < n && str[i] != ' ') ++i;
    while (i < n && str[i] == ' ') ++i;
    return i;
}

bool TextInput::handle_key(const KeyEvent& ev) {
    if (!m_focused || !m_enabled || !m_visible) return false;
    if (ev.action == InputAction::Release) return false;

    // --- Clipboard & Selection Shortcuts ---
    if (ev.has_ctrl()) {
        if (ev.key == KeyCode::A) {
            select_all();
            return true;
        }
        if (ev.key == KeyCode::C) {
            if (has_selection()) {
                std::string sel = selected_text();
                if (m_clip_set_cb) m_clip_set_cb(sel);
                else s_internal_clipboard = sel;
            }
            return true;
        }
        if (ev.key == KeyCode::X) {
            if (!m_read_only && has_selection()) {
                std::string sel = selected_text();
                if (m_clip_set_cb) m_clip_set_cb(sel);
                else s_internal_clipboard = sel;
                delete_selection();
            }
            return true;
        }
        if (ev.key == KeyCode::V) {
            if (!m_read_only) {
                std::string clip = (m_clip_get_cb ? m_clip_get_cb() : s_internal_clipboard);
                std::string sanitized;
                for (char c : clip) {
                    if (c >= 32 && c <= 126) sanitized += c;
                }
                if (!sanitized.empty()) {
                    insert_text(sanitized);
                }
            }
            return true;
        }
    }

    // --- Navigation & Deletion Keys ---
    switch (ev.key) {
        case KeyCode::Left: {
            if (ev.has_shift()) {
                if (!m_has_selection) {
                    m_sel_start = m_cursor_pos;
                    m_has_selection = true;
                }
                if (ev.has_ctrl()) {
                    m_cursor_pos = find_word_left(m_text, m_cursor_pos);
                } else if (m_cursor_pos > 0) {
                    m_cursor_pos--;
                }
                m_sel_end = m_cursor_pos;
                m_has_selection = (m_sel_start != m_sel_end);
            } else {
                if (has_selection()) {
                    m_cursor_pos = std::min(m_sel_start, m_sel_end);
                    clear_selection();
                } else {
                    if (ev.has_ctrl()) {
                        m_cursor_pos = find_word_left(m_text, m_cursor_pos);
                    } else if (m_cursor_pos > 0) {
                        m_cursor_pos--;
                    }
                }
            }
            reset_blink();
            return true;
        }

        case KeyCode::Right: {
            if (ev.has_shift()) {
                if (!m_has_selection) {
                    m_sel_start = m_cursor_pos;
                    m_has_selection = true;
                }
                if (ev.has_ctrl()) {
                    m_cursor_pos = find_word_right(m_text, m_cursor_pos);
                } else if (m_cursor_pos < m_text.size()) {
                    m_cursor_pos++;
                }
                m_sel_end = m_cursor_pos;
                m_has_selection = (m_sel_start != m_sel_end);
            } else {
                if (has_selection()) {
                    m_cursor_pos = std::max(m_sel_start, m_sel_end);
                    clear_selection();
                } else {
                    if (ev.has_ctrl()) {
                        m_cursor_pos = find_word_right(m_text, m_cursor_pos);
                    } else if (m_cursor_pos < m_text.size()) {
                        m_cursor_pos++;
                    }
                }
            }
            reset_blink();
            return true;
        }

        case KeyCode::Home: {
            if (ev.has_shift()) {
                if (!m_has_selection) {
                    m_sel_start = m_cursor_pos;
                    m_has_selection = true;
                }
                m_cursor_pos = 0;
                m_sel_end = 0;
                m_has_selection = (m_sel_start != m_sel_end);
            } else {
                clear_selection();
                m_cursor_pos = 0;
            }
            reset_blink();
            return true;
        }

        case KeyCode::End: {
            if (ev.has_shift()) {
                if (!m_has_selection) {
                    m_sel_start = m_cursor_pos;
                    m_has_selection = true;
                }
                m_cursor_pos = m_text.size();
                m_sel_end = m_cursor_pos;
                m_has_selection = (m_sel_start != m_sel_end);
            } else {
                clear_selection();
                m_cursor_pos = m_text.size();
            }
            reset_blink();
            return true;
        }

        case KeyCode::Backspace: {
            if (m_read_only) return true;
            if (has_selection()) {
                delete_selection();
            } else if (m_cursor_pos > 0) {
                m_text.erase(m_cursor_pos - 1, 1);
                m_cursor_pos--;
                reset_blink();
                if (m_change_cb) {
                    m_change_cb(m_text);
                }
            }
            return true;
        }

        case KeyCode::Delete: {
            if (m_read_only) return true;
            if (has_selection()) {
                delete_selection();
            } else if (m_cursor_pos < m_text.size()) {
                m_text.erase(m_cursor_pos, 1);
                reset_blink();
                if (m_change_cb) {
                    m_change_cb(m_text);
                }
            }
            return true;
        }

        case KeyCode::Enter: {
            if (m_submit_cb) {
                m_submit_cb(m_text);
            }
            return true;
        }

        case KeyCode::Escape: {
            set_focused(false);
            return true;
        }

        default:
            break;
    }

    return false;
}

bool TextInput::handle_text(const TextEvent& ev) {
    if (!m_focused || !m_enabled || !m_visible || m_read_only) return false;

    // Filter printable ASCII characters
    std::string printable;
    for (char c : ev.text) {
        if (c >= 32 && c <= 126) {
            printable += c;
        }
    }

    if (!printable.empty()) {
        insert_text(printable);
        return true;
    }

    return false;
}

void TextInput::update(float dt) {
    if (!m_focused) {
        m_cursor_visible = false;
        return;
    }

    m_blink_timer += dt;
    if (m_blink_timer >= m_style.blink_interval) {
        m_blink_timer -= m_style.blink_interval;
        m_cursor_visible = !m_cursor_visible;
    }
}

void TextInput::render(Renderer2D& renderer) {
    if (!m_visible || m_bounds.width <= 0.0f || m_bounds.height <= 0.0f) {
        return;
    }

    // 1. Determine visual state colors
    Color bg = m_enabled ? m_style.background_color : m_style.disabled_background;
    Color border;
    float b_width = m_style.border_width;

    if (!m_enabled) {
        border = m_style.border_color;
    } else if (m_focused) {
        border = m_style.focus_border_color;
        b_width = m_style.focus_border_width;
    } else if (m_hovered) {
        border = m_style.hover_border_color;
    } else {
        border = m_style.border_color;
    }

    // 2. Draw outer background container
    renderer.draw_rounded_rect(
        m_bounds,
        m_style.corner_radius,
        bg,
        border,
        b_width
    );

    // Step 3: Compute the padded inner rectangle and clip text to it.
    const Rect text_clip_rect = inner_rect();
    const float inner_x = text_clip_rect.x;
    renderer.push_clip_rect(text_clip_rect);

    // 4. Ensure scrolling accommodates cursor position
    ensure_cursor_visible(renderer.font());

    const float origin_x = inner_x - m_scroll_offset;
    const float font_h = renderer.font().line_height() * m_style.text_scale;
    // Step 4: Center the visible glyph mass with the shared font helper.
    const Vec2 text_origin = renderer.font().layout_text_in_rect("Ag", m_bounds, m_style.text_scale, TextAlignH::Left, TextAlignV::Center);
    const float origin_y = text_origin.y;

    // 5. Draw selection highlight box behind characters
    if (has_selection()) {
        size_t s1 = std::min(m_sel_start, m_sel_end);
        size_t s2 = std::max(m_sel_start, m_sel_end);

        float sel_x1 = origin_x + x_offset_from_index(s1, renderer.font());
        float sel_x2 = origin_x + x_offset_from_index(s2, renderer.font());
        constexpr float kSelectionInset = 4.0f;
        constexpr float kMinimumSelectionWidth = 2.0f;
        Rect sel_rect(sel_x1, m_bounds.y + kSelectionInset, std::max(kMinimumSelectionWidth, sel_x2 - sel_x1), m_bounds.height - 2.0f * kSelectionInset);

        renderer.draw_rounded_rect(sel_rect, 2.0f, m_style.selection_color);
    }

    // 6. Draw placeholder or text string
    if (m_text.empty()) {
        renderer.draw_text(
            m_placeholder,
            Vec2(origin_x, origin_y),
            m_style.placeholder_color,
            m_style.text_scale
        );
    } else {
        std::string disp = get_display_text();
        Color text_col = m_enabled ? m_style.text_color : m_style.disabled_text_color;
        renderer.draw_text(
            disp,
            Vec2(origin_x, origin_y),
            text_col,
            m_style.text_scale
        );
    }

    // 7. Draw blinking insertion cursor
    if (m_focused && m_cursor_visible && m_enabled) {
        float cur_x = origin_x + x_offset_from_index(m_cursor_pos, renderer.font());
        Rect cur_rect(cur_x, origin_y, m_style.cursor_width, font_h);
        renderer.draw_rect(cur_rect, m_style.cursor_color);
    }

    // 8. Restore clipping stack
    renderer.pop_clip_rect();
}

} // namespace arin
