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
#include <arin/progress_bar.hpp>

TEST(ArinProgressBarTest, DefaultAndParametricConstruction) {
    // 1. Default constructor
    arin::ProgressBar bar1;
    EXPECT_FLOAT_EQ(bar1.bounds().x, 0.0f);
    EXPECT_FLOAT_EQ(bar1.bounds().y, 0.0f);
    EXPECT_FLOAT_EQ(bar1.bounds().width, 260.0f);
    EXPECT_FLOAT_EQ(bar1.bounds().height, 20.0f);
    EXPECT_EQ(bar1.mode(), arin::ProgressBarMode::Determinate);
    EXPECT_FALSE(bar1.is_indeterminate());
    EXPECT_FLOAT_EQ(bar1.value(), 0.0f);
    EXPECT_FLOAT_EQ(bar1.min_value(), 0.0f);
    EXPECT_FLOAT_EQ(bar1.max_value(), 100.0f);
    EXPECT_FLOAT_EQ(bar1.percentage(), 0.0f);

    // 2. Parametric constructor (x, y, w, h)
    arin::ProgressBar bar2(10.0f, 25.0f, 300.0f, 18.0f);
    EXPECT_FLOAT_EQ(bar2.bounds().x, 10.0f);
    EXPECT_FLOAT_EQ(bar2.bounds().y, 25.0f);
    EXPECT_FLOAT_EQ(bar2.bounds().width, 300.0f);
    EXPECT_FLOAT_EQ(bar2.bounds().height, 18.0f);

    // 3. Full parametric constructor (x, y, w, h, val, min, max)
    arin::ProgressBar bar3(50.0f, 60.0f, 200.0f, 22.0f, 45.0f, 0.0f, 100.0f);
    EXPECT_FLOAT_EQ(bar3.bounds().x, 50.0f);
    EXPECT_FLOAT_EQ(bar3.bounds().y, 60.0f);
    EXPECT_FLOAT_EQ(bar3.value(), 45.0f);
    EXPECT_FLOAT_EQ(bar3.percentage(), 0.45f);

    // 4. Rect constructor
    arin::Rect rect(100.0f, 150.0f, 400.0f, 24.0f);
    arin::ProgressBar bar4(rect, arin::ProgressBarMode::Indeterminate);
    EXPECT_FLOAT_EQ(bar4.bounds().x, 100.0f);
    EXPECT_FLOAT_EQ(bar4.bounds().y, 150.0f);
    EXPECT_TRUE(bar4.is_indeterminate());
}

TEST(ArinProgressBarTest, FluentChainingAPI) {
    arin::ProgressBar bar;
    bar.set_position(30.0f, 40.0f)
       .set_size(320.0f, 24.0f)
       .set_range(0.0f, 200.0f)
       .set_value(100.0f)
       .set_corner_radius(3.0f);

    EXPECT_FLOAT_EQ(bar.bounds().x, 30.0f);
    EXPECT_FLOAT_EQ(bar.bounds().y, 40.0f);
    EXPECT_FLOAT_EQ(bar.bounds().width, 320.0f);
    EXPECT_FLOAT_EQ(bar.bounds().height, 24.0f);
    EXPECT_FLOAT_EQ(bar.value(), 100.0f);
    EXPECT_FLOAT_EQ(bar.percentage(), 0.50f);
    EXPECT_FLOAT_EQ(bar.style().corner_radius, 3.0f);
}

TEST(ArinProgressBarTest, ValueClampingAndPercentages) {
    arin::ProgressBar bar(0.0f, 0.0f, 200.0f, 20.0f);
    bar.set_range(10.0f, 60.0f);

    // Within range
    bar.set_value(35.0f);
    EXPECT_FLOAT_EQ(bar.value(), 35.0f);
    EXPECT_FLOAT_EQ(bar.percentage(), 0.5f); // (35 - 10) / (60 - 10) = 25/50 = 0.5

    // Below min
    bar.set_value(-5.0f);
    EXPECT_FLOAT_EQ(bar.value(), 10.0f);
    EXPECT_FLOAT_EQ(bar.percentage(), 0.0f);

    // Above max
    bar.set_value(120.0f);
    EXPECT_FLOAT_EQ(bar.value(), 60.0f);
    EXPECT_FLOAT_EQ(bar.percentage(), 1.0f);

    // Zero / degenerate range safety
    bar.set_range(50.0f, 50.0f);
    EXPECT_GT(bar.max_value(), bar.min_value()); // Safety bump prevents division by zero
}

TEST(ArinProgressBarTest, IndeterminateModeToggling) {
    arin::ProgressBar bar;
    EXPECT_FALSE(bar.is_indeterminate());

    bar.set_indeterminate(true);
    EXPECT_TRUE(bar.is_indeterminate());
    EXPECT_EQ(bar.mode(), arin::ProgressBarMode::Indeterminate);

    bar.set_indeterminate(false);
    EXPECT_FALSE(bar.is_indeterminate());
    EXPECT_EQ(bar.mode(), arin::ProgressBarMode::Determinate);

    bar.set_mode(arin::ProgressBarMode::Indeterminate);
    EXPECT_TRUE(bar.is_indeterminate());
}

TEST(ArinProgressBarTest, AnimationPhaseAdvancement) {
    arin::ProgressBar bar;
    EXPECT_FLOAT_EQ(bar.anim_phase(), 0.0f);

    float speed = bar.style().animation_speed;

    bar.update(0.5f);
    EXPECT_FLOAT_EQ(bar.anim_phase(), 0.5f * speed);

    bar.update(1.0f);
    EXPECT_FLOAT_EQ(bar.anim_phase(), 1.5f * speed);
}

TEST(ArinProgressBarTest, PredefinedStyles) {
    auto green_style = arin::ProgressBarStyle::green();
    EXPECT_FLOAT_EQ(green_style.fill_color.r, arin::Color::from_hex(0x06B025).r); // Windows 10 Green
    EXPECT_FLOAT_EQ(green_style.border_width, 1.0f);

    auto blue_style = arin::ProgressBarStyle::blue();
    EXPECT_FLOAT_EQ(blue_style.fill_color.r, arin::Color::from_hex(0x0067C0).r); // Windows 10 Blue #0067C0
    EXPECT_FLOAT_EQ(blue_style.border_width, 1.0f);

    auto dark_style = arin::ProgressBarStyle::dark();
    EXPECT_FLOAT_EQ(dark_style.track_color.r, arin::Color::from_hex(0x27272A).r);
}
