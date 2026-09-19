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
#include <arin/context_menu.hpp>

TEST(ArinContextMenuTest, DefaultConstruction) {
    arin::ContextMenu menu;
    EXPECT_FALSE(menu.is_visible());
    EXPECT_EQ(menu.item_count(), 0u);
    EXPECT_FLOAT_EQ(menu.style().min_width, 180.0f);
}

TEST(ArinContextMenuTest, ItemAdditionAndClearing) {
    arin::ContextMenu menu;
    menu.add_item("Undo")
        .add_item("Cut", arin::IconType::Cut)
        .add_item("Copy", arin::IconType::Copy, "Ctrl+C")
        .add_separator()
        .add_item("Paste", arin::IconType::Paste, "Ctrl+V");

    EXPECT_EQ(menu.item_count(), 5u);

    // Item 0: Plain Action
    EXPECT_EQ(menu.item_at(0).label, "Undo");
    EXPECT_EQ(menu.item_at(0).icon, arin::IconType::None);
    EXPECT_TRUE(menu.item_at(0).shortcut.empty());
    EXPECT_FALSE(menu.item_at(0).is_separator);
    EXPECT_TRUE(menu.item_at(0).enabled);

    // Item 1: Icon Action
    EXPECT_EQ(menu.item_at(1).label, "Cut");
    EXPECT_EQ(menu.item_at(1).icon, arin::IconType::Cut);

    // Item 2: Icon and Shortcut Action
    EXPECT_EQ(menu.item_at(2).label, "Copy");
    EXPECT_EQ(menu.item_at(2).icon, arin::IconType::Copy);
    EXPECT_EQ(menu.item_at(2).shortcut, "Ctrl+C");

    // Item 3: Separator
    EXPECT_TRUE(menu.item_at(3).is_separator);

    // Clear
    menu.clear();
    EXPECT_EQ(menu.item_count(), 0u);
}

TEST(ArinContextMenuTest, ShowAndHideLifecycle) {
    arin::ContextMenu menu;
    bool dismissed = false;
    menu.on_dismiss([&dismissed]() {
        dismissed = true;
    });

    menu.add_item("Option 1")
        .add_item("Option 2");

    EXPECT_FALSE(menu.is_visible());
    menu.show(50.0f, 75.0f);

    EXPECT_TRUE(menu.is_visible());
    EXPECT_FLOAT_EQ(menu.bounds().x, 50.0f);
    EXPECT_FLOAT_EQ(menu.bounds().y, 75.0f);
    EXPECT_GE(menu.bounds().width, menu.style().min_width);
    EXPECT_GT(menu.bounds().height, 0.0f);
    EXPECT_FALSE(dismissed);

    menu.hide();
    EXPECT_FALSE(menu.is_visible());
    EXPECT_TRUE(dismissed);
}

TEST(ArinContextMenuTest, ScreenBoundaryClamping) {
    arin::ContextMenu menu;
    menu.add_item("Test Option A")
        .add_item("Test Option B");

    // Clamp right and bottom: display size 800 x 600
    menu.show(780.0f, 580.0f, 800.0f, 600.0f);
    EXPECT_LE(menu.bounds().x + menu.bounds().width, 800.0f);
    EXPECT_LE(menu.bounds().y + menu.bounds().height, 600.0f);

    // Clamp left and top: negative coordinates
    menu.show(-50.0f, -20.0f, 800.0f, 600.0f);
    EXPECT_GE(menu.bounds().x, 4.0f);
    EXPECT_GE(menu.bounds().y, 4.0f);
}

TEST(ArinContextMenuTest, MouseInteractionAndCallbacks) {
    arin::ContextMenu menu;
    int triggered_action = 0;

    menu.add_item("First", [&triggered_action]() {
        triggered_action = 1;
    });
    menu.add_item("Second", [&triggered_action]() {
        triggered_action = 2;
    });

    menu.show(100.0f, 100.0f);

    // Mouse move over first item
    float first_item_center_y = menu.bounds().y + menu.style().padding.top + (menu.style().item_height * 0.5f);
    arin::MouseEvent move_ev = arin::MouseEvent::make_move(arin::Vec2(120.0f, first_item_center_y));
    EXPECT_TRUE(menu.handle_mouse(move_ev));

    // Mouse button up to trigger first item
    arin::MouseEvent click_ev = arin::MouseEvent::make_button_up(arin::Vec2(120.0f, first_item_center_y), arin::MouseButton::Left);
    EXPECT_TRUE(menu.handle_mouse(click_ev));
    EXPECT_EQ(triggered_action, 1);
    EXPECT_FALSE(menu.is_visible()); // Menu closes on action execution
}

TEST(ArinContextMenuTest, ClickOutsideDismisses) {
    arin::ContextMenu menu;
    menu.add_item("Sample Action");
    menu.show(100.0f, 100.0f);
    EXPECT_TRUE(menu.is_visible());

    // Click far outside bounds
    arin::MouseEvent down_outside = arin::MouseEvent::make_button_down(arin::Vec2(10.0f, 10.0f), arin::MouseButton::Left);
    EXPECT_FALSE(menu.handle_mouse(down_outside)); // returns false to allow click pass-through
    EXPECT_FALSE(menu.is_visible());
}

TEST(ArinContextMenuTest, EscapeKeyDismisses) {
    arin::ContextMenu menu;
    menu.add_item("Sample Action");
    menu.show(100.0f, 100.0f);
    EXPECT_TRUE(menu.is_visible());

    // Send other key (e.g. Space) -> does not dismiss
    arin::KeyEvent space_ev;
    space_ev.key = arin::KeyCode::Space;
    space_ev.action = arin::InputAction::Press;
    EXPECT_FALSE(menu.handle_key(space_ev));
    EXPECT_TRUE(menu.is_visible());

    // Send Escape key -> dismisses menu
    arin::KeyEvent esc_ev;
    esc_ev.key = arin::KeyCode::Escape;
    esc_ev.action = arin::InputAction::Press;
    EXPECT_TRUE(menu.handle_key(esc_ev));
    EXPECT_FALSE(menu.is_visible());
}
