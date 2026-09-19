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

#ifndef ARIN32_TABLE_VIEW_HPP
#define ARIN32_TABLE_VIEW_HPP

#include "widget.hpp"
#include "types.hpp"
#include "font.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace arin {

/**
 * @brief Represents a column descriptor in a TableView widget.
 */
struct TableColumn {
    std::string title;
    float width{120.0f};
    float min_width{40.0f};
    bool resizable{true};
    bool sortable{true};
    mutable Rect computed_header_bounds{0.0f, 0.0f, 0.0f, 0.0f};
};

/**
 * @brief Represents a data row in a TableView widget.
 */
struct TableRow {
    std::vector<std::string> cells;
    std::string icon_path;
    void* user_data{nullptr};
};

/**
 * @brief Visual styling configuration for a TableView.
 */
struct TableStyle {
    Color header_background{0.18f, 0.18f, 0.20f, 1.0f};
    Color header_border_color{0.24f, 0.24f, 0.27f, 1.0f};
    Color header_text_color{0.86f, 0.86f, 0.90f, 1.0f};
    Color body_background{0.15f, 0.15f, 0.17f, 1.0f};
    Color row_alt_background{0.17f, 0.17f, 0.19f, 1.0f};
    Color row_hover_color{0.22f, 0.22f, 0.26f, 1.0f};
    Color row_selected_color{0.0f, 0.45f, 0.85f, 0.40f};
    Color row_selected_border{0.0f, 0.48f, 0.88f, 0.80f};
    Color text_color{0.92f, 0.92f, 0.95f, 1.0f};
    Color grid_color{0.20f, 0.20f, 0.23f, 1.0f};
    float header_height{28.0f};
    float row_height{24.0f};
    float cell_padding_x{10.0f};
    float font_size{13.0f};
    float border_width{1.0f};
};

/**
 * @brief Multi-column data grid widget with resizable headers, row selection, and scrolling.
 */
class TableView : public IWidget {
public:
    TableView();
    explicit TableView(const Rect& bounds);
    ~TableView() override = default;

    // Column management
    size_t add_column(std::string title, float width = 120.0f);
    void set_column_width(size_t index, float width);
    float column_width(size_t index) const;
    const std::string& column_title(size_t index) const;
    const std::vector<TableColumn>& columns() const noexcept { return columns_; }
    size_t column_count() const noexcept;

    // Row management
    size_t add_row(std::vector<std::string> cells, void* user_data = nullptr);
    bool remove_row(size_t index);
    void set_cell(size_t row, size_t col, std::string value);
    const std::string& get_cell(size_t row, size_t col) const;
    size_t row_count() const noexcept;
    void clear_rows() noexcept;

    // Selection
    int selected_row() const noexcept;
    void select_row(int index);
    void clear_selection() noexcept;

    // Scrolling
    float scroll_y() const noexcept;
    void set_scroll_y(float offset) noexcept;

    // Callbacks
    void on_row_selected(std::function<void(int row_index)> cb);
    void on_row_double_clicked(std::function<void(int row_index)> cb);

    // Styling
    void set_style(const TableStyle& style) noexcept;
    const TableStyle& style() const noexcept;
    void set_font(const Font* font) noexcept;
    const Font* font() const noexcept;

    // IWidget geometry overrides
    const Rect& bounds() const override { return bounds_; }
    IWidget& set_bounds(const Rect& bounds) override { bounds_ = bounds; return *this; }
    IWidget& set_position(float x, float y) override { bounds_.x = x; bounds_.y = y; return *this; }
    IWidget& set_size(float width, float height) override { bounds_.width = width; bounds_.height = height; return *this; }
    bool is_visible() const override { return visible_; }
    bool is_enabled() const override { return enabled_; }
    void set_visible(bool v) noexcept { visible_ = v; }
    void set_enabled(bool e) noexcept { enabled_ = e; }

    // IWidget event overrides
    void render(Renderer2D& renderer) override;
    bool handle_mouse(const MouseEvent& ev) override;
    bool handle_key(const KeyEvent& ev) override;
    bool handle_text(const TextEvent& ev) override { (void)ev; return false; }

    // Convenience event helpers
    bool handle_mouse_move(const Vec2& mouse_pos);
    bool handle_mouse_button(MouseButton button, InputAction action, const Vec2& mouse_pos);
    bool handle_scroll(const Vec2& offset);
    bool handle_key_event(KeyCode key, InputAction action, KeyModifier mods);

private:
    void update_column_bounds() const;

    Rect bounds_{0.0f, 0.0f, 600.0f, 350.0f};
    bool visible_{true};
    bool enabled_{true};
    std::vector<TableColumn> columns_;
    std::vector<TableRow> rows_;
    int selected_row_{-1};
    int hovered_row_{-1};
    TableStyle style_;
    const Font* font_{nullptr};
    float scroll_y_{0.0f};

    // Column resizing state
    int resizing_column_{-1};
    float resize_start_x_{0.0f};
    float resize_start_width_{0.0f};

    std::function<void(int)> selection_cb_;
    std::function<void(int)> double_click_cb_;
};

} // namespace arin

#endif // ARIN32_TABLE_VIEW_HPP
