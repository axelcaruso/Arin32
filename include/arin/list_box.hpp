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

#ifndef ARIN32_LIST_BOX_HPP
#define ARIN32_LIST_BOX_HPP

#include "types.hpp"
#include "widget.hpp"
#include "renderer.hpp"
#include "input.hpp"
#include <string>
#include <vector>
#include <functional>

namespace arin {

/**
 * @brief Display mode for ListBox.
 */
enum class ListBoxMode : uint8_t {
    Standard, ///< Single-selection text list with highlight (e.g. roster/file list)
    CheckBox  ///< Multiple-selection list where every item has an interactive checkbox
};

/**
 * @brief Representation of an individual item inside a ListBox.
 */
struct ListBoxItem {
    std::string text;
    bool checked{false};
    bool enabled{true};

    ListBoxItem() = default;
    explicit ListBoxItem(std::string in_text, bool in_checked = false, bool in_enabled = true)
        : text(std::move(in_text)), checked(in_checked), enabled(in_enabled) {}
};

/**
 * @brief Modern, hardware-accelerated List Box and Check Box List widget.
 *
 * Features:
 * - 1:1 visual concordance with modern Windows 10 dialogs (white card, subtle 1px border, 3px radius).
 * - Smooth vertical scrolling via mouse wheel and interactive scrollbar thumb dragging.
 * - Single-selection mode with Windows 10 Accent Blue (#0067C0) highlight and white text.
 * - Interactive CheckBox mode with 16x16 rounded checkboxes and crisp checkmark rendering.
 * - Strict glScissor clipping ensuring items never bleed outside container boundaries.
 * - Fluid builder API.
 */
class ListBox : public IWidget {
public:
    using SelectionCallback = std::function<void(int index, const std::string& text)>;
    using ToggleCallback = std::function<void(int index, bool checked)>;

    ListBox();
    ListBox(float x, float y, float width = 220.0f, float height = 180.0f,
            ListBoxMode mode = ListBoxMode::Standard);
    explicit ListBox(const Rect& bounds, ListBoxMode mode = ListBoxMode::Standard);

    ~ListBox() override = default;

    // --- Fluent Configuration ---

    ListBox& set_bounds(const Rect& bounds) override;
    ListBox& set_bounds(float x, float y, float width, float height);
    ListBox& set_position(float x, float y) override;
    ListBox& set_size(float width, float height) override;
    ListBox& set_item_height(float height);
    ListBox& set_mode(ListBoxMode mode);
    ListBox& set_checkbox_mode(bool enable);

    // --- Item Management ---

    ListBox& add_item(const std::string& text, bool checked = false);
    ListBox& add_items(const std::vector<std::string>& items);
    ListBox& set_items(const std::vector<std::string>& items);
    ListBox& clear_items();

    // --- Queries ---

    const Rect& bounds() const override { return m_bounds; }
    ListBoxMode mode() const { return m_mode; }
    bool is_checkbox_mode() const { return m_mode == ListBoxMode::CheckBox; }
    size_t item_count() const { return m_items.size(); }
    float item_height() const { return m_item_height; }

    const ListBoxItem& item(size_t index) const;
    const std::string& item_text(size_t index) const;

    int selected_index() const { return m_selected_index; }
    std::string selected_item() const;

    ListBox& set_selected_index(int index);

    bool is_checked(size_t index) const;
    ListBox& set_checked(size_t index, bool checked);
    ListBox& toggle_checked(size_t index);

    std::vector<int> checked_indices() const;
    std::vector<std::string> checked_items() const;

    // --- Callbacks ---

    ListBox& on_selection_changed(SelectionCallback cb);
    ListBox& on_item_toggled(ToggleCallback cb);

    // --- Scroll & Scrollbar Queries ---

    bool needs_scrollbar() const;
    float max_scroll_offset() const;
    float scroll_offset() const { return m_scroll_offset; }
    Rect scrollbar_track_rect() const;
    Rect scrollbar_thumb_rect() const;

    // --- Widget Lifecycle & Interactions ---

    bool handle_mouse(const MouseEvent& ev) override;
    void render(Renderer2D& renderer) override;

private:
    void clamp_scroll();

    Rect m_bounds{0.0f, 0.0f, 220.0f, 180.0f};
    ListBoxMode m_mode{ListBoxMode::Standard};
    float m_item_height{26.0f};

    std::vector<ListBoxItem> m_items;
    int m_selected_index{-1};
    int m_hovered_index{-1};

    float m_scroll_offset{0.0f}; // Pixels scrolled from top
    bool m_dragging_scrollbar{false};
    float m_drag_start_y{0.0f};
    float m_drag_start_scroll{0.0f};

    SelectionCallback m_selection_cb;
    ToggleCallback m_toggle_cb;
};

/**
 * @brief Specialized convenience subclass for multiple-selection Check Box Lists.
 */
class CheckListBox : public ListBox {
public:
    CheckListBox()
        : ListBox(0.0f, 0.0f, 220.0f, 180.0f, ListBoxMode::CheckBox) {}

    CheckListBox(float x, float y, float width = 220.0f, float height = 180.0f)
        : ListBox(x, y, width, height, ListBoxMode::CheckBox) {}

    explicit CheckListBox(const Rect& bounds)
        : ListBox(bounds, ListBoxMode::CheckBox) {}
};

} // namespace arin

#endif // ARIN32_LIST_BOX_HPP
