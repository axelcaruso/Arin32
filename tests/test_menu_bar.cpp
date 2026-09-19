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
#include <arin/menu_bar.hpp>

TEST(ArinMenuBarTest, DefaultConstruction) {
    arin::MenuBar bar;
    EXPECT_TRUE(bar.is_visible());
    EXPECT_TRUE(bar.is_enabled());
    EXPECT_EQ(bar.menu_count(), 0u);
    EXPECT_FALSE(bar.is_open());
    EXPECT_EQ(bar.active_menu_index(), -1);
    EXPECT_FLOAT_EQ(bar.bounds().height, 28.0f);
}

TEST(ArinMenuBarTest, MenuAdditionAndRetrieval) {
    arin::MenuBar bar(arin::Rect{0.0f, 0.0f, 800.0f, 28.0f});

    auto& file_menu = bar.add_menu("File");
    file_menu.add_item("New", "Ctrl+N")
             .add_item("Open", "Ctrl+O")
             .add_separator()
             .add_item("Exit");

    auto& edit_menu = bar.add_menu("Edit");
    edit_menu.add_item("Undo")
             .add_item("Redo");

    EXPECT_EQ(bar.menu_count(), 2u);
    EXPECT_EQ(bar.menu_title(0), "File");
    EXPECT_EQ(bar.menu_title(1), "Edit");
    EXPECT_NE(bar.get_menu(0), nullptr);
    EXPECT_NE(bar.get_menu(1), nullptr);
    EXPECT_EQ(bar.get_menu(0)->item_count(), 4u);
    EXPECT_EQ(bar.get_menu(1)->item_count(), 2u);
}

TEST(ArinMenuBarTest, OpenAndCloseMenu) {
    arin::MenuBar bar(arin::Rect{0.0f, 0.0f, 800.0f, 28.0f});
    bar.add_menu("File");
    bar.add_menu("Edit");
    bar.add_menu("Help");

    EXPECT_FALSE(bar.is_open());

    bar.open_menu(1);
    EXPECT_TRUE(bar.is_open());
    EXPECT_EQ(bar.active_menu_index(), 1);
    EXPECT_TRUE(bar.get_menu(1)->is_visible());

    bar.open_menu(0);
    EXPECT_TRUE(bar.is_open());
    EXPECT_EQ(bar.active_menu_index(), 0);
    EXPECT_TRUE(bar.get_menu(0)->is_visible());
    EXPECT_FALSE(bar.get_menu(1)->is_visible());

    bar.close_active_menu();
    EXPECT_FALSE(bar.is_open());
    EXPECT_EQ(bar.active_menu_index(), -1);
    EXPECT_FALSE(bar.get_menu(0)->is_visible());
}

TEST(ArinMenuBarTest, MouseInteractionOpeningAndClosing) {
    arin::MenuBar bar(arin::Rect{0.0f, 0.0f, 800.0f, 28.0f});
    bar.add_menu("File");
    bar.add_menu("Edit");

    // Click on "File" (approx x=15, y=14)
    arin::MouseEvent click_file = arin::MouseEvent::make_button_down(arin::Vec2{15.0f, 14.0f}, arin::MouseButton::Left);
    EXPECT_TRUE(bar.handle_mouse(click_file));
    EXPECT_TRUE(bar.is_open());
    EXPECT_EQ(bar.active_menu_index(), 0);

    // Click outside menu bar (x=500, y=500)
    arin::MouseEvent click_outside = arin::MouseEvent::make_button_down(arin::Vec2{500.0f, 500.0f}, arin::MouseButton::Left);
    bar.handle_mouse(click_outside);
    EXPECT_FALSE(bar.is_open());
}

TEST(ArinMenuBarTest, KeyboardNavigation) {
    arin::MenuBar bar(arin::Rect{0.0f, 0.0f, 800.0f, 28.0f});
    bar.add_menu("File");
    bar.add_menu("Edit");
    bar.add_menu("View");

    bar.open_menu(0);
    EXPECT_EQ(bar.active_menu_index(), 0);

    // Right arrow -> Edit
    arin::KeyEvent right_ev = arin::KeyEvent::make_press(arin::KeyCode::Right);
    EXPECT_TRUE(bar.handle_key(right_ev));
    EXPECT_EQ(bar.active_menu_index(), 1);

    // Right arrow -> View
    EXPECT_TRUE(bar.handle_key(right_ev));
    EXPECT_EQ(bar.active_menu_index(), 2);

    // Right arrow wraps around -> File
    EXPECT_TRUE(bar.handle_key(right_ev));
    EXPECT_EQ(bar.active_menu_index(), 0);

    // Left arrow wraps around backwards -> View
    arin::KeyEvent left_ev = arin::KeyEvent::make_press(arin::KeyCode::Left);
    EXPECT_TRUE(bar.handle_key(left_ev));
    EXPECT_EQ(bar.active_menu_index(), 2);

    // Escape closes menu
    arin::KeyEvent esc_ev = arin::KeyEvent::make_press(arin::KeyCode::Escape);
    EXPECT_TRUE(bar.handle_key(esc_ev));
    EXPECT_FALSE(bar.is_open());
}

TEST(ArinMenuBarTest, ClearAndRemoveMenu) {
    arin::MenuBar bar;
    bar.add_menu("File");
    bar.add_menu("Edit");
    bar.add_menu("Help");
    EXPECT_EQ(bar.menu_count(), 3u);

    bar.remove_menu(1);
    EXPECT_EQ(bar.menu_count(), 2u);
    EXPECT_EQ(bar.menu_title(0), "File");
    EXPECT_EQ(bar.menu_title(1), "Help");

    bar.clear();
    EXPECT_EQ(bar.menu_count(), 0u);
}
