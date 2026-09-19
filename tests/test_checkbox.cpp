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

#include <gtest/gtest.h>
#include <arin/checkbox.hpp>

TEST(ArinCheckBoxTest, DefaultAndParametricConstruction) {
    arin::CheckBox cb1;
    EXPECT_EQ(cb1.label(), "CheckBox");
    EXPECT_FALSE(cb1.is_checked());
    EXPECT_TRUE(cb1.is_enabled());
    EXPECT_TRUE(cb1.is_visible());

    arin::CheckBox cb2("Enable Cloud Sync", 40.0f, 50.0f, true);
    EXPECT_EQ(cb2.label(), "Enable Cloud Sync");
    EXPECT_TRUE(cb2.is_checked());
    EXPECT_FLOAT_EQ(cb2.bounds().x, 40.0f);
    EXPECT_FLOAT_EQ(cb2.bounds().y, 50.0f);
}

TEST(ArinCheckBoxTest, FluentChainingAndToggle) {
    arin::CheckBox cb;
    cb.set_label("Auto Save")
      .set_position(10.0f, 20.0f)
      .set_size(140.0f, 24.0f)
      .set_checked(true);

    EXPECT_EQ(cb.label(), "Auto Save");
    EXPECT_FLOAT_EQ(cb.bounds().x, 10.0f);
    EXPECT_FLOAT_EQ(cb.bounds().y, 20.0f);
    EXPECT_FLOAT_EQ(cb.bounds().width, 140.0f);
    EXPECT_FLOAT_EQ(cb.bounds().height, 24.0f);
    EXPECT_TRUE(cb.is_checked());

    cb.toggle();
    EXPECT_FALSE(cb.is_checked());
}

TEST(ArinCheckBoxTest, ToggleCallbackTriggering) {
    arin::CheckBox cb("Notifications", 0.0f, 0.0f, false);

    int callback_count = 0;
    bool last_state = false;

    cb.on_toggled([&](bool checked) {
        callback_count++;
        last_state = checked;
    });

    cb.set_checked(true);
    EXPECT_EQ(callback_count, 1);
    EXPECT_TRUE(last_state);

    // Setting same state should not re-trigger callback
    cb.set_checked(true);
    EXPECT_EQ(callback_count, 1);

    cb.toggle();
    EXPECT_EQ(callback_count, 2);
    EXPECT_FALSE(last_state);
}

TEST(ArinCheckBoxTest, MouseClickInteraction) {
    arin::CheckBox cb("Remember Me", 50.0f, 50.0f, false);
    cb.set_size(150.0f, 24.0f);

    int toggled = 0;
    cb.on_toggled([&](bool) { toggled++; });

    // Press down inside
    arin::MouseEvent down = arin::MouseEvent::make_button_down(arin::Vec2(60.0f, 60.0f));
    EXPECT_TRUE(cb.handle_mouse(down));
    EXPECT_EQ(toggled, 0);

    // Release inside -> toggles state
    arin::MouseEvent up = arin::MouseEvent::make_button_up(arin::Vec2(60.0f, 60.0f));
    EXPECT_TRUE(cb.handle_mouse(up));
    EXPECT_EQ(toggled, 1);
    EXPECT_TRUE(cb.is_checked());

    // Press down inside, move out, release -> cancelled
    cb.handle_mouse(down);
    arin::MouseEvent up_outside = arin::MouseEvent::make_button_up(arin::Vec2(300.0f, 300.0f));
    cb.handle_mouse(up_outside);
    EXPECT_EQ(toggled, 1);
    EXPECT_TRUE(cb.is_checked());
}

TEST(ArinCheckBoxTest, DisabledStateIgnoresMouse) {
    arin::CheckBox cb("Disabled Option", 0.0f, 0.0f, false);
    cb.set_size(100.0f, 20.0f);
    cb.set_enabled(false);

    int toggled = 0;
    cb.on_toggled([&](bool) { toggled++; });

    arin::MouseEvent down = arin::MouseEvent::make_button_down(arin::Vec2(10.0f, 10.0f));
    EXPECT_FALSE(cb.handle_mouse(down));

    arin::MouseEvent up = arin::MouseEvent::make_button_up(arin::Vec2(10.0f, 10.0f));
    EXPECT_FALSE(cb.handle_mouse(up));
    EXPECT_EQ(toggled, 0);
    EXPECT_FALSE(cb.is_checked());
}

TEST(ArinCheckBoxTest, AutomaticContentSizing) {
    arin::CheckBox short_cb("OK", 0.0f, 0.0f);
    arin::CheckBox long_cb("Upload your content to the cloud.", 0.0f, 0.0f);

    EXPECT_GT(long_cb.bounds().width, short_cb.bounds().width);
    EXPECT_GE(long_cb.bounds().height, 20.0f);

    arin::Font font;
    arin::Vec2 long_text_size = font.measure_text("Upload your content to the cloud.", long_cb.style().text_scale);
    float expected_min_w = long_cb.style().box_size + long_cb.style().text_spacing + long_text_size.x;
    EXPECT_GE(long_cb.bounds().width, expected_min_w);
}

TEST(ArinCheckBoxTest, DynamicLabelUpdatesAndAutoResize) {
    arin::CheckBox cb("Short", 10.0f, 20.0f);
    float initial_width = cb.bounds().width;

    cb.set_label("A significantly longer label string that requires extra width");
    EXPECT_GT(cb.bounds().width, initial_width);

    // Explicit set_size disables auto_resize
    cb.set_size(120.0f, 24.0f);
    EXPECT_FALSE(cb.is_auto_resize());
    EXPECT_FLOAT_EQ(cb.bounds().width, 120.0f);

    cb.set_label("Even longer text but auto_resize is now false");
    EXPECT_FLOAT_EQ(cb.bounds().width, 120.0f);

    // Re-enabling auto_resize and fitting
    cb.set_auto_resize(true).fit_to_content();
    EXPECT_TRUE(cb.is_auto_resize());
    EXPECT_GT(cb.bounds().width, 120.0f);
}

TEST(ArinCheckBoxTest, EnsureContainmentGrowsBounds) {
    arin::CheckBox cb("Upload your content to the cloud.", 0.0f, 0.0f);
    arin::Font font;

    cb.set_auto_resize(true);
    cb.ensure_containment(font);

    arin::Vec2 text_size = font.measure_text(cb.label(), cb.style().text_scale);
    float required_w = cb.style().box_size + cb.style().text_spacing + text_size.x;
    EXPECT_GE(cb.bounds().width, required_w);
}
