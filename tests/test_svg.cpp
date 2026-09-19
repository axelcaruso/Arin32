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
#include <filesystem>
#include <arin/svg.hpp>
#include <arin/texture.hpp>
#include <arin/app.hpp>

static const char* kSampleCircleSvg =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 32 32\" width=\"32\" height=\"32\">"
    "<circle cx=\"16\" cy=\"16\" r=\"12\" fill=\"#0067C0\"/>"
    "</svg>";

static const char* kSampleRectSvg =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 64 32\" width=\"64\" height=\"32\">"
    "<rect x=\"4\" y=\"4\" width=\"56\" height=\"24\" rx=\"4\" fill=\"#10B981\"/>"
    "</svg>";

TEST(ArinSvgTest, ParseValidSvgString) {
    auto doc = arin::SvgDocument::load_from_memory(kSampleCircleSvg);
    ASSERT_NE(doc, nullptr);
    EXPECT_TRUE(doc->is_valid());
    EXPECT_FLOAT_EQ(doc->width(), 32.0f);
    EXPECT_FLOAT_EQ(doc->height(), 32.0f);
    EXPECT_FLOAT_EQ(doc->aspect_ratio(), 1.0f);
}

TEST(ArinSvgTest, AspectRatioCalculation) {
    auto doc = arin::SvgDocument::load_from_memory(kSampleRectSvg);
    ASSERT_NE(doc, nullptr);
    EXPECT_TRUE(doc->is_valid());
    EXPECT_FLOAT_EQ(doc->width(), 64.0f);
    EXPECT_FLOAT_EQ(doc->height(), 32.0f);
    EXPECT_FLOAT_EQ(doc->aspect_ratio(), 2.0f);
}

TEST(ArinSvgTest, ParseInvalidSvgMarkup) {
    auto doc1 = arin::SvgDocument::load_from_memory("");
    EXPECT_EQ(doc1, nullptr);

    auto doc2 = arin::SvgDocument::load_from_memory("<not an svg markup at all>");
    EXPECT_EQ(doc2, nullptr);

    auto doc3 = arin::SvgDocument::load_from_file("non_existent_file_path_12345.svg");
    EXPECT_EQ(doc3, nullptr);
}

TEST(ArinSvgTest, RasterizeToRgbaPixels) {
    auto doc = arin::SvgDocument::load_from_memory(kSampleCircleSvg);
    ASSERT_NE(doc, nullptr);

    int out_w = 0;
    int out_h = 0;
    std::vector<uint8_t> pixels = doc->rasterize_rgba(32, 32, 1.0f, &out_w, &out_h);

    EXPECT_EQ(out_w, 32);
    EXPECT_EQ(out_h, 32);
    EXPECT_EQ(pixels.size(), 32u * 32u * 4u);

    // Center pixel (16, 16) should be non-transparent and match the fill color (#0067C0: R=0, G=103, B=192)
    int center_idx = (16 * 32 + 16) * 4;
    EXPECT_EQ(pixels[center_idx + 0], 0);    // Red
    EXPECT_EQ(pixels[center_idx + 1], 103);  // Green
    EXPECT_EQ(pixels[center_idx + 2], 192);  // Blue
    EXPECT_GT(pixels[center_idx + 3], 200);  // Alpha
}

TEST(ArinSvgTest, RasterizeArbitraryResolution) {
    auto doc = arin::SvgDocument::load_from_memory(kSampleCircleSvg);
    ASSERT_NE(doc, nullptr);

    int out_w = 0;
    int out_h = 0;
    // Scale up from 32x32 to 128x128
    std::vector<uint8_t> pixels = doc->rasterize_rgba(128, 128, 1.0f, &out_w, &out_h);
    EXPECT_EQ(out_w, 128);
    EXPECT_EQ(out_h, 128);
    EXPECT_EQ(pixels.size(), 128u * 128u * 4u);
}

TEST(ArinSvgTest, MoveSemanticsTransfer) {
    auto doc = arin::SvgDocument::load_from_memory(kSampleCircleSvg);
    ASSERT_NE(doc, nullptr);
    EXPECT_TRUE(doc->is_valid());

    arin::SvgDocument moved(std::move(*doc));
    EXPECT_TRUE(moved.is_valid());
    EXPECT_FLOAT_EQ(moved.width(), 32.0f);
    EXPECT_FALSE(doc->is_valid());

    arin::SvgDocument assigned;
    assigned = std::move(moved);
    EXPECT_TRUE(assigned.is_valid());
    EXPECT_FLOAT_EQ(assigned.width(), 32.0f);
    EXPECT_FALSE(moved.is_valid());
}

TEST(ArinSvgTest, SvgImageWidgetAttributesAndChaining) {
    auto doc = arin::SvgDocument::load_from_memory(kSampleCircleSvg);
    arin::SvgImage img(doc, 20.0f, 30.0f, 64.0f, 64.0f, arin::ImageScaleMode::Fit);

    EXPECT_FLOAT_EQ(img.bounds().x, 20.0f);
    EXPECT_FLOAT_EQ(img.bounds().y, 30.0f);
    EXPECT_FLOAT_EQ(img.bounds().width, 64.0f);
    EXPECT_FLOAT_EQ(img.bounds().height, 64.0f);
    EXPECT_EQ(img.scale_mode(), arin::ImageScaleMode::Fit);
    EXPECT_TRUE(img.is_visible());
    EXPECT_TRUE(img.is_enabled());

    img.set_position(50.0f, 60.0f)
       .set_size(120.0f, 80.0f)
       .set_scale_mode(arin::ImageScaleMode::Center)
       .set_corner_radius(6.0f)
       .set_tint(arin::Color(200, 200, 200, 180));

    EXPECT_FLOAT_EQ(img.bounds().x, 50.0f);
    EXPECT_FLOAT_EQ(img.bounds().y, 60.0f);
    EXPECT_FLOAT_EQ(img.bounds().width, 120.0f);
    EXPECT_FLOAT_EQ(img.bounds().height, 80.0f);
    EXPECT_EQ(img.scale_mode(), arin::ImageScaleMode::Center);
    EXPECT_FLOAT_EQ(img.corner_radius(), 6.0f);
    EXPECT_EQ(img.tint().a, 180);
}

TEST(ArinSvgTest, SvgImageComputeContentRect) {
    auto doc = arin::SvgDocument::load_from_memory(kSampleRectSvg); // 64x32 (2:1 aspect ratio)
    arin::SvgImage img(doc, 0.0f, 0.0f, 100.0f, 100.0f, arin::ImageScaleMode::Fit);

    arin::Rect content = img.compute_content_rect();
    // In a 100x100 square with 2:1 aspect ratio, Fit makes it 100x50, centered vertically at y=25
    EXPECT_FLOAT_EQ(content.width, 100.0f);
    EXPECT_FLOAT_EQ(content.height, 50.0f);
    EXPECT_FLOAT_EQ(content.x, 0.0f);
    EXPECT_FLOAT_EQ(content.y, 25.0f);
}

static std::string resolve_icon_dir() {
    namespace fs = std::filesystem;
    if (fs::exists("assets/icons")) return "assets/icons";
    if (fs::exists("../assets/icons")) return "../assets/icons";
    if (fs::exists("../../assets/icons")) return "../../assets/icons";
    return "";
}

TEST(ArinSvgTest, LoadArsenalIconsFromDisk) {
    std::string dir = resolve_icon_dir();
    ASSERT_FALSE(dir.empty()) << "assets/icons directory not found from working directory";

    auto folder = arin::SvgDocument::load_from_file(dir + "/folder.svg");
    ASSERT_NE(folder, nullptr);
    EXPECT_TRUE(folder->is_valid());
    EXPECT_FLOAT_EQ(folder->width(), 32.0f);
    EXPECT_FLOAT_EQ(folder->height(), 32.0f);

    auto quick_access = arin::SvgDocument::load_from_file(dir + "/quick-access.svg");
    ASSERT_NE(quick_access, nullptr);
    EXPECT_TRUE(quick_access->is_valid());

    auto file_doc = arin::SvgDocument::load_from_file(dir + "/file-doc.svg");
    ASSERT_NE(file_doc, nullptr);
    EXPECT_TRUE(file_doc->is_valid());

    auto check_badge = arin::SvgDocument::load_from_file(dir + "/check-badge.svg");
    ASSERT_NE(check_badge, nullptr);
    EXPECT_TRUE(check_badge->is_valid());
}

TEST(ArinSvgTest, SvgImageFileLoading) {
    std::string dir = resolve_icon_dir();
    ASSERT_FALSE(dir.empty()) << "assets/icons directory not found from working directory";

    arin::SvgImage img(dir + "/folder.svg", 10.0f, 10.0f, 48.0f, 48.0f);
    ASSERT_NE(img.document(), nullptr);
    EXPECT_TRUE(img.document()->is_valid());
}

TEST(ArinSvgTest, ValidateAllArsenalIcons) {
    namespace fs = std::filesystem;
    std::string dir = resolve_icon_dir();
    ASSERT_FALSE(dir.empty()) << "assets/icons directory not found from working directory";

    int count = 0;
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() == ".svg") {
            auto doc = arin::SvgDocument::load_from_file(entry.path().string());
            ASSERT_NE(doc, nullptr) << "Failed to parse: " << entry.path();
            EXPECT_TRUE(doc->is_valid()) << "Invalid SVG: " << entry.path();
            EXPECT_GT(doc->width(), 0.0f);
            EXPECT_GT(doc->height(), 0.0f);

            int w = 0, h = 0;
            auto pixels = doc->rasterize_rgba(24, 24, 1.0f, &w, &h);
            EXPECT_EQ(w, 24);
            EXPECT_EQ(h, 24);
            EXPECT_FALSE(pixels.empty());
            count++;
        }
    }
    EXPECT_GE(count, 35);
}
