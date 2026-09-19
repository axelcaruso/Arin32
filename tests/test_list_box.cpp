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
#include <arin/list_box.hpp>

using namespace arin;

TEST(ArinListBoxTest, DefaultAndParametricConstruction) {
    ListBox default_lb;
    EXPECT_EQ(default_lb.item_count(), 0u);
    EXPECT_EQ(default_lb.selected_index(), -1);
    EXPECT_EQ(default_lb.mode(), ListBoxMode::Standard);

    ListBox param_lb(10.0f, 20.0f, 200.0f, 150.0f, ListBoxMode::CheckBox);
    EXPECT_FLOAT_EQ(param_lb.bounds().x, 10.0f);
    EXPECT_FLOAT_EQ(param_lb.bounds().y, 20.0f);
    EXPECT_FLOAT_EQ(param_lb.bounds().width, 200.0f);
    EXPECT_FLOAT_EQ(param_lb.bounds().height, 150.0f);
    EXPECT_EQ(param_lb.mode(), ListBoxMode::CheckBox);
}

TEST(ArinListBoxTest, ItemAdditionAndClearing) {
    ListBox lb(0.0f, 0.0f, 200.0f, 200.0f);

    lb.add_item("Alpha");
    lb.add_item("Beta", true);
    lb.add_items({"Gamma", "Delta"});

    EXPECT_EQ(lb.item_count(), 4u);
    EXPECT_EQ(lb.item_text(0), "Alpha");
    EXPECT_FALSE(lb.is_checked(0));
    EXPECT_EQ(lb.item_text(1), "Beta");
    EXPECT_TRUE(lb.is_checked(1));
    EXPECT_EQ(lb.item_text(2), "Gamma");
    EXPECT_EQ(lb.item_text(3), "Delta");

    lb.clear_items();
    EXPECT_EQ(lb.item_count(), 0u);
    EXPECT_EQ(lb.selected_index(), -1);
}

TEST(ArinListBoxTest, SingleSelectionAndCallbacks) {
    ListBox lb(0.0f, 0.0f, 200.0f, 200.0f);
    lb.add_items({"First", "Second", "Third"});

    int reported_idx = -1;
    std::string reported_text;

    lb.on_selection_changed([&](int idx, const std::string& text) {
        reported_idx = idx;
        reported_text = text;
    });

    lb.set_selected_index(1);
    EXPECT_EQ(lb.selected_index(), 1);
    EXPECT_EQ(lb.selected_item(), "Second");
    EXPECT_EQ(reported_idx, 1);
    EXPECT_EQ(reported_text, "Second");

    lb.set_selected_index(2);
    EXPECT_EQ(lb.selected_index(), 2);
    EXPECT_EQ(lb.selected_item(), "Third");
    EXPECT_EQ(reported_idx, 2);
    EXPECT_EQ(reported_text, "Third");
}

TEST(ArinListBoxTest, CheckboxTogglingAndCallbacks) {
    CheckListBox clb(0.0f, 0.0f, 200.0f, 200.0f);
    clb.add_items({"Feature A", "Feature B", "Feature C"});

    EXPECT_EQ(clb.mode(), ListBoxMode::CheckBox);
    EXPECT_FALSE(clb.is_checked(0));
    EXPECT_FALSE(clb.is_checked(1));

    int toggled_idx = -1;
    bool toggled_state = false;
    clb.on_item_toggled([&](int idx, bool checked) {
        toggled_idx = idx;
        toggled_state = checked;
    });

    clb.set_checked(0, true);
    EXPECT_TRUE(clb.is_checked(0));
    EXPECT_EQ(toggled_idx, 0);
    EXPECT_TRUE(toggled_state);

    clb.toggle_checked(1);
    EXPECT_TRUE(clb.is_checked(1));
    EXPECT_EQ(toggled_idx, 1);
    EXPECT_TRUE(toggled_state);

    std::vector<int> checked_indices = clb.checked_indices();
    EXPECT_EQ(checked_indices.size(), 2u);
    EXPECT_EQ(checked_indices[0], 0);
    EXPECT_EQ(checked_indices[1], 1);

    std::vector<std::string> checked_items = clb.checked_items();
    EXPECT_EQ(checked_items.size(), 2u);
    EXPECT_EQ(checked_items[0], "Feature A");
    EXPECT_EQ(checked_items[1], "Feature B");
}

TEST(ArinListBoxTest, MouseClickSelectionInStandardMode) {
    ListBox lb(100.0f, 100.0f, 200.0f, 120.0f);
    lb.set_item_height(24.0f);
    lb.add_items({"Item 0", "Item 1", "Item 2", "Item 3"});

    // Click on Item 1: y coordinate is 100 + 1 (border) + 24 * 1.5 = ~137
    MouseEvent click_item_1 = MouseEvent::make_button_down(
        Vec2(150.0f, 137.0f),
        MouseButton::Left
    );

    bool handled = lb.handle_mouse(click_item_1);
    EXPECT_TRUE(handled);
    EXPECT_EQ(lb.selected_index(), 1);
    EXPECT_EQ(lb.selected_item(), "Item 1");

    // Click on Item 2: y coordinate is 100 + 1 + 24 * 2.5 = ~161
    MouseEvent click_item_2 = MouseEvent::make_button_down(
        Vec2(150.0f, 161.0f),
        MouseButton::Left
    );

    handled = lb.handle_mouse(click_item_2);
    EXPECT_TRUE(handled);
    EXPECT_EQ(lb.selected_index(), 2);
    EXPECT_EQ(lb.selected_item(), "Item 2");

    // Click outside
    MouseEvent click_outside = MouseEvent::make_button_down(
        Vec2(50.0f, 50.0f),
        MouseButton::Left
    );
    handled = lb.handle_mouse(click_outside);
    EXPECT_FALSE(handled);
    EXPECT_EQ(lb.selected_index(), 2); // Unchanged
}

TEST(ArinListBoxTest, MouseClickToggleInCheckboxMode) {
    CheckListBox clb(0.0f, 0.0f, 200.0f, 100.0f);
    clb.set_item_height(25.0f);
    clb.add_items({"Option 1", "Option 2"});

    EXPECT_FALSE(clb.is_checked(0));

    // Click first item row
    MouseEvent click_0 = MouseEvent::make_button_down(
        Vec2(20.0f, 12.0f),
        MouseButton::Left
    );
    clb.handle_mouse(click_0);
    EXPECT_TRUE(clb.is_checked(0));

    // Click again to untoggle
    clb.handle_mouse(click_0);
    EXPECT_FALSE(clb.is_checked(0));
}

TEST(ArinListBoxTest, ScrollOffsetClampingAndWheel) {
    ListBox lb(0.0f, 0.0f, 200.0f, 100.0f);
    lb.set_item_height(25.0f);

    // 10 items * 25px = 250px total height, container is 100px -> needs scrollbar
    for (int i = 0; i < 10; ++i) {
        lb.add_item("Row " + std::to_string(i));
    }

    EXPECT_TRUE(lb.needs_scrollbar());
    EXPECT_FLOAT_EQ(lb.scroll_offset(), 0.0f);

    // Scroll down (negative y wheel delta scrolls down)
    MouseEvent wheel_down = MouseEvent::make_scroll(Vec2(50.0f, 50.0f), Vec2(0.0f, -1.0f));
    bool handled = lb.handle_mouse(wheel_down);
    EXPECT_TRUE(handled);
    EXPECT_GT(lb.scroll_offset(), 0.0f);

    // Scroll way down past maximum
    MouseEvent wheel_lots = MouseEvent::make_scroll(Vec2(50.0f, 50.0f), Vec2(0.0f, -50.0f));
    lb.handle_mouse(wheel_lots);
    EXPECT_LE(lb.scroll_offset(), lb.max_scroll_offset());

    // Scroll way back up
    MouseEvent wheel_up = MouseEvent::make_scroll(Vec2(50.0f, 50.0f), Vec2(0.0f, 50.0f));
    lb.handle_mouse(wheel_up);
    EXPECT_FLOAT_EQ(lb.scroll_offset(), 0.0f);
}

TEST(ArinListBoxTest, CustomStylingAndColors) {
    ListBox lb(0.0f, 0.0f, 200.0f, 100.0f);

    // Test default style
    EXPECT_EQ(lb.style().background_color, Color::white());
    EXPECT_EQ(lb.style().selection_color, Color::from_hex(0x0067C0));

    // Test individual setters
    lb.set_selection_color(Color::from_hex(0x10B981));
    EXPECT_EQ(lb.style().selection_color, Color::from_hex(0x10B981));

    lb.set_text_color(Color::from_hex(0x374151));
    EXPECT_EQ(lb.style().text_color, Color::from_hex(0x374151));

    lb.set_background_color(Color::from_hex(0xFAFAFA));
    EXPECT_EQ(lb.style().background_color, Color::from_hex(0xFAFAFA));

    lb.set_border_color(Color::from_hex(0xE5E7EB));
    EXPECT_EQ(lb.style().border_color, Color::from_hex(0xE5E7EB));

    // Test preset dark style
    lb.set_style(ListBoxStyle::dark());
    EXPECT_EQ(lb.style().background_color, Color::from_hex(0x1F2937));
    EXPECT_EQ(lb.style().selection_color, Color::from_hex(0x2563EB));
    EXPECT_EQ(lb.style().text_color, Color::from_hex(0xF9FAFB));

    // Test preset accent style
    lb.set_style(ListBoxStyle::accent(Color::from_hex(0x8B5CF6)));
    EXPECT_EQ(lb.style().selection_color, Color::from_hex(0x8B5CF6));
    EXPECT_EQ(lb.style().checkbox_checked_color, Color::from_hex(0x8B5CF6));
}

