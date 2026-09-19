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

#ifndef ARIN32_FILE_DIALOG_HPP
#define ARIN32_FILE_DIALOG_HPP

#include "widget.hpp"
#include "types.hpp"
#include "button.hpp"
#include "text_input.hpp"
#include "table_view.hpp"
#include "font.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace arin {

/**
 * @brief Operating modes for FileDialog.
 */
enum class FileDialogMode {
    OpenFile,
    SaveFile,
    SelectFolder
};

/**
 * @brief Visual styling configuration for FileDialog.
 */
struct FileDialogStyle {
    Color window_background{0.18f, 0.18f, 0.20f, 1.0f};
    Color titlebar_background{0.14f, 0.14f, 0.16f, 1.0f};
    Color sidebar_background{0.16f, 0.16f, 0.17f, 1.0f};
    Color border_color{0.26f, 0.26f, 0.30f, 1.0f};
    Color text_color{0.92f, 0.92f, 0.95f, 1.0f};
    float corner_radius{6.0f};
    float border_width{1.0f};
    float titlebar_height{32.0f};
    float sidebar_width{140.0f};
    float bottom_bar_height{48.0f};
};

/**
 * @brief Modern desktop file dialog modal for browsing, opening, and saving files and directories.
 */
class FileDialog : public IWidget {
public:
    explicit FileDialog(FileDialogMode mode = FileDialogMode::OpenFile);
    FileDialog(const Rect& bounds, FileDialogMode mode = FileDialogMode::OpenFile);
    ~FileDialog() override = default;

    // Mode and path configuration
    void set_mode(FileDialogMode mode) noexcept;
    FileDialogMode mode() const noexcept;
    void set_title(std::string title);
    const std::string& title() const noexcept;

    void set_directory(const std::string& path);
    const std::string& current_directory() const noexcept;
    void set_filename(const std::string& filename);
    std::string selected_path() const;

    void set_extension_filter(std::string extensions); // e.g. ".png,.jpg,.txt"
    const std::string& extension_filter() const noexcept;

    // Modal lifecycle
    void show();
    void hide() noexcept;
    bool is_open() const noexcept;

    // Callbacks
    void on_accept(std::function<void(const std::string& selected_path)> cb);
    void on_cancel(std::function<void()> cb);

    // Styling
    void set_style(const FileDialogStyle& style) noexcept;
    const FileDialogStyle& style() const noexcept;
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
    bool handle_text(const TextEvent& ev) override;

    // Convenience event helpers
    bool handle_mouse_move(const Vec2& mouse_pos);
    bool handle_mouse_button(MouseButton button, InputAction action, const Vec2& mouse_pos);
    bool handle_scroll(const Vec2& offset);
    bool handle_key_event(KeyCode key, InputAction action, KeyModifier mods);
    bool handle_text_string(const std::string& text);

    // Navigation triggers
    void refresh();
    void navigate_up();

private:
    void init_components();
    void update_layout() const;
    void populate_directory_contents();

    Rect bounds_{100.0f, 60.0f, 720.0f, 480.0f};
    bool visible_{true};
    bool enabled_{true};
    FileDialogMode mode_{FileDialogMode::OpenFile};
    std::string title_{"Open File"};
    std::string current_dir_;
    std::string filter_;
    bool is_open_{false};
    FileDialogStyle style_;
    const Font* font_{nullptr};

    // Sub-components
    std::unique_ptr<TableView> file_table_;
    std::unique_ptr<TextInput> path_input_;
    std::unique_ptr<TextInput> filename_input_;
    std::unique_ptr<Button> up_button_;
    std::unique_ptr<Button> accept_button_;
    std::unique_ptr<Button> cancel_button_;

    // Directory entry model
    struct FileEntry {
        std::string name;
        bool is_directory{false};
        uintmax_t size{0};
        std::string extension;
    };
    std::vector<FileEntry> entries_;

    std::function<void(const std::string&)> accept_cb_;
    std::function<void()> cancel_cb_;
};

} // namespace arin

#endif // ARIN32_FILE_DIALOG_HPP
