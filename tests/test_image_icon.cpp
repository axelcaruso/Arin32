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
#include "arin/arin.hpp"

using namespace arin;

// Mock texture helper for headless unit testing without requiring an active OpenGL display
class MockTexture : public Texture {
public:
    static std::shared_ptr<Texture> create_mock(int width, int height) {
        auto tex = std::make_shared<Texture>();
        // Move-construct or set mock state via reflection/derived wrapper
        struct ExposedTexture {
            uint32_t handle;
            int width;
            int height;
            int channels;
            TextureFilter filter;
        };
        auto* exp = reinterpret_cast<ExposedTexture*>(tex.get());
        exp->handle = 42; // Mock valid OpenGL texture ID
        exp->width = width;
        exp->height = height;
        exp->channels = 4;
        exp->filter = TextureFilter::Linear;
        return tex;
    }
};

// -----------------------------------------------------------------------------
// Texture Unit Tests
// -----------------------------------------------------------------------------

TEST(ArinTextureTest, DefaultStateIsInvalid) {
    Texture tex;
    EXPECT_FALSE(tex.is_valid());
    EXPECT_EQ(tex.handle(), 0u);
    EXPECT_EQ(tex.width(), 0);
    EXPECT_EQ(tex.height(), 0);
    EXPECT_EQ(tex.channels(), 0);
}

TEST(ArinTextureTest, MoveSemanticsTransferOwnership) {
    auto tex1 = MockTexture::create_mock(200, 100);
    EXPECT_TRUE(tex1->is_valid());
    EXPECT_EQ(tex1->width(), 200);
    EXPECT_EQ(tex1->height(), 100);

    Texture tex2 = std::move(*tex1);
    EXPECT_TRUE(tex2.is_valid());
    EXPECT_EQ(tex2.width(), 200);
    EXPECT_EQ(tex2.height(), 100);

    // Origin object must now be reset and invalid
    EXPECT_FALSE(tex1->is_valid());
    EXPECT_EQ(tex1->width(), 0);
    EXPECT_EQ(tex1->height(), 0);
}

TEST(ArinTextureTest, AspectRatioAndMetrics) {
    auto tex = MockTexture::create_mock(1920, 1080);
    EXPECT_FLOAT_EQ(tex->aspect_ratio(), 1920.0f / 1080.0f);
    EXPECT_FLOAT_EQ(tex->size().x, 1920.0f);
    EXPECT_FLOAT_EQ(tex->size().y, 1080.0f);
}

// -----------------------------------------------------------------------------
// Image Widget Unit Tests
// -----------------------------------------------------------------------------

TEST(ArinImageTest, DefaultConstructionAndProperties) {
    Image img;
    EXPECT_FLOAT_EQ(img.bounds().x, 0.0f);
    EXPECT_FLOAT_EQ(img.bounds().y, 0.0f);
    EXPECT_FLOAT_EQ(img.bounds().width, 100.0f);
    EXPECT_FLOAT_EQ(img.bounds().height, 100.0f);
    EXPECT_EQ(img.scale_mode(), ImageScaleMode::Fit);
    EXPECT_TRUE(img.is_visible());
    EXPECT_TRUE(img.is_enabled());

    img.set_position(50.0f, 75.0f)
       .set_size(300.0f, 200.0f)
       .set_scale_mode(ImageScaleMode::Stretch)
       .set_corner_radius(8.0f)
       .set_tint(Color(1.0f, 0.0f, 0.0f, 0.5f));

    EXPECT_FLOAT_EQ(img.bounds().x, 50.0f);
    EXPECT_FLOAT_EQ(img.bounds().y, 75.0f);
    EXPECT_FLOAT_EQ(img.bounds().width, 300.0f);
    EXPECT_FLOAT_EQ(img.bounds().height, 200.0f);
    EXPECT_EQ(img.scale_mode(), ImageScaleMode::Stretch);
    EXPECT_FLOAT_EQ(img.corner_radius(), 8.0f);
    EXPECT_FLOAT_EQ(img.tint().r, 1.0f);
    EXPECT_FLOAT_EQ(img.tint().g, 0.0f);
    EXPECT_FLOAT_EQ(img.tint().b, 0.0f);
    EXPECT_FLOAT_EQ(img.tint().a, 0.5f);
}

TEST(ArinImageTest, ScaleModeStretchFillsExactBounds) {
    auto tex = MockTexture::create_mock(800, 600);
    Image img(tex, 10.0f, 20.0f, 400.0f, 300.0f, ImageScaleMode::Stretch);

    Rect content_rect = img.compute_content_rect();
    EXPECT_FLOAT_EQ(content_rect.x, 10.0f);
    EXPECT_FLOAT_EQ(content_rect.y, 20.0f);
    EXPECT_FLOAT_EQ(content_rect.width, 400.0f);
    EXPECT_FLOAT_EQ(content_rect.height, 300.0f);
}

TEST(ArinImageTest, ScaleModeFitPreservesAspectRatio) {
    // 2:1 aspect ratio texture inside 1:1 square bounds (letterbox vertical margins)
    auto tex_wide = MockTexture::create_mock(200, 100);
    Image img_wide(tex_wide, 0.0f, 0.0f, 100.0f, 100.0f, ImageScaleMode::Fit);

    Rect rect_wide = img_wide.compute_content_rect();
    EXPECT_FLOAT_EQ(rect_wide.width, 100.0f);
    EXPECT_FLOAT_EQ(rect_wide.height, 50.0f);
    EXPECT_FLOAT_EQ(rect_wide.x, 0.0f);
    EXPECT_FLOAT_EQ(rect_wide.y, 25.0f); // Vertically centered

    // 1:2 aspect ratio texture inside 1:1 square bounds (pillarbox horizontal margins)
    auto tex_tall = MockTexture::create_mock(100, 200);
    Image img_tall(tex_tall, 0.0f, 0.0f, 100.0f, 100.0f, ImageScaleMode::Fit);

    Rect rect_tall = img_tall.compute_content_rect();
    EXPECT_FLOAT_EQ(rect_tall.width, 50.0f);
    EXPECT_FLOAT_EQ(rect_tall.height, 100.0f);
    EXPECT_FLOAT_EQ(rect_tall.x, 25.0f); // Horizontally centered
    EXPECT_FLOAT_EQ(rect_tall.y, 0.0f);
}

TEST(ArinImageTest, ScaleModeCenterRetainsNativePixelSize) {
    auto tex = MockTexture::create_mock(64, 48);
    Image img(tex, 100.0f, 100.0f, 200.0f, 200.0f, ImageScaleMode::Center);

    Rect content_rect = img.compute_content_rect();
    EXPECT_FLOAT_EQ(content_rect.width, 64.0f);
    EXPECT_FLOAT_EQ(content_rect.height, 48.0f);
    EXPECT_FLOAT_EQ(content_rect.x, 100.0f + (200.0f - 64.0f) * 0.5f);
    EXPECT_FLOAT_EQ(content_rect.y, 100.0f + (200.0f - 48.0f) * 0.5f);
}

// -----------------------------------------------------------------------------
// Icon Widget Unit Tests
// -----------------------------------------------------------------------------

TEST(ArinIconTest, ConstructionAndConfiguration) {
    Icon icon(IconType::Folder, 20.0f, 30.0f, 24.0f, Color::from_rgba8(255, 200, 0));

    EXPECT_EQ(icon.type(), IconType::Folder);
    EXPECT_FLOAT_EQ(icon.bounds().x, 20.0f);
    EXPECT_FLOAT_EQ(icon.bounds().y, 30.0f);
    EXPECT_FLOAT_EQ(icon.bounds().width, 24.0f);
    EXPECT_FLOAT_EQ(icon.bounds().height, 24.0f);
    EXPECT_TRUE(icon.is_visible());
    EXPECT_TRUE(icon.is_enabled());

    icon.set_type(IconType::Settings)
        .set_color(Color::from_rgba8(0, 120, 215))
        .set_position(10.0f, 15.0f)
        .set_size(32.0f, 32.0f);

    EXPECT_EQ(icon.type(), IconType::Settings);
    EXPECT_FLOAT_EQ(icon.bounds().x, 10.0f);
    EXPECT_FLOAT_EQ(icon.bounds().y, 15.0f);
    EXPECT_FLOAT_EQ(icon.bounds().width, 32.0f);
    EXPECT_FLOAT_EQ(icon.bounds().height, 32.0f);
}

// -----------------------------------------------------------------------------
// Button with Icon Integration Unit Tests
// -----------------------------------------------------------------------------

TEST(ArinButtonTest, ButtonWithIconAndFitToText) {
    Font font;
    Button btn("Save Project", 0.0f, 0.0f, 50.0f, 32.0f);

    EXPECT_FALSE(btn.has_icon());
    EXPECT_EQ(btn.icon(), IconType::None);

    btn.set_icon(IconType::File, 16.0f, 8.0f);
    EXPECT_TRUE(btn.has_icon());
    EXPECT_EQ(btn.icon(), IconType::File);

    // Calling fit_to_text must reserve room for icon (16px) + gap (8px) + text + padding
    btn.fit_to_text(font, 14.0f);

    Vec2 text_size = font.measure_text("Save Project", btn.style().text_scale);
    float expected_min_width = text_size.x + 16.0f + 8.0f + 14.0f * 2.0f;
    EXPECT_GE(btn.bounds().width, expected_min_width);
}
