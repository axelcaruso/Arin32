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
#include <arin/file_dialog.hpp>

TEST(ArinFileDialogTest, ConstructionAndModes) {
    arin::FileDialog dlg_open(arin::FileDialogMode::OpenFile);
    EXPECT_EQ(dlg_open.mode(), arin::FileDialogMode::OpenFile);
    EXPECT_EQ(dlg_open.title(), "Open File");
    EXPECT_FALSE(dlg_open.is_open());

    arin::FileDialog dlg_save(arin::FileDialogMode::SaveFile);
    EXPECT_EQ(dlg_save.mode(), arin::FileDialogMode::SaveFile);
    EXPECT_EQ(dlg_save.title(), "Save File");

    arin::FileDialog dlg_dir(arin::FileDialogMode::SelectFolder);
    EXPECT_EQ(dlg_dir.mode(), arin::FileDialogMode::SelectFolder);
    EXPECT_EQ(dlg_dir.title(), "Select Folder");
}

TEST(ArinFileDialogTest, ShowHideLifecycle) {
    arin::FileDialog dlg(arin::FileDialogMode::OpenFile);
    EXPECT_FALSE(dlg.is_open());

    dlg.show();
    EXPECT_TRUE(dlg.is_open());
    EXPECT_TRUE(dlg.is_visible());

    dlg.hide();
    EXPECT_FALSE(dlg.is_open());
    EXPECT_FALSE(dlg.is_visible());
}

TEST(ArinFileDialogTest, FilterManagement) {
    arin::FileDialog dlg;
    dlg.set_extension_filter(".txt,.cpp,.hpp");
    EXPECT_EQ(dlg.extension_filter(), ".txt,.cpp,.hpp");
}

TEST(ArinFileDialogTest, DirectoryNavigation) {
    arin::FileDialog dlg;
    dlg.set_directory("/");
    EXPECT_EQ(dlg.current_directory(), "/");

    dlg.set_filename("test.txt");
    EXPECT_EQ(dlg.selected_path(), "/test.txt");
}

TEST(ArinFileDialogTest, EscapeDismissesAndTriggersCancel) {
    arin::FileDialog dlg;
    dlg.show();
    EXPECT_TRUE(dlg.is_open());

    bool cancelled = false;
    dlg.on_cancel([&cancelled]() {
        cancelled = true;
    });

    arin::KeyEvent esc_ev = arin::KeyEvent::make_press(arin::KeyCode::Escape);
    EXPECT_TRUE(dlg.handle_key(esc_ev));
    EXPECT_FALSE(dlg.is_open());
    EXPECT_TRUE(cancelled);
}
