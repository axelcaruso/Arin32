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

#ifndef ARIN32_TEXTURE_HPP
#define ARIN32_TEXTURE_HPP

#include "types.hpp"
#include <cstdint>
#include <string>
#include <memory>

namespace arin {

/**
 * @brief Filtering strategies applied during OpenGL texture magnification and minification.
 */
enum class TextureFilter : uint8_t {
    Linear = 0, ///< Bilinear interpolation: smooth gradients, recommended for photos and UI bitmaps
    Nearest = 1 ///< Point sampling: preserves pixel sharpness, recommended for pixel art and icons
};

/**
 * @brief Layout scaling behavior when rendering an image inside bounded widget dimensions.
 */
enum class ImageScaleMode : uint8_t {
    Stretch = 0, ///< Stretches the image non-uniformly to exactly match the target bounding box
    Fit = 1,     ///< Uniformly scales the image to fit entirely inside bounds (letterbox/pillarbox)
    Fill = 2,    ///< Uniformly scales the image to completely cover bounds, clipping any excess
    Center = 3   ///< Retains native pixel dimensions and centers the image within bounds
};

/**
 * @brief Encapsulates a hardware-accelerated 2D OpenGL texture resource.
 *
 * Manages GPU texture memory allocation, image decoding (PNG, JPEG, BMP, TGA) via
 * portable stb_image, pixel uploads, and clean RAII lifecycle destruction.
 *
 * Designed to be OS-agnostic across Linux, FreeBSD, and future operating systems.
 */
class Texture {
public:
    /**
     * @brief Constructs an uninitialized, invalid texture object.
     */
    Texture();

    /**
     * @brief Releases the underlying OpenGL texture resource if allocated.
     */
    ~Texture();

    // Textures cannot be copied directly to prevent duplicate GPU deallocation (double free)
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Move semantics transfer GPU resource ownership cleanly
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    /**
     * @brief Loads an image from a filesystem path and uploads it to an OpenGL 2D texture.
     *
     * Automatically decodes PNG, JPG, BMP, and TGA formats into 32-bit RGBA pixels.
     *
     * @param filepath Path to the image file on disk.
     * @param filter Texture filtering mode (default: Linear).
     * @return Shared pointer to the new Texture, or nullptr if loading failed.
     */
    static std::shared_ptr<Texture> create_from_file(
        const std::string& filepath,
        TextureFilter filter = TextureFilter::Linear
    );

    /**
     * @brief Decodes an encoded image file stored in a contiguous memory buffer.
     *
     * Useful for embedded assets, network streams, or bundle archives.
     *
     * @param data Pointer to raw encoded file bytes (PNG/JPG/etc.).
     * @param size_bytes Total length of the data buffer in bytes.
     * @param filter Texture filtering mode (default: Linear).
     * @return Shared pointer to the new Texture, or nullptr if decoding failed.
     */
    static std::shared_ptr<Texture> create_from_memory(
        const uint8_t* data,
        size_t size_bytes,
        TextureFilter filter = TextureFilter::Linear
    );

    /**
     * @brief Creates an OpenGL texture directly from uncompressed 32-bit RGBA pixel buffers.
     *
     * @param width Width of the image in pixels.
     * @param height Height of the image in pixels.
     * @param rgba_pixels Pointer to width * height * 4 contiguous bytes.
     * @param filter Texture filtering mode (default: Linear).
     * @return Shared pointer to the new Texture, or nullptr if allocation failed.
     */
    static std::shared_ptr<Texture> create_from_rgba(
        int width,
        int height,
        const uint8_t* rgba_pixels,
        TextureFilter filter = TextureFilter::Linear
    );

    /**
     * @brief Allocates an uninitialized texture of specified dimensions on the GPU.
     *
     * Useful for dynamic render-to-texture targets or procedural canvas painting.
     *
     * @param width Width in pixels.
     * @param height Height in pixels.
     * @param filter Texture filtering mode (default: Linear).
     * @return Shared pointer to the new Texture.
     */
    static std::shared_ptr<Texture> create_empty(
        int width,
        int height,
        TextureFilter filter = TextureFilter::Linear
    );

    /**
     * @brief Updates a rectangular sub-region of the texture with new RGBA pixel data.
     *
     * @param x Left offset in pixels within the texture.
     * @param y Top offset in pixels within the texture.
     * @param width Width of the sub-region in pixels.
     * @param height Height of the sub-region in pixels.
     * @param rgba_pixels Pointer to width * height * 4 RGBA bytes.
     */
    void update_sub_rect(int x, int y, int width, int height, const uint8_t* rgba_pixels);

    /**
     * @brief Frees GPU texture memory and resets state to invalid.
     */
    void release();

    /// @brief Checks whether this texture possesses a valid OpenGL handle.
    bool is_valid() const { return m_handle != 0; }

    /// @brief Raw OpenGL texture object ID.
    uint32_t handle() const { return m_handle; }

    /// @brief Image width in pixels.
    int width() const { return m_width; }

    /// @brief Image height in pixels.
    int height() const { return m_height; }

    /// @brief Number of color channels (typically 4 for RGBA).
    int channels() const { return m_channels; }

    /// @brief Active texture filtering strategy.
    TextureFilter filter() const { return m_filter; }

    /// @brief Convenient Vec2 representing (width, height).
    Vec2 size() const {
        return Vec2(static_cast<float>(m_width), static_cast<float>(m_height));
    }

    /// @brief Computes aspect ratio (width / height).
    float aspect_ratio() const {
        return (m_height > 0) ? (static_cast<float>(m_width) / static_cast<float>(m_height)) : 1.0f;
    }

private:
    uint32_t m_handle{0};                  ///< OpenGL texture handle identifier
    int m_width{0};                        ///< Width in pixels
    int m_height{0};                       ///< Height in pixels
    int m_channels{0};                     ///< Channel count (4 = RGBA)
    TextureFilter m_filter{TextureFilter::Linear}; ///< Active filtering mode
};

} // namespace arin

#endif // ARIN32_TEXTURE_HPP
