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
#include <arin/tree_view.hpp>

TEST(ArinTreeViewTest, DefaultConstruction) {
    arin::TreeView tree;
    EXPECT_TRUE(tree.is_visible());
    EXPECT_TRUE(tree.is_enabled());
    EXPECT_TRUE(tree.roots().empty());
    EXPECT_EQ(tree.selected_node(), nullptr);
}

TEST(ArinTreeViewTest, NodeHierarchyAndNesting) {
    arin::TreeView tree;
    auto root1 = tree.add_root("Root 1");
    auto root2 = tree.add_root("Root 2");

    auto child1 = root1->add_child("Child 1.1");
    auto child2 = root1->add_child("Child 1.2");
    auto grandchild = child1->add_child("Grandchild 1.1.1");

    EXPECT_EQ(tree.roots().size(), 2u);
    EXPECT_EQ(root1->children.size(), 2u);
    EXPECT_EQ(child1->children.size(), 1u);
    EXPECT_EQ(grandchild->children.size(), 0u);

    EXPECT_TRUE(root1->parent.expired());
    EXPECT_EQ(child1->parent.lock(), root1);
    EXPECT_EQ(grandchild->parent.lock(), child1);

    EXPECT_EQ(grandchild->label, "Grandchild 1.1.1");
}

TEST(ArinTreeViewTest, ExpandCollapseAndSelection) {
    arin::TreeView tree;
    auto root = tree.add_root("Root");
    auto child = root->add_child("Child");

    EXPECT_FALSE(root->is_expanded);
    root->is_expanded = true;
    EXPECT_TRUE(root->is_expanded);

    std::string selected_label;
    tree.on_selection_changed([&selected_label](std::shared_ptr<arin::TreeNode> node) {
        selected_label = node ? node->label : "";
    });

    tree.select_node(child);
    EXPECT_EQ(tree.selected_node(), child);
    EXPECT_EQ(selected_label, "Child");

    tree.clear_selection();
    EXPECT_EQ(tree.selected_node(), nullptr);
    EXPECT_TRUE(selected_label.empty());
}

TEST(ArinTreeViewTest, KeyboardNavigation) {
    arin::TreeView tree(arin::Rect{0.0f, 0.0f, 300.0f, 400.0f});
    auto root1 = tree.add_root("Root 1");
    auto child1 = root1->add_child("Child 1");
    auto root2 = tree.add_root("Root 2");

    root1->is_expanded = true;

    // Select first node
    tree.select_node(root1);
    EXPECT_EQ(tree.selected_node(), root1);

    // Down arrow -> child1
    arin::KeyEvent down_ev = arin::KeyEvent::make_press(arin::KeyCode::Down);
    EXPECT_TRUE(tree.handle_key(down_ev));
    EXPECT_EQ(tree.selected_node(), child1);

    // Down arrow -> root2
    EXPECT_TRUE(tree.handle_key(down_ev));
    EXPECT_EQ(tree.selected_node(), root2);

    // Up arrow -> child1
    arin::KeyEvent up_ev = arin::KeyEvent::make_press(arin::KeyCode::Up);
    EXPECT_TRUE(tree.handle_key(up_ev));
    EXPECT_EQ(tree.selected_node(), child1);

    // Up arrow -> root1
    EXPECT_TRUE(tree.handle_key(up_ev));
    EXPECT_EQ(tree.selected_node(), root1);

    // Left arrow collapses root1
    arin::KeyEvent left_ev = arin::KeyEvent::make_press(arin::KeyCode::Left);
    EXPECT_TRUE(tree.handle_key(left_ev));
    EXPECT_FALSE(root1->is_expanded);

    // Right arrow expands root1
    arin::KeyEvent right_ev = arin::KeyEvent::make_press(arin::KeyCode::Right);
    EXPECT_TRUE(tree.handle_key(right_ev));
    EXPECT_TRUE(root1->is_expanded);
}

TEST(ArinTreeViewTest, ClearNodes) {
    arin::TreeView tree;
    tree.add_root("Node 1");
    tree.add_root("Node 2");
    EXPECT_EQ(tree.roots().size(), 2u);

    tree.clear();
    EXPECT_EQ(tree.roots().size(), 0u);
    EXPECT_EQ(tree.selected_node(), nullptr);
}
