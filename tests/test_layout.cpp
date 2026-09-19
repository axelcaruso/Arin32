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
#include <arin/layout.hpp>
#include <arin/button.hpp>

using namespace arin;

TEST(ArinLayoutTest, VBoxVerticalStacking) {
    VBox vbox(10.0f, 20.0f, 10.0f); // pos (10, 20), spacing 10
    vbox.set_padding(0.0f);

    auto btn1 = vbox.add_button("Button 1", 100.0f, 30.0f);
    auto btn2 = vbox.add_button("Button 2", 100.0f, 30.0f);
    auto btn3 = vbox.add_button("Button 3", 100.0f, 30.0f);

    vbox.update_layout();

    // btn1 at y = 20
    EXPECT_FLOAT_EQ(btn1->bounds().x, 10.0f);
    EXPECT_FLOAT_EQ(btn1->bounds().y, 20.0f);

    // btn2 at y = 20 + 30 + 10 = 60
    EXPECT_FLOAT_EQ(btn2->bounds().x, 10.0f);
    EXPECT_FLOAT_EQ(btn2->bounds().y, 60.0f);

    // btn3 at y = 60 + 30 + 10 = 100
    EXPECT_FLOAT_EQ(btn3->bounds().x, 10.0f);
    EXPECT_FLOAT_EQ(btn3->bounds().y, 100.0f);

    // Auto-sized bounds
    EXPECT_FLOAT_EQ(vbox.bounds().width, 100.0f);
    EXPECT_FLOAT_EQ(vbox.bounds().height, 110.0f); // 30*3 + 10*2 = 110
}

TEST(ArinLayoutTest, HBoxHorizontalStacking) {
    HBox hbox(50.0f, 50.0f, 15.0f); // pos (50, 50), spacing 15
    hbox.set_padding(0.0f);

    auto b1 = hbox.add_button("A", 80.0f, 32.0f);
    auto b2 = hbox.add_button("B", 80.0f, 32.0f);

    hbox.update_layout();

    // b1 at x = 50
    EXPECT_FLOAT_EQ(b1->bounds().x, 50.0f);
    EXPECT_FLOAT_EQ(b1->bounds().y, 50.0f);

    // b2 at x = 50 + 80 + 15 = 145
    EXPECT_FLOAT_EQ(b2->bounds().x, 145.0f);
    EXPECT_FLOAT_EQ(b2->bounds().y, 50.0f);

    // Total width = 80 + 15 + 80 = 175
    EXPECT_FLOAT_EQ(hbox.bounds().width, 175.0f);
    EXPECT_FLOAT_EQ(hbox.bounds().height, 32.0f);
}

TEST(ArinLayoutTest, PaddingOffsetsChildren) {
    VBox vbox(100.0f, 100.0f, 10.0f);
    vbox.set_padding(Padding(16.0f, 12.0f, 20.0f, 14.0f)); // left 16, top 12, right 20, bottom 14

    auto btn = vbox.add_button("Padded", 100.0f, 40.0f);
    vbox.update_layout();

    // Child should be shifted by left padding and top padding
    EXPECT_FLOAT_EQ(btn->bounds().x, 100.0f + 16.0f);
    EXPECT_FLOAT_EQ(btn->bounds().y, 100.0f + 12.0f);

    // Auto-size includes padding
    EXPECT_FLOAT_EQ(vbox.bounds().width, 100.0f + 16.0f + 20.0f);
    EXPECT_FLOAT_EQ(vbox.bounds().height, 40.0f + 12.0f + 14.0f);
}

TEST(ArinLayoutTest, Alignments) {
    // Center alignment in VBox
    VBox vbox(0.0f, 0.0f, 0.0f);
    vbox.set_bounds(Rect(0.0f, 0.0f, 200.0f, 200.0f)); // Fixed width 200
    vbox.set_alignment(LayoutAlignment::Center);

    auto btn = vbox.add_button("Centered", 80.0f, 30.0f);
    vbox.update_layout();

    // In 200px container, 80px button centered should start at (200 - 80)/2 = 60
    EXPECT_FLOAT_EQ(btn->bounds().x, 60.0f);

    // Stretch alignment
    vbox.set_alignment(LayoutAlignment::Stretch);
    vbox.update_layout();
    EXPECT_FLOAT_EQ(btn->bounds().x, 0.0f);
    EXPECT_FLOAT_EQ(btn->bounds().width, 200.0f);
}

TEST(ArinLayoutTest, NestedLayouts) {
    // A dialog VBox containing content and an HBox of action buttons
    VBox dialog(50.0f, 50.0f, 12.0f);
    dialog.set_padding(16.0f);

    auto list = dialog.add_list_box(300.0f, 150.0f);
    auto action_row = dialog.add_hbox(8.0f);
    auto cancel_btn = action_row->add_button("Cancel", 85.0f, 32.0f);
    auto ok_btn = action_row->add_button("OK", 85.0f, 32.0f);

    dialog.update_layout();

    EXPECT_EQ(dialog.child_count(), 2u);
    EXPECT_EQ(action_row->child_count(), 2u);

    // List placed at padding
    EXPECT_FLOAT_EQ(list->bounds().x, 50.0f + 16.0f);
    EXPECT_FLOAT_EQ(list->bounds().y, 50.0f + 16.0f);

    // Action row placed below list + spacing (12px)
    float expected_row_y = 50.0f + 16.0f + 150.0f + 12.0f;
    EXPECT_FLOAT_EQ(action_row->bounds().y, expected_row_y);
    EXPECT_FLOAT_EQ(cancel_btn->bounds().y, expected_row_y);
    EXPECT_FLOAT_EQ(ok_btn->bounds().y, expected_row_y);

    // OK button placed after cancel button + spacing (8px)
    EXPECT_FLOAT_EQ(ok_btn->bounds().x, cancel_btn->bounds().x + 85.0f + 8.0f);
}

TEST(ArinLayoutTest, MouseRoutingInLayout) {
    VBox vbox(100.0f, 100.0f, 10.0f);
    auto btn = vbox.add_button("Click", 100.0f, 40.0f);

    bool clicked = false;
    btn->on_click([&]() { clicked = true; });

    vbox.update_layout();

    // Mouse down inside button
    MouseEvent down = MouseEvent::make_button_down(Vec2(150.0f, 120.0f), MouseButton::Left);
    bool handled_down = vbox.handle_mouse(down);
    EXPECT_TRUE(handled_down);

    // Mouse up inside button
    MouseEvent up = MouseEvent::make_button_up(Vec2(150.0f, 120.0f), MouseButton::Left);
    bool handled_up = vbox.handle_mouse(up);
    EXPECT_TRUE(handled_up);

    EXPECT_TRUE(clicked);
}

TEST(ArinLayoutTest, LayoutJustifyCenter) {
    HBox hbox(Rect(0.0f, 0.0f, 300.0f, 50.0f), 10.0f);
    hbox.set_padding(0.0f);
    hbox.set_justify(LayoutJustify::Center);

    auto b1 = hbox.add_button("A", 80.0f, 30.0f);
    auto b2 = hbox.add_button("B", 80.0f, 30.0f);
    hbox.update_layout();

    // Total content = 80 + 10 + 80 = 170. Free = 300 - 170 = 130. Offset = 65.
    EXPECT_FLOAT_EQ(b1->bounds().x, 65.0f);
    EXPECT_FLOAT_EQ(b2->bounds().x, 65.0f + 80.0f + 10.0f);
}

TEST(ArinLayoutTest, LayoutJustifySpaceBetween) {
    HBox hbox(Rect(100.0f, 0.0f, 300.0f, 50.0f));
    hbox.set_padding(0.0f);
    hbox.set_justify(LayoutJustify::SpaceBetween);

    auto b1 = hbox.add_button("A", 50.0f, 30.0f);
    auto b2 = hbox.add_button("B", 50.0f, 30.0f);
    auto b3 = hbox.add_button("C", 50.0f, 30.0f);
    hbox.update_layout();

    // Total child width = 150. Free space = 300 - 150 = 150. Gaps = 150 / 2 = 75.
    EXPECT_FLOAT_EQ(b1->bounds().x, 100.0f);
    EXPECT_FLOAT_EQ(b2->bounds().x, 100.0f + 50.0f + 75.0f);
    EXPECT_FLOAT_EQ(b3->bounds().x, 100.0f + 300.0f - 50.0f);
}

TEST(ArinLayoutTest, LayoutJustifySpaceEvenly) {
    HBox hbox(Rect(0.0f, 0.0f, 240.0f, 50.0f));
    hbox.set_padding(0.0f);
    hbox.set_justify(LayoutJustify::SpaceEvenly);

    auto b1 = hbox.add_button("A", 60.0f, 30.0f);
    auto b2 = hbox.add_button("B", 60.0f, 30.0f);
    hbox.update_layout();

    // 2 children of 60 = 120. Free space = 240 - 120 = 120. Gaps = 120 / 3 = 40.
    EXPECT_FLOAT_EQ(b1->bounds().x, 40.0f);
    EXPECT_FLOAT_EQ(b2->bounds().x, 40.0f + 60.0f + 40.0f);
}

TEST(ArinLayoutTest, DistributeChildrenEqually) {
    HBox hbox(Rect(0.0f, 0.0f, 300.0f, 40.0f), 10.0f);
    hbox.set_padding(Padding(10.0f)); // left 10, right 10 -> available 280

    auto b1 = hbox.add_button("A", 10.0f, 30.0f);
    auto b2 = hbox.add_button("B", 10.0f, 30.0f);
    auto b3 = hbox.add_button("C", 10.0f, 30.0f);

    // Available 280, 2 gaps of 10 = 20. Total width for 3 buttons = 260. 260 / 3 = 86.6667
    hbox.distribute_children_equally();

    float expected_w = 260.0f / 3.0f;
    EXPECT_NEAR(b1->bounds().width, expected_w, 0.01f);
    EXPECT_NEAR(b2->bounds().width, expected_w, 0.01f);
    EXPECT_NEAR(b3->bounds().width, expected_w, 0.01f);
}

TEST(ArinLayoutTest, LayoutValidationDetectsOverlapAndOverflow) {
    HBox hbox(Rect(0.0f, 0.0f, 100.0f, 40.0f));
    hbox.set_padding(0.0f);

    auto b1 = hbox.add_button("Button 1", 60.0f, 30.0f);
    auto b2 = hbox.add_button("Button 2", 60.0f, 30.0f);
    hbox.update_layout();

    // Total width = 120 > container 100 -> overflow detected
    LayoutValidationResult res = hbox.validate();
    EXPECT_FALSE(res.is_valid);
    EXPECT_GT(res.issues.size(), 0u);
}
