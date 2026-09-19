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

#include "arin/tree_view.hpp"
#include "arin/renderer.hpp"
#include <algorithm>

namespace arin {

std::shared_ptr<TreeNode> TreeNode::add_child(std::string text, std::string icon_filepath) {
    auto child = std::make_shared<TreeNode>(std::move(text), std::move(icon_filepath));
    child->parent = shared_from_this();
    children.push_back(child);
    return child;
}

std::shared_ptr<TreeNode> TreeNode::add_child(std::shared_ptr<TreeNode> child) {
    if (child) {
        child->parent = shared_from_this();
        children.push_back(child);
    }
    return child;
}

bool TreeNode::remove_child(const std::string& child_id) {
    auto it = std::remove_if(children.begin(), children.end(), [&child_id](const std::shared_ptr<TreeNode>& node) {
        return node && node->id == child_id;
    });
    if (it != children.end()) {
        children.erase(it, children.end());
        return true;
    }
    return false;
}

void TreeNode::expand_all() noexcept {
    is_expanded = true;
    for (auto& child : children) {
        if (child) {
            child->expand_all();
        }
    }
}

void TreeNode::collapse_all() noexcept {
    is_expanded = false;
    for (auto& child : children) {
        if (child) {
            child->collapse_all();
        }
    }
}

std::shared_ptr<TreeNode> TreeNode::find_by_id(const std::string& target_id) {
    if (id == target_id) {
        return shared_from_this();
    }
    for (auto& child : children) {
        if (child) {
            auto found = child->find_by_id(target_id);
            if (found) {
                return found;
            }
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// TreeView Implementation
// ---------------------------------------------------------------------------

TreeView::TreeView()
    : bounds_{0.0f, 0.0f, 250.0f, 400.0f} {
}

TreeView::TreeView(const Rect& bounds)
    : bounds_(bounds) {
}

std::shared_ptr<TreeNode> TreeView::add_root(std::string label, std::string icon_filepath) {
    auto node = std::make_shared<TreeNode>(std::move(label), std::move(icon_filepath));
    roots_.push_back(node);
    return node;
}

std::shared_ptr<TreeNode> TreeView::add_root(std::shared_ptr<TreeNode> node) {
    if (node) {
        roots_.push_back(node);
    }
    return node;
}

const std::vector<std::shared_ptr<TreeNode>>& TreeView::roots() const noexcept {
    return roots_;
}

void TreeView::clear() noexcept {
    roots_.clear();
    selected_node_ = nullptr;
    visible_rows_.clear();
    scroll_y_ = 0.0f;
    hovered_row_ = -1;
}

std::shared_ptr<TreeNode> TreeView::selected_node() const noexcept {
    return selected_node_;
}

void TreeView::select_node(std::shared_ptr<TreeNode> node) {
    if (selected_node_ == node) {
        return;
    }
    if (selected_node_) {
        selected_node_->is_selected = false;
    }
    selected_node_ = node;
    if (selected_node_) {
        selected_node_->is_selected = true;
    }
    if (selection_cb_) {
        selection_cb_(selected_node_);
    }
}

void TreeView::clear_selection() noexcept {
    if (selected_node_) {
        selected_node_->is_selected = false;
        selected_node_ = nullptr;
        if (selection_cb_) {
            selection_cb_(nullptr);
        }
    }
}

float TreeView::scroll_offset() const noexcept {
    return scroll_y_;
}

void TreeView::set_scroll_offset(float offset) noexcept {
    scroll_y_ = std::max(0.0f, offset);
}

void TreeView::on_selection_changed(std::function<void(std::shared_ptr<TreeNode>)> cb) {
    selection_cb_ = std::move(cb);
}

void TreeView::on_node_expanded(std::function<void(std::shared_ptr<TreeNode>, bool)> cb) {
    expand_cb_ = std::move(cb);
}

void TreeView::on_node_double_clicked(std::function<void(std::shared_ptr<TreeNode>)> cb) {
    double_click_cb_ = std::move(cb);
}

void TreeView::set_style(const TreeStyle& style) noexcept {
    style_ = style;
}

const TreeStyle& TreeView::style() const noexcept {
    return style_;
}

void TreeView::set_font(const Font* font) noexcept {
    font_ = font;
}

const Font* TreeView::font() const noexcept {
    return font_;
}

void TreeView::flatten_visible_nodes(const std::vector<std::shared_ptr<TreeNode>>& nodes, int depth) const {
    for (const auto& node : nodes) {
        if (!node || !node->enabled) {
            continue;
        }

        VisibleTreeNode row;
        row.node = node;
        row.depth = depth;
        visible_rows_.push_back(row);

        if (node->is_expanded && !node->children.empty()) {
            flatten_visible_nodes(node->children, depth + 1);
        }
    }
}

void TreeView::update_layout() const {
    visible_rows_.clear();
    flatten_visible_nodes(roots_, 0);

    float current_y = bounds_.y + 2.0f - scroll_y_;
    const float row_w = bounds_.width - 4.0f;

    for (auto& row : visible_rows_) {
        row.row_bounds = Rect{bounds_.x + 2.0f, current_y, row_w, style_.row_height};

        // Toggle arrow bounds (+ / - or chevron)
        const float indent_x = bounds_.x + 6.0f + (static_cast<float>(row.depth) * style_.indent_width);
        row.toggle_bounds = Rect{indent_x, current_y + (style_.row_height - 12.0f) * 0.5f, 12.0f, 12.0f};

        current_y += style_.row_height + 1.0f;
    }
}

void TreeView::render(Renderer2D& renderer) {
    if (!visible_) {
        return;
    }

    update_layout();

    // Render widget background and border
    renderer.draw_rounded_rect(bounds_, style_.corner_radius, style_.background_color, style_.border_color, style_.border_width);

    // Render rows within bounds
    for (size_t i = 0; i < visible_rows_.size(); ++i) {
        const auto& row = visible_rows_[i];

        // Skip rows that are outside vertical bounds
        if (row.row_bounds.y + row.row_bounds.height < bounds_.y || row.row_bounds.y > bounds_.y + bounds_.height) {
            continue;
        }

        const bool is_sel = (row.node == selected_node_);
        const bool is_hov = (static_cast<int>(i) == hovered_row_);

        // Render row background
        if (is_sel) {
            renderer.draw_rounded_rect(row.row_bounds, style_.corner_radius, style_.row_selected_color, style_.row_selected_border_color, 1.0f);
        } else if (is_hov) {
            renderer.draw_rounded_rect(row.row_bounds, style_.corner_radius, style_.row_hover_color);
        }

        // Render expand/collapse arrow if node has children
        if (row.node->has_children()) {
            const char* symbol = row.node->is_expanded ? "v" : ">";
            const float sym_x = row.toggle_bounds.x + 2.0f;
            const float sym_y = row.toggle_bounds.y;
            renderer.draw_text(symbol, Vec2{sym_x, sym_y}, style_.arrow_color, 0.8f);
        }

        // Render label text
        const float indent_x = bounds_.x + 6.0f + (static_cast<float>(row.depth) * style_.indent_width) + 16.0f;
        const float text_y = row.row_bounds.y + (style_.row_height - style_.font_size) * 0.5f;
        const Color text_col = is_sel ? style_.text_selected_color : (row.node->enabled ? style_.text_color : style_.text_disabled_color);

        renderer.draw_text(row.node->label, Vec2{indent_x, text_y}, text_col, 1.0f);
    }
}

bool TreeView::handle_mouse_move(const Vec2& mouse_pos) {
    if (!visible_ || !enabled_) {
        return false;
    }

    update_layout();

    int prev_hover = hovered_row_;
    hovered_row_ = -1;

    if (bounds_.contains(mouse_pos)) {
        for (size_t i = 0; i < visible_rows_.size(); ++i) {
            if (visible_rows_[i].row_bounds.contains(mouse_pos)) {
                hovered_row_ = static_cast<int>(i);
                break;
            }
        }
    }

    return (hovered_row_ != prev_hover);
}

bool TreeView::handle_mouse(const MouseEvent& ev) {
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

bool TreeView::handle_mouse_button(MouseButton button, InputAction action, const Vec2& mouse_pos) {
    if (!visible_ || !enabled_ || !bounds_.contains(mouse_pos)) {
        return false;
    }

    update_layout();

    if (button == MouseButton::Left && action == InputAction::Press) {
        for (const auto& row : visible_rows_) {
            if (row.row_bounds.contains(mouse_pos)) {
                // If clicked toggle arrow, expand or collapse
                if (row.node->has_children() && row.toggle_bounds.contains(mouse_pos)) {
                    row.node->is_expanded = !row.node->is_expanded;
                    if (expand_cb_) {
                        expand_cb_(row.node, row.node->is_expanded);
                    }
                    return true;
                }

                // Select node
                select_node(row.node);
                return true;
            }
        }
    }

    return false;
}

bool TreeView::handle_scroll(const Vec2& offset) {
    if (!visible_ || !enabled_) {
        return false;
    }

    const float total_content_height = static_cast<float>(visible_rows_.size()) * (style_.row_height + 1.0f);
    const float max_scroll = std::max(0.0f, total_content_height - bounds_.height + 10.0f);

    scroll_y_ = std::clamp(scroll_y_ - (offset.y * 24.0f), 0.0f, max_scroll);
    return true;
}

bool TreeView::handle_key(const KeyEvent& ev) {
    return handle_key_event(ev.key, ev.action, static_cast<KeyModifier>(ev.modifiers));
}

bool TreeView::handle_key_event(KeyCode key, InputAction action, KeyModifier /*mods*/) {
    if (!visible_ || !enabled_ || action != InputAction::Press) {
        return false;
    }

    update_layout();
    if (visible_rows_.empty()) {
        return false;
    }

    // Find current selected row index
    int sel_idx = -1;
    for (size_t i = 0; i < visible_rows_.size(); ++i) {
        if (visible_rows_[i].node == selected_node_) {
            sel_idx = static_cast<int>(i);
            break;
        }
    }

    if (key == KeyCode::Down) {
        int next = (sel_idx < 0) ? 0 : std::min(sel_idx + 1, static_cast<int>(visible_rows_.size() - 1));
        select_node(visible_rows_[static_cast<size_t>(next)].node);
        return true;
    } else if (key == KeyCode::Up) {
        int prev = (sel_idx <= 0) ? 0 : sel_idx - 1;
        select_node(visible_rows_[static_cast<size_t>(prev)].node);
        return true;
    } else if (key == KeyCode::Right) {
        if (selected_node_ && selected_node_->has_children()) {
            if (!selected_node_->is_expanded) {
                selected_node_->is_expanded = true;
                if (expand_cb_) {
                    expand_cb_(selected_node_, true);
                }
                return true;
            }
        }
    } else if (key == KeyCode::Left) {
        if (selected_node_) {
            if (selected_node_->is_expanded) {
                selected_node_->is_expanded = false;
                if (expand_cb_) {
                    expand_cb_(selected_node_, false);
                }
                return true;
            } else if (auto parent_ptr = selected_node_->parent.lock()) {
                select_node(parent_ptr);
                return true;
            }
        }
    }

    return false;
}

} // namespace arin
