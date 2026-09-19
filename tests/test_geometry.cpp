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
#include <arin/types.hpp>
#include <arin/metrics.hpp>
#include <arin/font.hpp>
#include <arin/button.hpp>
#include <arin/text_input.hpp>
#include <arin/list_box.hpp>
#include <arin/context_menu.hpp>
#include <arin/layout.hpp>

using namespace arin;

TEST(ArinGeometryTest, RectCenteredPlacesContentInMiddle) {
    // Step 1: Define an outer container and an inner content size.
    const Rect outer(10.0f, 20.0f, 100.0f, 60.0f);
    // Step 2: Center a smaller rectangle and verify exact placement.
    const Rect centered = outer.centered(Vec2(40.0f, 20.0f));
    EXPECT_FLOAT_EQ(centered.x, 40.0f);
    EXPECT_FLOAT_EQ(centered.y, 40.0f);
    EXPECT_FLOAT_EQ(centered.width, 40.0f);
    EXPECT_FLOAT_EQ(centered.height, 20.0f);
}

TEST(ArinGeometryTest, RectInsetRemovesPaddingSymmetrically) {
    // Step 1: Inset a container by asymmetric padding.
    const Rect outer(0.0f, 0.0f, 100.0f, 50.0f);
    const Rect inner = outer.inset(Padding(10.0f, 5.0f, 10.0f, 5.0f));
    // Step 2: Verify position and clamped size.
    EXPECT_FLOAT_EQ(inner.x, 10.0f);
    EXPECT_FLOAT_EQ(inner.y, 5.0f);
    EXPECT_FLOAT_EQ(inner.width, 80.0f);
    EXPECT_FLOAT_EQ(inner.height, 40.0f);
}

TEST(ArinGeometryTest, FontLayoutCentersTextOptically) {
    // Step 1: Layout text in a fixed rectangle with default metrics.
    const Font font;
    const Rect bounds(0.0f, 0.0f, 200.0f, 40.0f);
    const Vec2 origin = font.layout_text_in_rect("Hello", bounds, 1.0f, TextAlignH::Center, TextAlignV::Center);
    // Step 2: Horizontal origin must match measured centering.
    const Vec2 measured = font.measure_text("Hello", 1.0f);
    EXPECT_NEAR(origin.x, (bounds.width - measured.x) * 0.5f, 0.001f);
    // Step 3: Vertical origin must sit slightly above line-box centering (optical correction).
    const float line_center = (bounds.height - measured.y) * 0.5f;
    EXPECT_LT(origin.y, line_center + bounds.y);
    EXPECT_GT(origin.y, line_center + bounds.y - measured.y);
}

TEST(ArinGeometryTest, ButtonContentSizeIncludesPaddingAndIcon) {
    // Step 1: Measure a text-only button through the shared helper.
    const Font font;
    Button text_only("Hi");
    const Vec2 content = text_only.content_size(font);
    EXPECT_GT(content.x, 0.0f);
    // Step 2: Adding an icon must widen the measured content.
    Button with_icon("Hi");
    with_icon.set_icon(IconType::Check);
    EXPECT_GT(with_icon.content_size(font).x, content.x);
}

TEST(ArinGeometryTest, TextInputInnerRectRespectsPadding) {
    // Step 1: Build a default input and query its text clipping rectangle.
    TextInput input;
    const Rect inner = input.inner_rect();
    // Step 2: Inner width must be smaller by twice the horizontal padding.
    EXPECT_FLOAT_EQ(inner.width, input.bounds().width - 2.0f * input.style().padding_x);
    EXPECT_GT(inner.x, input.bounds().x);
}

TEST(ArinGeometryTest, ListBoxRowRectStacksVertically) {
    // Step 1: Populate a list and query consecutive row rectangles.
    ListBox list(10.0f, 20.0f, 200.0f, 200.0f);
    list.add_item("First").add_item("Second");
    const Rect first = list.row_rect_at(0);
    const Rect second = list.row_rect_at(1);
    // Step 2: Rows must share width and advance by exactly one item height.
    EXPECT_FLOAT_EQ(first.width, second.width);
    EXPECT_FLOAT_EQ(second.y - first.y, list.item_height());
}

TEST(ArinGeometryTest, ContextMenuRowRectStacksItemsAndSeparators) {
    // Step 1: Build a menu with an action, separator, and action.
    ContextMenu menu;
    menu.add_item("Cut").add_separator().add_item("Copy");
    menu.show(0.0f, 0.0f);
    // Step 2: Separator rows use separator height, actions use item height.
    EXPECT_FLOAT_EQ(menu.row_rect_at(0).height, menu.style().item_height);
    EXPECT_FLOAT_EQ(menu.row_rect_at(1).height, menu.style().separator_height);
    EXPECT_FLOAT_EQ(menu.row_rect_at(2).y, menu.row_rect_at(0).y + menu.style().item_height + menu.style().separator_height);
}

TEST(ArinGeometryTest, LayoutClearHoverDoesNotMoveChildren) {
    // Step 1: Build a row with one button and record its bounds.
    HBox row(Rect(0.0f, 0.0f, 300.0f, 40.0f));
    auto button = row.add_button("Action", 80.0f, 30.0f);
    row.update_layout();
    const Rect before = button->bounds();
    // Step 2: Clearing hover must be a state-only operation.
    row.clear_hover();
    EXPECT_EQ(button->bounds(), before);
}
