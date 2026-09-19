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

#include "arin/context_menu.hpp"
#include "arin/metrics.hpp"
#include "arin/font.hpp"
#include <algorithm>
#include <stdexcept>

namespace arin {

ContextMenu::ContextMenu() {
    m_bounds = Rect(0.0f, 0.0f, m_style.min_width, 0.0f);
}

ContextMenu& ContextMenu::set_style(const ContextMenuStyle& style) {
    m_style = style;
    return *this;
}

ContextMenu& ContextMenu::add_item(MenuItem item) {
    m_items.push_back(std::move(item));
    return *this;
}

ContextMenu& ContextMenu::add_item(const std::string& label, std::function<void()> callback) {
    m_items.push_back(MenuItem::action(label, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_item(const std::string& label, IconType icon, std::function<void()> callback) {
    m_items.push_back(MenuItem::action(label, icon, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_item(
    const std::string& label,
    IconType icon,
    const std::string& shortcut,
    std::function<void()> callback
) {
    m_items.push_back(MenuItem::action(label, icon, shortcut, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_item(
    const std::string& label,
    const std::string& svg_path,
    std::function<void()> callback
) {
    m_items.push_back(MenuItem::action(label, svg_path, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_item(
    const std::string& label,
    const std::string& svg_path,
    const std::string& shortcut,
    std::function<void()> callback
) {
    m_items.push_back(MenuItem::action(label, svg_path, shortcut, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_item(
    const std::string& label,
    std::shared_ptr<SvgDocument> svg_doc,
    std::function<void()> callback
) {
    m_items.push_back(MenuItem::action(label, std::move(svg_doc), std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_item(
    const std::string& label,
    std::shared_ptr<SvgDocument> svg_doc,
    const std::string& shortcut,
    std::function<void()> callback
) {
    m_items.push_back(MenuItem::action(label, std::move(svg_doc), shortcut, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_submenu(const std::string& label, std::shared_ptr<ContextMenu> submenu, std::function<void()> callback) {
    m_items.push_back(MenuItem::submenu(label, std::move(submenu), std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_submenu(const std::string& label, std::shared_ptr<ContextMenu> submenu, const std::string& svg_path, std::function<void()> callback) {
    m_items.push_back(MenuItem::submenu(label, std::move(submenu), svg_path, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_submenu(const std::string& label, std::shared_ptr<ContextMenu> submenu, IconType icon, std::function<void()> callback) {
    m_items.push_back(MenuItem::submenu(label, std::move(submenu), icon, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_submenu(const std::string& label, std::function<void()> callback) {
    m_items.push_back(MenuItem::submenu(label, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_submenu(const std::string& label, const std::string& svg_path, std::function<void()> callback) {
    m_items.push_back(MenuItem::submenu(label, svg_path, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_submenu(const std::string& label, IconType icon, std::function<void()> callback) {
    m_items.push_back(MenuItem::submenu(label, icon, std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_submenu(const std::string& label, std::shared_ptr<SvgDocument> svg_doc, std::function<void()> callback) {
    m_items.push_back(MenuItem::submenu(label, std::move(svg_doc), std::move(callback)));
    return *this;
}

ContextMenu& ContextMenu::add_default_item(const std::string& label, std::function<void()> callback) {
    auto item = MenuItem::action(label, std::move(callback));
    item.is_default = true;
    m_items.push_back(std::move(item));
    return *this;
}

ContextMenu& ContextMenu::add_default_item(const std::string& label, const std::string& svg_path, std::function<void()> callback) {
    auto item = MenuItem::action(label, svg_path, std::move(callback));
    item.is_default = true;
    m_items.push_back(std::move(item));
    return *this;
}

ContextMenu& ContextMenu::add_default_item(const std::string& label, IconType icon, std::function<void()> callback) {
    auto item = MenuItem::action(label, icon, std::move(callback));
    item.is_default = true;
    m_items.push_back(std::move(item));
    return *this;
}

ContextMenu& ContextMenu::add_default_item(const std::string& label, std::shared_ptr<SvgDocument> svg_doc, std::function<void()> callback) {
    auto item = MenuItem::action(label, std::move(svg_doc), std::move(callback));
    item.is_default = true;
    m_items.push_back(std::move(item));
    return *this;
}

ContextMenu& ContextMenu::add_separator() {
    m_items.push_back(MenuItem::separator());
    return *this;
}

ContextMenu& ContextMenu::clear() {
    close_active_child();
    m_items.clear();
    m_hovered_index = -1;
    return *this;
}

MenuItem& ContextMenu::last_item() {
    if (m_items.empty()) {
        throw std::out_of_range("ContextMenu::last_item called on empty menu");
    }
    return m_items.back();
}

ContextMenu& ContextMenu::insert_item(size_t index, MenuItem item) {
    if (index >= m_items.size()) {
        m_items.push_back(std::move(item));
    } else {
        m_items.insert(m_items.begin() + index, std::move(item));
    }
    return *this;
}

ContextMenu& ContextMenu::insert_item(size_t index, const std::string& label, std::function<void()> callback) {
    return insert_item(index, MenuItem::action(label, std::move(callback)));
}

ContextMenu& ContextMenu::insert_item(size_t index, const std::string& label, const std::string& svg_path, std::function<void()> callback) {
    return insert_item(index, MenuItem::action(label, svg_path, std::move(callback)));
}

ContextMenu& ContextMenu::insert_item(size_t index, const std::string& label, IconType icon, std::function<void()> callback) {
    return insert_item(index, MenuItem::action(label, icon, std::move(callback)));
}

ContextMenu& ContextMenu::insert_item_before(const std::string& target_label_or_id, MenuItem item) {
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == target_label_or_id || m_items[i].label == target_label_or_id) {
            return insert_item(i, std::move(item));
        }
    }
    return add_item(std::move(item));
}

ContextMenu& ContextMenu::insert_item_after(const std::string& target_label_or_id, MenuItem item) {
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == target_label_or_id || m_items[i].label == target_label_or_id) {
            return insert_item(i + 1, std::move(item));
        }
    }
    return add_item(std::move(item));
}

ContextMenu& ContextMenu::insert_submenu(size_t index, const std::string& label, std::shared_ptr<ContextMenu> submenu, std::function<void()> callback) {
    return insert_item(index, MenuItem::submenu(label, std::move(submenu), std::move(callback)));
}

ContextMenu& ContextMenu::insert_submenu(size_t index, const std::string& label, std::shared_ptr<ContextMenu> submenu, const std::string& svg_path, std::function<void()> callback) {
    return insert_item(index, MenuItem::submenu(label, std::move(submenu), svg_path, std::move(callback)));
}

ContextMenu& ContextMenu::insert_separator(size_t index) {
    return insert_item(index, MenuItem::separator());
}

ContextMenu& ContextMenu::remove_item(size_t index) {
    if (index < m_items.size()) {
        if (m_active_child_index == static_cast<int>(index)) {
            close_active_child();
        } else if (m_active_child_index > static_cast<int>(index)) {
            m_active_child_index--;
        }
        m_items.erase(m_items.begin() + index);
    }
    return *this;
}

ContextMenu& ContextMenu::remove_item(const std::string& label) {
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].label == label) {
            return remove_item(i);
        }
    }
    return *this;
}

ContextMenu& ContextMenu::remove_item_by_id(const std::string& id) {
    if (id.empty()) return *this;
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            return remove_item(i);
        }
    }
    return *this;
}

MenuItem* ContextMenu::find_item(const std::string& label) {
    for (auto& item : m_items) {
        if (item.label == label) return &item;
    }
    return nullptr;
}

const MenuItem* ContextMenu::find_item(const std::string& label) const {
    for (const auto& item : m_items) {
        if (item.label == label) return &item;
    }
    return nullptr;
}

MenuItem* ContextMenu::find_item_by_id(const std::string& id) {
    if (id.empty()) return nullptr;
    for (auto& item : m_items) {
        if (item.id == id) return &item;
    }
    return nullptr;
}

const MenuItem* ContextMenu::find_item_by_id(const std::string& id) const {
    if (id.empty()) return nullptr;
    for (const auto& item : m_items) {
        if (item.id == id) return &item;
    }
    return nullptr;
}

bool ContextMenu::has_item(const std::string& label) const {
    return find_item(label) != nullptr;
}

bool ContextMenu::has_item_by_id(const std::string& id) const {
    return find_item_by_id(id) != nullptr;
}

ContextMenu& ContextMenu::set_item_visible(size_t index, bool visible) {
    if (index < m_items.size()) {
        m_items[index].visible = visible;
        if (!visible && m_active_child_index == static_cast<int>(index)) {
            close_active_child();
        }
    }
    return *this;
}

ContextMenu& ContextMenu::set_item_visible(const std::string& label, bool visible) {
    if (auto* it = find_item(label)) {
        it->visible = visible;
        if (!visible && m_active_child_menu && m_active_child_menu == it->child_menu) {
            close_active_child();
        }
    }
    return *this;
}

ContextMenu& ContextMenu::set_item_visible_by_id(const std::string& id, bool visible) {
    if (auto* it = find_item_by_id(id)) {
        it->visible = visible;
        if (!visible && m_active_child_menu && m_active_child_menu == it->child_menu) {
            close_active_child();
        }
    }
    return *this;
}

ContextMenu& ContextMenu::set_item_enabled(size_t index, bool enabled) {
    if (index < m_items.size()) {
        m_items[index].enabled = enabled;
        if (!enabled && m_active_child_index == static_cast<int>(index)) {
            close_active_child();
        }
    }
    return *this;
}

ContextMenu& ContextMenu::set_item_enabled(const std::string& label, bool enabled) {
    if (auto* it = find_item(label)) {
        it->enabled = enabled;
        if (!enabled && m_active_child_menu && m_active_child_menu == it->child_menu) {
            close_active_child();
        }
    }
    return *this;
}

ContextMenu& ContextMenu::set_item_enabled_by_id(const std::string& id, bool enabled) {
    if (auto* it = find_item_by_id(id)) {
        it->enabled = enabled;
        if (!enabled && m_active_child_menu && m_active_child_menu == it->child_menu) {
            close_active_child();
        }
    }
    return *this;
}

size_t ContextMenu::visible_item_count() const {
    size_t count = 0;
    for (const auto& it : m_items) {
        if (it.visible) count++;
    }
    return count;
}

void ContextMenu::close_active_child() {
    if (m_active_child_menu) {
        m_active_child_menu->hide();
        m_active_child_menu = nullptr;
        m_active_child_index = -1;
    }
}

void ContextMenu::open_child_menu(int index) {
    if (index < 0 || index >= static_cast<int>(m_items.size())) return;
    auto& item = m_items[index];
    if (!item.child_menu || !item.enabled) return;

    if (m_active_child_menu == item.child_menu && m_active_child_menu->is_visible()) {
        return;
    }

    close_active_child();

    m_active_child_menu = item.child_menu;
    m_active_child_index = index;

    Rect row = row_rect_at(index);

    if (m_font_ptr) {
        m_active_child_menu->calculate_dimensions(*m_font_ptr);
    } else {
        m_active_child_menu->calculate_dimensions(Font());
    }

    float child_w = m_active_child_menu->bounds().width;
    float child_h = m_active_child_menu->bounds().height;

    // Standard Windows 10 placement: to the right of parent, overlapping 1px border by 2px
    float child_x = m_bounds.x + m_bounds.width - 2.0f;
    float child_y = row.y - m_active_child_menu->style().padding.top;

    // Flip to left if overflowing screen on right
    if (m_screen_w > 0.0f && (child_x + child_w > m_screen_w - UiMetrics::kMenuScreenMargin)) {
        child_x = m_bounds.x - child_w + 2.0f;
    }

    // Clamp vertically if overflowing bottom
    if (m_screen_h > 0.0f && (child_y + child_h > m_screen_h - UiMetrics::kMenuScreenMargin)) {
        child_y = m_screen_h - child_h - UiMetrics::kMenuScreenMargin;
    }
    if (child_y < UiMetrics::kMenuScreenMargin) {
        child_y = UiMetrics::kMenuScreenMargin;
    }

    if (m_font_ptr) {
        m_active_child_menu->show(child_x, child_y, *m_font_ptr, m_screen_w, m_screen_h);
    } else {
        m_active_child_menu->show(child_x, child_y, m_screen_w, m_screen_h);
    }
}

void ContextMenu::calculate_dimensions(const Font& font) {
    float max_label_w = 0.0f;
    float max_shortcut_w = 0.0f;
    bool has_any_icons = false;
    bool has_any_submenus = false;

    float total_h = m_style.padding.top + m_style.padding.bottom;

    for (const auto& item : m_items) {
        if (!item.visible) continue;
        if (item.is_separator) {
            total_h += m_style.separator_height;
        } else {
            total_h += m_style.item_height;
            if (item.icon != IconType::None || item.svg_icon != nullptr || !item.svg_path.empty()) {
                has_any_icons = true;
            }
            if (item.has_submenu || item.child_menu != nullptr) {
                has_any_submenus = true;
            }
            Vec2 lbl_sz = font.measure_text(item.label, m_style.label_scale);
            max_label_w = std::max(max_label_w, lbl_sz.x);

            if (!item.shortcut.empty()) {
                Vec2 sc_sz = font.measure_text(item.shortcut, m_style.shortcut_scale);
                max_shortcut_w = std::max(max_shortcut_w, sc_sz.x);
            }
        }
    }

    const float icon_gap = (has_any_icons || (m_style.always_align_labels && !m_items.empty()))
                         ? m_style.icon_column_width
                         : UiMetrics::kMenuTextInsetNoIcon;
    const float shortcut_gap = (max_shortcut_w > 0.0f) ? (20.0f + max_shortcut_w) : 0.0f;
    const float submenu_gap = has_any_submenus ? 20.0f : 0.0f;
    const float required_w = m_style.padding.left + m_style.padding.right +
                             icon_gap + max_label_w + shortcut_gap + submenu_gap + 16.0f;

    m_bounds.width = std::max(m_style.min_width, required_w);
    m_bounds.height = total_h;
}

Rect ContextMenu::row_rect_at(size_t index) const {
    if (index >= m_items.size() || !m_items[index].visible) {
        return Rect(0.0f, 0.0f, 0.0f, 0.0f);
    }
    float cursor_y = m_bounds.y + m_style.padding.top;
    for (size_t i = 0; i < index && i < m_items.size(); ++i) {
        if (!m_items[i].visible) continue;
        cursor_y += m_items[i].is_separator ? m_style.separator_height : m_style.item_height;
    }
    const float content_w = m_bounds.width - m_style.padding.left - m_style.padding.right;
    const float row_h = (index < m_items.size() && m_items[index].is_separator)
        ? m_style.separator_height
        : m_style.item_height;
    return Rect(m_bounds.x + m_style.padding.left, cursor_y, content_w, row_h);
}

void ContextMenu::show(float x, float y, float screen_width, float screen_height) {
    show(x, y, Font(), screen_width, screen_height);
}

void ContextMenu::show(float x, float y, const Font& font, float screen_width, float screen_height) {
    if (m_before_show_cb) {
        m_before_show_cb(*this);
    }

    m_visible = true;
    m_hovered_index = -1;
    m_screen_w = screen_width;
    m_screen_h = screen_height;
    m_font_ptr = &font;

    close_active_child();
    calculate_dimensions(font);

    // Clamp the popup so it never leaves the visible window surface
    if (screen_width > 0.0f) {
        if (x + m_bounds.width > screen_width - UiMetrics::kMenuScreenMargin) {
            x = screen_width - m_bounds.width - UiMetrics::kMenuScreenMargin;
        }
        if (x < UiMetrics::kMenuScreenMargin) {
            x = UiMetrics::kMenuScreenMargin;
        }
    }

    if (screen_height > 0.0f) {
        if (y + m_bounds.height > screen_height - UiMetrics::kMenuScreenMargin) {
            y = screen_height - m_bounds.height - UiMetrics::kMenuScreenMargin;
        }
        if (y < UiMetrics::kMenuScreenMargin) {
            y = UiMetrics::kMenuScreenMargin;
        }
    }

    m_bounds.x = x;
    m_bounds.y = y;
}

void ContextMenu::hide() {
    if (m_visible) {
        m_visible = false;
        m_hovered_index = -1;
        close_active_child();
        if (m_dismiss_cb) {
            m_dismiss_cb();
        }
    }
}

bool ContextMenu::handle_mouse(const MouseEvent& ev) {
    if (!m_visible) return false;

    // Prioritize active cascading child submenu if cursor is inside it
    if (m_active_child_menu && m_active_child_menu->is_visible()) {
        if (m_active_child_menu->bounds().contains(ev.position)) {
            bool handled = m_active_child_menu->handle_mouse(ev);
            if (!m_active_child_menu->is_visible()) {
                hide(); // Child action executed: dismiss parent tree
            }
            return handled;
        }

        // If mouse is still on the parent row that spawned the child, keep child alive
        if (m_active_child_index >= 0 && m_active_child_index < static_cast<int>(m_items.size())) {
            Rect owner_row = row_rect_at(m_active_child_index);
            if (owner_row.contains(ev.position)) {
                m_hovered_index = m_active_child_index;
                return true;
            }
        }
    }

    // 1. Mouse move
    if (ev.type == MouseEventType::Move) {
        if (!m_bounds.contains(ev.position)) {
            if (!m_active_child_menu || !m_active_child_menu->is_visible()) {
                m_hovered_index = -1;
            }
            return false;
        }

        float cur_y = m_bounds.y + m_style.padding.top;
        int found_idx = -1;
        for (size_t i = 0; i < m_items.size(); ++i) {
            const auto& item = m_items[i];
            if (!item.visible) continue;
            float item_h = item.is_separator ? m_style.separator_height : m_style.item_height;
            Rect item_rect(m_bounds.x, cur_y, m_bounds.width, item_h);

            if (!item.is_separator && item.enabled && item_rect.contains(ev.position)) {
                found_idx = static_cast<int>(i);
                break;
            }
            cur_y += item_h;
        }

        m_hovered_index = found_idx;

        if (found_idx >= 0) {
            const auto& item = m_items[found_idx];
            if (item.child_menu) {
                if (m_active_child_index != found_idx) {
                    open_child_menu(found_idx);
                }
            } else {
                close_active_child();
            }
        }

        return true;
    }

    // 2. Mouse button down
    if (ev.type == MouseEventType::ButtonDown) {
        bool in_parent = m_bounds.contains(ev.position);
        bool in_child = m_active_child_menu && m_active_child_menu->is_visible() &&
                        m_active_child_menu->bounds().contains(ev.position);

        if (!in_parent && !in_child) {
            hide();
            return false; // Click outside dismisses menu tree
        }
        return true; // Click inside is consumed
    }

    // 3. Mouse button up (click)
    if (ev.type == MouseEventType::ButtonUp && ev.button == MouseButton::Left) {
        if (m_active_child_menu && m_active_child_menu->is_visible() &&
            m_active_child_menu->bounds().contains(ev.position)) {
            bool handled = m_active_child_menu->handle_mouse(ev);
            if (!m_active_child_menu->is_visible()) {
                hide();
            }
            return handled;
        }

        if (!m_bounds.contains(ev.position)) {
            hide();
            return false;
        }

        // If hovered index was not established via move, resolve row at click position
        int active_idx = m_hovered_index;
        if (active_idx < 0 || active_idx >= static_cast<int>(m_items.size())) {
            float cur_y = m_bounds.y + m_style.padding.top;
            for (size_t i = 0; i < m_items.size(); ++i) {
                const auto& item = m_items[i];
                if (!item.visible) continue;
                float item_h = item.is_separator ? m_style.separator_height : m_style.item_height;
                Rect item_rect(m_bounds.x, cur_y, m_bounds.width, item_h);
                if (!item.is_separator && item.enabled && item_rect.contains(ev.position)) {
                    active_idx = static_cast<int>(i);
                    break;
                }
                cur_y += item_h;
            }
        }

        if (active_idx >= 0 && active_idx < static_cast<int>(m_items.size())) {
            const auto& item = m_items[active_idx];
            if (!item.is_separator && item.enabled) {
                if (item.child_menu) {
                    open_child_menu(active_idx);
                    return true;
                } else if (item.callback) {
                    auto cb = item.callback;
                    hide();
                    cb();
                    return true;
                }
            }
        }
        hide();
        return true;
    }

    return m_bounds.contains(ev.position);
}

bool ContextMenu::handle_key(const KeyEvent& ev) {
    if (!m_visible) return false;

    // Dismiss context menu on Escape key
    if (ev.key == KeyCode::Escape && ev.action == InputAction::Press) {
        if (m_active_child_menu && m_active_child_menu->is_visible()) {
            close_active_child();
            return true;
        }
        hide();
        return true;
    }

    // If child submenu is active
    if (m_active_child_menu && m_active_child_menu->is_visible()) {
        // Left arrow closes the child and returns focus to parent
        if (ev.key == KeyCode::Left && (ev.action == InputAction::Press || ev.action == InputAction::Repeat)) {
            close_active_child();
            return true;
        }

        // If child already has a focused item, route keyboard input directly to child
        if (m_active_child_menu->hovered_index() >= 0) {
            bool handled = m_active_child_menu->handle_key(ev);
            if (!m_active_child_menu->is_visible()) {
                hide();
            }
            return handled;
        }

        // If child is open but not yet focused: Right arrow or Enter focuses into the child
        if ((ev.key == KeyCode::Right || ev.key == KeyCode::Enter) &&
            (ev.action == InputAction::Press || ev.action == InputAction::Repeat)) {
            for (size_t i = 0; i < m_active_child_menu->item_count(); ++i) {
                if (m_active_child_menu->item_at(i).visible && !m_active_child_menu->item_at(i).is_separator && m_active_child_menu->item_at(i).enabled) {
                    m_active_child_menu->set_hovered_index(static_cast<int>(i));
                    break;
                }
            }
            return true;
        }
    }

    // Keyboard navigation in parent menu
    if (ev.action == InputAction::Press || ev.action == InputAction::Repeat) {
        if (ev.key == KeyCode::Down) {
            int next = m_hovered_index + 1;
            while (next < static_cast<int>(m_items.size()) && (!m_items[next].visible || m_items[next].is_separator || !m_items[next].enabled)) {
                next++;
            }
            if (next < static_cast<int>(m_items.size())) {
                m_hovered_index = next;
                if (m_items[next].child_menu) {
                    open_child_menu(next);
                } else {
                    close_active_child();
                }
            }
            return true;
        } else if (ev.key == KeyCode::Up) {
            int prev = m_hovered_index - 1;
            while (prev >= 0 && (!m_items[prev].visible || m_items[prev].is_separator || !m_items[prev].enabled)) {
                prev--;
            }
            if (prev >= 0) {
                m_hovered_index = prev;
                if (m_items[prev].child_menu) {
                    open_child_menu(prev);
                } else {
                    close_active_child();
                }
            }
            return true;
        } else if (ev.key == KeyCode::Right) {
            if (m_hovered_index >= 0 && m_hovered_index < static_cast<int>(m_items.size())) {
                if (m_items[m_hovered_index].child_menu) {
                    open_child_menu(m_hovered_index);
                    if (m_active_child_menu) {
                        m_active_child_menu->set_hovered_index(0);
                    }
                    return true;
                }
            }
        } else if (ev.key == KeyCode::Enter) {
            if (m_hovered_index >= 0 && m_hovered_index < static_cast<int>(m_items.size())) {
                const auto& item = m_items[m_hovered_index];
                if (item.child_menu) {
                    open_child_menu(m_hovered_index);
                    if (m_active_child_menu) {
                        m_active_child_menu->set_hovered_index(0);
                    }
                    return true;
                } else if (!item.is_separator && item.enabled && item.callback) {
                    auto cb = item.callback;
                    hide();
                    cb();
                    return true;
                }
            }
            return true;
        }
    }

    return false;
}

void ContextMenu::update(float dt) {
    (void)dt;
    if (m_active_child_menu && m_active_child_menu->is_visible()) {
        m_active_child_menu->update(dt);
    }
}

void ContextMenu::render(Renderer2D& renderer) {
    if (!m_visible || m_items.empty()) return;

    // 1. Soft Drop Shadow
    renderer.draw_shadow(
        m_bounds,
        m_style.corner_radius,
        m_style.shadow_color,
        m_style.shadow_offset,
        m_style.shadow_blur
    );

    // 2. Card Surface and Crisp 1px Border
    renderer.draw_rounded_rect(
        m_bounds,
        m_style.corner_radius,
        m_style.background_color,
        m_style.border_color,
        1.0f
    );

    bool has_any_icons = false;
    for (const auto& item : m_items) {
        if (!item.visible) continue;
        if (item.icon != IconType::None || item.svg_icon != nullptr || !item.svg_path.empty()) {
            has_any_icons = true;
            break;
        }
    }

    const bool align_icons = has_any_icons || (m_style.always_align_labels && !m_items.empty());

    // 3. Render Items
    float cur_y = m_bounds.y + m_style.padding.top;
    float content_w = m_bounds.width - m_style.padding.left - m_style.padding.right;

    for (size_t i = 0; i < m_items.size(); ++i) {
        auto& item = m_items[i];
        if (!item.visible) continue;

        if (item.is_separator) {
            // Inset separator line
            const float line_y = cur_y + m_style.separator_height * detail::kHalf;
            renderer.draw_rect(
                Rect(m_bounds.x + 2.0f, line_y, m_bounds.width - 4.0f, 1.0f),
                m_style.separator_color
            );
            cur_y += m_style.separator_height;
        } else {
            Rect item_rect(m_bounds.x + m_style.padding.left, cur_y, content_w, m_style.item_height);
            bool is_hovered = (static_cast<int>(i) == m_hovered_index) && item.enabled;

            Color text_col;
            Color icon_col;
            Color sc_col;
            Color chev_col;

            if (is_hovered) {
                // Windows 10 style light grey selection highlight
                renderer.draw_rounded_rect(
                    item_rect,
                    m_style.hover_corner_radius,
                    m_style.hover_color,
                    m_style.hover_border_color,
                    (m_style.hover_border_color.a > 0.0f) ? 1.0f : 0.0f
                );
                text_col = m_style.hover_text_color;
                icon_col = m_style.icon_hover_color;
                sc_col   = m_style.shortcut_hover_color;
                chev_col = m_style.chevron_hover_color;
            } else {
                text_col = item.enabled ? m_style.text_color : m_style.text_disabled_color;
                icon_col = item.enabled ? m_style.icon_color : m_style.text_disabled_color;
                sc_col   = item.enabled ? m_style.shortcut_color : m_style.text_disabled_color;
                chev_col = item.enabled ? m_style.chevron_color : m_style.text_disabled_color;
            }

            // Draw Icon (SVG or vector glyph)
            const float icon_sz = m_style.icon_size;
            const float icon_x = item_rect.x + 6.0f;
            const float icon_y = item_rect.y + (item_rect.height - icon_sz) * detail::kHalf;
            const Rect icon_bounds(icon_x, icon_y, icon_sz, icon_sz);

            if (item.svg_icon != nullptr || !item.svg_path.empty()) {
                if (!item.cached_texture) {
                    if (item.svg_icon) {
                        item.cached_texture = item.svg_icon->create_texture(
                            static_cast<int>(icon_sz * 2.0f),
                            static_cast<int>(icon_sz * 2.0f)
                        );
                    } else if (!item.svg_path.empty()) {
                        item.cached_texture = Texture::create_from_svg_file(
                            item.svg_path,
                            static_cast<int>(icon_sz * 2.0f),
                            static_cast<int>(icon_sz * 2.0f)
                        );
                    }
                }
                if (item.cached_texture && item.cached_texture->is_valid()) {
                    Color svg_tint = item.enabled ? Color::white() : Color(0.6f, 0.6f, 0.6f, 0.6f);
                    renderer.draw_image(*item.cached_texture, icon_bounds, svg_tint);
                }
            } else if (item.icon != IconType::None) {
                renderer.draw_icon(item.icon, icon_bounds, icon_col);
            }

            // Label text placement
            const float text_x = item_rect.x + (align_icons ? m_style.icon_column_width : UiMetrics::kMenuTextInsetNoIcon);
            float text_right = item_rect.right() - 8.0f;
            if (item.has_submenu || item.child_menu != nullptr) {
                text_right -= 18.0f;
            }
            if (!item.shortcut.empty()) {
                const Vec2 sc_sz = renderer.font().measure_text(item.shortcut, m_style.shortcut_scale);
                text_right -= (sc_sz.x + 12.0f);
            }
            const Rect label_bounds(text_x, item_rect.y, std::max(0.0f, text_right - text_x), item_rect.height);

            if (item.is_default) {
                // Windows default action (e.g. "Abrir") rendered bold via faux-bold overdraw
                renderer.draw_text_in_rect(item.label, label_bounds, text_col, m_style.label_scale, TextAlignH::Left, TextAlignV::Center);
                Rect bold_bounds(label_bounds.x + 0.5f, label_bounds.y, label_bounds.width, label_bounds.height);
                renderer.draw_text_in_rect(item.label, bold_bounds, text_col, m_style.label_scale, TextAlignH::Left, TextAlignV::Center);
            } else {
                renderer.draw_text_in_rect(item.label, label_bounds, text_col, m_style.label_scale, TextAlignH::Left, TextAlignV::Center);
            }

            // Submenu chevron
            if (item.has_submenu || item.child_menu != nullptr) {
                const float ch_sz = 9.0f;
                const float ch_x = item_rect.right() - ch_sz - 8.0f;
                const float ch_y = item_rect.y + (item_rect.height - ch_sz) * detail::kHalf;
                renderer.draw_icon(IconType::ChevronRight, Rect(ch_x, ch_y, ch_sz, ch_sz), chev_col);
            }

            // Shortcut text
            if (!item.shortcut.empty()) {
                const Vec2 sc_sz = renderer.font().measure_text(item.shortcut, m_style.shortcut_scale);
                float sc_x = item_rect.right() - sc_sz.x - 8.0f;
                if (item.has_submenu || item.child_menu != nullptr) {
                    sc_x -= 18.0f;
                }
                const Rect shortcut_bounds(sc_x, item_rect.y, sc_sz.x, item_rect.height);
                renderer.draw_text_in_rect(item.shortcut, shortcut_bounds, sc_col, m_style.shortcut_scale, TextAlignH::Left, TextAlignV::Center);
            }

            cur_y += m_style.item_height;
        }
    }

    // 4. Render cascading child submenu on top
    if (m_active_child_menu && m_active_child_menu->is_visible()) {
        m_active_child_menu->render(renderer);
    }
}

} // namespace arin
