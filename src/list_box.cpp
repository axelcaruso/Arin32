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

#include "arin/list_box.hpp"
#include <GL/glew.h>
#include <cmath>
#include <algorithm>

namespace arin {

static const ListBoxItem s_empty_item;

ListBox::ListBox() = default;

ListBox::ListBox(float x, float y, float width, float height, ListBoxMode mode)
    : m_bounds(x, y, width, height), m_mode(mode) {}

ListBox::ListBox(const Rect& bounds, ListBoxMode mode)
    : m_bounds(bounds), m_mode(mode) {}

ListBox& ListBox::set_bounds(const Rect& bounds) {
    m_bounds = bounds;
    clamp_scroll();
    return *this;
}

ListBox& ListBox::set_bounds(float x, float y, float width, float height) {
    return set_bounds(Rect(x, y, width, height));
}

ListBox& ListBox::set_position(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
    return *this;
}

ListBox& ListBox::set_size(float width, float height) {
    m_bounds.width = width;
    m_bounds.height = height;
    clamp_scroll();
    return *this;
}

ListBox& ListBox::set_item_height(float height) {
    m_item_height = std::max(16.0f, height);
    clamp_scroll();
    return *this;
}

ListBox& ListBox::set_mode(ListBoxMode mode) {
    m_mode = mode;
    return *this;
}

ListBox& ListBox::set_checkbox_mode(bool enable) {
    m_mode = enable ? ListBoxMode::CheckBox : ListBoxMode::Standard;
    return *this;
}

ListBox& ListBox::add_item(const std::string& text, bool checked) {
    m_items.emplace_back(text, checked);
    clamp_scroll();
    return *this;
}

ListBox& ListBox::add_items(const std::vector<std::string>& items) {
    for (const auto& it : items) {
        m_items.emplace_back(it, false);
    }
    clamp_scroll();
    return *this;
}

ListBox& ListBox::set_items(const std::vector<std::string>& items) {
    clear_items();
    return add_items(items);
}

ListBox& ListBox::clear_items() {
    m_items.clear();
    m_selected_index = -1;
    m_hovered_index = -1;
    m_scroll_offset = 0.0f;
    return *this;
}

const ListBoxItem& ListBox::item(size_t index) const {
    if (index < m_items.size()) {
        return m_items[index];
    }
    return s_empty_item;
}

const std::string& ListBox::item_text(size_t index) const {
    if (index < m_items.size()) {
        return m_items[index].text;
    }
    static const std::string s_empty_str;
    return s_empty_str;
}

std::string ListBox::selected_item() const {
    if (m_selected_index >= 0 && static_cast<size_t>(m_selected_index) < m_items.size()) {
        return m_items[static_cast<size_t>(m_selected_index)].text;
    }
    return "";
}

ListBox& ListBox::set_selected_index(int index) {
    if (index >= -1 && index < static_cast<int>(m_items.size())) {
        m_selected_index = index;
        if (m_selection_cb && m_selected_index >= 0) {
            m_selection_cb(m_selected_index, m_items[static_cast<size_t>(m_selected_index)].text);
        }
    }
    return *this;
}

bool ListBox::is_checked(size_t index) const {
    if (index < m_items.size()) {
        return m_items[index].checked;
    }
    return false;
}

ListBox& ListBox::set_checked(size_t index, bool checked) {
    if (index < m_items.size()) {
        if (m_items[index].checked != checked) {
            m_items[index].checked = checked;
            if (m_toggle_cb) {
                m_toggle_cb(static_cast<int>(index), checked);
            }
        }
    }
    return *this;
}

ListBox& ListBox::toggle_checked(size_t index) {
    if (index < m_items.size()) {
        set_checked(index, !m_items[index].checked);
    }
    return *this;
}

std::vector<int> ListBox::checked_indices() const {
    std::vector<int> result;
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].checked) {
            result.push_back(static_cast<int>(i));
        }
    }
    return result;
}

std::vector<std::string> ListBox::checked_items() const {
    std::vector<std::string> result;
    for (const auto& item : m_items) {
        if (item.checked) {
            result.push_back(item.text);
        }
    }
    return result;
}

ListBox& ListBox::on_selection_changed(SelectionCallback cb) {
    m_selection_cb = std::move(cb);
    return *this;
}

ListBox& ListBox::on_item_toggled(ToggleCallback cb) {
    m_toggle_cb = std::move(cb);
    return *this;
}

bool ListBox::needs_scrollbar() const {
    return (m_items.size() * m_item_height) > (m_bounds.height - 2.0f);
}

float ListBox::max_scroll_offset() const {
    float total_h = m_items.size() * m_item_height;
    return std::max(0.0f, total_h - (m_bounds.height - 4.0f));
}

void ListBox::clamp_scroll() {
    float max_s = max_scroll_offset();
    m_scroll_offset = std::clamp(m_scroll_offset, 0.0f, max_s);
}

Rect ListBox::scrollbar_track_rect() const {
    const float track_w = 7.0f;
    const float margin_top = 6.0f;
    const float margin_bottom = 6.0f;
    const float margin_right = 5.0f;
    return Rect(
        m_bounds.x + m_bounds.width - track_w - margin_right,
        m_bounds.y + margin_top,
        track_w,
        std::max(0.0f, m_bounds.height - margin_top - margin_bottom)
    );
}

Rect ListBox::scrollbar_thumb_rect() const {
    Rect track = scrollbar_track_rect();
    float total_h = m_items.size() * m_item_height;
    if (total_h <= 0.0f) return track;

    float ratio = std::min(1.0f, track.height / total_h);
    float thumb_h = std::max(24.0f, track.height * ratio);
    float scrollable_track = track.height - thumb_h;
    float max_s = max_scroll_offset();
    float thumb_y = track.y + (max_s > 0.0f ? (m_scroll_offset / max_s) * scrollable_track : 0.0f);

    return Rect(track.x, thumb_y, track.width, thumb_h);
}

bool ListBox::handle_mouse(const MouseEvent& ev) {
    // 1. Mouse wheel scrolling
    if (ev.type == MouseEventType::Scroll) {
        if (m_bounds.contains(ev.position) && needs_scrollbar()) {
            m_scroll_offset -= ev.scroll_delta.y * (m_item_height * 1.5f);
            clamp_scroll();
            return true;
        }
        return false;
    }

    bool inside = m_bounds.contains(ev.position);

    // 2. Button down
    if (ev.type == MouseEventType::ButtonDown && ev.button == MouseButton::Left) {
        if (!inside) {
            m_hovered_index = -1;
            return false;
        }

        // Check if clicked inside scrollbar (using expanded hit-box for easy grabbing)
        Rect hit_track = scrollbar_track_rect().expanded(3.0f);
        if (needs_scrollbar() && hit_track.contains(ev.position)) {
            Rect thumb = scrollbar_thumb_rect().expanded(2.0f);
            if (thumb.contains(ev.position)) {
                m_dragging_scrollbar = true;
                m_drag_start_y = ev.position.y;
                m_drag_start_scroll = m_scroll_offset;
            } else {
                // Clicked in track: jump scroll towards click
                if (ev.position.y < thumb.y) {
                    m_scroll_offset -= m_item_height * 3.0f;
                } else {
                    m_scroll_offset += m_item_height * 3.0f;
                }
                clamp_scroll();
            }
            return true;
        }

        // Clicked on item row
        float local_y = (ev.position.y - m_bounds.y - 1.0f) + m_scroll_offset;
        int clicked_idx = static_cast<int>(std::floor(local_y / m_item_height));

        if (clicked_idx >= 0 && clicked_idx < static_cast<int>(m_items.size())) {
            if (m_mode == ListBoxMode::CheckBox) {
                toggle_checked(static_cast<size_t>(clicked_idx));
                set_selected_index(clicked_idx);
            } else {
                set_selected_index(clicked_idx);
            }
            return true;
        }
        return true;
    }

    // 3. Button up
    if (ev.type == MouseEventType::ButtonUp && ev.button == MouseButton::Left) {
        if (m_dragging_scrollbar) {
            m_dragging_scrollbar = false;
            return true;
        }
    }

    // 4. Mouse move
    if (ev.type == MouseEventType::Move) {
        if (m_dragging_scrollbar) {
            Rect track = scrollbar_track_rect();
            float thumb_h = scrollbar_thumb_rect().height;
            float scrollable_track = track.height - thumb_h;
            if (scrollable_track > 0.0f) {
                float dy = ev.position.y - m_drag_start_y;
                float scroll_delta = (dy / scrollable_track) * max_scroll_offset();
                m_scroll_offset = m_drag_start_scroll + scroll_delta;
                clamp_scroll();
            }
            return true;
        }

        if (inside) {
            float local_y = (ev.position.y - m_bounds.y - 1.0f) + m_scroll_offset;
            int idx = static_cast<int>(std::floor(local_y / m_item_height));
            if (idx >= 0 && idx < static_cast<int>(m_items.size())) {
                m_hovered_index = idx;
            } else {
                m_hovered_index = -1;
            }
            return true;
        } else {
            m_hovered_index = -1;
        }
    }

    return false;
}

void ListBox::render(Renderer2D& renderer) {
    if (m_bounds.width <= 0.0f || m_bounds.height <= 0.0f) return;

    // 1. Outer Container: Flat white card with subtle 1px border (#D1D5DB) and 3px radius
    renderer.draw_rounded_rect(
        m_bounds,
        3.0f,
        Color::white(),
        Color::from_hex(0xD1D5DB),
        1.0f
    );

    // Calculate content geometry
    bool has_scroll = needs_scrollbar();
    float content_w = has_scroll ? (m_bounds.width - 14.0f) : (m_bounds.width - 2.0f);

    // 2. Hardware Scissor Clipping for item list
    int scissor_x = static_cast<int>(std::max(0.0f, m_bounds.x + 1.0f));
    int scissor_y = renderer.viewport_height() - static_cast<int>(m_bounds.y + m_bounds.height - 1.0f);
    int scissor_w = static_cast<int>(content_w);
    int scissor_h = static_cast<int>(m_bounds.height - 2.0f);

    glEnable(GL_SCISSOR_TEST);
    glScissor(scissor_x, std::max(0, scissor_y), std::max(0, scissor_w), std::max(0, scissor_h));

    // 3. Render Items
    for (size_t i = 0; i < m_items.size(); ++i) {
        float item_y = m_bounds.y + 1.0f - m_scroll_offset + static_cast<float>(i) * m_item_height;

        // Skip items outside visible viewport bounds
        if (item_y + m_item_height < m_bounds.y || item_y > m_bounds.y + m_bounds.height) {
            continue;
        }

        Rect row_rect(m_bounds.x + 1.0f, item_y, content_w, m_item_height);
        bool is_selected = (static_cast<int>(i) == m_selected_index);
        bool is_hovered = (static_cast<int>(i) == m_hovered_index);

        // Background highlight
        if (is_selected && m_mode == ListBoxMode::Standard) {
            // Windows 10 Accent Blue selection
            renderer.draw_rounded_rect(row_rect, 0.0f, Color::from_hex(0x0067C0));
        } else if (is_hovered) {
            // Subtle hover slate tint
            renderer.draw_rounded_rect(row_rect, 0.0f, Color::from_hex(0xF3F4F6));
        }

        // Text & Checkbox coordinates
        Color text_color;
        if (is_selected && m_mode == ListBoxMode::Standard) {
            text_color = Color::white();
        } else {
            text_color = Color::from_hex(0x111827);
        }

        float text_x = m_bounds.x + 8.0f;

        if (m_mode == ListBoxMode::CheckBox) {
            // 16x16 Checkbox with 3px corner radius
            float chk_size = 16.0f;
            float chk_x = m_bounds.x + 8.0f;
            float chk_y = item_y + (m_item_height - chk_size) * 0.5f;
            Rect chk_rect(chk_x, chk_y, chk_size, chk_size);

            if (m_items[i].checked) {
                // Checked: Accent Blue filled box with white checkmark
                renderer.draw_rounded_rect(chk_rect, 3.0f, Color::from_hex(0x0067C0));
                renderer.draw_checkmark(chk_rect, Color::white(), 1.8f);
            } else {
                // Unchecked: White box with subtle grey border
                renderer.draw_rounded_rect(
                    chk_rect,
                    3.0f,
                    Color::white(),
                    Color::from_hex(0x9CA3AF),
                    1.0f
                );
            }

            text_x = chk_x + chk_size + 8.0f;
        }

        // Draw item text (centered vertically in row)
        float text_y = item_y + (m_item_height - 12.0f) * 0.5f - 1.0f;
        renderer.draw_text(m_items[i].text, Vec2(text_x, text_y), text_color, 0.95f);
    }

    renderer.flush(); // Flush text batch inside scissor
    glDisable(GL_SCISSOR_TEST);

    // 4. Render Scrollbar (if content overflows)
    if (has_scroll) {
        Rect track = scrollbar_track_rect();
        Rect thumb = scrollbar_thumb_rect();

        // Subtle track background capsule
        renderer.draw_rounded_rect(
            track,
            4.0f,
            Color::from_hex(0xF3F4F6)
        );

        // Sleek thumb capsule
        Color thumb_col = m_dragging_scrollbar
            ? Color::from_hex(0x6B7280)
            : Color::from_hex(0x9CA3AF);

        renderer.draw_rounded_rect(thumb, 4.0f, thumb_col);
    }
}

} // namespace arin
