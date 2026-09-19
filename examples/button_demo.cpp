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
 * @brief Interactive showcase demonstrating Arin32's Buttons, Windows 10 Progress Bars,
 *        Automatic Layout Containers (VBox, HBox), and List Boxes (Standard & CheckBox lists).
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

    std::string global_status = "Status: Ready. Interact with lists, layouts, buttons, or progress bars.";
    int action_counter = 0;

    // -------------------------------------------------------------------------
    // 2. Dialog Box with Automatic Horizontal Layout Container (HBox)
    //    Demonstrates: No manual pixel coordinates for action buttons!
    // -------------------------------------------------------------------------
    const float dialog_x = 40.0f;
    const float dialog_y = 65.0f;
    const float dialog_w = 440.0f;
    const float dialog_h = 210.0f;
    const float footer_y = dialog_y + 145.0f;

    // Create an HBox aligned inside the dialog footer
    auto dialog_actions = app.add_hbox(dialog_x + 65.0f, footer_y + 16.0f, 10.0f);

    auto save_btn = dialog_actions->add_button("Save", 85.0f, 32.0f);
    save_btn->set_style(arin::ButtonStyle::primary());
    save_btn->on_click([&]() {
        action_counter++;
        global_status = "Dialog: 'Save' clicked (Action #" + std::to_string(action_counter) + ")";
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
    // 3. Windows 10 Progress Bars (Determinate & Indeterminate)
    // -------------------------------------------------------------------------
    const float pb_x = 40.0f;
    const float pb_w = 440.0f;
    const float pb_h = 18.0f;

    // Determinate Progress Bar (0 to 100 with white shimmer sweep)
    auto det_bar = app.add_progress_bar(pb_x, 325.0f, pb_w, pb_h, 68.0f, 0.0f, 100.0f);
    det_bar->set_style(arin::ProgressBarStyle::green());

    // Controls for Determinate Bar using an HBox layout
    auto pb_controls = app.add_hbox(pb_x, 355.0f, 8.0f);

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

    auto green_btn = pb_controls->add_button("Green Style", 90.0f, 28.0f);
    green_btn->set_style(arin::ButtonStyle::secondary());
    green_btn->on_click([&]() {
        det_bar->set_style(arin::ProgressBarStyle::green());
        global_status = "Style: Windows 10 Green (#06B025)";
    });

    auto blue_btn = pb_controls->add_button("Blue Style", 85.0f, 28.0f);
    blue_btn->set_style(arin::ButtonStyle::secondary());
    blue_btn->on_click([&]() {
        det_bar->set_style(arin::ProgressBarStyle::blue());
        global_status = "Style: Accent Blue (#0067C0)";
    });

    pb_controls->update_layout();

    // Indeterminate Progress Bar (Traveling Chunk marquee)
    auto indet_bar = app.add_progress_bar(pb_x, 420.0f, pb_w, pb_h);
    indet_bar->set_indeterminate(true);
    indet_bar->set_style(arin::ProgressBarStyle::green());

    auto indet_controls = app.add_hbox(pb_x, 450.0f, 8.0f);
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
    const float roster_x = 520.0f;
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
    // Select Jonathan Haas initially (matches lb-roster.png)
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
    // 5. CheckBox List - "Windows Features" (from ctrl-list-boxes.html)
    // -------------------------------------------------------------------------
    const float chk_list_x = 520.0f;
    const float chk_list_y = 310.0f;
    const float chk_list_w = 440.0f;
    const float chk_list_h = 175.0f;

    auto features_list = app.add_check_list_box(chk_list_x, chk_list_y + 25.0f, chk_list_w, chk_list_h);
    features_list->add_item(".NET Framework 3.5.1", true);
    features_list->add_item("Active Directory Lightweight Directory Services", false);
    features_list->add_item("Hyper-V Virtualization Platform", true);
    features_list->add_item("Internet Information Services (IIS)", true);
    features_list->add_item("Microsoft Message Queue (MSMQ) Server", false);
    features_list->add_item("Print and Document Services", true);
    features_list->add_item("Remote Differential Compression", false);
    features_list->add_item("RIP Listener & Routing Tools", false);
    features_list->add_item("Services for NFS", false);
    features_list->add_item("Simple Network Management Protocol (SNMP)", false);
    features_list->add_item("Telnet Client", true);
    features_list->add_item("TFTP Client", false);
    features_list->add_item("Windows PowerShell 2.0 Engine", true);
    features_list->add_item("Windows Subsystem for Linux (WSL)", true);

    features_list->on_item_toggled([&](int idx, bool checked) {
        std::string name = features_list->item_text(idx);
        global_status = "Feature " + (checked ? std::string("Enabled: ") : std::string("Disabled: ")) + name;
    });

    // -------------------------------------------------------------------------
    // 6. Automatic Layout Demonstration (HBox of Action Buttons + Palette)
    // -------------------------------------------------------------------------
    const float bottom_y = 540.0f;

    auto palette_row = app.add_hbox(40.0f, bottom_y + 26.0f, 10.0f);

    auto prim_b = palette_row->add_button("Primary", 100.0f, 32.0f);
    prim_b->set_style(arin::ButtonStyle::primary());
    prim_b->on_click([&]() { global_status = "Palette: Primary button clicked"; });

    auto sec_b = palette_row->add_button("Secondary", 100.0f, 32.0f);
    sec_b->set_style(arin::ButtonStyle::secondary());
    sec_b->on_click([&]() { global_status = "Palette: Secondary button clicked"; });

    auto succ_b = palette_row->add_button("Success", 100.0f, 32.0f);
    succ_b->set_style(arin::ButtonStyle::success());
    succ_b->on_click([&]() { global_status = "Palette: Success button clicked"; });

    auto dang_b = palette_row->add_button("Danger", 100.0f, 32.0f);
    dang_b->set_style(arin::ButtonStyle::danger());
    dang_b->on_click([&]() { global_status = "Palette: Danger button clicked"; });

    auto outl_b = palette_row->add_button("Outline", 100.0f, 32.0f);
    outl_b->set_style(arin::ButtonStyle::outline(arin::Color::from_hex(0x0067C0)));
    outl_b->on_click([&]() { global_status = "Palette: Outline button clicked"; });

    auto dis_b = palette_row->add_button("Disabled", 100.0f, 32.0f);
    dis_b->set_style(arin::ButtonStyle::secondary()).set_enabled(false);

    auto exit_b = palette_row->add_button("Exit Demo", 100.0f, 32.0f);
    exit_b->set_style(arin::ButtonStyle::secondary());
    exit_b->on_click([&]() {
        std::cout << "[Arin32 Event] Closing demo.\n";
        app.close();
    });

    palette_row->update_layout();

    // -------------------------------------------------------------------------
    // 7. Custom Frame Callback: Cards, Shadows, Labels, and Visual Styling
    // -------------------------------------------------------------------------
    app.on_frame([&](arin::Renderer2D& r) {
        // App Title Banner
        r.draw_text(
            "Arin32 & ArinOS - Complete Graphical Library & Layout Engine",
            arin::Vec2(40.0f, 16.0f),
            arin::Color::from_hex(0x1F2937),
            1.25f
        );

        r.draw_text(
            "Buttons, Progress Bars (Determinate/Indeterminate), Layout Containers (VBox/HBox), ListBox & CheckListBox",
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

        r.draw_text(
            "Save your work?",
            arin::Vec2(dialog_x + 24.0f, dialog_y + 20.0f),
            arin::Color::from_hex(0x111827),
            1.2f
        );

        r.draw_text(
            "HBox container aligns the dialog buttons below automatically.",
            arin::Vec2(dialog_x + 24.0f, dialog_y + 54.0f),
            arin::Color::from_hex(0x4B5563),
            0.90f
        );

        // Checkbox sample inside card
        const float c_chk_x = dialog_x + 24.0f;
        const float c_chk_y = dialog_y + 88.0f;
        r.draw_rounded_rect(
            arin::Rect(c_chk_x, c_chk_y, 16.0f, 16.0f),
            3.0f,
            arin::Color::white(),
            arin::Color::from_hex(0x9CA3AF),
            1.0f
        );
        r.draw_text(
            "Upload your content to the cloud.",
            arin::Vec2(c_chk_x + 24.0f, c_chk_y),
            arin::Color::from_hex(0x1F2937),
            0.92f
        );

        // Divider separating dialog body from footer
        r.draw_rect(
            arin::Rect(dialog_x, footer_y, dialog_w, 1.0f),
            arin::Color::from_hex(0xE5E7EB)
        );

        // Dialog Footer Background (#F9FAFB)
        r.draw_rounded_rect(
            arin::Rect(dialog_x, footer_y + 1.0f, dialog_w, dialog_h - 146.0f),
            6.0f,
            arin::Color::from_hex(0xF9FAFB)
        );

        // --- Progress Bars Section Labels ---
        std::string det_label = "Copying items... (" +
                                std::to_string(static_cast<int>(det_bar->percentage() * 100.0f)) +
                                "% - Determinate with Shimmer Sweep)";
        r.draw_text(
            det_label,
            arin::Vec2(pb_x, 305.0f),
            arin::Color::from_hex(0x1F2937),
            0.90f
        );

        r.draw_text(
            "Searching for updates... (Indeterminate Traveling Marquee Chunk)",
            arin::Vec2(pb_x, 400.0f),
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

        // --- Windows Features Section Label ---
        r.draw_text(
            "Windows Features: (CheckListBox with independent item toggling)",
            arin::Vec2(chk_list_x, chk_list_y + 4.0f),
            arin::Color::from_hex(0x111827),
            0.95f
        );

        // --- Palette Section Label ---
        r.draw_text(
            "Automatic HBox Widget Layout & Color Palette Showcase:",
            arin::Vec2(40.0f, bottom_y + 4.0f),
            arin::Color::from_hex(0x1F2937),
            0.95f
        );

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
            // Lock visible animation phases for optimal screenshot demonstration
            det_bar->set_anim_phase(0.40f);
            indet_bar->set_anim_phase(0.45f);

            if (captured_frames >= 2) {
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
    // 8. Run the Application Main Loop
    // -------------------------------------------------------------------------
    app.run();

    return 0;
}
