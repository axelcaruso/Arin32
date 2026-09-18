/*
 * Arin32 - Modern OpenGL Graphical User Interface Library
 *
 * Copyright (c) 2026, Arin32 Contributors
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

TEST(ArinTypesTest, Vec2Arithmetic) {
    arin::Vec2 a(10.0f, 20.0f);
    arin::Vec2 b(5.0f, 4.0f);

    arin::Vec2 add = a + b;
    EXPECT_FLOAT_EQ(add.x, 15.0f);
    EXPECT_FLOAT_EQ(add.y, 24.0f);

    arin::Vec2 sub = a - b;
    EXPECT_FLOAT_EQ(sub.x, 5.0f);
    EXPECT_FLOAT_EQ(sub.y, 16.0f);

    arin::Vec2 mul = b * 2.0f;
    EXPECT_FLOAT_EQ(mul.x, 10.0f);
    EXPECT_FLOAT_EQ(mul.y, 8.0f);

    arin::Vec2 div = a / 2.0f;
    EXPECT_FLOAT_EQ(div.x, 5.0f);
    EXPECT_FLOAT_EQ(div.y, 10.0f);
}

TEST(ArinTypesTest, Vec2DistanceAndLerp) {
    arin::Vec2 p1(0.0f, 0.0f);
    arin::Vec2 p2(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(p1.distance_to(p2), 5.0f);

    arin::Vec2 mid = p1.lerp(p2, 0.5f);
    EXPECT_FLOAT_EQ(mid.x, 1.5f);
    EXPECT_FLOAT_EQ(mid.y, 2.0f);
}

TEST(ArinTypesTest, RectBoundariesAndContains) {
    arin::Rect r(100.0f, 50.0f, 200.0f, 80.0f);

    EXPECT_FLOAT_EQ(r.left(), 100.0f);
    EXPECT_FLOAT_EQ(r.top(), 50.0f);
    EXPECT_FLOAT_EQ(r.right(), 300.0f);
    EXPECT_FLOAT_EQ(r.bottom(), 130.0f);

    EXPECT_FLOAT_EQ(r.center().x, 200.0f);
    EXPECT_FLOAT_EQ(r.center().y, 90.0f);

    // Points inside
    EXPECT_TRUE(r.contains(arin::Vec2(100.0f, 50.0f)));   // Top-left boundary
    EXPECT_TRUE(r.contains(arin::Vec2(300.0f, 130.0f)));  // Bottom-right boundary
    EXPECT_TRUE(r.contains(arin::Vec2(200.0f, 90.0f)));   // Center

    // Points outside
    EXPECT_FALSE(r.contains(arin::Vec2(99.0f, 50.0f)));   // Just left
    EXPECT_FALSE(r.contains(arin::Vec2(100.0f, 49.0f)));  // Just above
    EXPECT_FALSE(r.contains(arin::Vec2(301.0f, 100.0f))); // Just right
    EXPECT_FALSE(r.contains(arin::Vec2(200.0f, 131.0f))); // Just below
}

TEST(ArinTypesTest, ColorConversions) {
    // White preset
    arin::Color white = arin::Color::white();
    EXPECT_FLOAT_EQ(white.r, 1.0f);
    EXPECT_FLOAT_EQ(white.g, 1.0f);
    EXPECT_FLOAT_EQ(white.b, 1.0f);
    EXPECT_FLOAT_EQ(white.a, 1.0f);

    // RGB Hex parsing
    arin::Color red = arin::Color::from_hex(0xFF0000);
    EXPECT_NEAR(red.r, 1.0f, 0.01f);
    EXPECT_NEAR(red.g, 0.0f, 0.01f);
    EXPECT_NEAR(red.b, 0.0f, 0.01f);
    EXPECT_FLOAT_EQ(red.a, 1.0f);

    // RGBA Hex parsing
    arin::Color transparent_blue = arin::Color::from_hex(0x0000FF80, true);
    EXPECT_NEAR(transparent_blue.r, 0.0f, 0.01f);
    EXPECT_NEAR(transparent_blue.g, 0.0f, 0.01f);
    EXPECT_NEAR(transparent_blue.b, 1.0f, 0.01f);
    EXPECT_NEAR(transparent_blue.a, 0.501f, 0.01f);

    // Alpha modification
    arin::Color semi_white = white.with_alpha(0.5f);
    EXPECT_FLOAT_EQ(semi_white.a, 0.5f);
}
