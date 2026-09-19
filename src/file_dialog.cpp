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

#include "arin/file_dialog.hpp"
#include "arin/renderer.hpp"
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

namespace arin {

static std::string format_file_size(uintmax_t bytes) {
    if (bytes == 0) {
        return "-";
    }
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int s = 0;
    double count = static_cast<double>(bytes);
    while (count >= 1024.0 && s < 4) {
        s++;
        count /= 1024.0;
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << count << " " << suffixes[s];
    return ss.str();
}

FileDialog::FileDialog(FileDialogMode mode)
    : bounds_{100.0f, 60.0f, 720.0f, 480.0f}, mode_(mode) {
    if (mode_ == FileDialogMode::SaveFile) {
        title_ = "Save File";
    } else if (mode_ == FileDialogMode::SelectFolder) {
        title_ = "Select Folder";
    } else {
        title_ = "Open File";
    }
    init_components();
}

FileDialog::FileDialog(const Rect& bounds, FileDialogMode mode)
    : bounds_(bounds), mode_(mode) {
    if (mode_ == FileDialogMode::SaveFile) {
        title_ = "Save File";
    } else if (mode_ == FileDialogMode::SelectFolder) {
        title_ = "Select Folder";
    } else {
        title_ = "Open File";
    }
    init_components();
}

void FileDialog::init_components() {
    try {
        current_dir_ = fs::current_path().string();
    } catch (...) {
        current_dir_ = "/";
    }

    file_table_ = std::make_unique<TableView>();
    file_table_->add_column("Name", 320.0f);
    file_table_->add_column("Size", 90.0f);
    file_table_->add_column("Type", 110.0f);

    path_input_ = std::make_unique<TextInput>(current_dir_);
    filename_input_ = std::make_unique<TextInput>("");

    up_button_ = std::make_unique<Button>("Up");
    up_button_->on_click([this]() { navigate_up(); });

    std::string accept_text = (mode_ == FileDialogMode::SaveFile) ? "Save" : "Open";
    accept_button_ = std::make_unique<Button>(accept_text);
    accept_button_->on_click([this]() {
        const std::string sel = selected_path();
        if (!sel.empty()) {
            if (accept_cb_) {
                accept_cb_(sel);
            }
            hide();
        }
    });

    cancel_button_ = std::make_unique<Button>("Cancel");
    cancel_button_->on_click([this]() {
        if (cancel_cb_) {
            cancel_cb_();
        }
        hide();
    });

    // Wire table selection
    file_table_->on_row_selected([this](int row_idx) {
        if (row_idx >= 0 && static_cast<size_t>(row_idx) < entries_.size()) {
            const auto& entry = entries_[static_cast<size_t>(row_idx)];
            if (!entry.is_directory) {
                filename_input_->set_text(entry.name);
            }
        }
    });

    // Wire double-click to navigate folders
    file_table_->on_row_double_clicked([this](int row_idx) {
        if (row_idx >= 0 && static_cast<size_t>(row_idx) < entries_.size()) {
            const auto& entry = entries_[static_cast<size_t>(row_idx)];
            if (entry.is_directory) {
                fs::path p(current_dir_);
                p /= entry.name;
                set_directory(p.lexically_normal().string());
            } else {
                filename_input_->set_text(entry.name);
                if (accept_cb_) {
                    accept_cb_(selected_path());
                }
                hide();
            }
        }
    });

    populate_directory_contents();
}

void FileDialog::set_mode(FileDialogMode mode) noexcept {
    if (title_ == "Open File" || title_ == "Save File" || title_ == "Select Folder") {
        if (mode == FileDialogMode::SaveFile) {
            title_ = "Save File";
        } else if (mode == FileDialogMode::SelectFolder) {
            title_ = "Select Folder";
        } else {
            title_ = "Open File";
        }
    }
    mode_ = mode;
    if (accept_button_) {
        accept_button_->set_text((mode_ == FileDialogMode::SaveFile) ? "Save" : "Open");
    }
}

FileDialogMode FileDialog::mode() const noexcept {
    return mode_;
}

void FileDialog::set_title(std::string title) {
    title_ = std::move(title);
}

const std::string& FileDialog::title() const noexcept {
    return title_;
}

void FileDialog::set_directory(const std::string& path) {
    try {
        fs::path p(path);
        if (fs::exists(p) && fs::is_directory(p)) {
            current_dir_ = fs::canonical(p).string();
            if (path_input_) {
                path_input_->set_text(current_dir_);
            }
            populate_directory_contents();
        }
    } catch (...) {
        // Fallback or ignore invalid paths
    }
}

const std::string& FileDialog::current_directory() const noexcept {
    return current_dir_;
}

void FileDialog::set_filename(const std::string& filename) {
    if (filename_input_) {
        filename_input_->set_text(filename);
    }
}

std::string FileDialog::selected_path() const {
    std::string fn = filename_input_ ? filename_input_->text() : "";
    if (fn.empty()) {
        return "";
    }
    fs::path p(current_dir_);
    p /= fn;
    return p.lexically_normal().string();
}

void FileDialog::set_extension_filter(std::string extensions) {
    filter_ = std::move(extensions);
    populate_directory_contents();
}

const std::string& FileDialog::extension_filter() const noexcept {
    return filter_;
}

void FileDialog::show() {
    is_open_ = true;
    visible_ = true;
    populate_directory_contents();
}

void FileDialog::hide() noexcept {
    is_open_ = false;
    visible_ = false;
}

bool FileDialog::is_open() const noexcept {
    return is_open_;
}

void FileDialog::on_accept(std::function<void(const std::string&)> cb) {
    accept_cb_ = std::move(cb);
}

void FileDialog::on_cancel(std::function<void()> cb) {
    cancel_cb_ = std::move(cb);
}

void FileDialog::set_style(const FileDialogStyle& style) noexcept {
    style_ = style;
}

const FileDialogStyle& FileDialog::style() const noexcept {
    return style_;
}

void FileDialog::set_font(const Font* font) noexcept {
    font_ = font;
    if (file_table_) file_table_->set_font(font);
}

const Font* FileDialog::font() const noexcept {
    return font_;
}

void FileDialog::navigate_up() {
    try {
        fs::path p(current_dir_);
        if (p.has_parent_path()) {
            set_directory(p.parent_path().string());
        }
    } catch (...) {
    }
}

void FileDialog::refresh() {
    populate_directory_contents();
}

void FileDialog::populate_directory_contents() {
    entries_.clear();
    if (file_table_) {
        file_table_->clear_rows();
    }

    try {
        fs::path p(current_dir_);
        if (!fs::exists(p) || !fs::is_directory(p)) {
            return;
        }

        std::vector<FileEntry> dirs;
        std::vector<FileEntry> files;

        for (const auto& entry : fs::directory_iterator(p, fs::directory_options::skip_permission_denied)) {
            std::string name = entry.path().filename().string();
            if (name.empty() || name[0] == '.') {
                continue; // Skip hidden by default
            }

            FileEntry fe;
            fe.name = name;
            fe.is_directory = entry.is_directory();

            if (fe.is_directory) {
                fe.size = 0;
                dirs.push_back(fe);
            } else if (mode_ != FileDialogMode::SelectFolder) {
                try {
                    fe.size = entry.file_size();
                } catch (...) {
                    fe.size = 0;
                }
                fe.extension = entry.path().extension().string();

                // Apply filter if specified
                if (!filter_.empty() && filter_ != "*.*") {
                    if (filter_.find(fe.extension) == std::string::npos) {
                        continue;
                    }
                }
                files.push_back(fe);
            }
        }

        // Sort directories alphabetically, then files
        auto sorter = [](const FileEntry& a, const FileEntry& b) {
            return a.name < b.name;
        };
        std::sort(dirs.begin(), dirs.end(), sorter);
        std::sort(files.begin(), files.end(), sorter);

        for (auto& d : dirs) {
            entries_.push_back(d);
            if (file_table_) {
                file_table_->add_row({"[Folder] " + d.name, "-", "File Folder"});
            }
        }

        for (auto& f : files) {
            entries_.push_back(f);
            if (file_table_) {
                file_table_->add_row({f.name, format_file_size(f.size), f.extension + " File"});
            }
        }
    } catch (...) {
    }
}

void FileDialog::update_layout() const {
    const float pad = 10.0f;
    const float content_x = bounds_.x + pad;
    const float content_w = bounds_.width - (pad * 2.0f);

    // 1. Path bar at top
    const float top_bar_y = bounds_.y + style_.titlebar_height + pad;
    const float up_w = 48.0f;
    const float input_h = 28.0f;

    if (up_button_) {
        up_button_->set_bounds(Rect{content_x, top_bar_y, up_w, input_h});
    }
    if (path_input_) {
        path_input_->set_bounds(Rect{content_x + up_w + 6.0f, top_bar_y, content_w - up_w - 6.0f, input_h});
    }

    // 2. Table view in center
    const float table_y = top_bar_y + input_h + pad;
    const float bottom_y = bounds_.y + bounds_.height - style_.bottom_bar_height;
    const float table_h = bottom_y - table_y - pad;

    if (file_table_) {
        file_table_->set_bounds(Rect{content_x, table_y, content_w, table_h});
    }

    // 3. Bottom bar controls (Filename, Open/Save, Cancel)
    const float btn_w = 80.0f;
    const float btn_h = 30.0f;
    const float btn_y = bottom_y + (style_.bottom_bar_height - btn_h) * 0.5f;

    if (cancel_button_) {
        cancel_button_->set_bounds(Rect{bounds_.x + bounds_.width - pad - btn_w, btn_y, btn_w, btn_h});
    }
    if (accept_button_) {
        accept_button_->set_bounds(Rect{bounds_.x + bounds_.width - pad - (btn_w * 2.0f) - 8.0f, btn_y, btn_w, btn_h});
    }
    if (filename_input_) {
        const float fn_w = content_w - (btn_w * 2.0f) - 24.0f;
        filename_input_->set_bounds(Rect{content_x, btn_y, std::max(120.0f, fn_w), btn_h});
    }
}

void FileDialog::render(Renderer2D& renderer) {
    if (!visible_ || !is_open_) {
        return;
    }

    update_layout();

    // Dialog window drop shadow & main background
    renderer.draw_rounded_rect(bounds_, style_.corner_radius, style_.window_background, style_.border_color, style_.border_width);

    // Titlebar
    Rect title_rect{bounds_.x, bounds_.y, bounds_.width, style_.titlebar_height};
    renderer.draw_rounded_rect(title_rect, style_.corner_radius, style_.titlebar_background, style_.border_color, 1.0f);

    const float title_x = bounds_.x + 12.0f;
    const float title_y = bounds_.y + (style_.titlebar_height - 14.0f) * 0.5f;
    renderer.draw_text(title_, Vec2{title_x, title_y}, style_.text_color, 1.0f);

    // Render components
    if (up_button_) up_button_->render(renderer);
    if (path_input_) path_input_->render(renderer);
    if (file_table_) file_table_->render(renderer);
    if (filename_input_) filename_input_->render(renderer);
    if (accept_button_) accept_button_->render(renderer);
    if (cancel_button_) cancel_button_->render(renderer);
}

bool FileDialog::handle_mouse(const MouseEvent& ev) {
    if (!visible_ || !is_open_) {
        return false;
    }
    update_layout();

    bool handled = false;
    if (up_button_) handled |= up_button_->handle_mouse(ev);
    if (path_input_) handled |= path_input_->handle_mouse(ev);
    if (file_table_) handled |= file_table_->handle_mouse(ev);
    if (filename_input_) handled |= filename_input_->handle_mouse(ev);
    if (accept_button_) handled |= accept_button_->handle_mouse(ev);
    if (cancel_button_) handled |= cancel_button_->handle_mouse(ev);

    return handled || bounds_.contains(ev.position);
}

bool FileDialog::handle_mouse_move(const Vec2& mouse_pos) {
    MouseEvent ev = MouseEvent::make_move(mouse_pos);
    return handle_mouse(ev);
}

bool FileDialog::handle_mouse_button(MouseButton button, InputAction action, const Vec2& mouse_pos) {
    MouseEvent ev = (action == InputAction::Press)
        ? MouseEvent::make_button_down(mouse_pos, button)
        : MouseEvent::make_button_up(mouse_pos, button);
    return handle_mouse(ev);
}

bool FileDialog::handle_scroll(const Vec2& offset) {
    if (!visible_ || !is_open_) {
        return false;
    }
    if (file_table_) {
        return file_table_->handle_scroll(offset);
    }
    return false;
}

bool FileDialog::handle_key(const KeyEvent& ev) {
    if (!visible_ || !is_open_) {
        return false;
    }
    if (ev.action == InputAction::Press && ev.key == KeyCode::Escape) {
        hide();
        if (cancel_cb_) {
            cancel_cb_();
        }
        return true;
    }
    if (ev.action == InputAction::Press && ev.key == KeyCode::Enter) {
        const std::string sel = selected_path();
        if (!sel.empty()) {
            if (accept_cb_) {
                accept_cb_(sel);
            }
            hide();
            return true;
        }
    }

    if (filename_input_ && filename_input_->handle_key(ev)) return true;
    if (path_input_ && path_input_->handle_key(ev)) return true;
    if (file_table_ && file_table_->handle_key(ev)) return true;

    return false;
}

bool FileDialog::handle_key_event(KeyCode key, InputAction action, KeyModifier mods) {
    KeyEvent ev;
    ev.key = key;
    ev.action = action;
    ev.modifiers = static_cast<uint8_t>(mods);
    return handle_key(ev);
}

bool FileDialog::handle_text(const TextEvent& ev) {
    if (!visible_ || !is_open_) {
        return false;
    }
    if (filename_input_ && filename_input_->handle_text(ev)) return true;
    if (path_input_ && path_input_->handle_text(ev)) return true;
    return false;
}

bool FileDialog::handle_text_string(const std::string& text) {
    TextEvent ev;
    ev.text = text;
    return handle_text(ev);
}

} // namespace arin
