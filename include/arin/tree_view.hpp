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

#ifndef ARIN32_TREE_VIEW_HPP
#define ARIN32_TREE_VIEW_HPP

#include "widget.hpp"
#include "types.hpp"
#include "font.hpp"
#include "svg.hpp"
#include "icon.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace arin {

/**
 * @brief Represents a hierarchical node within a TreeView widget.
 */
class TreeNode : public std::enable_shared_from_this<TreeNode> {
public:
    std::string id;
    std::string label;
    std::string icon_path;
    IconType icon{IconType::None};
    bool is_expanded{false};
    bool is_selected{false};
    bool enabled{true};
    void* user_data{nullptr};
    std::weak_ptr<TreeNode> parent;
    std::vector<std::shared_ptr<TreeNode>> children;

    TreeNode() = default;
    explicit TreeNode(std::string text, std::string icon_filepath = "")
        : label(std::move(text)), icon_path(std::move(icon_filepath)) {}

    std::shared_ptr<TreeNode> add_child(std::string text, std::string icon_filepath = "");
    std::shared_ptr<TreeNode> add_child(std::shared_ptr<TreeNode> child);
    bool remove_child(const std::string& child_id);
    bool has_children() const noexcept { return !children.empty(); }
    void expand_all() noexcept;
    void collapse_all() noexcept;
    std::shared_ptr<TreeNode> find_by_id(const std::string& target_id);
};

/**
 * @brief Visual styling configuration for a TreeView.
 */
struct TreeStyle {
    Color background_color{0.16f, 0.16f, 0.18f, 1.0f};
    Color border_color{0.22f, 0.22f, 0.25f, 1.0f};
    Color row_hover_color{0.22f, 0.22f, 0.26f, 1.0f};
    Color row_selected_color{0.0f, 0.45f, 0.85f, 0.35f};
    Color row_selected_border_color{0.0f, 0.48f, 0.88f, 0.80f};
    Color text_color{0.92f, 0.92f, 0.95f, 1.0f};
    Color text_selected_color{1.0f, 1.0f, 1.0f, 1.0f};
    Color text_disabled_color{0.50f, 0.50f, 0.53f, 1.0f};
    Color arrow_color{0.75f, 0.75f, 0.80f, 1.0f};
    float row_height{24.0f};
    float indent_width{18.0f};
    float icon_size{16.0f};
    float font_size{13.0f};
    float corner_radius{3.0f};
    float border_width{1.0f};
};

/**
 * @brief Flattened visible row descriptor for rendering and hit detection.
 */
struct VisibleTreeNode {
    std::shared_ptr<TreeNode> node;
    int depth{0};
    Rect row_bounds{0.0f, 0.0f, 0.0f, 0.0f};
    Rect toggle_bounds{0.0f, 0.0f, 0.0f, 0.0f};
};

/**
 * @brief Hierarchical tree view widget with collapsible nodes, icons, and keyboard navigation.
 */
class TreeView : public IWidget {
public:
    TreeView();
    explicit TreeView(const Rect& bounds);
    ~TreeView() override = default;

    // Node management
    std::shared_ptr<TreeNode> add_root(std::string label, std::string icon_filepath = "");
    std::shared_ptr<TreeNode> add_root(std::shared_ptr<TreeNode> node);
    const std::vector<std::shared_ptr<TreeNode>>& roots() const noexcept;
    void clear() noexcept;

    // Selection
    std::shared_ptr<TreeNode> selected_node() const noexcept;
    void select_node(std::shared_ptr<TreeNode> node);
    void clear_selection() noexcept;

    // Scrolling
    float scroll_offset() const noexcept;
    void set_scroll_offset(float offset) noexcept;

    // Callbacks
    void on_selection_changed(std::function<void(std::shared_ptr<TreeNode> node)> cb);
    void on_node_expanded(std::function<void(std::shared_ptr<TreeNode> node, bool is_expanded)> cb);
    void on_node_double_clicked(std::function<void(std::shared_ptr<TreeNode> node)> cb);

    // Styling
    void set_style(const TreeStyle& style) noexcept;
    const TreeStyle& style() const noexcept;
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
    void flatten_visible_nodes(const std::vector<std::shared_ptr<TreeNode>>& nodes, int depth) const;
    void update_layout() const;

    Rect bounds_{0.0f, 0.0f, 250.0f, 400.0f};
    bool visible_{true};
    bool enabled_{true};
    std::vector<std::shared_ptr<TreeNode>> roots_;
    std::shared_ptr<TreeNode> selected_node_{nullptr};
    mutable std::vector<VisibleTreeNode> visible_rows_;
    TreeStyle style_;
    const Font* font_{nullptr};
    float scroll_y_{0.0f};
    int hovered_row_{-1};
    std::function<void(std::shared_ptr<TreeNode>)> selection_cb_;
    std::function<void(std::shared_ptr<TreeNode>, bool)> expand_cb_;
    std::function<void(std::shared_ptr<TreeNode>)> double_click_cb_;
};

} // namespace arin

#endif // ARIN32_TREE_VIEW_HPP
