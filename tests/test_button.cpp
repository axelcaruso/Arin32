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
#include <arin/button.hpp>

TEST(ArinButtonTest, DefaultAndParametricConstruction) {
    arin::Button btn1;
    EXPECT_EQ(btn1.text(), "Button");
    EXPECT_EQ(btn1.state(), arin::ButtonState::Normal);
    EXPECT_TRUE(btn1.is_enabled());

    arin::Button btn2("Submit", 50.0f, 60.0f, 150.0f, 45.0f);
    EXPECT_EQ(btn2.text(), "Submit");
    EXPECT_FLOAT_EQ(btn2.bounds().x, 50.0f);
    EXPECT_FLOAT_EQ(btn2.bounds().y, 60.0f);
    EXPECT_FLOAT_EQ(btn2.bounds().width, 150.0f);
    EXPECT_FLOAT_EQ(btn2.bounds().height, 45.0f);
}

TEST(ArinButtonTest, FluentChainingAPI) {
    arin::Button btn;
    btn.set_text("Fluent Action")
       .set_position(20.0f, 30.0f)
       .set_size(100.0f, 50.0f)
       .set_corner_radius(12.0f);

    EXPECT_EQ(btn.text(), "Fluent Action");
    EXPECT_FLOAT_EQ(btn.bounds().x, 20.0f);
    EXPECT_FLOAT_EQ(btn.bounds().y, 30.0f);
    EXPECT_FLOAT_EQ(btn.bounds().width, 100.0f);
    EXPECT_FLOAT_EQ(btn.bounds().height, 50.0f);
    EXPECT_FLOAT_EQ(btn.style().corner_radius, 12.0f);
}

TEST(ArinButtonTest, HoverStateTransitions) {
    arin::Button btn("Hover Test", 100.0f, 100.0f, 100.0f, 50.0f);

    bool hover_cb_invoked = false;
    bool hover_val = false;
    btn.on_hover([&](bool hovered) {
        hover_cb_invoked = true;
        hover_val = hovered;
    });

    // Move cursor over button
    arin::MouseEvent move_in;
    move_in.type = arin::MouseEventType::Move;
    move_in.position = arin::Vec2(150.0f, 125.0f);

    bool handled = btn.handle_mouse(move_in);
    EXPECT_TRUE(handled);
    EXPECT_EQ(btn.state(), arin::ButtonState::Hovered);
    EXPECT_TRUE(btn.is_hovered());
    EXPECT_TRUE(hover_cb_invoked);
    EXPECT_TRUE(hover_val);

    // Move cursor outside button
    arin::MouseEvent move_out;
    move_out.type = arin::MouseEventType::Move;
    move_out.position = arin::Vec2(50.0f, 50.0f);

    handled = btn.handle_mouse(move_out);
    EXPECT_FALSE(handled);
    EXPECT_EQ(btn.state(), arin::ButtonState::Normal);
    EXPECT_FALSE(btn.is_hovered());
    EXPECT_FALSE(hover_val);
}

TEST(ArinButtonTest, ClickEventLifecycle) {
    arin::Button btn("Click Test", 100.0f, 100.0f, 100.0f, 50.0f);

    int clicks = 0;
    btn.on_click([&]() {
        clicks++;
    });

    // 1. Move inside
    arin::MouseEvent ev_move;
    ev_move.type = arin::MouseEventType::Move;
    ev_move.position = arin::Vec2(120.0f, 120.0f);
    btn.handle_mouse(ev_move);
    EXPECT_EQ(btn.state(), arin::ButtonState::Hovered);

    // 2. Press mouse down inside
    arin::MouseEvent ev_down;
    ev_down.type = arin::MouseEventType::ButtonDown;
    ev_down.button = arin::MouseButton::Left;
    ev_down.action = arin::InputAction::Press;
    ev_down.position = arin::Vec2(120.0f, 120.0f);
    btn.handle_mouse(ev_down);
    EXPECT_EQ(btn.state(), arin::ButtonState::Pressed);
    EXPECT_TRUE(btn.is_pressed());
    EXPECT_EQ(clicks, 0); // No click yet until release

    // 3. Release mouse inside
    arin::MouseEvent ev_up;
    ev_up.type = arin::MouseEventType::ButtonUp;
    ev_up.button = arin::MouseButton::Left;
    ev_up.action = arin::InputAction::Release;
    ev_up.position = arin::Vec2(120.0f, 120.0f);
    btn.handle_mouse(ev_up);
    EXPECT_EQ(btn.state(), arin::ButtonState::Hovered);
    EXPECT_EQ(clicks, 1); // Click successfully triggered!
}

TEST(ArinButtonTest, ClickCancelledWhenReleasedOutside) {
    arin::Button btn("Cancel Test", 100.0f, 100.0f, 100.0f, 50.0f);

    int clicks = 0;
    btn.on_click([&]() {
        clicks++;
    });

    // Press down inside
    arin::MouseEvent ev_down;
    ev_down.type = arin::MouseEventType::ButtonDown;
    ev_down.button = arin::MouseButton::Left;
    ev_down.action = arin::InputAction::Press;
    ev_down.position = arin::Vec2(120.0f, 120.0f);
    btn.handle_mouse(ev_down);
    EXPECT_EQ(btn.state(), arin::ButtonState::Pressed);

    // Drag cursor outside
    arin::MouseEvent ev_move_out;
    ev_move_out.type = arin::MouseEventType::Move;
    ev_move_out.position = arin::Vec2(10.0f, 10.0f);
    btn.handle_mouse(ev_move_out);

    // Release outside
    arin::MouseEvent ev_up;
    ev_up.type = arin::MouseEventType::ButtonUp;
    ev_up.button = arin::MouseButton::Left;
    ev_up.action = arin::InputAction::Release;
    ev_up.position = arin::Vec2(10.0f, 10.0f);
    btn.handle_mouse(ev_up);

    // Click must NOT be triggered
    EXPECT_EQ(clicks, 0);
}

TEST(ArinButtonTest, DisabledButtonIgnoresInputs) {
    arin::Button btn("Disabled Test", 100.0f, 100.0f, 100.0f, 50.0f);
    btn.set_enabled(false);

    int clicks = 0;
    btn.on_click([&]() { clicks++; });

    EXPECT_FALSE(btn.is_enabled());
    EXPECT_EQ(btn.state(), arin::ButtonState::Disabled);

    arin::MouseEvent ev_down;
    ev_down.type = arin::MouseEventType::ButtonDown;
    ev_down.button = arin::MouseButton::Left;
    ev_down.position = arin::Vec2(120.0f, 120.0f);

    bool handled = btn.handle_mouse(ev_down);
    EXPECT_FALSE(handled);
    EXPECT_EQ(clicks, 0);
}

TEST(ArinButtonTest, DetailedClickCallbackModifiesButton) {
    arin::Button btn("Initial", 100.0f, 100.0f, 100.0f, 50.0f);

    btn.on_click([](arin::Button& b) {
        b.set_text("Updated!");
    });

    arin::MouseEvent ev_down;
    ev_down.type = arin::MouseEventType::ButtonDown;
    ev_down.button = arin::MouseButton::Left;
    ev_down.position = arin::Vec2(120.0f, 120.0f);
    btn.handle_mouse(ev_down);

    arin::MouseEvent ev_up;
    ev_up.type = arin::MouseEventType::ButtonUp;
    ev_up.button = arin::MouseButton::Left;
    ev_up.position = arin::Vec2(120.0f, 120.0f);
    btn.handle_mouse(ev_up);

    EXPECT_EQ(btn.text(), "Updated!");
}

TEST(ArinButtonTest, AutoResizeEnsuresTextNeverOverflows) {
    arin::Font font;
    arin::Button btn("This is a very long button label", 10.0f, 10.0f, 40.0f, 32.0f);

    EXPECT_TRUE(btn.is_auto_resize());

    // Before explicit fit, bounds width was set to 40
    EXPECT_FLOAT_EQ(btn.bounds().width, 40.0f);

    // Call fit_to_text
    btn.fit_to_text(font, 14.0f);

    // Width must have expanded significantly to contain the label
    arin::Vec2 text_sz = font.measure_text("This is a very long button label");
    EXPECT_GE(btn.bounds().width, text_sz.x + 28.0f);
}

TEST(ArinButtonTest, DisableAutoResizeMaintainsFixedBounds) {
    arin::Button btn("Fixed Width", 0.0f, 0.0f, 100.0f, 32.0f);
    btn.set_auto_resize(false);
    EXPECT_FALSE(btn.is_auto_resize());
    EXPECT_FLOAT_EQ(btn.bounds().width, 100.0f);
}
