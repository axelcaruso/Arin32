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

/**
 * @file button_demo.cpp
 * @brief Interactive showcase demonstrating Arin32's Buttons, Modern Progress Bars,
 *        Editable TextInput (TextBox), Interactive CheckBox, Automatic Layout Containers,
 *        List Boxes, and dedicated Vector Icons & Graphics Showcase.
 */

#include <arin/arin.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <GL/glew.h>

/**
 * @brief Saves current OpenGL framebuffer to a PPM image file.
 */
static void save_screenshot_ppm(const std::string& filename, int width, int height) {
    std::vector<uint8_t> pixels(width * height * 3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << width << " " << height << "\n255\n";

    // OpenGL (0,0) is bottom-left; flip vertically for standard image orientation
    for (int y = height - 1; y >= 0; --y) {
        out.write(reinterpret_cast<const char*>(&pixels[y * width * 3]), width * 3);
    }
}

struct PlayerStats {
    std::string name;
    std::string position;
    int games;
    int goals;
};

int main(int argc, char** argv) {
    std::string screenshot_path;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--screenshot" && i + 1 < argc) {
            screenshot_path = argv[++i];
        }
    }

    // -------------------------------------------------------------------------
    // 1. Create the Application Window (1024x768 pixels)
    // -------------------------------------------------------------------------
    arin::App app("Arin32 & ArinOS - Comprehensive UI & Layout Showcase", 1024, 768);

    // Modern neutral slate grey canvas
    app.theme().background_color = arin::Color::from_hex(0xECEFF1);

    std::string global_status = "Status: Ready. Interact with text input, checkbox, lists, layouts, or buttons.";
    int action_counter = 0;

    // -------------------------------------------------------------------------
    // 2. Dialog Box with TextInput, Interactive CheckBox, and HBox Actions
    //    Demonstrates: Editable text input field, functional checkbox, and clean buttons
    // -------------------------------------------------------------------------
    const float dialog_x = 40.0f;
    const float dialog_y = 65.0f;
    const float dialog_w = 440.0f;
    const float dialog_h = 225.0f;
    const float footer_y = dialog_y + 160.0f;

    // Editable text field (TextBox / TextInput) for document title
    auto title_input = app.add_text_input(
        "annual_report_2026.docx",
        dialog_x + 24.0f,
        dialog_y + 72.0f,
        dialog_w - 48.0f,
        32.0f
    );
    title_input->set_placeholder("Enter filename...");
    title_input->on_text_changed([&](const std::string& text) {
        global_status = "Typing filename: \"" + text + "\"";
    });
    title_input->on_submit([&](const std::string& text) {
        action_counter++;
        global_status = "File saved as: \"" + text + "\" (Action #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] Filename submitted: " << text << "\n";
    });

    // Fully interactive CheckBox widget
    auto cloud_chk = app.add_checkbox(
        "Upload your content to the cloud.",
        dialog_x + 24.0f,
        dialog_y + 118.0f,
        true
    );
    cloud_chk->on_toggled([&](bool checked) {
        action_counter++;
        global_status = "Cloud Sync " + std::string(checked ? "ENABLED" : "DISABLED") +
                        " (Action #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] Cloud CheckBox toggled: " << (checked ? "ON" : "OFF") << "\n";
    });

    // Center HBox inside dialog footer (Save 85px + 10px + Don't Save 100px + 10px + Cancel 85px = 290px)
    const float dialog_actions_x = dialog_x + (dialog_w - 290.0f) * 0.5f;
    auto dialog_actions = app.add_hbox(dialog_actions_x, footer_y + 16.0f, 10.0f);

    auto save_btn = dialog_actions->add_button("Save", 85.0f, 32.0f);
    save_btn->set_style(arin::ButtonStyle::primary());
    save_btn->on_click([&]() {
        action_counter++;
        global_status = "Dialog: 'Save' clicked for file \"" + title_input->text() +
                        "\" (Action #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] 'Save' button clicked!\n";
    });

    auto dont_save_btn = dialog_actions->add_button("Don't Save", 100.0f, 32.0f);
    dont_save_btn->set_style(arin::ButtonStyle::secondary());
    dont_save_btn->on_click([&]() {
        action_counter++;
        global_status = "Dialog: 'Don't Save' clicked (Action #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] 'Don't Save' button clicked!\n";
    });

    auto cancel_btn = dialog_actions->add_button("Cancel", 85.0f, 32.0f);
    cancel_btn->set_style(arin::ButtonStyle::secondary());
    cancel_btn->on_click([&]() {
        action_counter++;
        global_status = "Dialog: 'Cancel' clicked (Action #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] 'Cancel' button clicked!\n";
    });

    dialog_actions->update_layout();

    // -------------------------------------------------------------------------
    // 3. Modern Progress Bars (Determinate & Indeterminate)
    // -------------------------------------------------------------------------
    const float pb_x = 40.0f;
    const float pb_w = 440.0f;
    const float pb_h = 18.0f;

    // Determinate Progress Bar (0 to 100 with white shimmer sweep)
    auto det_bar = app.add_progress_bar(pb_x, 320.0f, pb_w, pb_h, 68.0f, 0.0f, 100.0f);
    det_bar->set_style(arin::ProgressBarStyle::green());

    // Controls for Determinate Bar using an HBox layout
    auto pb_controls = app.add_hbox(pb_x, 348.0f, 8.0f);

    auto dec_btn = pb_controls->add_button("- 10%", 70.0f, 28.0f);
    dec_btn->set_style(arin::ButtonStyle::secondary());
    dec_btn->on_click([&]() {
        det_bar->set_value(det_bar->value() - 10.0f);
        global_status = "Progress: " + std::to_string(static_cast<int>(det_bar->percentage() * 100.0f)) + "%";
    });

    auto inc_btn = pb_controls->add_button("+ 10%", 70.0f, 28.0f);
    inc_btn->set_style(arin::ButtonStyle::secondary());
    inc_btn->on_click([&]() {
        det_bar->set_value(det_bar->value() + 10.0f);
        global_status = "Progress: " + std::to_string(static_cast<int>(det_bar->percentage() * 100.0f)) + "%";
    });

    // Auto-sized buttons with plenty of clearance so Green Style and Blue Style never touch
    auto green_btn = pb_controls->add_button("Green Style", 100.0f, 28.0f);
    green_btn->set_style(arin::ButtonStyle::secondary());

    auto blue_btn = pb_controls->add_button("Blue Style", 90.0f, 28.0f);
    blue_btn->set_style(arin::ButtonStyle::secondary());

    pb_controls->update_layout();

    // Indeterminate Progress Bar (Traveling Chunk marquee)
    auto indet_bar = app.add_progress_bar(pb_x, 405.0f, pb_w, pb_h);
    indet_bar->set_indeterminate(true);
    indet_bar->set_style(arin::ProgressBarStyle::green());

    // Clicking style switches color of BOTH determinate and indeterminate (Searching updates) bars
    green_btn->on_click([&, det_bar, indet_bar]() {
        det_bar->set_style(arin::ProgressBarStyle::green());
        indet_bar->set_style(arin::ProgressBarStyle::green());
        global_status = "Style: Classic Green (#06B025)";
    });

    blue_btn->on_click([&, det_bar, indet_bar]() {
        det_bar->set_style(arin::ProgressBarStyle::blue());
        indet_bar->set_style(arin::ProgressBarStyle::blue());
        global_status = "Style: Accent Blue (#0067C0)";
    });

    auto indet_controls = app.add_hbox(pb_x, 433.0f, 8.0f);
    auto toggle_indet_btn = indet_controls->add_button("Toggle Indeterminate", 155.0f, 28.0f);
    toggle_indet_btn->set_style(arin::ButtonStyle::secondary());
    toggle_indet_btn->on_click([&]() {
        if (indet_bar->is_indeterminate()) {
            indet_bar->set_indeterminate(false);
            indet_bar->set_value(45.0f);
            global_status = "Second Bar: Determinate mode (45%)";
        } else {
            indet_bar->set_indeterminate(true);
            global_status = "Second Bar: Indeterminate Traveling Chunk";
        }
    });

    indet_controls->update_layout();

    // -------------------------------------------------------------------------
    // 4. Standard List Box - "Today's roster:" (from lb-roster.png)
    // -------------------------------------------------------------------------
    const float roster_x = 544.0f;
    const float roster_y = 65.0f;
    const float roster_w = 230.0f;
    const float roster_h = 195.0f;

    const std::vector<PlayerStats> roster_data = {
        {"Anderberg, Michael", "Defense", 12, 1},
        {"Bouchard, Thomas", "Forward", 15, 8},
        {"Camp, David", "Goalkeeper", 20, 0},
        {"Haas, Jonathan", "Midfield", 18, 4},
        {"Hanif, Kerim", "Midfield", 16, 3},
        {"Jelitto, Jacek", "Forward", 14, 6},
        {"Joseph, Brad", "Defense", 17, 2},
        {"Kohl, Franz", "Midfield", 19, 5},
        {"Pai, Jyothi", "Forward", 11, 7},
        {"Raposo, Rui", "Defense", 13, 0}
    };

    auto roster_list = app.add_list_box(roster_x, roster_y + 25.0f, roster_w, roster_h);
    for (const auto& p : roster_data) {
        roster_list->add_item(p.name);
    }
    roster_list->set_selected_index(3);

    PlayerStats current_player = roster_data[3];

    roster_list->on_selection_changed([&](int index, const std::string& name) {
        if (index >= 0 && index < static_cast<int>(roster_data.size())) {
            current_player = roster_data[index];
            global_status = "Roster Selected: " + name + " (" + current_player.position + ")";
        }
    });

    // OK Button for Roster Dialog
    auto roster_ok_btn = app.add_button("OK", roster_x + 255.0f, roster_y + 175.0f, 85.0f, 32.0f);
    roster_ok_btn->set_style(arin::ButtonStyle::secondary());
    roster_ok_btn->on_click([&]() {
        global_status = "Roster Confirmed: " + current_player.name;
    });

    // -------------------------------------------------------------------------
    // 5. CheckBox List - "System Features" (Interactive multi-item options)
    // -------------------------------------------------------------------------
    const float chk_list_x = 544.0f;
    const float chk_list_y = 295.0f;
    const float chk_list_w = 440.0f;
    const float chk_list_h = 165.0f;

    auto features_list = app.add_check_list_box(chk_list_x, chk_list_y + 25.0f, chk_list_w, chk_list_h);
    features_list->add_item("Modern C++ Runtime & Toolchain", true);
    features_list->add_item("Directory & Authentication Services", false);
    features_list->add_item("Hardware Virtualization Hypervisor", true);
    features_list->add_item("HTTP Web Server & Networking Daemon", true);
    features_list->add_item("IPC Message Queue Service", false);
    features_list->add_item("Print and Document Spooler", true);
    features_list->add_item("High-Performance Data Compression", false);
    features_list->add_item("IP Routing & Packet Filtering Tools", false);
    features_list->add_item("Network File System (NFS) Client", false);
    features_list->add_item("SNMP Monitoring Daemon", false);
    features_list->add_item("SSH & Remote Terminal Client", true);
    features_list->add_item("TFTP File Transfer Client", false);
    features_list->add_item("Command Line Shell & Automation Engine", true);
    features_list->add_item("POSIX Compatibility Subsystem", true);

    features_list->on_item_toggled([&](int idx, bool checked) {
        std::string name = features_list->item_text(idx);
        global_status = "Feature " + (checked ? std::string("Enabled: ") : std::string("Disabled: ")) + name;
    });

    // -------------------------------------------------------------------------
    // 6. Action Buttons Palette (Clean, No Forced Icons, Symmetrical Padding)
    //    Card width: 440px. 4 buttons @ 94px + 3 gaps @ 10px = 406px.
    //    Symmetrical margins: (440 - 406) / 2 = 17px left & right.
    // -------------------------------------------------------------------------
    const float bottom_y = 485.0f;
    const float palette_x = 40.0f + 17.0f; // 57.0f (exact 17px card padding)
    const float palette_btn_w = 94.0f;

    // Row 1 of Palette: Primary, Secondary, Success, Danger
    auto palette_row1 = app.add_hbox(palette_x, bottom_y + 28.0f, 10.0f);

    auto prim_b = palette_row1->add_button("Primary", palette_btn_w, 32.0f);
    prim_b->set_style(arin::ButtonStyle::primary());
    prim_b->on_click([&]() { global_status = "Palette: Primary button clicked"; });

    auto sec_b = palette_row1->add_button("Secondary", palette_btn_w, 32.0f);
    sec_b->set_style(arin::ButtonStyle::secondary());
    sec_b->on_click([&]() { global_status = "Palette: Secondary button clicked"; });

    auto succ_b = palette_row1->add_button("Success", palette_btn_w, 32.0f);
    succ_b->set_style(arin::ButtonStyle::success());
    succ_b->on_click([&]() { global_status = "Palette: Success button clicked"; });

    auto dang_b = palette_row1->add_button("Danger", palette_btn_w, 32.0f);
    dang_b->set_style(arin::ButtonStyle::danger());
    dang_b->on_click([&]() { global_status = "Palette: Danger button clicked"; });

    palette_row1->update_layout();

    // Row 2 of Palette: Outline, Disabled, Exit Demo
    auto palette_row2 = app.add_hbox(palette_x, bottom_y + 70.0f, 10.0f);

    auto outl_b = palette_row2->add_button("Outline", palette_btn_w, 32.0f);
    outl_b->set_style(arin::ButtonStyle::outline(arin::Color::from_hex(0x0067C0)));
    outl_b->on_click([&]() { global_status = "Palette: Outline button clicked"; });

    auto dis_b = palette_row2->add_button("Disabled", palette_btn_w, 32.0f);
    dis_b->set_style(arin::ButtonStyle::secondary()).set_enabled(false);

    auto exit_b = palette_row2->add_button("Exit Demo", palette_btn_w, 32.0f);
    exit_b->set_style(arin::ButtonStyle::secondary());
    exit_b->on_click([&]() {
        std::cout << "[Arin32 Event] Closing demo.\n";
        app.close();
    });

    palette_row2->update_layout();

    // -------------------------------------------------------------------------
    // 7. Dedicated Vector Icons & Graphics Showcase (Right Column Bottom)
    // -------------------------------------------------------------------------
    const float showcase_x = 510.0f;
    const float showcase_y = 485.0f;
    const float showcase_w = 474.0f;
    const float showcase_h = 155.0f;

    // Load sample image thumbnail with GPU SDF corner rounding
    auto sample_img = app.add_image(
        "button-dialog-example.png",
        showcase_x + 16.0f,
        showcase_y + 34.0f,
        110.0f,
        100.0f,
        arin::ImageScaleMode::Fit
    );
    sample_img->set_corner_radius(5.0f);

    // -------------------------------------------------------------------------
    // 8. Custom Frame Callback: Cards, Shadows, Labels, and Visual Styling
    // -------------------------------------------------------------------------
    app.on_frame([&](arin::Renderer2D& r) {
        // App Title Banner
        r.draw_text(
            "Arin32 & ArinOS - Comprehensive Graphical Library Showcase",
            arin::Vec2(40.0f, 16.0f),
            arin::Color::from_hex(0x1F2937),
            1.25f
        );

        r.draw_text(
            "Editable TextInput, Interactive CheckBox, Modern Progress Bars, List Boxes, and Dedicated Icon Showcase",
            arin::Vec2(40.0f, 40.0f),
            arin::Color::from_hex(0x4B5563),
            0.88f
        );

        // --- Dialog Card Container (Left) ---
        r.draw_shadow(
            arin::Rect(dialog_x, dialog_y, dialog_w, dialog_h),
            6.0f,
            arin::Color(0.0f, 0.0f, 0.0f, 0.15f),
            arin::Vec2(0.0f, 3.0f),
            10.0f
        );

        r.draw_rounded_rect(
            arin::Rect(dialog_x, dialog_y, dialog_w, dialog_h),
            6.0f,
            arin::Color::white(),
            arin::Color::from_hex(0xD1D5DB),
            1.0f
        );

        // Info vector icon badge in dialog
        r.draw_icon(
            arin::IconType::Info,
            arin::Rect(dialog_x + 24.0f, dialog_y + 18.0f, 22.0f, 22.0f),
            arin::Color::from_hex(0x0067C0)
        );

        r.draw_text(
            "Save your work?",
            arin::Vec2(dialog_x + 54.0f, dialog_y + 18.0f),
            arin::Color::from_hex(0x111827),
            1.2f
        );

        r.draw_text(
            "File name and cloud backup options:",
            arin::Vec2(dialog_x + 24.0f, dialog_y + 48.0f),
            arin::Color::from_hex(0x4B5563),
            0.90f
        );

        // Divider separating dialog body from footer (strictly inside card border)
        r.draw_rect(
            arin::Rect(dialog_x + 1.0f, footer_y, dialog_w - 2.0f, 1.0f),
            arin::Color::from_hex(0xE5E7EB)
        );

        // Dialog Footer Background (#F9FAFB) strictly inside the card border
        r.draw_rounded_rect(
            arin::Rect(dialog_x + 1.0f, footer_y + 1.0f, dialog_w - 2.0f, dialog_h - 162.0f),
            5.0f,
            arin::Color::from_hex(0xF9FAFB)
        );
        // Square off top corners of footer below divider
        r.draw_rect(
            arin::Rect(dialog_x + 1.0f, footer_y + 1.0f, dialog_w - 2.0f, 12.0f),
            arin::Color::from_hex(0xF9FAFB)
        );

        // --- Progress Bars Section Labels ---
        std::string det_label = "Copying items... (" +
                                std::to_string(static_cast<int>(det_bar->percentage() * 100.0f)) +
                                "% - Determinate with Shimmer Sweep)";
        r.draw_text(
            det_label,
            arin::Vec2(pb_x, 302.0f),
            arin::Color::from_hex(0x1F2937),
            0.90f
        );

        r.draw_text(
            "Searching for updates... (Indeterminate Traveling Marquee Chunk)",
            arin::Vec2(pb_x, 386.0f),
            arin::Color::from_hex(0x1F2937),
            0.90f
        );

        // --- Roster Section Labels & Details Card (Right) ---
        r.draw_text(
            "Today's roster: (Standard ListBox with smooth scrollbar)",
            arin::Vec2(roster_x, roster_y + 4.0f),
            arin::Color::from_hex(0x111827),
            0.95f
        );

        // Player Details Card
        const float info_x = roster_x + 250.0f;
        const float info_y = roster_y + 25.0f;
        const float info_w = 190.0f;
        const float info_h = 135.0f;

        r.draw_rounded_rect(
            arin::Rect(info_x, info_y, info_w, info_h),
            4.0f,
            arin::Color::white(),
            arin::Color::from_hex(0xD1D5DB),
            1.0f
        );

        r.draw_text(
            "Player Details",
            arin::Vec2(info_x + 12.0f, info_y + 10.0f),
            arin::Color::from_hex(0x0067C0),
            1.0f
        );

        r.draw_text(
            "Position: " + current_player.position,
            arin::Vec2(info_x + 12.0f, info_y + 36.0f),
            arin::Color::from_hex(0x374151),
            0.90f
        );

        r.draw_text(
            "Games played: " + std::to_string(current_player.games),
            arin::Vec2(info_x + 12.0f, info_y + 60.0f),
            arin::Color::from_hex(0x374151),
            0.90f
        );

        r.draw_text(
            "Goals: " + std::to_string(current_player.goals),
            arin::Vec2(info_x + 12.0f, info_y + 84.0f),
            arin::Color::from_hex(0x374151),
            0.90f
        );

        // --- System Features Section Label ---
        r.draw_text(
            "System Features: (CheckListBox with independent item toggling)",
            arin::Vec2(chk_list_x, chk_list_y + 4.0f),
            arin::Color::from_hex(0x111827),
            0.95f
        );

        // --- Action Buttons Section Card (Bottom Left) ---
        r.draw_rounded_rect(
            arin::Rect(40.0f, bottom_y, 440.0f, showcase_h),
            6.0f,
            arin::Color::white(),
            arin::Color::from_hex(0xD1D5DB),
            1.0f
        );

        r.draw_text(
            "Action Buttons Palette (Clean, No Forced Icons):",
            arin::Vec2(palette_x, bottom_y + 8.0f),
            arin::Color::from_hex(0x111827),
            0.92f
        );

        r.draw_text(
            "Status: Standard button styles for dialogs, forms, and toolbars",
            arin::Vec2(palette_x, bottom_y + 112.0f),
            arin::Color::from_hex(0x6B7280),
            0.82f
        );

        // --- Dedicated Vector Icons & Graphics Showcase Card (Bottom Right) ---
        r.draw_rounded_rect(
            arin::Rect(showcase_x, showcase_y, showcase_w, showcase_h),
            6.0f,
            arin::Color::white(),
            arin::Color::from_hex(0xD1D5DB),
            1.0f
        );

        r.draw_text(
            "Sample Icons & Graphics Showcase (GPU Vector Glyphs):",
            arin::Vec2(showcase_x + 16.0f, showcase_y + 8.0f),
            arin::Color::from_hex(0x111827),
            0.92f
        );

        r.draw_text(
            "Loaded Image",
            arin::Vec2(showcase_x + 32.0f, showcase_y + 136.0f),
            arin::Color::from_hex(0x6B7280),
            0.78f
        );

        // Draw crisp sample vector icons in a dedicated grid
        const float icon_grid_x = showcase_x + 145.0f;
        const float icon_grid_y = showcase_y + 36.0f;
        const float icon_step = 44.0f;

        const std::vector<std::pair<arin::IconType, std::string>> sample_icons_row1 = {
            {arin::IconType::Folder, "Folder"},
            {arin::IconType::File, "File"},
            {arin::IconType::Settings, "Settings"},
            {arin::IconType::Search, "Search"},
            {arin::IconType::Check, "Check"},
            {arin::IconType::Close, "Close"},
            {arin::IconType::Trash, "Trash"}
        };

        const std::vector<std::pair<arin::IconType, std::string>> sample_icons_row2 = {
            {arin::IconType::Info, "Info"},
            {arin::IconType::Warning, "Warning"},
            {arin::IconType::Error, "Error"},
            {arin::IconType::Cut, "Cut"},
            {arin::IconType::Copy, "Copy"},
            {arin::IconType::Paste, "Paste"},
            {arin::IconType::Edit, "Edit"}
        };

        // Render Row 1
        for (size_t i = 0; i < sample_icons_row1.size(); ++i) {
            float ix = icon_grid_x + i * icon_step;
            float iy = icon_grid_y;

            // Subtle badge background
            r.draw_rounded_rect(
                arin::Rect(ix, iy, 34.0f, 34.0f),
                4.0f,
                arin::Color::from_hex(0xF3F4F6),
                arin::Color::from_hex(0xE5E7EB),
                1.0f
            );

            r.draw_icon(
                sample_icons_row1[i].first,
                arin::Rect(ix + 8.0f, iy + 8.0f, 18.0f, 18.0f),
                arin::Color::from_hex(0x0067C0)
            );

            r.draw_text(
                sample_icons_row1[i].second,
                arin::Vec2(ix + 2.0f, iy + 36.0f),
                arin::Color::from_hex(0x6B7280),
                0.65f
            );
        }

        // Render Row 2
        for (size_t i = 0; i < sample_icons_row2.size(); ++i) {
            float ix = icon_grid_x + i * icon_step;
            float iy = icon_grid_y + 52.0f;

            r.draw_rounded_rect(
                arin::Rect(ix, iy, 34.0f, 34.0f),
                4.0f,
                arin::Color::from_hex(0xF3F4F6),
                arin::Color::from_hex(0xE5E7EB),
                1.0f
            );

            r.draw_icon(
                sample_icons_row2[i].first,
                arin::Rect(ix + 8.0f, iy + 8.0f, 18.0f, 18.0f),
                arin::Color::from_hex(0x1F2937)
            );

            r.draw_text(
                sample_icons_row2[i].second,
                arin::Vec2(ix + 2.0f, iy + 36.0f),
                arin::Color::from_hex(0x6B7280),
                0.65f
            );
        }

        // --- Interactive Status Bar ---
        r.draw_rounded_rect(
            arin::Rect(40.0f, 655.0f, static_cast<float>(r.viewport_width()) - 80.0f, 32.0f),
            4.0f,
            arin::Color::white(),
            arin::Color::from_hex(0xD1D5DB),
            1.0f
        );

        r.draw_text(
            global_status,
            arin::Vec2(52.0f, 663.0f),
            arin::Color::from_hex(0x111827),
            0.92f
        );

        // Footer copyright info
        r.draw_text_centered(
            "Copyright (c) 2026, Arin32 & ArinOS Contributors * BSD 2-Clause License * C++17 OpenGL",
            arin::Rect(0.0f, 715.0f, static_cast<float>(r.viewport_width()), 20.0f),
            arin::Color::from_hex(0x6B7280),
            0.85f
        );
    });

    // If requested, take an automated screenshot after widgets render and close
    if (!screenshot_path.empty()) {
        int captured_frames = 0;
        app.on_after_frame([&](arin::Renderer2D& r) {
            captured_frames++;
            det_bar->set_anim_phase(0.40f);
            indet_bar->set_anim_phase(0.45f);

            if (captured_frames >= 3) {
                save_screenshot_ppm(screenshot_path, r.viewport_width(), r.viewport_height());
                std::cout << "[Arin32] Frame rendered! Screenshot saved to " << screenshot_path << std::endl;
                app.close();
            }
        });
    }

    std::cout << "==========================================================" << std::endl;
    std::cout << " Arin32 & ArinOS Comprehensive Showcase started." << std::endl;
    std::cout << " OS: Linux / FreeBSD agnostic" << std::endl;
    std::cout << " License: BSD 2-Clause" << std::endl;
    std::cout << " Contributors: Arin32 & ArinOS Contributors" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // 9. Run the Application Main Loop
    // -------------------------------------------------------------------------
    app.run();

    return 0;
}
