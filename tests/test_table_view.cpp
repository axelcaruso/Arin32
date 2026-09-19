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
#include <arin/table_view.hpp>

TEST(ArinTableViewTest, DefaultConstruction) {
    arin::TableView table;
    EXPECT_TRUE(table.is_visible());
    EXPECT_TRUE(table.is_enabled());
    EXPECT_EQ(table.column_count(), 0u);
    EXPECT_EQ(table.row_count(), 0u);
    EXPECT_EQ(table.selected_row(), -1);
}

TEST(ArinTableViewTest, ColumnAndRowManagement) {
    arin::TableView table;
    table.add_column("ID", 60.0f);
    table.add_column("Name", 150.0f);
    table.add_column("Score", 80.0f);

    EXPECT_EQ(table.column_count(), 3u);
    EXPECT_EQ(table.column_title(1), "Name");
    EXPECT_FLOAT_EQ(table.column_width(1), 150.0f);

    table.set_column_width(1, 200.0f);
    EXPECT_FLOAT_EQ(table.column_width(1), 200.0f);

    table.add_row({"1", "Alice", "95.5"});
    table.add_row({"2", "Bob", "88.0"});

    EXPECT_EQ(table.row_count(), 2u);
    EXPECT_EQ(table.get_cell(0, 1), "Alice");
    EXPECT_EQ(table.get_cell(1, 1), "Bob");

    table.set_cell(0, 1, "Alicia");
    EXPECT_EQ(table.get_cell(0, 1), "Alicia");

    table.remove_row(0);
    EXPECT_EQ(table.row_count(), 1u);
    EXPECT_EQ(table.get_cell(0, 1), "Bob");

    table.clear_rows();
    EXPECT_EQ(table.row_count(), 0u);
}

TEST(ArinTableViewTest, RowSelectionAndCallbacks) {
    arin::TableView table;
    table.add_column("Col1", 100.0f);
    table.add_row({"Item 0"});
    table.add_row({"Item 1"});
    table.add_row({"Item 2"});

    int selected_idx = -1;
    table.on_row_selected([&selected_idx](int row) {
        selected_idx = row;
    });

    table.select_row(1);
    EXPECT_EQ(table.selected_row(), 1);
    EXPECT_EQ(selected_idx, 1);

    table.clear_selection();
    EXPECT_EQ(table.selected_row(), -1);
}

TEST(ArinTableViewTest, KeyboardNavigation) {
    arin::TableView table(arin::Rect{0.0f, 0.0f, 400.0f, 300.0f});
    table.add_column("Item", 200.0f);
    table.add_row({"Row 0"});
    table.add_row({"Row 1"});
    table.add_row({"Row 2"});

    table.select_row(0);
    EXPECT_EQ(table.selected_row(), 0);

    arin::KeyEvent down_ev = arin::KeyEvent::make_press(arin::KeyCode::Down);
    EXPECT_TRUE(table.handle_key(down_ev));
    EXPECT_EQ(table.selected_row(), 1);

    EXPECT_TRUE(table.handle_key(down_ev));
    EXPECT_EQ(table.selected_row(), 2);

    arin::KeyEvent up_ev = arin::KeyEvent::make_press(arin::KeyCode::Up);
    EXPECT_TRUE(table.handle_key(up_ev));
    EXPECT_EQ(table.selected_row(), 1);
}
