/*
 * Arin32 - Modern OpenGL Graphical User Interface Library
 *
 * Copyright (c) 2026, Arin32 Contributors
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
 * @brief Interactive showcase demonstrating Arin32's absurdly simple Button API.
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
    arin::App app("Arin32 - Interactive Button Showcase", 800, 600);

    int click_count = 0;
    bool is_dark_mode = true;

    // -------------------------------------------------------------------------
    // 2. Add Buttons using the Fluent & Absurdly Simple API
    // -------------------------------------------------------------------------

    // Primary Interactive Counter Button
    auto counter_btn = app.add_button("Click Me! (0 clicks)", 260.0f, 150.0f, 280.0f, 52.0f);
    counter_btn->set_style(arin::ButtonStyle::primary())
               .set_corner_radius(10.0f);

    counter_btn->on_click([&]() {
        click_count++;
        counter_btn->set_text("Clicks: " + std::to_string(click_count));
        std::cout << "[Arin32 Event] Counter button clicked! Total clicks: " << click_count << std::endl;
    });

    // Success Button: Toggle Theme
    auto theme_btn = app.add_button("Toggle Theme (Dark/Light)", 260.0f, 220.0f, 280.0f, 52.0f);
    theme_btn->set_style(arin::ButtonStyle::success())
             .set_corner_radius(10.0f);

    theme_btn->on_click([&]() {
        is_dark_mode = !is_dark_mode;
        if (is_dark_mode) {
            app.theme() = arin::Theme::dark();
            std::cout << "[Arin32 Event] Switched to Dark Theme." << std::endl;
        } else {
            app.theme() = arin::Theme::light();
            std::cout << "[Arin32 Event] Switched to Light Theme." << std::endl;
        }
    });

    // Danger Button: Reset Counter
    auto reset_btn = app.add_button("Reset Counter", 260.0f, 290.0f, 280.0f, 52.0f);
    reset_btn->set_style(arin::ButtonStyle::danger())
             .set_corner_radius(10.0f);

    reset_btn->on_click([&]() {
        click_count = 0;
        counter_btn->set_text("Click Me! (0 clicks)");
        std::cout << "[Arin32 Event] Counter was reset to 0." << std::endl;
    });

    // Outline Button: Close Application
    auto exit_btn = app.add_button("Exit Application", 260.0f, 360.0f, 280.0f, 52.0f);
    exit_btn->set_style(arin::ButtonStyle::outline(arin::Color::from_hex(0x38BDF8)))
            .set_corner_radius(10.0f);

    exit_btn->on_click([&]() {
        std::cout << "[Arin32 Event] Exit button clicked. Closing application." << std::endl;
        app.close();
    });

    // Disabled Button example
    auto disabled_btn = app.add_button("Disabled Button (Inactive)", 260.0f, 430.0f, 280.0f, 52.0f);
    disabled_btn->set_enabled(false)
                .set_corner_radius(10.0f);

    // -------------------------------------------------------------------------
    // 3. Custom Frame Drawing: Headers, Subtitles & Info
    // -------------------------------------------------------------------------
    app.on_frame([&](arin::Renderer2D& r) {
        // App Title Banner
        r.draw_text_centered(
            "Arin32 GUI Library",
            arin::Rect(0.0f, 40.0f, static_cast<float>(r.viewport_width()), 30.0f),
            r.viewport_width() > 0 && is_dark_mode ? arin::Color::white() : arin::Color::from_hex(0x0F172A),
            1.6f
        );

        // Subtitle & OS Agnostic Note
        r.draw_text_centered(
            "OpenGL 3.3 Core Profile * OS Agnostic * BSD License",
            arin::Rect(0.0f, 80.0f, static_cast<float>(r.viewport_width()), 20.0f),
            arin::Color::from_hex(0x94A3B8),
            1.0f
        );

        // Footer hint
        r.draw_text_centered(
            "Absurdly easy API: Add buttons, attach lambdas, and run!",
            arin::Rect(0.0f, 520.0f, static_cast<float>(r.viewport_width()), 20.0f),
            arin::Color::from_hex(0x64748B),
            0.9f
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
    std::cout << " Arin32 Button Showcase started." << std::endl;
    std::cout << " OS: Linux / FreeBSD agnostic" << std::endl;
    std::cout << " License: BSD 2-Clause" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // 4. Run the Application Main Loop
    // -------------------------------------------------------------------------
    app.run();

    return 0;
}
