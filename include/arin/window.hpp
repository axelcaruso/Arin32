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

#ifndef ARIN32_WINDOW_HPP
#define ARIN32_WINDOW_HPP

#include "types.hpp"
#include "input.hpp"
#include <string>
#include <functional>
#include <memory>

struct GLFWwindow; // Forward declaration of GLFW handle

namespace arin {

/**
 * @brief Abstract platform interface for windowing, event pump, and graphics contexts.
 *
 * Decouples Arin32 from any specific windowing toolkit or desktop environment.
 * On POSIX systems (Linux, FreeBSD), Arin32 provides the GLFW-based GlfwPlatformBackend.
 * For a custom future operating system, implementing this interface allows Arin32
 * to run directly on kernel framebuffers, custom display servers, or compositor protocols.
 */
class IPlatformBackend {
public:
    using MouseCallback = std::function<void(const MouseEvent&)>;
    using ResizeCallback = std::function<void(int width, int height)>;

    virtual ~IPlatformBackend() = default;

    /**
     * @brief Creates a window and establishes an OpenGL context.
     * @param title Title text for the window.
     * @param width Desired initial window width in pixels.
     * @param height Desired initial window height in pixels.
     * @return true if initialization succeeded, false otherwise.
     */
    virtual bool create_window(const std::string& title, int width, int height) = 0;

    /**
     * @brief Destroys the window and releases the OpenGL context.
     */
    virtual void destroy_window() = 0;

    /**
     * @brief Checks if the user or OS requested to close the window.
     */
    virtual bool should_close() const = 0;

    /**
     * @brief Pumps system input events from the operating system event queue.
     */
    virtual void poll_events() = 0;

    /**
     * @brief Swaps the front and back display buffers (presenting the frame).
     */
    virtual void swap_buffers() = 0;

    /**
     * @brief Gets current logical window dimensions.
     */
    virtual Vec2 get_window_size() const = 0;

    /**
     * @brief Gets current physical framebuffer dimensions in pixels.
     */
    virtual Vec2 get_framebuffer_size() const = 0;

    /**
     * @brief Registers callback for incoming mouse events.
     */
    virtual void set_mouse_callback(MouseCallback cb) = 0;

    /**
     * @brief Registers callback for window resize events.
     */
    virtual void set_resize_callback(ResizeCallback cb) = 0;
};

/**
 * @brief Platform backend implementation using GLFW for Linux and FreeBSD.
 */
class GlfwPlatformBackend : public IPlatformBackend {
public:
    GlfwPlatformBackend();
    ~GlfwPlatformBackend() override;

    bool create_window(const std::string& title, int width, int height) override;
    void destroy_window() override;
    bool should_close() const override;
    void poll_events() override;
    void swap_buffers() override;
    Vec2 get_window_size() const override;
    Vec2 get_framebuffer_size() const override;
    void set_mouse_callback(MouseCallback cb) override;
    void set_resize_callback(ResizeCallback cb) override;

    /// @brief Raw GLFW window handle if direct integration is required.
    GLFWwindow* handle() const { return m_window; }

private:
    static void cursor_pos_callback(GLFWwindow* win, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* win, int button, int action, int mods);
    static void scroll_callback(GLFWwindow* win, double xoffset, double yoffset);
    static void framebuffer_size_callback(GLFWwindow* win, int width, int height);

    GLFWwindow* m_window{nullptr};
    MouseCallback m_mouse_cb;
    ResizeCallback m_resize_cb;
    InputState m_input_state;
};

/**
 * @brief High-level window wrapper managing lifecycle, events, and display.
 */
class Window {
public:
    /**
     * @brief Constructs and displays a new window.
     * @param title Title bar text.
     * @param width Initial width.
     * @param height Initial height.
     * @param backend Custom platform backend (defaults to GLFW if nullptr).
     */
    Window(const std::string& title, int width, int height,
           std::unique_ptr<IPlatformBackend> backend = nullptr);

    ~Window() = default;

    // Non-copyable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /// @brief Returns true if the window is open and operational.
    bool is_open() const;

    /// @brief Polls operating system inputs.
    void poll_events();

    /// @brief Swaps the rendering back buffer to screen.
    void swap_buffers();

    /// @brief Logical window width and height.
    Vec2 size() const;

    /// @brief Physical pixel dimensions of the framebuffer.
    Vec2 framebuffer_size() const;

    /// @brief Registers a handler for mouse interactions.
    void on_mouse_event(IPlatformBackend::MouseCallback cb);

    /// @brief Registers a handler for window resize notifications.
    void on_resize(IPlatformBackend::ResizeCallback cb);

    /// @brief Direct access to underlying backend.
    IPlatformBackend* backend() { return m_backend.get(); }

private:
    std::unique_ptr<IPlatformBackend> m_backend;
};

} // namespace arin

#endif // ARIN32_WINDOW_HPP
