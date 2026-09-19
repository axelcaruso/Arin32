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
#include <arin/text_input.hpp>

TEST(ArinTextInputTest, DefaultAndParametricConstruction) {
    arin::TextInput input1;
    EXPECT_EQ(input1.text(), "");
    EXPECT_EQ(input1.placeholder(), "Enter text...");
    EXPECT_FALSE(input1.is_focused());
    EXPECT_TRUE(input1.is_enabled());
    EXPECT_TRUE(input1.is_focusable());

    arin::TextInput input2("Initial Text", 10.0f, 20.0f, 250.0f, 36.0f);
    EXPECT_EQ(input2.text(), "Initial Text");
    EXPECT_EQ(input2.cursor_position(), 12u);
    EXPECT_FLOAT_EQ(input2.bounds().x, 10.0f);
    EXPECT_FLOAT_EQ(input2.bounds().y, 20.0f);
    EXPECT_FLOAT_EQ(input2.bounds().width, 250.0f);
    EXPECT_FLOAT_EQ(input2.bounds().height, 36.0f);
}

TEST(ArinTextInputTest, InsertionAndDeletion) {
    arin::TextInput input;
    input.set_text("Hello World");
    EXPECT_EQ(input.text(), "Hello World");

    // Position cursor at index 5 (after 'Hello')
    input.set_cursor_position(5);
    input.insert_text(" Beautiful");
    EXPECT_EQ(input.text(), "Hello Beautiful World");
    EXPECT_EQ(input.cursor_position(), 15u);
}

TEST(ArinTextInputTest, SelectionAndDeletion) {
    arin::TextInput input("Quick Brown Fox");
    input.select_all();
    EXPECT_TRUE(input.has_selection());
    EXPECT_EQ(input.selected_text(), "Quick Brown Fox");

    input.delete_selection();
    EXPECT_FALSE(input.has_selection());
    EXPECT_EQ(input.text(), "");
    EXPECT_EQ(input.cursor_position(), 0u);
}

TEST(ArinTextInputTest, ArrowKeyNavigation) {
    arin::TextInput input("Test String");
    input.set_focused(true);
    input.set_cursor_position(4);

    // Left arrow
    arin::KeyEvent left_ev = arin::KeyEvent::make_press(arin::KeyCode::Left);
    input.handle_key(left_ev);
    EXPECT_EQ(input.cursor_position(), 3u);

    // Right arrow
    arin::KeyEvent right_ev = arin::KeyEvent::make_press(arin::KeyCode::Right);
    input.handle_key(right_ev);
    EXPECT_EQ(input.cursor_position(), 4u);

    // Home key
    arin::KeyEvent home_ev = arin::KeyEvent::make_press(arin::KeyCode::Home);
    input.handle_key(home_ev);
    EXPECT_EQ(input.cursor_position(), 0u);

    // End key
    arin::KeyEvent end_ev = arin::KeyEvent::make_press(arin::KeyCode::End);
    input.handle_key(end_ev);
    EXPECT_EQ(input.cursor_position(), 11u);
}

TEST(ArinTextInputTest, BackspaceAndDeleteKeys) {
    arin::TextInput input("ABCDEF");
    input.set_focused(true);
    input.set_cursor_position(3); // cursor between C and D

    // Backspace removes C
    arin::KeyEvent bksp = arin::KeyEvent::make_press(arin::KeyCode::Backspace);
    input.handle_key(bksp);
    EXPECT_EQ(input.text(), "ABDEF");
    EXPECT_EQ(input.cursor_position(), 2u);

    // Delete removes D
    arin::KeyEvent del = arin::KeyEvent::make_press(arin::KeyCode::Delete);
    input.handle_key(del);
    EXPECT_EQ(input.text(), "ABEF");
    EXPECT_EQ(input.cursor_position(), 2u);
}

TEST(ArinTextInputTest, CharacterTypingInput) {
    arin::TextInput input;
    input.set_focused(true);

    arin::TextEvent t1 = arin::TextEvent::from_char('A');
    arin::TextEvent t2 = arin::TextEvent::from_char('r');
    arin::TextEvent t3 = arin::TextEvent::from_char('i');
    arin::TextEvent t4 = arin::TextEvent::from_char('n');

    input.handle_text(t1);
    input.handle_text(t2);
    input.handle_text(t3);
    input.handle_text(t4);

    EXPECT_EQ(input.text(), "Arin");
    EXPECT_EQ(input.cursor_position(), 4u);
}

TEST(ArinTextInputTest, ClipboardCopyAndPaste) {
    arin::TextInput input("Copied Content");
    input.set_focused(true);
    input.select_all();

    std::string test_clipboard;
    input.set_clipboard_provider(
        [&]() { return test_clipboard; },
        [&](const std::string& str) { test_clipboard = str; }
    );

    // Ctrl+C
    arin::KeyEvent copy_ev = arin::KeyEvent::make_press(
        arin::KeyCode::C,
        static_cast<uint8_t>(arin::KeyModifier::Control)
    );
    input.handle_key(copy_ev);
    EXPECT_EQ(test_clipboard, "Copied Content");

    // Clear input
    input.set_text("");
    EXPECT_EQ(input.text(), "");

    // Ctrl+V
    arin::KeyEvent paste_ev = arin::KeyEvent::make_press(
        arin::KeyCode::V,
        static_cast<uint8_t>(arin::KeyModifier::Control)
    );
    input.handle_key(paste_ev);
    EXPECT_EQ(input.text(), "Copied Content");
}

TEST(ArinTextInputTest, ChangeAndSubmitCallbacks) {
    arin::TextInput input;
    input.set_focused(true);

    int change_count = 0;
    std::string last_changed;
    input.on_text_changed([&](const std::string& txt) {
        change_count++;
        last_changed = txt;
    });

    int submit_count = 0;
    std::string last_submitted;
    input.on_submit([&](const std::string& txt) {
        submit_count++;
        last_submitted = txt;
    });

    input.handle_text(arin::TextEvent::from_char('X'));
    EXPECT_EQ(change_count, 1);
    EXPECT_EQ(last_changed, "X");

    arin::KeyEvent enter = arin::KeyEvent::make_press(arin::KeyCode::Enter);
    input.handle_key(enter);
    EXPECT_EQ(submit_count, 1);
    EXPECT_EQ(last_submitted, "X");
}

TEST(ArinTextInputTest, PasswordMasking) {
    arin::TextInput input("secret123");
    input.set_password(true);
    EXPECT_TRUE(input.is_password());
    EXPECT_EQ(input.text(), "secret123"); // Internal text remains intact
}
