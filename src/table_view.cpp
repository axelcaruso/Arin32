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

#include "arin/table_view.hpp"
#include "arin/renderer.hpp"
#include <algorithm>
#include <stdexcept>

namespace arin {

TableView::TableView()
    : bounds_{0.0f, 0.0f, 600.0f, 350.0f} {
}

TableView::TableView(const Rect& bounds)
    : bounds_(bounds) {
}

size_t TableView::add_column(std::string title, float width) {
    TableColumn col;
    col.title = std::move(title);
    col.width = std::max(col.min_width, width);
    columns_.push_back(std::move(col));
    return columns_.size() - 1;
}

void TableView::set_column_width(size_t index, float width) {
    if (index >= columns_.size()) {
        throw std::out_of_range("TableView::set_column_width index out of range");
    }
    columns_[index].width = std::max(columns_[index].min_width, width);
}

float TableView::column_width(size_t index) const {
    if (index >= columns_.size()) {
        throw std::out_of_range("TableView::column_width index out of range");
    }
    return columns_[index].width;
}

const std::string& TableView::column_title(size_t index) const {
    if (index >= columns_.size()) {
        static const std::string empty;
        return empty;
    }
    return columns_[index].title;
}

size_t TableView::column_count() const noexcept {
    return columns_.size();
}

size_t TableView::add_row(std::vector<std::string> cells, void* user_data) {
    TableRow row;
    row.cells = std::move(cells);
    row.user_data = user_data;
    rows_.push_back(std::move(row));
    return rows_.size() - 1;
}

bool TableView::remove_row(size_t index) {
    if (index >= rows_.size()) {
        return false;
    }
    if (selected_row_ == static_cast<int>(index)) {
        selected_row_ = -1;
    } else if (selected_row_ > static_cast<int>(index)) {
        selected_row_--;
    }
    rows_.erase(rows_.begin() + index);
    return true;
}

void TableView::set_cell(size_t row, size_t col, std::string value) {
    if (row >= rows_.size()) {
        throw std::out_of_range("TableView::set_cell row out of range");
    }
    if (col >= rows_[row].cells.size()) {
        rows_[row].cells.resize(col + 1);
    }
    rows_[row].cells[col] = std::move(value);
}

const std::string& TableView::get_cell(size_t row, size_t col) const {
    if (row >= rows_.size() || col >= rows_[row].cells.size()) {
        static const std::string empty;
        return empty;
    }
    return rows_[row].cells[col];
}

size_t TableView::row_count() const noexcept {
    return rows_.size();
}

void TableView::clear_rows() noexcept {
    rows_.clear();
    selected_row_ = -1;
    hovered_row_ = -1;
    scroll_y_ = 0.0f;
}

int TableView::selected_row() const noexcept {
    return selected_row_;
}

void TableView::select_row(int index) {
    if (index < 0 || static_cast<size_t>(index) >= rows_.size()) {
        clear_selection();
        return;
    }
    if (selected_row_ != index) {
        selected_row_ = index;
        if (selection_cb_) {
            selection_cb_(selected_row_);
        }
    }
}

void TableView::clear_selection() noexcept {
    if (selected_row_ >= 0) {
        selected_row_ = -1;
        if (selection_cb_) {
            selection_cb_(-1);
        }
    }
}

float TableView::scroll_y() const noexcept {
    return scroll_y_;
}

void TableView::set_scroll_y(float offset) noexcept {
    scroll_y_ = std::max(0.0f, offset);
}

void TableView::on_row_selected(std::function<void(int)> cb) {
    selection_cb_ = std::move(cb);
}

void TableView::on_row_double_clicked(std::function<void(int)> cb) {
    double_click_cb_ = std::move(cb);
}

void TableView::set_style(const TableStyle& style) noexcept {
    style_ = style;
}

const TableStyle& TableView::style() const noexcept {
    return style_;
}

void TableView::set_font(const Font* font) noexcept {
    font_ = font;
}

const Font* TableView::font() const noexcept {
    return font_;
}

void TableView::update_column_bounds() const {
    float cur_x = bounds_.x;
    for (auto& col : columns_) {
        col.computed_header_bounds = Rect{cur_x, bounds_.y, col.width, style_.header_height};
        cur_x += col.width;
    }
}

void TableView::render(Renderer2D& renderer) {
    if (!visible_) {
        return;
    }

    update_column_bounds();

    // 1. Render main body background
    renderer.draw_rounded_rect(bounds_, 0.0f, style_.body_background, style_.header_border_color, style_.border_width);

    // 2. Render data rows
    const float body_y = bounds_.y + style_.header_height;
    const float body_h = std::max(0.0f, bounds_.height - style_.header_height);

    for (size_t r = 0; r < rows_.size(); ++r) {
        const float row_y = body_y + (static_cast<float>(r) * style_.row_height) - scroll_y_;

        // Skip rows that are outside visible vertical bounds
        if (row_y + style_.row_height < body_y || row_y > body_y + body_h) {
            continue;
        }

        const bool is_sel = (static_cast<int>(r) == selected_row_);
        const bool is_hov = (static_cast<int>(r) == hovered_row_);

        Color bg = (r % 2 == 1) ? style_.row_alt_background : style_.body_background;
        if (is_sel) {
            bg = style_.row_selected_color;
        } else if (is_hov) {
            bg = style_.row_hover_color;
        }

        Rect row_rect{bounds_.x, row_y, bounds_.width, style_.row_height};
        renderer.draw_rounded_rect(row_rect, 0.0f, bg, is_sel ? style_.row_selected_border : Color::transparent(), is_sel ? 1.0f : 0.0f);

        // Render cells
        float cur_x = bounds_.x;
        for (size_t c = 0; c < columns_.size(); ++c) {
            const float col_w = columns_[c].width;
            if (c < rows_[r].cells.size()) {
                const auto& text = rows_[r].cells[c];
                const float text_x = cur_x + style_.cell_padding_x;
                const float text_y = row_y + (style_.row_height - style_.font_size) * 0.5f;

                renderer.draw_text(text, Vec2{text_x, text_y}, style_.text_color, 1.0f);
            }
            cur_x += col_w;
        }
    }

    // 3. Render header row on top
    Rect header_rect{bounds_.x, bounds_.y, bounds_.width, style_.header_height};
    renderer.draw_rounded_rect(header_rect, 0.0f, style_.header_background, style_.header_border_color, style_.border_width);

    float cur_col_x = bounds_.x;
    for (const auto& col : columns_) {
        // Header title
        const float text_x = cur_col_x + style_.cell_padding_x;
        const float text_y = bounds_.y + (style_.header_height - style_.font_size) * 0.5f;

        renderer.draw_text(col.title, Vec2{text_x, text_y}, style_.header_text_color, 1.0f);

        // Column divider line
        Rect divider{cur_col_x + col.width - 1.0f, bounds_.y + 4.0f, 1.0f, style_.header_height - 8.0f};
        renderer.draw_rect(divider, style_.header_border_color);

        cur_col_x += col.width;
    }
}

bool TableView::handle_mouse_move(const Vec2& mouse_pos) {
    if (!visible_ || !enabled_) {
        return false;
    }

    update_column_bounds();

    // Column resizing drag in progress
    if (resizing_column_ >= 0 && static_cast<size_t>(resizing_column_) < columns_.size()) {
        const float delta = mouse_pos.x - resize_start_x_;
        columns_[static_cast<size_t>(resizing_column_)].width = std::max(
            columns_[static_cast<size_t>(resizing_column_)].min_width,
            resize_start_width_ + delta
        );
        return true;
    }

    int prev_hover = hovered_row_;
    hovered_row_ = -1;

    const float body_y = bounds_.y + style_.header_height;
    if (mouse_pos.x >= bounds_.x && mouse_pos.x <= bounds_.x + bounds_.width &&
        mouse_pos.y >= body_y && mouse_pos.y <= bounds_.y + bounds_.height) {
        const float rel_y = mouse_pos.y - body_y + scroll_y_;
        if (rel_y >= 0.0f) {
            int row_idx = static_cast<int>(rel_y / style_.row_height);
            if (row_idx >= 0 && static_cast<size_t>(row_idx) < rows_.size()) {
                hovered_row_ = row_idx;
            }
        }
    }

    return (hovered_row_ != prev_hover);
}

bool TableView::handle_mouse(const MouseEvent& ev) {
    if (!visible_ || !enabled_) {
        return false;
    }
    if (ev.type == MouseEventType::Move) {
        return handle_mouse_move(ev.position);
    } else if (ev.type == MouseEventType::ButtonDown) {
        return handle_mouse_button(ev.button, InputAction::Press, ev.position);
    } else if (ev.type == MouseEventType::ButtonUp) {
        return handle_mouse_button(ev.button, InputAction::Release, ev.position);
    } else if (ev.type == MouseEventType::Scroll) {
        return handle_scroll(ev.scroll_delta);
    }
    return false;
}

bool TableView::handle_mouse_button(MouseButton button, InputAction action, const Vec2& mouse_pos) {
    if (!visible_ || !enabled_) {
        return false;
    }

    update_column_bounds();

    if (button == MouseButton::Left) {
        if (action == InputAction::Press) {
            // Check if user clicked header separator to start resizing
            if (mouse_pos.y >= bounds_.y && mouse_pos.y <= bounds_.y + style_.header_height) {
                float cur_x = bounds_.x;
                for (size_t c = 0; c < columns_.size(); ++c) {
                    cur_x += columns_[c].width;
                    if (std::abs(mouse_pos.x - cur_x) <= 4.0f && columns_[c].resizable) {
                        resizing_column_ = static_cast<int>(c);
                        resize_start_x_ = mouse_pos.x;
                        resize_start_width_ = columns_[c].width;
                        return true;
                    }
                }
            }

            // Check if user clicked a data row
            const float body_y = bounds_.y + style_.header_height;
            if (mouse_pos.x >= bounds_.x && mouse_pos.x <= bounds_.x + bounds_.width &&
                mouse_pos.y >= body_y && mouse_pos.y <= bounds_.y + bounds_.height) {
                const float rel_y = mouse_pos.y - body_y + scroll_y_;
                if (rel_y >= 0.0f) {
                    int row_idx = static_cast<int>(rel_y / style_.row_height);
                    if (row_idx >= 0 && static_cast<size_t>(row_idx) < rows_.size()) {
                        select_row(row_idx);
                        return true;
                    }
                }
            }
        } else if (action == InputAction::Release) {
            if (resizing_column_ >= 0) {
                resizing_column_ = -1;
                return true;
            }
        }
    }

    return false;
}

bool TableView::handle_scroll(const Vec2& offset) {
    if (!visible_ || !enabled_) {
        return false;
    }

    const float total_content_height = static_cast<float>(rows_.size()) * style_.row_height;
    const float body_h = std::max(0.0f, bounds_.height - style_.header_height);
    const float max_scroll = std::max(0.0f, total_content_height - body_h + 10.0f);

    scroll_y_ = std::clamp(scroll_y_ - (offset.y * 24.0f), 0.0f, max_scroll);
    return true;
}

bool TableView::handle_key(const KeyEvent& ev) {
    return handle_key_event(ev.key, ev.action, static_cast<KeyModifier>(ev.modifiers));
}

bool TableView::handle_key_event(KeyCode key, InputAction action, KeyModifier /*mods*/) {
    if (!visible_ || !enabled_ || action != InputAction::Press) {
        return false;
    }

    if (rows_.empty()) {
        return false;
    }

    if (key == KeyCode::Down) {
        int next = (selected_row_ < 0) ? 0 : std::min(selected_row_ + 1, static_cast<int>(rows_.size() - 1));
        select_row(next);
        return true;
    } else if (key == KeyCode::Up) {
        int prev = (selected_row_ <= 0) ? 0 : selected_row_ - 1;
        select_row(prev);
        return true;
    }

    return false;
}

} // namespace arin
