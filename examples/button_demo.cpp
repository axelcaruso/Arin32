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
 * @brief Comprehensive Desktop UI Showcase demonstrating Arin32's full widget arsenal:
 *        MenuBar, TabView, TreeView, TableView, FileDialog Modal, TextInput, CheckBox,
 *        Progress Bars, ListBox, CheckListBox, Automatic Layouts, and SVG Vector Arsenal.
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
 * @brief Named colors used by the demo canvas.
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
 */
namespace demo_metrics {
constexpr float kPageMargin       = 30.0f;  ///< Outer canvas margin.
constexpr float kCardPadding      = 16.0f;  ///< Inner card padding.
constexpr float kCardCorner       = 6.0f;   ///< Card corner radius.
constexpr float kInnerCorner      = 4.0f;   ///< Radius for inner surfaces and badges.
constexpr float kBorderWidth      = 1.0f;   ///< Card and control border thickness.
constexpr float kTitleScale       = 1.20f;  ///< Hero banner text scale.
constexpr float kDialogTitleScale = 1.15f;  ///< Dialog heading text scale.
constexpr float kIconLabelScale   = 0.65f;  ///< Icon caption text scale.
constexpr float kIconBadgeSize    = 34.0f;  ///< Icon badge square size.
constexpr float kIconGlyphSize    = 18.0f;  ///< Icon glyph size inside a badge.
constexpr float kIconStep         = 44.0f;  ///< Horizontal stride between icons.
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
    // 1. Create Application Window (1280x820 pixels)
    // -------------------------------------------------------------------------
    arin::App app("Arin32 & ArinOS - Comprehensive UI & Desktop Arsenal Showcase", 1280, 820);

    // Modern neutral slate grey canvas
    app.theme().background_color = arin::Color::from_hex(arin::palette::kCanvasLight);

    std::string global_status = "Listo. Explore el MenuBar superior, cambie de pestana en TabView o haga clic derecho para el menu contextual.";
    int action_counter = 0;

    // -------------------------------------------------------------------------
    // 2. Left Column - Dialog Box with TextInput, CheckBox, and Actions
    // -------------------------------------------------------------------------
    const float left_col_x = demo_metrics::kPageMargin;
    const float left_col_w = 460.0f;

    const float dialog_y = 68.0f;
    const float dialog_h = 205.0f;
    const float footer_y = dialog_y + 146.0f;

    auto title_input = app.add_text_input(
        "informe_anual_2026.docx",
        left_col_x + 20.0f,
        dialog_y + 66.0f,
        left_col_w - 40.0f,
        32.0f
    );
    title_input->set_placeholder("Escriba el nombre del archivo...");
    title_input->on_text_changed([&](const std::string& text) {
        global_status = "Escribiendo archivo: \"" + text + "\"";
    });
    title_input->on_submit([&](const std::string& text) {
        action_counter++;
        global_status = "Archivo enviado: \"" + text + "\" (Accion #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] Filename submitted: " << text << "\n";
    });

    auto cloud_chk = app.add_checkbox(
        "Copia de seguridad en la nube activada.",
        left_col_x + 20.0f,
        dialog_y + 108.0f,
        true
    );
    cloud_chk->on_toggled([&](bool checked) {
        action_counter++;
        global_status = "Sincronizacion nube: " + std::string(checked ? "ACTIVADA" : "DESACTIVADA") +
                        " (Accion #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] Cloud CheckBox: " << (checked ? "ON" : "OFF") << "\n";
    });

    auto dialog_actions = app.add_hbox(left_col_x, footer_y + 14.0f, 10.0f);
    dialog_actions->set_size(left_col_w, 32.0f);
    dialog_actions->set_padding(arin::Padding(20.0f, 0.0f));
    dialog_actions->set_justify(arin::LayoutJustify::Start);

    auto save_btn = dialog_actions->add_button("Guardar", 85.0f, 32.0f);
    save_btn->set_style(arin::ButtonStyle::primary());
    save_btn->on_click([&]() {
        action_counter++;
        global_status = "Dialogo: 'Guardar' presionado para \"" + title_input->text() +
                        "\" (Accion #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] Save button clicked\n";
    });

    auto dont_save_btn = dialog_actions->add_button("Descartar", 100.0f, 32.0f);
    dont_save_btn->set_style(arin::ButtonStyle::secondary());
    dont_save_btn->on_click([&]() {
        action_counter++;
        global_status = "Dialogo: 'Descartar' presionado (Accion #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] Don't Save button clicked\n";
    });

    auto cancel_btn = dialog_actions->add_button("Cancelar", 85.0f, 32.0f);
    cancel_btn->set_style(arin::ButtonStyle::secondary());
    cancel_btn->on_click([&]() {
        action_counter++;
        global_status = "Dialogo: 'Cancelar' presionado (Accion #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] Cancel button clicked\n";
    });

    save_btn->set_auto_resize(false);
    dont_save_btn->set_auto_resize(false);
    cancel_btn->set_auto_resize(false);
    dialog_actions->distribute_children_equally();

    // -------------------------------------------------------------------------
    // 3. Left Column - Progress Bars (Determinate & Indeterminate)
    // -------------------------------------------------------------------------
    const float pb_y = 285.0f;
    const float pb_h = 16.0f;

    auto det_bar = app.add_progress_bar(left_col_x + 16.0f, pb_y + 30.0f, left_col_w - 32.0f, pb_h, 68.0f, 0.0f, 100.0f);
    det_bar->set_style(arin::ProgressBarStyle::green());

    auto pb_controls = app.add_hbox(left_col_x + 16.0f, pb_y + 54.0f, 8.0f);
    pb_controls->set_size(left_col_w - 32.0f, 26.0f);
    pb_controls->set_justify(arin::LayoutJustify::Start);

    auto dec_btn = pb_controls->add_button("- 10%", 65.0f, 26.0f);
    dec_btn->set_style(arin::ButtonStyle::secondary());
    dec_btn->on_click([&]() {
        det_bar->set_value(det_bar->value() - 10.0f);
        global_status = "Progreso determinado: " + std::to_string(static_cast<int>(det_bar->percentage() * 100.0f)) + "%";
    });

    auto inc_btn = pb_controls->add_button("+ 10%", 65.0f, 26.0f);
    inc_btn->set_style(arin::ButtonStyle::secondary());
    inc_btn->on_click([&]() {
        det_bar->set_value(det_bar->value() + 10.0f);
        global_status = "Progreso determinado: " + std::to_string(static_cast<int>(det_bar->percentage() * 100.0f)) + "%";
    });

    auto green_btn = pb_controls->add_button("Estilo Verde", 85.0f, 26.0f);
    green_btn->set_style(arin::ButtonStyle::secondary());

    auto blue_btn = pb_controls->add_button("Estilo Azul", 85.0f, 26.0f);
    blue_btn->set_style(arin::ButtonStyle::secondary());

    dec_btn->set_auto_resize(false);
    inc_btn->set_auto_resize(false);
    green_btn->set_auto_resize(false);
    blue_btn->set_auto_resize(false);
    pb_controls->distribute_children_equally();

    auto indet_bar = app.add_progress_bar(left_col_x + 16.0f, pb_y + 98.0f, left_col_w - 32.0f, pb_h);
    indet_bar->set_indeterminate(true);
    indet_bar->set_style(arin::ProgressBarStyle::green());

    green_btn->on_click([&, det_bar, indet_bar]() {
        det_bar->set_style(arin::ProgressBarStyle::green());
        indet_bar->set_style(arin::ProgressBarStyle::green());
        global_status = "Estilo de barra: Verde (#06B025)";
    });

    blue_btn->on_click([&, det_bar, indet_bar]() {
        det_bar->set_style(arin::ProgressBarStyle::blue());
        indet_bar->set_style(arin::ProgressBarStyle::blue());
        global_status = "Estilo de barra: Azul (#0067C0)";
    });

    auto indet_controls = app.add_hbox(left_col_x + 16.0f, pb_y + 120.0f, 8.0f);
    indet_controls->set_size(left_col_w - 32.0f, 26.0f);
    indet_controls->set_justify(arin::LayoutJustify::Start);

    auto toggle_indet_btn = indet_controls->add_button("Alternar Modo Indeterminado", 180.0f, 26.0f);
    toggle_indet_btn->set_style(arin::ButtonStyle::secondary());
    toggle_indet_btn->set_auto_resize(false);
    toggle_indet_btn->on_click([&]() {
        if (indet_bar->is_indeterminate()) {
            indet_bar->set_indeterminate(false);
            indet_bar->set_value(50.0f);
            global_status = "Segunda barra: Modo determinado (50%)";
        } else {
            indet_bar->set_indeterminate(true);
            global_status = "Segunda barra: Modo indeterminado activo";
        }
    });

    // -------------------------------------------------------------------------
    // 4. Left Column - Action Buttons Palette
    // -------------------------------------------------------------------------
    const float palette_y = 448.0f;

    auto palette_row1 = app.add_hbox(left_col_x + 16.0f, palette_y + 28.0f, 8.0f);
    palette_row1->set_size(left_col_w - 32.0f, 30.0f);
    palette_row1->set_justify(arin::LayoutJustify::Start);

    auto prim_b = palette_row1->add_button("Primary", 0.0f, 30.0f);
    prim_b->set_style(arin::ButtonStyle::primary());
    prim_b->on_click([&]() { global_status = "Boton presionado: Primary"; });

    auto sec_b = palette_row1->add_button("Secondary", 0.0f, 30.0f);
    sec_b->set_style(arin::ButtonStyle::secondary());
    sec_b->on_click([&]() { global_status = "Boton presionado: Secondary"; });

    auto succ_b = palette_row1->add_button("Success", 0.0f, 30.0f);
    succ_b->set_style(arin::ButtonStyle::success());
    succ_b->on_click([&]() { global_status = "Boton presionado: Success"; });

    auto dang_b = palette_row1->add_button("Danger", 0.0f, 30.0f);
    dang_b->set_style(arin::ButtonStyle::danger());
    dang_b->on_click([&]() { global_status = "Boton presionado: Danger"; });

    for (auto* btn : {prim_b.get(), sec_b.get(), succ_b.get(), dang_b.get()}) {
        btn->set_auto_resize(false);
    }
    palette_row1->distribute_children_equally();

    auto palette_row2 = app.add_hbox(left_col_x + 16.0f, palette_y + 64.0f, 8.0f);
    palette_row2->set_size(left_col_w - 32.0f, 30.0f);
    palette_row2->set_justify(arin::LayoutJustify::Start);

    auto outl_b = palette_row2->add_button("Outline", 0.0f, 30.0f);
    outl_b->set_style(arin::ButtonStyle::outline(arin::Color::from_hex(arin::palette::kAccentBlue)));
    outl_b->on_click([&]() { global_status = "Boton presionado: Outline"; });

    auto dis_b = palette_row2->add_button("Disabled", 0.0f, 30.0f);
    dis_b->set_style(arin::ButtonStyle::secondary()).set_enabled(false);

    auto exit_b = palette_row2->add_button("Cerrar Demo", 0.0f, 30.0f);
    exit_b->set_style(arin::ButtonStyle::secondary());
    exit_b->on_click([&]() {
        std::cout << "[Arin32 Event] Closing application from demo button.\n";
        app.close();
    });

    outl_b->set_auto_resize(false);
    dis_b->set_auto_resize(false);
    exit_b->set_auto_resize(false);
    palette_row2->distribute_children_equally();

    // -------------------------------------------------------------------------
    // 5. Left Column - Vector Icons & Graphics Showcase
    // -------------------------------------------------------------------------
    const float icons_y = 590.0f;

    auto sample_img = app.add_image(
        "button-dialog-example.png",
        left_col_x + 16.0f,
        icons_y + 30.0f,
        96.0f,
        86.0f,
        arin::ImageScaleMode::Fit
    );
    sample_img->set_corner_radius(arin::UiMetrics::kDefaultImageCornerRadius);

    // -------------------------------------------------------------------------
    // 6. Right Column - TabView hosting TreeView, TableView, FileDialog & Lists
    // -------------------------------------------------------------------------
    const float right_col_x = 510.0f;
    const float right_col_y = 68.0f;
    const float right_col_w = 740.0f;
    const float right_col_h = 672.0f;

    auto tab_view = std::make_shared<arin::TabView>(arin::Rect{right_col_x, right_col_y, right_col_w, right_col_h});

    // --- Tab 1: TreeView (Arbol de Proyecto) ---
    auto tree_view = std::make_shared<arin::TreeView>();
    auto root_node = tree_view->add_root("Arin32 Workspace", "assets/icons/folder.svg");

    auto inc_node = root_node->add_child("include", "assets/icons/folder.svg");
    auto inc_arin_node = inc_node->add_child("arin", "assets/icons/folder.svg");
    inc_arin_node->add_child("menu_bar.hpp", "assets/icons/file-code.svg");
    inc_arin_node->add_child("tab_view.hpp", "assets/icons/file-code.svg");
    inc_arin_node->add_child("tree_view.hpp", "assets/icons/file-code.svg");
    inc_arin_node->add_child("table_view.hpp", "assets/icons/file-code.svg");
    inc_arin_node->add_child("file_dialog.hpp", "assets/icons/file-code.svg");
    inc_arin_node->add_child("context_menu.hpp", "assets/icons/file-code.svg");
    inc_arin_node->add_child("renderer.hpp", "assets/icons/file-code.svg");
    inc_arin_node->add_child("button.hpp", "assets/icons/file-code.svg");

    auto src_node = root_node->add_child("src", "assets/icons/folder.svg");
    src_node->add_child("menu_bar.cpp", "assets/icons/file-code.svg");
    src_node->add_child("tab_view.cpp", "assets/icons/file-code.svg");
    src_node->add_child("tree_view.cpp", "assets/icons/file-code.svg");
    src_node->add_child("table_view.cpp", "assets/icons/file-code.svg");
    src_node->add_child("file_dialog.cpp", "assets/icons/file-code.svg");
    src_node->add_child("context_menu.cpp", "assets/icons/file-code.svg");

    auto tests_node = root_node->add_child("tests", "assets/icons/folder.svg");
    tests_node->add_child("test_menu_bar.cpp", "assets/icons/file-code.svg");
    tests_node->add_child("test_tab_view.cpp", "assets/icons/file-code.svg");
    tests_node->add_child("test_tree_view.cpp", "assets/icons/file-code.svg");
    tests_node->add_child("test_table_view.cpp", "assets/icons/file-code.svg");
    tests_node->add_child("test_file_dialog.cpp", "assets/icons/file-code.svg");

    auto dist_node = root_node->add_child("dist", "assets/icons/folder.svg");
    dist_node->add_child("libarin32.a", "assets/icons/file-archive.svg");
    dist_node->add_child("libarin32.so", "assets/icons/file-archive.svg");
    dist_node->add_child("arin32.7z", "assets/icons/file-archive.svg");
    dist_node->add_child("SHA256", "assets/icons/file-text.svg");

    root_node->add_child("CMakeLists.txt", "assets/icons/file-code.svg");
    root_node->add_child("DOCS.md", "assets/icons/file-doc.svg");
    root_node->add_child("README.md", "assets/icons/file-text.svg");
    root_node->add_child("LICENSE", "assets/icons/shield.svg");

    root_node->is_expanded = true;
    inc_node->is_expanded = true;
    inc_arin_node->is_expanded = true;
    src_node->is_expanded = true;

    tree_view->on_selection_changed([&](std::shared_ptr<arin::TreeNode> node) {
        if (node) {
            action_counter++;
            global_status = "Arbol: Elemento seleccionado: \"" + node->label + "\" (Accion #" + std::to_string(action_counter) + ")";
            std::cout << "[Arin32 Event] TreeView selected: " << node->label << "\n";
        }
    });

    tree_view->on_node_double_clicked([&](std::shared_ptr<arin::TreeNode> node) {
        if (node) {
            action_counter++;
            global_status = "Arbol: Doble clic en \"" + node->label + "\" (Accion #" + std::to_string(action_counter) + ")";
        }
    });

    tab_view->add_tab("Arbol", tree_view);

    // --- Tab 2: TableView (Monitor del Sistema) ---
    auto table_view = std::make_shared<arin::TableView>();
    table_view->add_column("Proceso", 185.0f);
    table_view->add_column("PID", 65.0f);
    table_view->add_column("Memoria", 110.0f);
    table_view->add_column("CPU %", 85.0f);
    table_view->add_column("Prioridad", 95.0f);
    table_view->add_column("Estado", 115.0f);

    table_view->add_row({"systemd", "1", "14.2 MB", "0.1%", "Normal", "Ejecutando"});
    table_view->add_row({"wayland-compositor", "812", "142.6 MB", "3.2%", "Alta", "Ejecutando"});
    table_view->add_row({"arin32_demo", "1044", "45.1 MB", "1.8%", "Normal", "Ejecutando"});
    table_view->add_row({"opengl_driver", "1045", "128.0 MB", "4.5%", "Alta", "Ejecutando"});
    table_view->add_row({"pipewire", "1120", "24.5 MB", "0.3%", "Tiempo Real", "Ejecutando"});
    table_view->add_row({"networkmanager", "1205", "19.8 MB", "0.0%", "Normal", "Dormido"});
    table_view->add_row({"dbus-daemon", "1310", "8.4 MB", "0.0%", "Normal", "Dormido"});
    table_view->add_row({"bash", "1450", "6.2 MB", "0.0%", "Normal", "Espera"});
    table_view->add_row({"ssh-agent", "1502", "3.8 MB", "0.0%", "Baja", "Dormido"});
    table_view->add_row({"cron", "1620", "4.1 MB", "0.0%", "Baja", "Dormido"});
    table_view->add_row({"python3_pkg", "1830", "32.4 MB", "0.2%", "Normal", "Dormido"});
    table_view->add_row({"auditd", "1940", "12.0 MB", "0.0%", "Normal", "Dormido"});

    table_view->on_row_selected([table_view, &global_status, &action_counter](int row) {
        if (row >= 0 && static_cast<size_t>(row) < table_view->row_count()) {
            action_counter++;
            global_status = "Tabla: Proceso seleccionado: " + table_view->get_cell(row, 0) +
                            " (PID " + table_view->get_cell(row, 1) + ", Mem: " + table_view->get_cell(row, 2) +
                            ") (Accion #" + std::to_string(action_counter) + ")";
            std::cout << "[Arin32 Event] TableView selected row: " << table_view->get_cell(row, 0) << "\n";
        }
    });

    table_view->on_row_double_clicked([table_view, &global_status, &action_counter](int row) {
        if (row >= 0 && static_cast<size_t>(row) < table_view->row_count()) {
            action_counter++;
            global_status = "Tabla: Doble clic en proceso '" + table_view->get_cell(row, 0) +
                            "' - Inspeccionando propiedades (Accion #" + std::to_string(action_counter) + ")";
            std::cout << "[Arin32 Event] TableView double click: " << table_view->get_cell(row, 0) << "\n";
        }
    });

    tab_view->add_tab("Procesos", table_view);

    // --- Tab 3: FileDialog Launcher Panel ---
    auto file_dlg_page = std::make_shared<arin::VBox>(0.0f, 0.0f, 14.0f);
    file_dlg_page->set_padding(arin::Padding(24.0f, 24.0f));

    auto open_dlg_btn = file_dlg_page->add_button("Abrir Archivo (FileDialogMode::OpenFile)", 380.0f, 36.0f);
    open_dlg_btn->set_style(arin::ButtonStyle::primary());
    open_dlg_btn->set_icon(arin::IconType::Folder);

    auto save_dlg_btn = file_dlg_page->add_button("Guardar Archivo (FileDialogMode::SaveFile)", 380.0f, 36.0f);
    save_dlg_btn->set_style(arin::ButtonStyle::secondary());
    save_dlg_btn->set_icon(arin::IconType::File);

    auto dir_dlg_btn = file_dlg_page->add_button("Seleccionar Carpeta (FileDialogMode::SelectFolder)", 380.0f, 36.0f);
    dir_dlg_btn->set_style(arin::ButtonStyle::secondary());
    dir_dlg_btn->set_icon(arin::IconType::Folder);

    tab_view->add_tab("Dialogos", file_dlg_page);

    // --- Tab 4: Standard ListBox & CheckListBox ---
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

    auto roster_list = std::make_shared<arin::ListBox>();
    for (const auto& p : roster_data) {
        roster_list->add_item(p.name);
    }
    roster_list->set_selected_index(3);

    roster_list->on_selection_changed([&](int index, const std::string& name) {
        if (index >= 0 && index < static_cast<int>(roster_data.size())) {
            const auto& p = roster_data[index];
            action_counter++;
            global_status = "Roster: " + name + " | " + p.position + " | Partidos: " +
                            std::to_string(p.games) + " | Goles: " + std::to_string(p.goals) +
                            " (Accion #" + std::to_string(action_counter) + ")";
        }
    });

    tab_view->add_tab("Roster", roster_list);

    // --- Tab 5: CheckListBox (Funciones de Sistema) ---
    auto features_list = std::make_shared<arin::CheckListBox>();
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
        action_counter++;
        global_status = "Caracteristica " + (checked ? std::string("HABILITADA: ") : std::string("DESHABILITADA: ")) +
                        name + " (Accion #" + std::to_string(action_counter) + ")";
    });

    tab_view->add_tab("Funciones", features_list);

    // --- Tab 6: Closable Release Notes Tab ---
    auto notes_page = std::make_shared<arin::VBox>(0.0f, 0.0f, 12.0f);
    notes_page->set_padding(arin::Padding(24.0f, 24.0f));

    auto close_tab_btn = notes_page->add_button("Cerrar esta Pestana de Notas", 240.0f, 32.0f);
    close_tab_btn->set_style(arin::ButtonStyle::secondary());
    close_tab_btn->on_click([tab_view]() {
        if (tab_view->tab_count() > 5) {
            tab_view->remove_tab(5);
        }
    });

    tab_view->add_tab("Notas v1.0.0", notes_page, true);

    tab_view->on_tab_changed([&](size_t idx) {
        global_status = "Pestana activa cambiada: '" + tab_view->tab_title(idx) + "'";
    });

    tab_view->on_tab_close_requested([tab_view, &global_status](size_t idx) {
        global_status = "Pestana '" + tab_view->tab_title(idx) + "' cerrada.";
        tab_view->remove_tab(idx);
    });

    app.add_widget(tab_view);

    // -------------------------------------------------------------------------
    // 7. Modal FileDialog Component
    // -------------------------------------------------------------------------
    auto file_dialog = std::make_shared<arin::FileDialog>();
    file_dialog->set_bounds(arin::Rect{280.0f, 170.0f, 720.0f, 480.0f});
    file_dialog->hide();

    file_dialog->on_accept([&](const std::string& path) {
        action_counter++;
        global_status = "FileDialog ACEPTADO: \"" + path + "\" (Accion #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] FileDialog accepted path: " << path << "\n";
    });

    file_dialog->on_cancel([&]() {
        global_status = "FileDialog: Accion cancelada por el usuario.";
        std::cout << "[Arin32 Event] FileDialog cancelled.\n";
    });

    // Wire buttons in Tab 3 to FileDialog
    open_dlg_btn->on_click([&, file_dialog]() {
        file_dialog->set_mode(arin::FileDialogMode::OpenFile);
        file_dialog->show();
        global_status = "Modal: Seleccione un archivo para abrir...";
    });

    save_dlg_btn->on_click([&, file_dialog]() {
        file_dialog->set_mode(arin::FileDialogMode::SaveFile);
        file_dialog->show();
        global_status = "Modal: Especifique la ruta y nombre para guardar...";
    });

    dir_dlg_btn->on_click([&, file_dialog]() {
        file_dialog->set_mode(arin::FileDialogMode::SelectFolder);
        file_dialog->show();
        global_status = "Modal: Seleccione un directorio destino...";
    });

    // -------------------------------------------------------------------------
    // 8. Application Top MenuBar
    // -------------------------------------------------------------------------
    auto menu_bar = std::make_shared<arin::MenuBar>(arin::Rect{0.0f, 0.0f, 1280.0f, 28.0f});

    auto& file_menu = menu_bar->add_menu("Archivo");
    file_menu.add_item("Abrir Archivo...", "assets/icons/folder-open.svg", "Ctrl+O", [&]() {
        file_dialog->set_mode(arin::FileDialogMode::OpenFile);
        file_dialog->show();
        global_status = "MenuBar -> Archivo: Abriendo selector de archivos...";
    });
    file_menu.add_item("Guardar Como...", "assets/icons/file.svg", "Ctrl+S", [&]() {
        file_dialog->set_mode(arin::FileDialogMode::SaveFile);
        file_dialog->show();
        global_status = "MenuBar -> Archivo: Abriendo cuadro de guardado...";
    });
    file_menu.add_item("Seleccionar Carpeta...", "assets/icons/folder.svg", [&]() {
        file_dialog->set_mode(arin::FileDialogMode::SelectFolder);
        file_dialog->show();
        global_status = "MenuBar -> Archivo: Abriendo seleccion de carpeta...";
    });
    file_menu.add_separator();
    file_menu.add_item("Salir", "assets/icons/trash.svg", "Alt+F4", [&]() {
        app.close();
    });

    auto& edit_menu = menu_bar->add_menu("Edicion");
    edit_menu.add_item("Deshacer", "assets/icons/refresh.svg", "Ctrl+Z", [&]() {
        global_status = "MenuBar -> Edicion: Deshacer";
    });
    edit_menu.add_item("Rehacer", "Ctrl+Y", [&]() {
        global_status = "MenuBar -> Edicion: Rehacer";
    });
    edit_menu.add_separator();
    edit_menu.add_item("Cortar", "assets/icons/cut.svg", "Ctrl+X", [&]() {
        global_status = "MenuBar -> Edicion: Cortar";
    });
    edit_menu.add_item("Copiar", "assets/icons/copy.svg", "Ctrl+C", [&]() {
        global_status = "MenuBar -> Edicion: Copiar";
    });
    edit_menu.add_item("Pegar", "assets/icons/paste.svg", "Ctrl+V", [&]() {
        global_status = "MenuBar -> Edicion: Pegar";
    });

    auto& view_menu = menu_bar->add_menu("Vistas");
    view_menu.add_item("Arbol de Proyecto (TreeView)", "assets/icons/folder.svg", [&]() {
        tab_view->set_active_tab(0);
    });
    view_menu.add_item("Monitor de Procesos (TableView)", "assets/icons/view-list.svg", [&]() {
        tab_view->set_active_tab(1);
    });
    view_menu.add_item("Dialogo de Archivos (FileDialog)", "assets/icons/folder-open.svg", [&]() {
        tab_view->set_active_tab(2);
    });
    view_menu.add_item("Roster de Jugadores (ListBox)", [&]() {
        tab_view->set_active_tab(3);
    });
    view_menu.add_item("Funciones de Sistema (CheckListBox)", [&]() {
        tab_view->set_active_tab(4);
    });

    auto& tools_menu = menu_bar->add_menu("Herramientas");
    tools_menu.add_item("Abrir Terminal Arin32", "assets/icons/file-code.svg", [&]() {
        action_counter++;
        global_status = "Herramientas: Terminal Arin32 iniciada (Accion #" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] Terminal Arin32 opened\n";
    });
    tools_menu.add_item("Validar Auto-Layouts", "assets/icons/properties.svg", [&]() {
        global_status = "Herramientas: Inspeccion de layouts ejecutada con exito (0 advertencias).";
    });

    auto& help_menu = menu_bar->add_menu("Ayuda");
    help_menu.add_item("Documentacion de Arin32", "assets/icons/file-text.svg", "F1", [&]() {
        global_status = "Ayuda: Revise DOCS.md para la guia completa de componentes y estilos.";
    });
    help_menu.add_item("Acerca de Arin32...", "assets/icons/shield.svg", [&]() {
        global_status = "Arin32 GUI Library v1.0.0 (C++17, OpenGL 3.3+, Licencia BSD 2-Clause)";
    });

    app.add_widget(menu_bar);

    // Add FileDialog modal to app last so it renders on top when shown
    app.add_widget(file_dialog);

    // -------------------------------------------------------------------------
    // 9. Desktop Context Menu System
    // -------------------------------------------------------------------------
    auto ctx_menu = app.create_context_menu();
    ctx_menu->add_default_item("Abrir", "assets/icons/folder-open.svg", [&]() {
        global_status = "Menu Contextual: Abrir";
    });
    ctx_menu->add_item("Anclar a Acceso rapido", "assets/icons/quick-access.svg", [&]() {
        global_status = "Menu Contextual: Anclado a Acceso rapido";
    });
    ctx_menu->add_separator();

    auto access_menu = app.create_context_menu();
    access_menu->add_item("Usuarios especificos...", "assets/icons/network.svg", [&]() {
        global_status = "Conceder acceso: Usuarios especificos";
    });
    access_menu->add_separator();
    access_menu->add_item("Quitar el acceso", "assets/icons/trash.svg", [&]() {
        global_status = "Conceder acceso: Quitar el acceso";
    });
    ctx_menu->add_submenu("Conceder acceso a", access_menu, "assets/icons/network.svg");

    ctx_menu->add_item("Escanear con Seguridad", "assets/icons/shield.svg", [&]() {
        global_status = "Menu Contextual: Escaneo de seguridad iniciado";
    });
    ctx_menu->add_separator();

    ctx_menu->add_item("Cortar", "assets/icons/cut.svg", "Ctrl+X", [&]() {
        global_status = "Menu Contextual: Cortar";
    });
    ctx_menu->add_item("Copiar", "assets/icons/copy.svg", "Ctrl+C", [&]() {
        global_status = "Menu Contextual: Copiar";
    });
    ctx_menu->add_separator();

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
    ctx_menu->add_item("Propiedades", "assets/icons/properties.svg", "Alt+Enter", [&]() {
        global_status = "Menu Contextual: Propiedades";
    });

    app.set_default_context_menu(ctx_menu);

    // Dedicated context menu for roster list
    auto roster_menu = app.create_context_menu();
    roster_menu->add_item("Ver perfil del jugador", "assets/icons/folder-open.svg", [&]() {
        global_status = "Menu Roster: Ver perfil de jugador";
    });
    roster_menu->add_item("Enviar mensaje de equipo", "assets/icons/network.svg", [&]() {
        global_status = "Menu Roster: Mensaje enviado";
    });
    roster_menu->add_separator();
    roster_menu->add_item("Quitar del roster", "assets/icons/trash.svg", "Del", [&]() {
        global_status = "Menu Roster: Jugador retirado";
    });
    roster_list->set_context_menu(roster_menu);

    // -------------------------------------------------------------------------
    // 10. Frame Rendering Callback (Cards, Shadows, Backgrounds & Status Bar)
    // -------------------------------------------------------------------------
    app.on_frame([&](arin::Renderer2D& r) {
        // App Title Banner
        const float header_w = static_cast<float>(r.viewport_width()) - 2.0f * demo_metrics::kPageMargin;
        r.draw_text_in_rect(
            "Arin32 & ArinOS - Comprehensive Desktop UI Showcase",
            arin::Rect(demo_metrics::kPageMargin, 34.0f, header_w, 20.0f),
            arin::Color::from_hex(demo_style::kTextPrimary),
            demo_metrics::kTitleScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "MenuBar, TabView, TreeView, TableView, FileDialog Modal, Inputs, Progress Bars & SVG Icons",
            arin::Rect(demo_metrics::kPageMargin, 53.0f, header_w, 16.0f),
            arin::Color::from_hex(demo_style::kTextSecondary),
            arin::UiMetrics::kMenuLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        // --- Dialog Card Container (Left Top) ---
        r.draw_shadow(
            arin::Rect(left_col_x, dialog_y, left_col_w, dialog_h),
            demo_metrics::kCardCorner,
            arin::Color(0.0f, 0.0f, 0.0f, 0.12f),
            arin::Vec2(0.0f, 2.0f),
            8.0f
        );

        r.draw_rounded_rect(
            arin::Rect(left_col_x, dialog_y, left_col_w, dialog_h),
            demo_metrics::kCardCorner,
            arin::Color::white(),
            arin::Color::from_hex(demo_style::kBorder),
            1.0f
        );

        r.draw_icon(
            arin::IconType::Info,
            arin::Rect(left_col_x + 20.0f, dialog_y + 16.0f, 20.0f, 20.0f),
            arin::Color::from_hex(arin::palette::kAccentBlue)
        );

        r.draw_text_in_rect(
            "Desea guardar los cambios?",
            arin::Rect(left_col_x + 48.0f, dialog_y + 14.0f, left_col_w - 68.0f, 22.0f),
            arin::Color::from_hex(demo_style::kTextStrong),
            demo_metrics::kDialogTitleScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "Nombre del documento y opciones de respaldo:",
            arin::Rect(left_col_x + 20.0f, dialog_y + 44.0f, left_col_w - 40.0f, 16.0f),
            arin::Color::from_hex(demo_style::kTextSecondary),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_rect(
            arin::Rect(left_col_x + 1.0f, footer_y, left_col_w - 2.0f, 1.0f),
            arin::Color::from_hex(demo_style::kSoftBorder)
        );

        r.draw_rounded_rect(
            arin::Rect(left_col_x + 1.0f, footer_y + 1.0f, left_col_w - 2.0f, dialog_h - 148.0f),
            demo_metrics::kInnerCorner,
            arin::Color::from_hex(demo_style::kFooterFill)
        );
        r.draw_rect(
            arin::Rect(left_col_x + 1.0f, footer_y + 1.0f, left_col_w - 2.0f, 10.0f),
            arin::Color::from_hex(demo_style::kFooterFill)
        );

        // --- Progress Bars Card ---
        r.draw_rounded_rect(
            arin::Rect(left_col_x, pb_y, left_col_w, 155.0f),
            demo_metrics::kCardCorner,
            arin::Color::white(),
            arin::Color::from_hex(demo_style::kBorder),
            1.0f
        );

        std::string det_label = "Copiando archivos... (" +
                                std::to_string(static_cast<int>(det_bar->percentage() * 100.0f)) +
                                "% - Determinado con Shimmer Sweep)";
        r.draw_text_in_rect(
            det_label,
            arin::Rect(left_col_x + 16.0f, pb_y + 10.0f, left_col_w - 32.0f, 16.0f),
            arin::Color::from_hex(demo_style::kTextPrimary),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "Buscando actualizaciones... (Indeterminado Traveling Chunk)",
            arin::Rect(left_col_x + 16.0f, pb_y + 80.0f, left_col_w - 32.0f, 16.0f),
            arin::Color::from_hex(demo_style::kTextPrimary),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        // --- Action Buttons Card ---
        r.draw_rounded_rect(
            arin::Rect(left_col_x, palette_y, left_col_w, 132.0f),
            demo_metrics::kCardCorner,
            arin::Color::white(),
            arin::Color::from_hex(demo_style::kBorder),
            1.0f
        );

        r.draw_text_in_rect(
            "Paleta de Botones de Accion (Auto-Layout):",
            arin::Rect(left_col_x + 16.0f, palette_y + 6.0f, left_col_w - 32.0f, 18.0f),
            arin::Color::from_hex(demo_style::kTextStrong),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "Distribucion simetrica y alineacion dinamica de widgets",
            arin::Rect(left_col_x + 16.0f, palette_y + 104.0f, left_col_w - 32.0f, 16.0f),
            arin::Color::from_hex(demo_style::kTextMuted),
            arin::UiMetrics::kMenuShortcutScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        // --- Vector Icons Showcase Card ---
        r.draw_rounded_rect(
            arin::Rect(left_col_x, icons_y, left_col_w, 150.0f),
            demo_metrics::kCardCorner,
            arin::Color::white(),
            arin::Color::from_hex(demo_style::kBorder),
            1.0f
        );

        r.draw_text_in_rect(
            "Arsenal de Iconos Vectoriales GPU & Imagen:",
            arin::Rect(left_col_x + 16.0f, icons_y + 6.0f, left_col_w - 32.0f, 18.0f),
            arin::Color::from_hex(demo_style::kTextStrong),
            arin::UiMetrics::kSmallLabelScale,
            arin::TextAlignH::Left,
            arin::TextAlignV::Center
        );

        r.draw_text_in_rect(
            "Imagen Cargada",
            arin::Rect(left_col_x + 16.0f, icons_y + 120.0f, 96.0f, 16.0f),
            arin::Color::from_hex(demo_style::kTextMuted),
            arin::UiMetrics::kMenuShortcutScale,
            arin::TextAlignH::Center,
            arin::TextAlignV::Center
        );

        // Draw sample icons
        const float icon_grid_x = left_col_x + 125.0f;
        const float icon_grid_y = icons_y + 32.0f;
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

        for (size_t i = 0; i < sample_icons_row1.size(); ++i) {
            float ix = icon_grid_x + i * icon_step;
            float iy = icon_grid_y;
            const arin::Rect badge(ix, iy, demo_metrics::kIconBadgeSize, demo_metrics::kIconBadgeSize);
            r.draw_rounded_rect(badge, demo_metrics::kInnerCorner, arin::Color::from_hex(demo_style::kBadgeFill),
                                arin::Color::from_hex(demo_style::kSoftBorder), demo_metrics::kBorderWidth);
            r.draw_icon(sample_icons_row1[i].first, badge.centered(arin::Vec2(demo_metrics::kIconGlyphSize, demo_metrics::kIconGlyphSize)),
                        arin::Color::from_hex(arin::palette::kAccentBlue));
            r.draw_text_in_rect(sample_icons_row1[i].second, arin::Rect(ix, iy + demo_metrics::kIconBadgeSize, demo_metrics::kIconBadgeSize, 12.0f),
                                arin::Color::from_hex(demo_style::kTextMuted), demo_metrics::kIconLabelScale, arin::TextAlignH::Center, arin::TextAlignV::Top);
        }

        for (size_t i = 0; i < sample_icons_row2.size(); ++i) {
            float ix = icon_grid_x + i * icon_step;
            float iy = icon_grid_y + 48.0f;
            const arin::Rect badge(ix, iy, demo_metrics::kIconBadgeSize, demo_metrics::kIconBadgeSize);
            r.draw_rounded_rect(badge, demo_metrics::kInnerCorner, arin::Color::from_hex(demo_style::kBadgeFill),
                                arin::Color::from_hex(demo_style::kSoftBorder), demo_metrics::kBorderWidth);
            r.draw_icon(sample_icons_row2[i].first, badge.centered(arin::Vec2(demo_metrics::kIconGlyphSize, demo_metrics::kIconGlyphSize)),
                        arin::Color::from_hex(demo_style::kTextPrimary));
            r.draw_text_in_rect(sample_icons_row2[i].second, arin::Rect(ix, iy + demo_metrics::kIconBadgeSize, demo_metrics::kIconBadgeSize, 12.0f),
                                arin::Color::from_hex(demo_style::kTextMuted), demo_metrics::kIconLabelScale, arin::TextAlignH::Center, arin::TextAlignV::Top);
        }

        // --- Interactive Status Bar ---
        const arin::Rect status_bar(demo_metrics::kPageMargin, 748.0f, static_cast<float>(r.viewport_width()) - 2.0f * demo_metrics::kPageMargin, 28.0f);
        r.draw_rounded_rect(status_bar, demo_metrics::kInnerCorner, arin::Color::white(), arin::Color::from_hex(demo_style::kBorder), 1.0f);
        r.draw_text_in_rect(global_status, status_bar.inset(arin::Padding(10.0f, 0.0f)), arin::Color::from_hex(demo_style::kTextStrong),
                            arin::UiMetrics::kSmallLabelScale, arin::TextAlignH::Left, arin::TextAlignV::Center);

        // Footer copyright info
        r.draw_text_centered(
            "Arin32 GUI Library * Copyright (c) 2026, Arin32 & ArinOS Contributors * Licencia BSD 2-Clause * C++17 OpenGL",
            arin::Rect(0.0f, 786.0f, static_cast<float>(r.viewport_width()), 18.0f),
            arin::Color::from_hex(demo_style::kTextMuted),
            arin::UiMetrics::kMenuLabelScale
        );
    });

    // Screenshot automation
    if (!screenshot_path.empty()) {
        int captured_frames = 0;
        app.on_after_frame([&](arin::Renderer2D& r) {
            captured_frames++;
            det_bar->set_anim_phase(0.40f);
            indet_bar->set_anim_phase(0.45f);

            if (screenshot_context_menu && captured_frames >= 2) {
                if (captured_frames == 2) {
                    app.show_context_menu(ctx_menu, 220.0f, 100.0f);
                    ctx_menu->open_child_menu(3);
                }
                ctx_menu->set_hovered_index(3);
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
    std::cout << " Arin32 Comprehensive Desktop UI Showcase v1.0.0" << std::endl;
    std::cout << " Componentes activos: MenuBar, TabView, TreeView, TableView," << std::endl;
    std::cout << "                     FileDialog, Inputs, Buttons, Vector Icons" << std::endl;
    std::cout << " Licencia: BSD 2-Clause | C++17 OpenGL" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // 11. Run Application Main Loop
    // -------------------------------------------------------------------------
    app.run();

    return 0;
}
