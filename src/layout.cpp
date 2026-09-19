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

#include "arin/layout.hpp"
#include "arin/metrics.hpp"
#include <algorithm>
#include <iostream>

namespace arin {

namespace {

class SpacerWidget : public IWidget {
public:
    SpacerWidget(float w, float h) : m_bounds(0.0f, 0.0f, w, h) {}
    const Rect& bounds() const override { return m_bounds; }
    IWidget& set_bounds(const Rect& b) override { m_bounds = b; return *this; }
    IWidget& set_position(float x, float y) override { m_bounds.x = x; m_bounds.y = y; return *this; }
    IWidget& set_size(float w, float h) override { m_bounds.width = w; m_bounds.height = h; return *this; }
    void render(Renderer2D&) override {}
private:
    Rect m_bounds;
};

} // anonymous namespace

Layout::Layout(LayoutOrientation orientation, float x, float y, float spacing)
    : m_bounds(x, y, 0.0f, 0.0f),
      m_orientation(orientation),
      m_spacing(spacing) {}

Layout& Layout::set_bounds(const Rect& bounds) {
    m_bounds = bounds;
    m_auto_size = false;
    m_needs_layout = true;
    return *this;
}

Layout& Layout::set_position(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
    m_needs_layout = true;
    return *this;
}

Layout& Layout::set_size(float width, float height) {
    m_bounds.width = width;
    m_bounds.height = height;
    m_auto_size = false;
    m_needs_layout = true;
    return *this;
}

Layout& Layout::set_spacing(float spacing) {
    m_spacing = spacing;
    m_needs_layout = true;
    return *this;
}

Layout& Layout::set_padding(const Padding& padding) {
    m_padding = padding;
    m_needs_layout = true;
    return *this;
}

Layout& Layout::set_padding(float uniform_padding) {
    return set_padding(Padding(uniform_padding));
}

Layout& Layout::set_alignment(LayoutAlignment alignment) {
    m_alignment = alignment;
    m_needs_layout = true;
    return *this;
}

Layout& Layout::set_auto_size(bool auto_size) {
    m_auto_size = auto_size;
    m_needs_layout = true;
    return *this;
}

Layout& Layout::set_justify(LayoutJustify justify) {
    m_justify = justify;
    m_needs_layout = true;
    return *this;
}

Layout& Layout::set_validation_logging(bool enable) {
    m_validation_logging = enable;
    return *this;
}

Layout& Layout::distribute_children_equally() {
    if (m_children.empty() || m_auto_size) return *this;
    size_t n = m_children.size();
    if (m_orientation == LayoutOrientation::Horizontal) {
        float avail = m_bounds.width - m_padding.left - m_padding.right;
        float total_spacing = (n > 1) ? (n - 1) * m_spacing : 0.0f;
        float equal_w = std::max(0.0f, (avail - total_spacing) / static_cast<float>(n));
        for (auto& child : m_children) {
            child->set_size(equal_w, child->bounds().height);
        }
    } else {
        float avail = m_bounds.height - m_padding.top - m_padding.bottom;
        float total_spacing = (n > 1) ? (n - 1) * m_spacing : 0.0f;
        float equal_h = std::max(0.0f, (avail - total_spacing) / static_cast<float>(n));
        for (auto& child : m_children) {
            child->set_size(child->bounds().width, equal_h);
        }
    }
    update_layout();
    return *this;
}

std::shared_ptr<IWidget> Layout::add_widget(std::shared_ptr<IWidget> widget) {
    m_children.push_back(widget);
    m_needs_layout = true;
    update_layout();
    return widget;
}

std::shared_ptr<Button> Layout::add_button(const std::string& label, float width, float height) {
    auto btn = std::make_shared<Button>(label, 0.0f, 0.0f, width, height);
    add_widget(btn);
    return btn;
}

std::shared_ptr<ProgressBar> Layout::add_progress_bar(float width, float height, float value) {
    auto bar = std::make_shared<ProgressBar>(0.0f, 0.0f, width, height, value);
    add_widget(bar);
    return bar;
}

std::shared_ptr<ListBox> Layout::add_list_box(float width, float height, ListBoxMode mode) {
    auto list = std::make_shared<ListBox>(0.0f, 0.0f, width, height, mode);
    add_widget(list);
    return list;
}

std::shared_ptr<CheckListBox> Layout::add_check_list_box(float width, float height) {
    auto list = std::make_shared<CheckListBox>(0.0f, 0.0f, width, height);
    add_widget(list);
    return list;
}

std::shared_ptr<VBox> Layout::add_vbox(float spacing) {
    auto vbox = std::make_shared<VBox>(0.0f, 0.0f, spacing);
    add_widget(vbox);
    return vbox;
}

std::shared_ptr<HBox> Layout::add_hbox(float spacing) {
    auto hbox = std::make_shared<HBox>(0.0f, 0.0f, spacing);
    add_widget(hbox);
    return hbox;
}

void Layout::add_spacer(float size) {
    float w = (m_orientation == LayoutOrientation::Horizontal) ? size : 0.0f;
    float h = (m_orientation == LayoutOrientation::Vertical) ? size : 0.0f;
    add_widget(std::make_shared<SpacerWidget>(w, h));
}

void Layout::clear() {
    m_children.clear();
    m_needs_layout = true;
}

std::shared_ptr<IWidget> Layout::child_at(size_t index) const {
    if (index < m_children.size()) {
        return m_children[index];
    }
    return nullptr;
}

void Layout::update_layout() {
    float start_x = m_bounds.x + m_padding.left;
    float start_y = m_bounds.y + m_padding.top;

    if (m_orientation == LayoutOrientation::Vertical) {
        // Compute cross-axis (horizontal) content width
        float max_child_w = 0.0f;
        for (const auto& child : m_children) {
            max_child_w = std::max(max_child_w, child->bounds().width);
        }

        float content_w = max_child_w;
        if (!m_auto_size && m_bounds.width > (m_padding.left + m_padding.right)) {
            content_w = m_bounds.width - m_padding.left - m_padding.right;
        }

        float cursor_y = start_y;
        float step_gap = m_spacing;
        size_t n = m_children.size();
        if (!m_auto_size && m_bounds.height > (m_padding.top + m_padding.bottom) && n > 0) {
            float avail_h = m_bounds.height - m_padding.top - m_padding.bottom;
            float total_child_h = 0.0f;
            for (const auto& child : m_children) {
                total_child_h += child->bounds().height;
            }
            float free_space = avail_h - total_child_h;
            switch (m_justify) {
                case LayoutJustify::Start:
                    cursor_y = start_y;
                    step_gap = m_spacing;
                    break;
                case LayoutJustify::Center: {
                    float total_with_spacing = total_child_h + (n > 1 ? (n - 1) * m_spacing : 0.0f);
                    cursor_y = start_y + std::max(0.0f, (avail_h - total_with_spacing) * 0.5f);
                    step_gap = m_spacing;
                    break;
                }
                case LayoutJustify::End: {
                    float total_with_spacing = total_child_h + (n > 1 ? (n - 1) * m_spacing : 0.0f);
                    cursor_y = start_y + std::max(0.0f, avail_h - total_with_spacing);
                    step_gap = m_spacing;
                    break;
                }
                case LayoutJustify::SpaceBetween:
                    cursor_y = start_y;
                    step_gap = (n > 1 && free_space > 0.0f) ? (free_space / static_cast<float>(n - 1)) : m_spacing;
                    break;
                case LayoutJustify::SpaceAround:
                    step_gap = (n > 0 && free_space > 0.0f) ? (free_space / static_cast<float>(n)) : m_spacing;
                    cursor_y = start_y + step_gap * 0.5f;
                    break;
                case LayoutJustify::SpaceEvenly:
                    step_gap = (free_space > 0.0f) ? (free_space / static_cast<float>(n + 1)) : m_spacing;
                    cursor_y = start_y + step_gap;
                    break;
            }
        }

        for (auto& child : m_children) {
            float cw = child->bounds().width;
            float ch = child->bounds().height;
            float cx = start_x;

            switch (m_alignment) {
                case LayoutAlignment::Start:
                    cx = start_x;
                    break;
                case LayoutAlignment::Center:
                    cx = start_x + (content_w - cw) * 0.5f;
                    break;
                case LayoutAlignment::End:
                    cx = start_x + (content_w - cw);
                    break;
                case LayoutAlignment::Stretch:
                    cx = start_x;
                    child->set_size(content_w, ch);
                    break;
            }

            child->set_position(cx, cursor_y);

            // If child is also a nested layout, recursively update it
            if (auto sub = std::dynamic_pointer_cast<Layout>(child)) {
                sub->update_layout();
                ch = sub->bounds().height;
            }

            cursor_y += ch + step_gap;
        }

        if (m_auto_size) {
            float total_h = m_children.empty()
                ? 0.0f
                : (cursor_y - start_y - step_gap);
            m_bounds.width = content_w + m_padding.left + m_padding.right;
            m_bounds.height = total_h + m_padding.top + m_padding.bottom;
        }
    } else {
        // Horizontal Layout
        float max_child_h = 0.0f;
        for (const auto& child : m_children) {
            max_child_h = std::max(max_child_h, child->bounds().height);
        }

        float content_h = max_child_h;
        if (!m_auto_size && m_bounds.height > (m_padding.top + m_padding.bottom)) {
            content_h = m_bounds.height - m_padding.top - m_padding.bottom;
        }

        float cursor_x = start_x;
        float step_gap = m_spacing;
        size_t n = m_children.size();
        if (!m_auto_size && m_bounds.width > (m_padding.left + m_padding.right) && n > 0) {
            float avail_w = m_bounds.width - m_padding.left - m_padding.right;
            float total_child_w = 0.0f;
            for (const auto& child : m_children) {
                total_child_w += child->bounds().width;
            }
            float free_space = avail_w - total_child_w;
            switch (m_justify) {
                case LayoutJustify::Start:
                    cursor_x = start_x;
                    step_gap = m_spacing;
                    break;
                case LayoutJustify::Center: {
                    float total_with_spacing = total_child_w + (n > 1 ? (n - 1) * m_spacing : 0.0f);
                    cursor_x = start_x + std::max(0.0f, (avail_w - total_with_spacing) * 0.5f);
                    step_gap = m_spacing;
                    break;
                }
                case LayoutJustify::End: {
                    float total_with_spacing = total_child_w + (n > 1 ? (n - 1) * m_spacing : 0.0f);
                    cursor_x = start_x + std::max(0.0f, avail_w - total_with_spacing);
                    step_gap = m_spacing;
                    break;
                }
                case LayoutJustify::SpaceBetween:
                    cursor_x = start_x;
                    step_gap = (n > 1 && free_space > 0.0f) ? (free_space / static_cast<float>(n - 1)) : m_spacing;
                    break;
                case LayoutJustify::SpaceAround:
                    step_gap = (n > 0 && free_space > 0.0f) ? (free_space / static_cast<float>(n)) : m_spacing;
                    cursor_x = start_x + step_gap * 0.5f;
                    break;
                case LayoutJustify::SpaceEvenly:
                    step_gap = (free_space > 0.0f) ? (free_space / static_cast<float>(n + 1)) : m_spacing;
                    cursor_x = start_x + step_gap;
                    break;
            }
        }

        for (auto& child : m_children) {
            float cw = child->bounds().width;
            float ch = child->bounds().height;
            float cy = start_y;

            switch (m_alignment) {
                case LayoutAlignment::Start:
                    cy = start_y;
                    break;
                case LayoutAlignment::Center:
                    cy = start_y + (content_h - ch) * 0.5f;
                    break;
                case LayoutAlignment::End:
                    cy = start_y + (content_h - ch);
                    break;
                case LayoutAlignment::Stretch:
                    cy = start_y;
                    child->set_size(cw, content_h);
                    break;
            }

            child->set_position(cursor_x, cy);

            // If child is also a nested layout, recursively update it
            if (auto sub = std::dynamic_pointer_cast<Layout>(child)) {
                sub->update_layout();
                cw = sub->bounds().width;
            }

            cursor_x += cw + step_gap;
        }

        if (m_auto_size) {
            float total_w = m_children.empty()
                ? 0.0f
                : (cursor_x - start_x - step_gap);
            m_bounds.width = total_w + m_padding.left + m_padding.right;
            m_bounds.height = content_h + m_padding.top + m_padding.bottom;
        }
    }

    m_needs_layout = false;
    m_last_validation = validate();
    if (m_validation_logging && !m_last_validation.is_valid) {
        for (const auto& issue : m_last_validation.issues) {
            std::cerr << issue.message << "\n";
        }
    }
}

LayoutValidationResult Layout::validate() const {
    LayoutValidationResult res;
    res.is_valid = true;

    // 1. Check for overlapping sibling children
    for (size_t i = 0; i < m_children.size(); ++i) {
        for (size_t j = i + 1; j < m_children.size(); ++j) {
            const Rect& b1 = m_children[i]->bounds();
            const Rect& b2 = m_children[j]->bounds();
            if (b1.intersects(b2)) {
                res.is_valid = false;
                LayoutIssue issue;
                issue.severity = LayoutIssue::Severity::Error;
                issue.message = "[Arin32 Layout Warning] Overlap detected between child #" +
                                std::to_string(i) + " at (" + std::to_string(b1.x) + "," + std::to_string(b1.y) +
                                ") and child #" + std::to_string(j) + " at (" + std::to_string(b2.x) + "," +
                                std::to_string(b2.y) + ") in layout at (" + std::to_string(m_bounds.x) + "," +
                                std::to_string(m_bounds.y) + ")";
                res.issues.push_back(issue);
            }
        }
    }

    // 2. Check for container boundary overflow
    if (!m_auto_size && (m_bounds.width > 0.0f || m_bounds.height > 0.0f)) {
        float max_right = m_bounds.x + m_bounds.width - m_padding.right + UiMetrics::kValidationTolerance;
        float max_bottom = m_bounds.y + m_bounds.height - m_padding.bottom + UiMetrics::kValidationTolerance;

        for (size_t i = 0; i < m_children.size(); ++i) {
            const Rect& b = m_children[i]->bounds();
            if (b.x + b.width > max_right) {
                res.is_valid = false;
                LayoutIssue issue;
                issue.severity = LayoutIssue::Severity::Warning;
                issue.message = "[Arin32 Layout Warning] Child #" + std::to_string(i) +
                                " exceeds container right edge by " +
                                std::to_string((b.x + b.width) - (m_bounds.x + m_bounds.width - m_padding.right)) +
                                "px in layout at (" + std::to_string(m_bounds.x) + "," + std::to_string(m_bounds.y) + ")";
                res.issues.push_back(issue);
            }
            if (b.y + b.height > max_bottom) {
                res.is_valid = false;
                LayoutIssue issue;
                issue.severity = LayoutIssue::Severity::Warning;
                issue.message = "[Arin32 Layout Warning] Child #" + std::to_string(i) +
                                " exceeds container bottom edge by " +
                                std::to_string((b.y + b.height) - (m_bounds.y + m_bounds.height - m_padding.bottom)) +
                                "px in layout at (" + std::to_string(m_bounds.x) + "," + std::to_string(m_bounds.y) + ")";
                res.issues.push_back(issue);
            }
        }
    }

    return res;
}

/**
 * @brief Clears hover state on all children without synthetic coordinates.
 *
 * Sends one out-of-bounds move event so buttons and rows release their hover
 * visuals. This replaces ad-hoc magic coordinates at call sites.
 */
void Layout::clear_hover() {
    const MouseEvent offscreen = MouseEvent::make_move(Vec2(UiMetrics::kOffscreenCoordinate, UiMetrics::kOffscreenCoordinate));
    for (auto& child : m_children) {
        child->handle_mouse(offscreen);
    }
}

bool Layout::handle_mouse(const MouseEvent& ev) {
    if (m_needs_layout) {
        update_layout();
    }

    bool captured = false;
    // Traverse in reverse z-order (top-most gets priority)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handle_mouse(ev)) {
            captured = true;
            if (ev.type == MouseEventType::Move) {
                // Step 1: Release hover on siblings so only the top child stays highlighted.
                MouseEvent offscreen_ev = ev;
                offscreen_ev.position = Vec2(UiMetrics::kOffscreenCoordinate, UiMetrics::kOffscreenCoordinate);
                for (auto& other : m_children) {
                    if (other != *it) {
                        other->handle_mouse(offscreen_ev);
                    }
                }
            }
            return true;
        }
    }

    if (!captured && ev.type == MouseEventType::Move) {
        for (auto& child : m_children) {
            child->handle_mouse(ev);
        }
    }

    return false;
}

void Layout::update(float dt) {
    for (auto& child : m_children) {
        child->update(dt);
    }
}

void Layout::render(Renderer2D& renderer) {
    if (m_needs_layout) {
        update_layout();
    }

    for (auto& child : m_children) {
        child->render(renderer);
    }
}

} // namespace arin
