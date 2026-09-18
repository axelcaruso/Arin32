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

#include <gtest/gtest.h>
#include <arin/input.hpp>

TEST(ArinInputTest, MouseMoveTracking) {
    arin::InputState input;
    EXPECT_FLOAT_EQ(input.mouse_pos().x, 0.0f);
    EXPECT_FLOAT_EQ(input.mouse_pos().y, 0.0f);

    arin::MouseEvent ev = input.on_mouse_move(250.0f, 180.0f);
    EXPECT_EQ(ev.type, arin::MouseEventType::Move);
    EXPECT_FLOAT_EQ(ev.position.x, 250.0f);
    EXPECT_FLOAT_EQ(ev.position.y, 180.0f);
    EXPECT_FLOAT_EQ(input.mouse_pos().x, 250.0f);
    EXPECT_FLOAT_EQ(input.mouse_pos().y, 180.0f);
}

TEST(ArinInputTest, MouseButtonStateTracking) {
    arin::InputState input;
    EXPECT_FALSE(input.is_mouse_down(arin::MouseButton::Left));
    EXPECT_FALSE(input.is_mouse_down(arin::MouseButton::Right));

    arin::MouseEvent down = input.on_mouse_button(arin::MouseButton::Left, arin::InputAction::Press);
    EXPECT_EQ(down.type, arin::MouseEventType::ButtonDown);
    EXPECT_EQ(down.button, arin::MouseButton::Left);
    EXPECT_TRUE(input.is_mouse_down(arin::MouseButton::Left));
    EXPECT_FALSE(input.is_mouse_down(arin::MouseButton::Right));

    arin::MouseEvent up = input.on_mouse_button(arin::MouseButton::Left, arin::InputAction::Release);
    EXPECT_EQ(up.type, arin::MouseEventType::ButtonUp);
    EXPECT_EQ(up.button, arin::MouseButton::Left);
    EXPECT_FALSE(input.is_mouse_down(arin::MouseButton::Left));
}

TEST(ArinInputTest, ScrollEventGeneration) {
    arin::InputState input;
    input.on_mouse_move(100.0f, 200.0f);

    arin::MouseEvent scroll = input.on_mouse_scroll(0.0f, 1.5f);
    EXPECT_EQ(scroll.type, arin::MouseEventType::Scroll);
    EXPECT_FLOAT_EQ(scroll.position.x, 100.0f);
    EXPECT_FLOAT_EQ(scroll.position.y, 200.0f);
    EXPECT_FLOAT_EQ(scroll.scroll_delta.x, 0.0f);
    EXPECT_FLOAT_EQ(scroll.scroll_delta.y, 1.5f);
}
