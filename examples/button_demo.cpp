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
 * @brief Interactive showcase demonstrating Arin32's Button widget with the exact
 *        visual styling, dimensions, and corner radius from the ArinOS design reference.
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

int main(int argc, char** argv) {
    std::string screenshot_path;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--screenshot" && i + 1 < argc) {
            screenshot_path = argv[++i];
        }
    }

    // -------------------------------------------------------------------------
    // 1. Create the Application Window (800x600 pixels)
    // -------------------------------------------------------------------------
    arin::App app("Arin32 & ArinOS - Button Showcase", 800, 600);

    // Default neutral grey background for the desktop/window canvas
    app.theme().background_color = arin::Color::from_hex(0xA8A8A8);

    std::string dialog_status = "Status: Waiting for user action...";
    int action_counter = 0;
    bool checkbox_checked = false;

    // -------------------------------------------------------------------------
    // 2. Exact Dialog Box Buttons (Save, Don't Save, Cancel)
    //    Dimensions: Height = 32px, Corner Radius = 4.5px, exact styling
    // -------------------------------------------------------------------------
    const float dialog_x = 215.0f;
    const float dialog_y = 80.0f;
    const float dialog_w = 370.0f;
    const float dialog_h = 250.0f;
    const float footer_y = dialog_y + 175.0f;
    const float btn_y = footer_y + 20.0f;

    // Button 1: "Save" (Primary Accent Blue #0067C0, 82x32, radius 4.5)
    auto save_btn = app.add_button("Save", dialog_x + 24.0f, btn_y, 82.0f, 32.0f);
    save_btn->set_style(arin::ButtonStyle::primary());

    save_btn->on_click([&]() {
        action_counter++;
        dialog_status = "Status: Work Saved! (" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] 'Save' button clicked! Action #" << action_counter << std::endl;
    });

    // Button 2: "Don't Save" (Secondary White with 1px border, 96x32, radius 4.5)
    auto dont_save_btn = app.add_button("Don't Save", dialog_x + 118.0f, btn_y, 96.0f, 32.0f);
    dont_save_btn->set_style(arin::ButtonStyle::secondary());

    dont_save_btn->on_click([&]() {
        action_counter++;
        dialog_status = "Status: Work Discarded. (" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] 'Don't Save' button clicked! Action #" << action_counter << std::endl;
    });

    // Button 3: "Cancel" (Secondary White with 1px border, 80x32, radius 4.5)
    auto cancel_btn = app.add_button("Cancel", dialog_x + 226.0f, btn_y, 80.0f, 32.0f);
    cancel_btn->set_style(arin::ButtonStyle::secondary());

    cancel_btn->on_click([&]() {
        action_counter++;
        dialog_status = "Status: Action Cancelled. (" + std::to_string(action_counter) + ")";
        std::cout << "[Arin32 Event] 'Cancel' button clicked! Action #" << action_counter << std::endl;
    });

    // -------------------------------------------------------------------------
    // 3. Additional Palette Showcase: Success, Danger, Outline, Disabled
    //    All sharing the exact same 32px height, 4.5px radius, and form factor!
    // -------------------------------------------------------------------------
    const float palette_y = 415.0f;

    auto success_btn = app.add_button("Success", 125.0f, palette_y, 100.0f, 32.0f);
    success_btn->set_style(arin::ButtonStyle::success());
    success_btn->on_click([&]() {
        dialog_status = "Status: Success action triggered.";
        std::cout << "[Arin32 Event] Success button clicked!\n";
    });

    auto danger_btn = app.add_button("Danger", 237.0f, palette_y, 100.0f, 32.0f);
    danger_btn->set_style(arin::ButtonStyle::danger());
    danger_btn->on_click([&]() {
        dialog_status = "Status: Danger action triggered.";
        std::cout << "[Arin32 Event] Danger button clicked!\n";
    });

    auto outline_btn = app.add_button("Outline", 349.0f, palette_y, 100.0f, 32.0f);
    outline_btn->set_style(arin::ButtonStyle::outline(arin::Color::from_hex(0x0067C0)));
    outline_btn->on_click([&]() {
        dialog_status = "Status: Outline button clicked.";
        std::cout << "[Arin32 Event] Outline button clicked!\n";
    });

    auto disabled_btn = app.add_button("Disabled", 461.0f, palette_y, 100.0f, 32.0f);
    disabled_btn->set_style(arin::ButtonStyle::secondary())
                .set_enabled(false);

    auto exit_btn = app.add_button("Exit Demo", 573.0f, palette_y, 100.0f, 32.0f);
    exit_btn->set_style(arin::ButtonStyle::secondary());
    exit_btn->on_click([&]() {
        std::cout << "[Arin32 Event] Closing demo application.\n";
        app.close();
    });

    // -------------------------------------------------------------------------
    // 4. Custom Frame Drawing: Dialog Card, Typography & UI Structure
    // -------------------------------------------------------------------------
    app.on_frame([&](arin::Renderer2D& r) {
        // App Header Banner
        r.draw_text_centered(
            "Arin32 & ArinOS - Interactive Button & Dialog Showcase",
            arin::Rect(0.0f, 18.0f, static_cast<float>(r.viewport_width()), 24.0f),
            arin::Color::from_hex(0x1F2937),
            1.2f
        );

        // Subtitle note
        r.draw_text_centered(
            "Exact button shape, corner radius (4.5px), and dimensions matching ArinOS specification",
            arin::Rect(0.0f, 42.0f, static_cast<float>(r.viewport_width()), 18.0f),
            arin::Color::from_hex(0x4B5563),
            0.9f
        );

        // --- Dialog Card Container ---
        // 1. Soft Card Shadow
        r.draw_shadow(
            arin::Rect(dialog_x, dialog_y, dialog_w, dialog_h),
            8.0f,
            arin::Color(0.0f, 0.0f, 0.0f, 0.18f),
            arin::Vec2(0.0f, 4.0f),
            12.0f
        );

        // 2. White Card Background
        r.draw_rounded_rect(
            arin::Rect(dialog_x, dialog_y, dialog_w, dialog_h),
            8.0f,
            arin::Color::white(),
            arin::Color::from_hex(0xE5E7EB),
            1.0f
        );

        // 3. Dialog Header & Body Text
        r.draw_text(
            "Save your work?",
            arin::Vec2(dialog_x + 24.0f, dialog_y + 24.0f),
            arin::Color::from_hex(0x111827),
            1.3f
        );

        r.draw_text(
            "Lorem ipsum dolor sit amet, adipisicing elit.",
            arin::Vec2(dialog_x + 24.0f, dialog_y + 66.0f),
            arin::Color::from_hex(0x374151),
            0.95f
        );

        // 4. Checkbox Preview (18x18, corner radius 3.5px, 1px border)
        const float chk_x = dialog_x + 24.0f;
        const float chk_y = dialog_y + 98.0f;
        r.draw_rounded_rect(
            arin::Rect(chk_x, chk_y, 18.0f, 18.0f),
            3.5f,
            arin::Color::white(),
            arin::Color::from_hex(0x9CA3AF),
            1.0f
        );

        r.draw_text(
            "Upload your content to the cloud.",
            arin::Vec2(chk_x + 28.0f, chk_y + 1.0f),
            arin::Color::from_hex(0x1F2937),
            0.95f
        );

        // 5. Divider Line separating top card from footer
        r.draw_rect(
            arin::Rect(dialog_x, footer_y, dialog_w, 1.0f),
            arin::Color::from_hex(0xE5E7EB)
        );

        // 6. Dialog Footer Background (#F3F3F3)
        // Draw lower rounded section for the footer
        r.draw_rounded_rect(
            arin::Rect(dialog_x, footer_y + 1.0f, dialog_w, dialog_h - 191.0f),
            8.0f,
            arin::Color::from_hex(0xF3F4F6)
        );

        // Re-cover the upper half of the footer with a sharp rectangle so top corners remain straight
        r.draw_rect(
            arin::Rect(dialog_x, footer_y + 1.0f, dialog_w, 20.0f),
            arin::Color::from_hex(0xF3F4F6)
        );

        // --- Bottom Palette Showcase Section ---
        r.draw_text_centered(
            "Extended Color Palette (Height: 32px, Radius: 4.5px):",
            arin::Rect(0.0f, 395.0f, static_cast<float>(r.viewport_width()), 20.0f),
            arin::Color::from_hex(0x1F2937),
            0.95f
        );

        // Dynamic Status Bar
        r.draw_text_centered(
            dialog_status,
            arin::Rect(0.0f, 490.0f, static_cast<float>(r.viewport_width()), 24.0f),
            arin::Color::from_hex(0x111827),
            1.0f
        );

        // Footer copyright info
        r.draw_text_centered(
            "Copyright (c) 2026, Arin32 & ArinOS Contributors * BSD 2-Clause License",
            arin::Rect(0.0f, 560.0f, static_cast<float>(r.viewport_width()), 20.0f),
            arin::Color::from_hex(0x4B5563),
            0.85f
        );
    });

    // If requested, take an automated screenshot after buttons render and close
    if (!screenshot_path.empty()) {
        int captured_frames = 0;
        app.on_after_frame([&](arin::Renderer2D& r) {
            captured_frames++;
            if (captured_frames >= 2) {
                save_screenshot_ppm(screenshot_path, r.viewport_width(), r.viewport_height());
                std::cout << "[Arin32] Frame rendered! Screenshot saved to " << screenshot_path << std::endl;
                app.close();
            }
        });
    }

    std::cout << "==========================================================" << std::endl;
    std::cout << " Arin32 & ArinOS Button Showcase started." << std::endl;
    std::cout << " OS: Linux / FreeBSD agnostic" << std::endl;
    std::cout << " License: BSD 2-Clause" << std::endl;
    std::cout << " Contributors: Arin32 & ArinOS Contributors" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // 5. Run the Application Main Loop
    // -------------------------------------------------------------------------
    app.run();

    return 0;
}
