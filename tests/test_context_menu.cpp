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
#include <arin/button.hpp>

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

TEST(ArinContextMenuTest, Windows10DefaultStyling) {
    arin::ContextMenu menu;
    const auto& style = menu.style();

    // Verify 1:1 Windows 10 metrics and color fidelity
    EXPECT_FLOAT_EQ(style.corner_radius, 0.0f); // Windows 10 square corners
    EXPECT_FLOAT_EQ(style.item_height, 22.0f);  // Authentic desktop item height
    EXPECT_FLOAT_EQ(style.icon_size, 16.0f);    // Standard 16x16 icon size
    EXPECT_EQ(style.background_color, arin::Color::white());
    EXPECT_EQ(style.border_color, arin::Color::from_hex(0xCCCCCC));
    EXPECT_EQ(style.hover_color, arin::Color::from_hex(0xE5E5E5));
    EXPECT_EQ(style.hover_text_color, arin::Color::from_hex(0x000000)); // Stays black on hover
    EXPECT_EQ(style.text_color, arin::Color::from_hex(0x000000));
}

TEST(ArinContextMenuTest, SvgIconItemAddition) {
    arin::ContextMenu menu;

    // Add with SVG path
    menu.add_item("Open", "assets/icons/folder-open.svg")
        .add_item("Cut", "assets/icons/cut.svg", "Ctrl+X");

    // Add with in-memory SvgDocument
    const char* kCircleSvg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 32 32\" width=\"32\" height=\"32\">"
        "<circle cx=\"16\" cy=\"16\" r=\"10\" fill=\"#0067C0\"/>"
        "</svg>";
    auto doc = arin::SvgDocument::load_from_memory(kCircleSvg);
    ASSERT_NE(doc, nullptr);
    menu.add_item("Circle", doc);

    EXPECT_EQ(menu.item_count(), 3u);
    EXPECT_EQ(menu.item_at(0).svg_path, "assets/icons/folder-open.svg");
    EXPECT_EQ(menu.item_at(1).svg_path, "assets/icons/cut.svg");
    EXPECT_EQ(menu.item_at(1).shortcut, "Ctrl+X");
    EXPECT_NE(menu.item_at(2).svg_icon, nullptr);
    EXPECT_TRUE(menu.item_at(2).svg_icon->is_valid());
}

TEST(ArinContextMenuTest, SubmenuAndDefaultActionFlags) {
    arin::ContextMenu menu;
    menu.add_default_item("Abrir")
        .add_submenu("Conceder acceso a")
        .add_submenu("Enviar a", "assets/icons/arrow-right.svg");

    EXPECT_EQ(menu.item_count(), 3u);

    // Item 0: Default action (bold in Windows 10)
    EXPECT_EQ(menu.item_at(0).label, "Abrir");
    EXPECT_TRUE(menu.item_at(0).is_default);
    EXPECT_FALSE(menu.item_at(0).has_submenu);

    // Item 1: Plain Submenu (has chevron >)
    EXPECT_EQ(menu.item_at(1).label, "Conceder acceso a");
    EXPECT_FALSE(menu.item_at(1).is_default);
    EXPECT_TRUE(menu.item_at(1).has_submenu);

    // Item 2: Submenu with SVG icon
    EXPECT_EQ(menu.item_at(2).label, "Enviar a");
    EXPECT_TRUE(menu.item_at(2).has_submenu);
    EXPECT_EQ(menu.item_at(2).svg_path, "assets/icons/arrow-right.svg");
}

TEST(ArinContextMenuTest, FluentChainingAndLastItem) {
    arin::ContextMenu menu;
    menu.add_item("Custom Item");
    menu.last_item()
        .set_default(true)
        .set_submenu(true)
        .set_shortcut("Ctrl+Shift+P");

    EXPECT_EQ(menu.item_count(), 1u);
    const auto& item = menu.item_at(0);
    EXPECT_TRUE(item.is_default);
    EXPECT_TRUE(item.has_submenu);
    EXPECT_EQ(item.shortcut, "Ctrl+Shift+P");
}

TEST(ArinContextMenuTest, CascadingSubmenuAttachmentAndHierarchy) {
    arin::ContextMenu parent;
    auto child = std::make_shared<arin::ContextMenu>();
    child->add_item("Sub-Action 1")
         .add_item("Sub-Action 2");

    parent.add_item("Parent Action 1")
          .add_submenu("Send To", child, "assets/icons/arrow-right.svg")
          .add_item("Parent Action 2");

    EXPECT_EQ(parent.item_count(), 3u);
    EXPECT_TRUE(parent.item_at(1).has_submenu);
    EXPECT_EQ(parent.item_at(1).child_menu, child);
    EXPECT_EQ(child->item_count(), 2u);
}

TEST(ArinContextMenuTest, CascadingSubmenuHoverOpeningAndClosing) {
    arin::ContextMenu parent;
    auto child = std::make_shared<arin::ContextMenu>();
    child->add_item("Sub-Action 1")
         .add_item("Sub-Action 2");

    parent.add_item("Action 1")
          .add_submenu("Nested Menu", child)
          .add_item("Action 2");

    parent.show(100.0f, 100.0f, 1024.0f, 768.0f);
    EXPECT_TRUE(parent.is_visible());
    EXPECT_EQ(parent.active_child_menu(), nullptr);

    // Hover over item 1 ("Nested Menu")
    float item1_y = parent.bounds().y + parent.style().padding.top + parent.style().item_height * 1.5f;
    arin::MouseEvent hover_submenu = arin::MouseEvent::make_move(arin::Vec2(150.0f, item1_y));
    parent.handle_mouse(hover_submenu);

    EXPECT_NE(parent.active_child_menu(), nullptr);
    EXPECT_TRUE(child->is_visible());
    // Child should be positioned to the right of parent (m_bounds.x + m_bounds.width - 2.0f)
    EXPECT_FLOAT_EQ(child->bounds().x, parent.bounds().x + parent.bounds().width - 2.0f);

    // Hover over item 0 ("Action 1") which does not have a submenu -> child closes
    float item0_y = parent.bounds().y + parent.style().padding.top + parent.style().item_height * 0.5f;
    arin::MouseEvent hover_action0 = arin::MouseEvent::make_move(arin::Vec2(150.0f, item0_y));
    parent.handle_mouse(hover_action0);

    EXPECT_EQ(parent.active_child_menu(), nullptr);
    EXPECT_FALSE(child->is_visible());
}

TEST(ArinContextMenuTest, CascadingSubmenuChildClickDismissesAll) {
    arin::ContextMenu parent;
    auto child = std::make_shared<arin::ContextMenu>();
    int child_action_called = 0;
    child->add_item("Sub-Action 1", [&child_action_called]() {
        child_action_called = 1;
    });

    parent.add_submenu("Nested", child);
    parent.show(100.0f, 100.0f, 1024.0f, 768.0f);

    // Open child submenu
    parent.open_child_menu(0);
    EXPECT_TRUE(child->is_visible());

    // Click inside child menu on Sub-Action 1
    float child_item_y = child->bounds().y + child->style().padding.top + child->style().item_height * 0.5f;
    arin::Vec2 click_pos(child->bounds().x + 20.0f, child_item_y);

    arin::MouseEvent down_ev = arin::MouseEvent::make_button_down(click_pos, arin::MouseButton::Left);
    EXPECT_TRUE(parent.handle_mouse(down_ev));

    arin::MouseEvent up_ev = arin::MouseEvent::make_button_up(click_pos, arin::MouseButton::Left);
    EXPECT_TRUE(parent.handle_mouse(up_ev));

    EXPECT_EQ(child_action_called, 1);
    EXPECT_FALSE(child->is_visible());
    EXPECT_FALSE(parent.is_visible());
}

TEST(ArinContextMenuTest, CascadingSubmenuKeyboardNavigation) {
    arin::ContextMenu parent;
    auto child = std::make_shared<arin::ContextMenu>();
    int child_action_called = 0;
    child->add_item("Target Action", [&child_action_called]() {
        child_action_called = 42;
    });

    parent.add_item("First")
          .add_submenu("Sub", child);

    parent.show(100.0f, 100.0f, 1024.0f, 768.0f);

    // Press Down to navigate to item 0 ("First")
    arin::KeyEvent down_ev;
    down_ev.key = arin::KeyCode::Down;
    down_ev.action = arin::InputAction::Press;
    parent.handle_key(down_ev);
    EXPECT_EQ(parent.hovered_index(), 0);

    // Press Down again to navigate to item 1 ("Sub") -> auto opens submenu
    parent.handle_key(down_ev);
    EXPECT_EQ(parent.hovered_index(), 1);
    EXPECT_TRUE(child->is_visible());

    // Press Right arrow to focus into child submenu
    arin::KeyEvent right_ev;
    right_ev.key = arin::KeyCode::Right;
    right_ev.action = arin::InputAction::Press;
    parent.handle_key(right_ev);
    EXPECT_EQ(child->hovered_index(), 0);

    // Press Left arrow to close child and return to parent
    arin::KeyEvent left_ev;
    left_ev.key = arin::KeyCode::Left;
    left_ev.action = arin::InputAction::Press;
    parent.handle_key(left_ev);
    EXPECT_FALSE(child->is_visible());
    EXPECT_TRUE(parent.is_visible());

    // Press Right to open again
    parent.handle_key(right_ev);
    EXPECT_TRUE(child->is_visible());

    // Press Enter to execute child target action
    arin::KeyEvent enter_ev;
    enter_ev.key = arin::KeyCode::Enter;
    enter_ev.action = arin::InputAction::Press;
    parent.handle_key(enter_ev);

    EXPECT_EQ(child_action_called, 42);
    EXPECT_FALSE(child->is_visible());
    EXPECT_FALSE(parent.is_visible());
}

TEST(ArinContextMenuTest, CascadingSubmenuScreenBoundaryFlipping) {
    arin::ContextMenu parent;
    auto child = std::make_shared<arin::ContextMenu>();
    child->add_item("Overflow Test Item");

    parent.add_submenu("Cascade", child);

    // Position parent very close to the right edge of 800px screen
    // Parent min_width is 180px, so parent will be at x ~ 610.
    // Child placed to the right would be at 610 + 180 = 790, overflowing margin.
    // It should flip to the left: child_x = parent.bounds.x - child_w + 2.0f
    parent.show(610.0f, 100.0f, 800.0f, 600.0f);
    parent.open_child_menu(0);

    EXPECT_TRUE(child->is_visible());
    EXPECT_LT(child->bounds().x, parent.bounds().x);
    EXPECT_LE(child->bounds().right(), 800.0f);
}

TEST(ArinContextMenuTest, DynamicItemInsertionAndOrder) {
    arin::ContextMenu menu;
    menu.add_item("Item 1")
        .add_item("Item 3");

    // Insert "Item 2" at index 1
    menu.insert_item(1, "Item 2");
    EXPECT_EQ(menu.item_count(), 3u);
    EXPECT_EQ(menu.item_at(0).label, "Item 1");
    EXPECT_EQ(menu.item_at(1).label, "Item 2");
    EXPECT_EQ(menu.item_at(2).label, "Item 3");

    // Insert item at the very beginning (index 0)
    menu.insert_item(0, "Item 0");
    EXPECT_EQ(menu.item_count(), 4u);
    EXPECT_EQ(menu.item_at(0).label, "Item 0");
    EXPECT_EQ(menu.item_at(1).label, "Item 1");

    // Insert item before "Item 3"
    menu.insert_item_before("Item 3", arin::MenuItem::action("Item 2.5"));
    EXPECT_EQ(menu.item_count(), 5u);
    EXPECT_EQ(menu.item_at(3).label, "Item 2.5");
    EXPECT_EQ(menu.item_at(4).label, "Item 3");

    // Insert item after "Item 3"
    menu.insert_item_after("Item 3", arin::MenuItem::action("Item 4"));
    EXPECT_EQ(menu.item_count(), 6u);
    EXPECT_EQ(menu.item_at(5).label, "Item 4");

    // Insert separator at index 2
    menu.insert_separator(2);
    EXPECT_EQ(menu.item_count(), 7u);
    EXPECT_TRUE(menu.item_at(2).is_separator);
}

TEST(ArinContextMenuTest, DynamicItemRemovalAndCleanup) {
    arin::ContextMenu menu;
    menu.add_item("Open")
        .add_item("Cut")
        .add_item("Copy")
        .add_item("Delete");

    EXPECT_EQ(menu.item_count(), 4u);

    // Remove by index (Cut at index 1)
    menu.remove_item(1);
    EXPECT_EQ(menu.item_count(), 3u);
    EXPECT_EQ(menu.item_at(0).label, "Open");
    EXPECT_EQ(menu.item_at(1).label, "Copy");
    EXPECT_EQ(menu.item_at(2).label, "Delete");

    // Remove by label
    menu.remove_item("Copy");
    EXPECT_EQ(menu.item_count(), 2u);
    EXPECT_EQ(menu.item_at(0).label, "Open");
    EXPECT_EQ(menu.item_at(1).label, "Delete");

    // Remove by ID
    menu.add_item(arin::MenuItem::action("Custom").set_id("my_custom_id"));
    EXPECT_TRUE(menu.has_item_by_id("my_custom_id"));
    menu.remove_item_by_id("my_custom_id");
    EXPECT_FALSE(menu.has_item_by_id("my_custom_id"));
}

TEST(ArinContextMenuTest, ItemVisibilityAndFiltering) {
    arin::ContextMenu menu;
    menu.add_item("Item 1")
        .add_item(arin::MenuItem::action("Item 2").set_id("id_item_2"))
        .add_item("Item 3");

    EXPECT_EQ(menu.item_count(), 3u);
    EXPECT_EQ(menu.visible_item_count(), 3u);

    menu.show(100.0f, 100.0f);
    float initial_height = menu.bounds().height;

    // Hide Item 2 by ID
    menu.set_item_visible_by_id("id_item_2", false);
    EXPECT_EQ(menu.visible_item_count(), 2u);

    // Recalculate dimensions via show
    menu.show(100.0f, 100.0f);
    float filtered_height = menu.bounds().height;
    EXPECT_LT(filtered_height, initial_height);

    // Hidden item has an empty row rect
    arin::Rect hidden_rect = menu.row_rect_at(1);
    EXPECT_FLOAT_EQ(hidden_rect.width, 0.0f);
    EXPECT_FLOAT_EQ(hidden_rect.height, 0.0f);

    // Visible item 3 has a valid row rect shifted up to where Item 2 was
    arin::Rect item3_rect = menu.row_rect_at(2);
    EXPECT_GT(item3_rect.height, 0.0f);
    EXPECT_FLOAT_EQ(item3_rect.y, menu.bounds().y + menu.style().padding.top + menu.style().item_height);

    // Re-enable Item 2
    menu.set_item_visible("Item 2", true);
    EXPECT_EQ(menu.visible_item_count(), 3u);
}

TEST(ArinContextMenuTest, ItemSearchAndFindById) {
    arin::ContextMenu menu;
    menu.add_item("Save")
        .add_item(arin::MenuItem::action("Print").set_id("action_print"));

    // Find by label
    auto* save_item = menu.find_item("Save");
    ASSERT_NE(save_item, nullptr);
    EXPECT_EQ(save_item->label, "Save");
    save_item->label = "Save As...";
    EXPECT_EQ(menu.item_at(0).label, "Save As...");

    // Find by ID
    auto* print_item = menu.find_item_by_id("action_print");
    ASSERT_NE(print_item, nullptr);
    EXPECT_EQ(print_item->label, "Print");
    EXPECT_TRUE(print_item->enabled);

    // Disable item via helper
    menu.set_item_enabled_by_id("action_print", false);
    EXPECT_FALSE(print_item->enabled);
}

TEST(ArinContextMenuTest, WidgetSpecificContextMenuAttachment) {
    arin::Button btn("My Button", 50.0f, 50.0f, 120.0f, 36.0f);
    EXPECT_EQ(btn.context_menu(), nullptr);

    auto custom_menu = std::make_shared<arin::ContextMenu>();
    custom_menu->add_item("Button-Specific Action 1")
               .add_item("Button-Specific Action 2");

    btn.set_context_menu(custom_menu);
    EXPECT_EQ(btn.context_menu(), custom_menu);
    EXPECT_EQ(btn.context_menu()->item_count(), 2u);
}

TEST(ArinContextMenuTest, DynamicBeforeShowHookCustomization) {
    arin::ContextMenu menu;
    menu.add_item("Standard Action");

    int dynamic_state = 100;
    menu.on_before_show([&dynamic_state](arin::ContextMenu& m) {
        if (!m.has_item_by_id("dynamic_item")) {
            m.add_item(arin::MenuItem::action("Dynamic Value: " + std::to_string(dynamic_state))
                           .set_id("dynamic_item"));
        } else {
            auto* item = m.find_item_by_id("dynamic_item");
            if (item) {
                item->label = "Dynamic Value: " + std::to_string(dynamic_state);
            }
        }
    });

    EXPECT_EQ(menu.item_count(), 1u);

    // Call show -> on_before_show dynamically appends the custom item
    menu.show(50.0f, 50.0f);
    EXPECT_EQ(menu.item_count(), 2u);
    EXPECT_EQ(menu.item_at(1).label, "Dynamic Value: 100");

    // Change external state and show again -> dynamic label updates
    dynamic_state = 250;
    menu.show(50.0f, 50.0f);
    EXPECT_EQ(menu.item_count(), 2u);
    EXPECT_EQ(menu.item_at(1).label, "Dynamic Value: 250");
}


