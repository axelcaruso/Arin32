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
#include <arin/tab_view.hpp>
#include <arin/button.hpp>

TEST(ArinTabViewTest, DefaultConstruction) {
    arin::TabView tab_view;
    EXPECT_TRUE(tab_view.is_visible());
    EXPECT_TRUE(tab_view.is_enabled());
    EXPECT_EQ(tab_view.tab_count(), 0u);
    EXPECT_EQ(tab_view.active_tab_index(), -1);
    EXPECT_EQ(tab_view.active_content(), nullptr);
}

TEST(ArinTabViewTest, TabAdditionAndActiveSelection) {
    arin::TabView tab_view(arin::Rect{0.0f, 0.0f, 600.0f, 400.0f});
    auto btn1 = std::make_shared<arin::Button>("Button 1");
    auto btn2 = std::make_shared<arin::Button>("Button 2");

    tab_view.add_tab("First Tab", btn1);
    EXPECT_EQ(tab_view.tab_count(), 1u);
    EXPECT_EQ(tab_view.active_tab_index(), 0);
    EXPECT_EQ(tab_view.active_content(), btn1);

    tab_view.add_tab("Second Tab", btn2, true);
    EXPECT_EQ(tab_view.tab_count(), 2u);
    EXPECT_EQ(tab_view.active_tab_index(), 0); // remains first
    EXPECT_EQ(tab_view.tab_title(1), "Second Tab");
    EXPECT_TRUE(tab_view.is_tab_closable(1));

    tab_view.set_active_tab(1);
    EXPECT_EQ(tab_view.active_tab_index(), 1);
    EXPECT_EQ(tab_view.active_content(), btn2);
}

TEST(ArinTabViewTest, TabRemovalAndCallback) {
    arin::TabView tab_view(arin::Rect{0.0f, 0.0f, 600.0f, 400.0f});
    tab_view.add_tab("Tab 0");
    tab_view.add_tab("Tab 1");
    tab_view.add_tab("Tab 2");

    int close_requested_index = -1;
    tab_view.on_tab_close_requested([&close_requested_index](size_t idx) {
        close_requested_index = static_cast<int>(idx);
    });

    tab_view.set_active_tab(2);
    EXPECT_EQ(tab_view.active_tab_index(), 2);

    tab_view.remove_tab(1);
    EXPECT_EQ(tab_view.tab_count(), 2u);
    EXPECT_EQ(tab_view.tab_title(0), "Tab 0");
    EXPECT_EQ(tab_view.tab_title(1), "Tab 2");
    EXPECT_EQ(tab_view.active_tab_index(), 1); // clamped to last valid
}

TEST(ArinTabViewTest, ContentAreaBounds) {
    arin::TabView tab_view(arin::Rect{10.0f, 20.0f, 500.0f, 300.0f});
    arin::Rect content_rect = tab_view.content_area_bounds();

    EXPECT_FLOAT_EQ(content_rect.x, 10.0f);
    EXPECT_FLOAT_EQ(content_rect.y, 20.0f + tab_view.style().tab_height);
    EXPECT_FLOAT_EQ(content_rect.width, 500.0f);
    EXPECT_FLOAT_EQ(content_rect.height, 300.0f - tab_view.style().tab_height);
}

TEST(ArinTabViewTest, KeyboardTabCycling) {
    arin::TabView tab_view(arin::Rect{0.0f, 0.0f, 600.0f, 400.0f});
    tab_view.add_tab("Tab 0");
    tab_view.add_tab("Tab 1");
    tab_view.add_tab("Tab 2");

    EXPECT_EQ(tab_view.active_tab_index(), 0);

    // Ctrl+Tab
    arin::KeyEvent ctrl_tab = arin::KeyEvent::make_press(arin::KeyCode::Tab, static_cast<uint8_t>(arin::KeyModifier::Control));
    EXPECT_TRUE(tab_view.handle_key(ctrl_tab));
    EXPECT_EQ(tab_view.active_tab_index(), 1);

    EXPECT_TRUE(tab_view.handle_key(ctrl_tab));
    EXPECT_EQ(tab_view.active_tab_index(), 2);

    EXPECT_TRUE(tab_view.handle_key(ctrl_tab));
    EXPECT_EQ(tab_view.active_tab_index(), 0);
}
