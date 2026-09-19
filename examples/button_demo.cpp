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

/**
 * @brief Named colors used by the demo canvas so raw hex literals stay out of
 *        the drawing code. Values mirror a light neutral desktop palette.
 */
namespace demo_style {
constexpr uint32_t kTextStrong    = 0x111827; ///< Headings and emphasized labels.
constexpr uint32_t kTextPrimary   = 0x1F2937; ///< Section titles.
constexpr uint32_t kTextBody      = 0x374151; ///< Body copy inside detail cards.
constexpr uint32_t kTextSecondary = 0x4B5563; ///< Subtitles and helper text.
constexpr uint32_t kTextMuted     = 0x6B7280; ///< Captions and footer text.
constexpr uint32_t kBorder        = 0xD1D5DB; ///< Card and control outlines.
constexpr uint32_t kSoftBorder    = 0xE5E7EB; ///< Dividers and badge outlines.
constexpr uint32_t kBadgeFill     = 0xF3F4F6; ///< Icon badge background.
constexpr uint32_t kFooterFill    = 0xF9FAFB; ///< Dialog footer background.
} // namespace demo_style

/**
 * @brief Named geometry and typography values for the demo layout.
 *
 * Keeping these together documents the composition grid and removes repeated
 * floating point literals from the widget construction code.
 */
namespace demo_metrics {
constexpr float kPageMargin      = 40.0f;  ///< Outer canvas margin.
constexpr float kCardPadding     = 16.0f;  ///< Inner card padding.
constexpr float kCardCorner      = 6.0f;   ///< Card corner radius.
constexpr float kInnerCorner     = 4.0f;   ///< Radius for inner surfaces and badges.
constexpr float kBorderWidth     = 1.0f;   ///< Card and control border thickness.
constexpr float kTitleScale      = 1.25f;  ///< Hero banner text scale.
constexpr float kDialogTitleScale= 1.2f;   ///< Dialog heading text scale.
constexpr float kCardTitleScale  = 1.0f;   ///< Card heading text scale.
constexpr float kIconLabelScale  = 0.65f;  ///< Icon caption text scale.
constexpr float kIconBadgeSize   = 34.0f;  ///< Icon badge square size.
constexpr float kIconGlyphSize   = 18.0f;  ///< Icon glyph size inside a badge.
constexpr float kIconStep        = 44.0f;  ///< Horizontal stride between icons.
} // namespace demo_metrics

struct PlayerStats {
    std::string name;
    std::string position;
    int games;
    int goals;
};

int main(int argc, char** argv) {
    std::string screenshot_path;
    bool screenshot_context_menu = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--screenshot" && i + 1 < argc) {
            screenshot_path = argv[++i];
        } else if (arg == "--show-menu") {
            screenshot_context_menu = true;
        }
    }

    // -------------------------------------------------------------------------
    // 1. Create the Application Window (1024x768 pixels)
    // -------------------------------------------------------------------------
    arin::App app("Arin32 & ArinOS - Comprehensive UI & Layout Showcase", 1024, 768);

    // Modern neutral slate grey canvas
    app.theme().background_color = arin::Color::from_hex(arin::palette::kCanvasLight);

    std::string global_status = "Status: Ready. Right-click anywhere for Context Menu; interact with inputs or layouts.";
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

    // Automatic action row spanning the dialog footer. Equal widths keep the
    // footer symmetrical without absolute per-button coordinates.
    auto dialog_actions = app.add_hbox(dialog_x, footer_y + 16.0f, 10.0f);
    dialog_actions->set_size(dialog_w, 32.0f);
    dialog_actions->set_padding(arin::Padding(20.0f, 0.0f));
    dialog_actions->set_justify(arin::LayoutJustify::Center);

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

    // Equal action widths keep the footer symmetrical on every window size.
    save_btn->set_auto_resize(false);
    dont_save_btn->set_auto_resize(false);
    cancel_btn->set_auto_resize(false);
    dialog_actions->set_justify(arin::LayoutJustify::Start);
    dialog_actions->distribute_children_equally();

    // -------------------------------------------------------------------------
    // 3. Modern Progress Bars (Determinate & Indeterminate)
    // -------------------------------------------------------------------------
    const float pb_x = 40.0f;
    const float pb_w = 440.0f;
    const float pb_h = 18.0f;

    // Determinate Progress Bar (0 to 100 with white shimmer sweep)
    auto det_bar = app.add_progress_bar(pb_x, 320.0f, pb_w, pb_h, 68.0f, 0.0f, 100.0f);
    det_bar->set_style(arin::ProgressBarStyle::green());

    // Control row for the determinate bar. Equal widths fill the bar exactly,
    // so buttons can never overflow or leave ragged gaps (new layout API).
    auto pb_controls = app.add_hbox(pb_x, 348.0f, 10.0f);
    pb_controls->set_size(pb_w, 28.0f);
    pb_controls->set_justify(arin::LayoutJustify::Start);
    pb_controls->set_validation_logging(true);

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

    // Equal-width style buttons fill the remaining row space exactly.
    auto green_btn = pb_controls->add_button("Green Style", 100.0f, 28.0f);
    green_btn->set_style(arin::ButtonStyle::secondary());

    auto blue_btn = pb_controls->add_button("Blue Style", 90.0f, 28.0f);
    blue_btn->set_style(arin::ButtonStyle::secondary());

    // Freeze the control row: fixed-size buttons shrink text instead of
    // growing bounds, and equal widths fill the bar width exactly.
    dec_btn->set_auto_resize(false);
    inc_btn->set_auto_resize(false);
    green_btn->set_auto_resize(false);
    blue_btn->set_auto_resize(false);
    pb_controls->distribute_children_equally();

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

    // Single-button row: pinned to the start of the track with a fixed extent.
    auto indet_controls = app.add_hbox(pb_x, 433.0f, 8.0f);
    indet_controls->set_size(pb_w, 28.0f);
    indet_controls->set_justify(arin::LayoutJustify::Start);
    indet_controls->set_validation_logging(true);

    auto toggle_indet_btn = indet_controls->add_button("Toggle Indeterminate", 155.0f, 28.0f);
    toggle_indet_btn->set_style(arin::ButtonStyle::secondary());
    toggle_indet_btn->set_auto_resize(false);
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

    // Run one explicit layout pass so the validation report covers this row.
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
    constexpr int kDefaultRosterSelection = 3;
    roster_list->set_selected_index(kDefaultRosterSelection);

    PlayerStats current_player = roster_data[kDefaultRosterSelection];

    roster_list->on_selection_changed([&](int index, const std::string& name) {
        if (index >= 0 && index < static_cast<int>(roster_data.size())) {
            current_player = roster_data[index];
            global_status = "Roster Selected: " + name + " (" + current_player.position + ")";
        }
    });

    // OK button for the roster dialog, placed relative to the list rectangle.
    const arin::Rect roster_bounds(roster_x, roster_y, roster_w, roster_h);
    auto roster_ok_btn = app.add_button(
        "OK",
        roster_bounds.right() + 25.0f,
        roster_bounds.y + roster_bounds.height - 32.0f,
        85.0f,
        32.0f
    );
    roster_ok_btn->set_auto_resize(false);
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
    // 6. Action Buttons Palette (Clean, Un-hardcoded, Symmetrical Layouts)
    //    Card width: 440px. Symmetrical 16px padding on left and right.
    // -------------------------------------------------------------------------
    const float bottom_y = 485.0f;
    const float palette_card_w = 440.0f;
    const float palette_x = demo_metrics::kPageMargin + demo_metrics::kCardPadding;

    // Row 1 of Palette: Primary, Secondary, Success, Danger
    // Uses distribute_children_equally() to dynamically compute button widths
    auto palette_row1 = app.add_hbox(40.0f, bottom_y + 28.0f, 10.0f);
    palette_row1->set_size(palette_card_w, 32.0f);
    palette_row1->set_padding(arin::Padding(demo_metrics::kCardPadding, 0.0f));

    auto prim_b = palette_row1->add_button("Primary", 0.0f, 32.0f);
    prim_b->set_style(arin::ButtonStyle::primary());
    prim_b->on_click([&]() { global_status = "Palette: Primary button clicked"; });

    auto sec_b = palette_row1->add_button("Secondary", 0.0f, 32.0f);
    sec_b->set_style(arin::ButtonStyle::secondary());
    sec_b->on_click([&]() { global_status = "Palette: Secondary button clicked"; });

    auto succ_b = palette_row1->add_button("Success", 0.0f, 32.0f);
    succ_b->set_style(arin::ButtonStyle::success());
    succ_b->on_click([&]() { global_status = "Palette: Success button clicked"; });

    auto dang_b = palette_row1->add_button("Danger", 0.0f, 32.0f);
    dang_b->set_style(arin::ButtonStyle::danger());
    dang_b->on_click([&]() { global_status = "Palette: Danger button clicked"; });

    // Freeze palette buttons so render never invalidates the equal layout.
    for (auto* btn : {prim_b.get(), sec_b.get(), succ_b.get(), dang_b.get()}) {
        btn->set_auto_resize(false);
    }
    palette_row1->set_justify(arin::LayoutJustify::Start);
    palette_row1->distribute_children_equally();
    palette_row1->set_validation_logging(true);

    // Row 2 of Palette: Outline, Disabled, Exit Demo
    // Matches row 1 button dimensions and centers them symmetrically in the card
    auto palette_row2 = app.add_hbox(40.0f, bottom_y + 70.0f, 10.0f);
    palette_row2->set_size(palette_card_w, 32.0f);
    palette_row2->set_padding(arin::Padding(demo_metrics::kCardPadding, 0.0f));
    palette_row2->set_justify(arin::LayoutJustify::Start);

    float row2_btn_w = prim_b->bounds().width;
    auto outl_b = palette_row2->add_button("Outline", row2_btn_w, 32.0f);
    outl_b->set_style(arin::ButtonStyle::outline(arin::Color::from_hex(arin::palette::kAccentBlue)));
    outl_b->on_click([&]() { global_status = "Palette: Outline button clicked"; });

    auto dis_b = palette_row2->add_button("Disabled", row2_btn_w, 32.0f);
    dis_b->set_style(arin::ButtonStyle::secondary()).set_enabled(false);

    auto exit_b = palette_row2->add_button("Exit Demo", row2_btn_w, 32.0f);
    exit_b->set_style(arin::ButtonStyle::secondary());
    exit_b->on_click([&]() {
        std::cout << "[Arin32 Event] Closing demo.\n";
        app.close();
    });

    // Match row 1 widths with the same equal-distribution API.
    outl_b->set_auto_resize(false);
    dis_b->set_auto_resize(false);
    exit_b->set_auto_resize(false);
    palette_row2->distribute_children_equally();
    palette_row2->set_validation_logging(true);

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
        showcase_x + demo_metrics::kCardPadding,
        showcase_y + 34.0f,
        110.0f,
        100.0f,
        arin::ImageScaleMode::Fit
    );
    sample_img->set_corner_radius(arin::UiMetrics::kDefaultImageCornerRadius);

    // -------------------------------------------------------------------------
    // 8. Desktop Context Menu & Layout Validation System
    // -------------------------------------------------------------------------
    std::vector<std::shared_ptr<arin::Layout>> demo_layouts = {
        dialog_actions, pb_controls, indet_controls, palette_row1, palette_row2
    };

    auto run_layout_inspection = [&]() {
        bool all_valid = true;
        int total_issues = 0;
        std::cout << "\n================ [Arin32 Layout Inspector] ================\n";
        for (size_t i = 0; i < demo_layouts.size(); ++i) {
            auto res = demo_layouts[i]->validate();
            std::cout << "Container #" << (i + 1) << " bounds: ("
                      << demo_layouts[i]->bounds().x << ", " << demo_layouts[i]->bounds().y << ", "
                      << demo_layouts[i]->bounds().width << "x" << demo_layouts[i]->bounds().height
                      << ") -> " << (res.is_valid ? "[VALID]" : "[HAS ISSUES]") << "\n";
            if (!res.is_valid) {
                all_valid = false;
                for (const auto& issue : res.issues) {
                    std::cout << "  * " << issue.message << "\n";
                    total_issues++;
                }
            }
        }
        std::cout << "Summary: " << (all_valid ? "All layouts pass validation! 0 overlaps, 0 overflows."
                                               : std::to_string(total_issues) + " layout issue(s) detected.") << "\n";
        std::cout << "============================================================\n\n";
        global_status = all_valid ? "Layout Inspector: All containers PASS validation (0 overlaps, 0 overflows)."
                                  : "Layout Inspector Warning: " + std::to_string(total_issues) + " issue(s) found!";
    };

    // Run layout validation report once on startup
    run_layout_inspection();

    // Create authentic 1:1 Windows 10 desktop context menu with rich SVG icons
    auto ctx_menu = app.create_context_menu();

    // Group 1: Default action & Quick Access
    ctx_menu->add_default_item("Abrir", "assets/icons/folder-open.svg", [&]() {
        global_status = "Menu Contextual: Abrir directorio";
    });
    ctx_menu->add_item("Anclar a Acceso rapido", "assets/icons/quick-access.svg", [&]() {
        global_status = "Menu Contextual: Anclado a Acceso rapido";
    });
    ctx_menu->add_separator();

    // Cascading Submenu 1: Conceder acceso a
    auto access_menu = app.create_context_menu();
    access_menu->add_item("Usuarios especificos...", "assets/icons/network.svg", [&]() {
        global_status = "Menu Contextual -> Conceder acceso: Usuarios especificos";
    });
    access_menu->add_separator();
    access_menu->add_item("Quitar el acceso", "assets/icons/trash.svg", [&]() {
        global_status = "Menu Contextual -> Conceder acceso: Quitar el acceso";
    });

    // Cascading Submenu 2: Incluir en biblioteca
    auto library_menu = app.create_context_menu();
    library_menu->add_item("Documentos", "assets/icons/folder-documents.svg", [&]() {
        global_status = "Menu Contextual -> Biblioteca: Documentos";
    });
    library_menu->add_item("Imagenes", "assets/icons/folder-pictures.svg", [&]() {
        global_status = "Menu Contextual -> Biblioteca: Imagenes";
    });
    library_menu->add_item("Musica", "assets/icons/folder-music.svg", [&]() {
        global_status = "Menu Contextual -> Biblioteca: Musica";
    });
    library_menu->add_item("Videos", "assets/icons/folder-videos.svg", [&]() {
        global_status = "Menu Contextual -> Biblioteca: Videos";
    });
    library_menu->add_separator();
    library_menu->add_item("Crear una biblioteca nueva", "assets/icons/folder.svg", [&]() {
        global_status = "Menu Contextual -> Biblioteca: Crear nueva biblioteca";
    });

    // Cascading Submenu 3: Enviar a
    auto sendto_menu = app.create_context_menu();
    sendto_menu->add_item("Carpeta comprimida (en zip)", "assets/icons/file-archive.svg", [&]() {
        global_status = "Menu Contextual -> Enviar a: Carpeta comprimida (en zip)";
    });
    sendto_menu->add_item("Destinatario de correo", "assets/icons/file.svg", [&]() {
        global_status = "Menu Contextual -> Enviar a: Destinatario de correo";
    });
    sendto_menu->add_item("Escritorio (crear acceso directo)", "assets/icons/folder-desktop.svg", [&]() {
        global_status = "Menu Contextual -> Enviar a: Escritorio (acceso directo)";
    });
    sendto_menu->add_item("Documentos", "assets/icons/folder-documents.svg", [&]() {
        global_status = "Menu Contextual -> Enviar a: Documentos";
    });
    sendto_menu->add_item("Unidad USB (D:)", "assets/icons/drive-harddisk.svg", [&]() {
        global_status = "Menu Contextual -> Enviar a: Unidad USB";
    });

    // Group 2: Shell access, security, history, library, pinning
    ctx_menu->add_submenu("Conceder acceso a", access_menu, "assets/icons/network.svg");
    ctx_menu->add_item("Escanear con Seguridad", "assets/icons/shield.svg", [&]() {
        global_status = "Menu Contextual: Escaneo de seguridad iniciado";
    });
    ctx_menu->add_item("Restaurar versiones anteriores", "assets/icons/refresh.svg", [&]() {
        global_status = "Menu Contextual: Restaurar versiones anteriores";
    });
    ctx_menu->add_submenu("Incluir en biblioteca", library_menu, "assets/icons/folder-documents.svg");
    ctx_menu->add_item("Anclar a Inicio", "assets/icons/pin.svg", [&]() {
        global_status = "Menu Contextual: Anclado a Inicio";
    });
    ctx_menu->add_item("Copiar como ruta de acceso", "assets/icons/file-text.svg", [&]() {
        global_status = "Menu Contextual: Ruta copiada al portapapeles";
    });
    ctx_menu->add_separator();

    // Group 3: Send To
    ctx_menu->add_submenu("Enviar a", sendto_menu, "assets/icons/arrow-right.svg");
    ctx_menu->add_separator();

    // Group 4: Cut & Copy
    ctx_menu->add_item("Cortar", "assets/icons/cut.svg", "Ctrl+X", [&]() {
        global_status = "Menu Contextual: Cortar elemento";
    });
    ctx_menu->add_item("Copiar", "assets/icons/copy.svg", "Ctrl+C", [&]() {
        global_status = "Menu Contextual: Copiar elemento";
    });
    ctx_menu->add_separator();

    // Group 5: Shortcut, Delete, Rename
    ctx_menu->add_item("Crear acceso directo", "assets/icons/shortcut.svg", [&]() {
        global_status = "Menu Contextual: Acceso directo creado";
    });
    ctx_menu->add_item("Eliminar", "assets/icons/trash.svg", "Del", [&]() {
        global_status = "Menu Contextual: Eliminar elemento";
    });
    ctx_menu->add_item("Cambiar nombre", "assets/icons/rename.svg", "F2", [&]() {
        global_status = "Menu Contextual: Cambiar nombre";
    });
    ctx_menu->add_separator();

    // Group 6: Properties
    ctx_menu->add_item("Propiedades", "assets/icons/properties.svg", "Alt+Enter", [&]() {
        global_status = "Menu Contextual: Propiedades";
    });

    // Custom Extension: Insert user-defined action right before "Propiedades"
    ctx_menu->insert_item_before("Propiedades",
        arin::MenuItem::action("Abrir con Terminal Arin32", "assets/icons/file-code.svg", [&]() {
            action_counter++;
            global_status = "Menu Personalizado: Terminal Arin32 iniciado (Accion #" + std::to_string(action_counter) + ")";
            std::cout << "[Arin32 Event] Custom Context Menu Action: Terminal Arin32\n";
        }).set_id("open_terminal")
    );

    app.set_default_context_menu(ctx_menu);

    // Dedicated Widget-Specific Context Menu for the ListBox (roster_list)
    auto roster_menu = app.create_context_menu();
    roster_menu->add_item("Ver perfil del jugador", "assets/icons/folder-open.svg", [&]() {
        action_counter++;
        global_status = "Menu Roster: Ver perfil de " + current_player.name + " (Accion #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] Roster context menu: View Profile\n";
    });
    roster_menu->add_item("Enviar mensaje de equipo", "assets/icons/network.svg", [&]() {
        action_counter++;
        global_status = "Menu Roster: Mensaje enviado a " + current_player.name + " (Accion #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] Roster context menu: Send Message\n";
    });
    roster_menu->add_separator();
    roster_menu->add_item("Quitar del roster activo", "assets/icons/trash.svg", "Del", [&]() {
        action_counter++;
        global_status = "Menu Roster: Jugador " + current_player.name + " retirado (Accion #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] Roster context menu: Remove Player\n";
    });
    roster_list->set_context_menu(roster_menu);

    // -------------------------------------------------------------------------
    // 9. Custom Frame Callback: Cards, Shadows, Labels, and Visual Styling
    // -------------------------------------------------------------------------
    app.on_frame([&](arin::Renderer2D& r) {
        // App Title Banner (rect-based alignment keeps text on the page grid).
        const float header_w = static_cast<float>(r.viewport_width()) - 2.0f * demo_metrics::kPageMargin;
        r.draw_text_in_rect(
            "Arin32 & ArinOS - Comprehensive Graphical Library Showcase",
            arin::Rect(demo_metrics::kPageMargin, 12.0f, header_w, 26.0f),
            arin::Color::from_hex(demo_style::kTextPrimary),
            demo_metrics::kTitleScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "Editable TextInput, Interactive CheckBox, Modern Progress Bars, List Boxes, and Dedicated Icon Showcase",
            arin::Rect(demo_metrics::kPageMargin, 38.0f, header_w, 18.0f),
            arin::Color::from_hex(demo_style::kTextSecondary),
            arin::UiMetrics::kMenuLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        // --- Dialog Card Container (Left) ---
        r.draw_shadow(
            arin::Rect(dialog_x, dialog_y, dialog_w, dialog_h),
            demo_metrics::kCardCorner,
            arin::Color(0.0f, 0.0f, 0.0f, 0.15f),
            arin::Vec2(0.0f, 3.0f),
            10.0f
        );

        r.draw_rounded_rect(
            arin::Rect(dialog_x, dialog_y, dialog_w, dialog_h),
            demo_metrics::kCardCorner,
            arin::Color::white(),
            arin::Color::from_hex(demo_style::kBorder),
            1.0f
        );

        // Info vector icon badge in dialog
        r.draw_icon(
            arin::IconType::Info,
            arin::Rect(dialog_x + 24.0f, dialog_y + 18.0f, 22.0f, 22.0f),
            arin::Color::from_hex(arin::palette::kAccentBlue)
        );

        r.draw_text_in_rect(
            "Save your work?",
            arin::Rect(dialog_x + 54.0f, dialog_y + 14.0f, dialog_w - 54.0f - 24.0f, 26.0f),
            arin::Color::from_hex(demo_style::kTextStrong),
            demo_metrics::kDialogTitleScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "File name and cloud backup options:",
            arin::Rect(dialog_x + 24.0f, dialog_y + 46.0f, dialog_w - 48.0f, 18.0f),
            arin::Color::from_hex(demo_style::kTextSecondary),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        // Divider separating dialog body from footer (strictly inside card border)
        r.draw_rect(
            arin::Rect(dialog_x + 1.0f, footer_y, dialog_w - 2.0f, 1.0f),
            arin::Color::from_hex(demo_style::kSoftBorder)
        );

        // Dialog Footer Background (#F9FAFB) strictly inside the card border
        r.draw_rounded_rect(
            arin::Rect(dialog_x + 1.0f, footer_y + 1.0f, dialog_w - 2.0f, dialog_h - 162.0f),
            demo_metrics::kInnerCorner,
            arin::Color::from_hex(demo_style::kFooterFill)
        );
        // Square off top corners of footer below divider
        r.draw_rect(
            arin::Rect(dialog_x + 1.0f, footer_y + 1.0f, dialog_w - 2.0f, 12.0f),
            arin::Color::from_hex(demo_style::kFooterFill)
        );

        // --- Progress Bars Section Labels ---
        std::string det_label = "Copying items... (" +
                                std::to_string(static_cast<int>(det_bar->percentage() * 100.0f)) +
                                "% - Determinate with Shimmer Sweep)";
        r.draw_text_in_rect(
            det_label,
            arin::Rect(pb_x, 300.0f, pb_w, 18.0f),
            arin::Color::from_hex(demo_style::kTextPrimary),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "Searching for updates... (Indeterminate Traveling Marquee Chunk)",
            arin::Rect(pb_x, 384.0f, pb_w, 18.0f),
            arin::Color::from_hex(demo_style::kTextPrimary),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        // --- Roster Section Labels & Details Card (Right) ---
        r.draw_text_in_rect(
            "Today's roster: (Standard ListBox with smooth scrollbar)",
            arin::Rect(roster_x, roster_y, roster_w + 190.0f, 20.0f),
            arin::Color::from_hex(demo_style::kTextStrong),
            arin::UiMetrics::kListLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        // Player Details Card
        const float info_x = roster_x + 250.0f;
        const float info_y = roster_y + 25.0f;
        const float info_w = 190.0f;
        const float info_h = 135.0f;

        r.draw_rounded_rect(
            arin::Rect(info_x, info_y, info_w, info_h),
            demo_metrics::kInnerCorner,
            arin::Color::white(),
            arin::Color::from_hex(demo_style::kBorder),
            1.0f
        );

        // Detail rows are derived from the card rectangle and share one row height,
        // so no absolute per-line offsets are required.
        const arin::Rect info_card(info_x, info_y, info_w, info_h);
        const float info_pad = 12.0f;
        const float info_row_h = 24.0f;
        const auto info_row = [&](float top_offset) {
            return arin::Rect(info_card.x + info_pad, info_card.y + top_offset, info_w - 2.0f * info_pad, info_row_h);
        };

        r.draw_text_in_rect(
            "Player Details",
            info_row(8.0f),
            arin::Color::from_hex(arin::palette::kAccentBlue),
            demo_metrics::kCardTitleScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "Position: " + current_player.position,
            info_row(34.0f),
            arin::Color::from_hex(demo_style::kTextBody),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "Games played: " + std::to_string(current_player.games),
            info_row(58.0f),
            arin::Color::from_hex(demo_style::kTextBody),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "Goals: " + std::to_string(current_player.goals),
            info_row(82.0f),
            arin::Color::from_hex(demo_style::kTextBody),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        // --- System Features Section Label ---
        r.draw_text_in_rect(
            "System Features: (CheckListBox with independent item toggling)",
            arin::Rect(chk_list_x, chk_list_y, chk_list_w, 20.0f),
            arin::Color::from_hex(demo_style::kTextStrong),
            arin::UiMetrics::kListLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        // --- Action Buttons Section Card (Bottom Left) ---
        r.draw_rounded_rect(
            arin::Rect(demo_metrics::kPageMargin, bottom_y, 440.0f, showcase_h),
            demo_metrics::kCardCorner,
            arin::Color::white(),
            arin::Color::from_hex(demo_style::kBorder),
            1.0f
        );

        r.draw_text_in_rect(
            "Action Buttons Palette (Auto-Layout, Symmetrical Spacing):",
            arin::Rect(palette_x, bottom_y + 4.0f, 440.0f - 2.0f * demo_metrics::kCardPadding, 22.0f),
            arin::Color::from_hex(demo_style::kTextStrong),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "Validated layout: dynamic widths & symmetrical alignment",
            arin::Rect(palette_x, bottom_y + 108.0f, 440.0f - 2.0f * demo_metrics::kCardPadding, 18.0f),
            arin::Color::from_hex(demo_style::kTextMuted),
            arin::UiMetrics::kMenuShortcutScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        // --- Dedicated Vector Icons & Graphics Showcase Card (Bottom Right) ---
        r.draw_rounded_rect(
            arin::Rect(showcase_x, showcase_y, showcase_w, showcase_h),
            demo_metrics::kCardCorner,
            arin::Color::white(),
            arin::Color::from_hex(demo_style::kBorder),
            1.0f
        );

        r.draw_text_in_rect(
            "Sample Icons & Graphics Showcase (GPU Vector Glyphs):",
            arin::Rect(showcase_x + demo_metrics::kCardPadding, showcase_y + 4.0f,
                       showcase_w - 2.0f * demo_metrics::kCardPadding, 22.0f),
            arin::Color::from_hex(demo_style::kTextStrong),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "Loaded Image",
            arin::Rect(showcase_x + demo_metrics::kCardPadding, showcase_y + 132.0f, 110.0f, 18.0f),
            arin::Color::from_hex(demo_style::kTextMuted),
            arin::UiMetrics::kMenuShortcutScale,
            arin::TextAlignH::Center,
            arin::TextAlignV::Center
        );

        // Draw crisp sample vector icons in a dedicated grid
        const float icon_grid_x = showcase_x + 145.0f;
        const float icon_grid_y = showcase_y + 36.0f;
        const float icon_step = demo_metrics::kIconStep;

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

            // Subtle badge background with centered glyph (new geometry API).
            const arin::Rect badge1(ix, iy, demo_metrics::kIconBadgeSize, demo_metrics::kIconBadgeSize);
            r.draw_rounded_rect(
                badge1,
                demo_metrics::kInnerCorner,
                arin::Color::from_hex(demo_style::kBadgeFill),
                arin::Color::from_hex(demo_style::kSoftBorder),
                demo_metrics::kBorderWidth
            );

            r.draw_icon(
                sample_icons_row1[i].first,
                badge1.centered(arin::Vec2(demo_metrics::kIconGlyphSize, demo_metrics::kIconGlyphSize)),
                arin::Color::from_hex(arin::palette::kAccentBlue)
            );

            r.draw_text_in_rect(
                sample_icons_row1[i].second,
                arin::Rect(ix, iy + demo_metrics::kIconBadgeSize, demo_metrics::kIconBadgeSize, 14.0f),
                arin::Color::from_hex(demo_style::kTextMuted),
                demo_metrics::kIconLabelScale,
                arin::TextAlignH::Center,
                arin::TextAlignV::Top
            );
        }

        // Render Row 2
        for (size_t i = 0; i < sample_icons_row2.size(); ++i) {
            float ix = icon_grid_x + i * icon_step;
            float iy = icon_grid_y + 52.0f;

            const arin::Rect badge2(ix, iy, demo_metrics::kIconBadgeSize, demo_metrics::kIconBadgeSize);
            r.draw_rounded_rect(
                badge2,
                demo_metrics::kInnerCorner,
                arin::Color::from_hex(demo_style::kBadgeFill),
                arin::Color::from_hex(demo_style::kSoftBorder),
                demo_metrics::kBorderWidth
            );

            r.draw_icon(
                sample_icons_row2[i].first,
                badge2.centered(arin::Vec2(demo_metrics::kIconGlyphSize, demo_metrics::kIconGlyphSize)),
                arin::Color::from_hex(demo_style::kTextPrimary)
            );

            r.draw_text_in_rect(
                sample_icons_row2[i].second,
                arin::Rect(ix, iy + demo_metrics::kIconBadgeSize, demo_metrics::kIconBadgeSize, 14.0f),
                arin::Color::from_hex(demo_style::kTextMuted),
                demo_metrics::kIconLabelScale,
                arin::TextAlignH::Center,
                arin::TextAlignV::Top
            );
        }

        // --- Interactive Status Bar (new geometry + text API) ---
        const float status_margin = demo_metrics::kPageMargin;
        const arin::Rect status_bar(status_margin, 655.0f, static_cast<float>(r.viewport_width()) - 2.0f * status_margin, 32.0f);
        r.draw_rounded_rect(
            status_bar,
            demo_metrics::kInnerCorner,
            arin::Color::white(),
            arin::Color::from_hex(demo_style::kBorder),
            1.0f
        );

        r.draw_text_in_rect(
            global_status,
            status_bar.inset(arin::Padding(12.0f, 0.0f)),
            arin::Color::from_hex(demo_style::kTextStrong),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        // Footer copyright info
        r.draw_text_centered(
            "Copyright (c) 2026, Arin32 & ArinOS Contributors * BSD 2-Clause License * C++17 OpenGL",
            arin::Rect(0.0f, 715.0f, static_cast<float>(r.viewport_width()), 20.0f),
            arin::Color::from_hex(demo_style::kTextMuted),
            arin::UiMetrics::kMenuLabelScale
        );
    });

    // If requested, take an automated screenshot after widgets render and close
    if (!screenshot_path.empty()) {
        int captured_frames = 0;
        app.on_after_frame([&](arin::Renderer2D& r) {
            captured_frames++;
            det_bar->set_anim_phase(0.40f);
            indet_bar->set_anim_phase(0.45f);

            if (screenshot_context_menu && captured_frames >= 2) {
                if (captured_frames == 2) {
                    app.show_context_menu(ctx_menu, 220.0f, 100.0f);
                    ctx_menu->open_child_menu(10); // Open cascading "Enviar a" submenu
                }
                ctx_menu->set_hovered_index(10); // Row "Enviar a" hovered 1:1 like maxresdefault.jpg
            }

            int target_frame = screenshot_context_menu ? 4 : 3;
            if (captured_frames >= target_frame) {
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
    // 10. Run the Application Main Loop
    // -------------------------------------------------------------------------
    app.run();

    return 0;
}
