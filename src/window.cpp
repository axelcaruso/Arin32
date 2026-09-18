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

#include "arin/window.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace arin {

// -----------------------------------------------------------------------------
// GlfwPlatformBackend Implementation (Linux, FreeBSD, POSIX)
// -----------------------------------------------------------------------------

GlfwPlatformBackend::GlfwPlatformBackend() = default;

GlfwPlatformBackend::~GlfwPlatformBackend() {
    destroy_window();
}

/**
 * @brief Initializes GLFW, sets up OpenGL 3.3 Core profile context, and binds GLEW.
 */
bool GlfwPlatformBackend::create_window(const std::string& title, int width, int height) {
    // Initialize the GLFW library
    if (!glfwInit()) {
        std::cerr << "[Arin32::GlfwBackend] Failed to initialize GLFW." << std::endl;
        return false;
    }

    // Configure OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__) || defined(__FreeBSD__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Enable multi-sampling anti-aliasing buffer hint (4 samples)
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create the window surface
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "[Arin32::GlfwBackend] Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return false;
    }

    // Make the OpenGL rendering context current for this thread
    glfwMakeContextCurrent(m_window);

    // Enable vertical synchronization (VSync = 1) to prevent tearing and cap frame rate
    glfwSwapInterval(1);

    // Initialize GLEW extension wrangler to load modern OpenGL function pointers
    glewExperimental = GL_TRUE;
    GLenum glew_status = glewInit();
    // On Wayland / EGL environments (Linux & FreeBSD), glewInit() may return GLEW_ERROR_NO_GLX_DISPLAY (code 4)
    // because no X11/GLX display is active, yet all core OpenGL entry points are fully functional.
    if (glew_status != GLEW_OK && glew_status != GLEW_ERROR_NO_GLX_DISPLAY) {
        std::cerr << "[Arin32::GlfwBackend] GLEW initialization error: "
                  << glewGetErrorString(glew_status) << std::endl;
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        glfwTerminate();
        return false;
    }

    // Clear benign GL_INVALID_ENUM error flag often set by glewExperimental
    glGetError();

    // Store 'this' pointer in the window user data slot for callback dispatching
    glfwSetWindowUserPointer(m_window, this);

    // Register GLFW event callbacks
    glfwSetCursorPosCallback(m_window, cursor_pos_callback);
    glfwSetMouseButtonCallback(m_window, mouse_button_callback);
    glfwSetScrollCallback(m_window, scroll_callback);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

    return true;
}

void GlfwPlatformBackend::destroy_window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        glfwTerminate();
    }
}

bool GlfwPlatformBackend::should_close() const {
    if (!m_window) return true;
    return glfwWindowShouldClose(m_window) != 0;
}

void GlfwPlatformBackend::poll_events() {
    glfwPollEvents();
}

void GlfwPlatformBackend::swap_buffers() {
    if (m_window) {
        glfwSwapBuffers(m_window);
    }
}

Vec2 GlfwPlatformBackend::get_window_size() const {
    if (!m_window) return Vec2(0.0f, 0.0f);
    int w = 0, h = 0;
    glfwGetWindowSize(m_window, &w, &h);
    return Vec2(static_cast<float>(w), static_cast<float>(h));
}

Vec2 GlfwPlatformBackend::get_framebuffer_size() const {
    if (!m_window) return Vec2(0.0f, 0.0f);
    int w = 0, h = 0;
    glfwGetFramebufferSize(m_window, &w, &h);
    return Vec2(static_cast<float>(w), static_cast<float>(h));
}

void GlfwPlatformBackend::set_mouse_callback(MouseCallback cb) {
    m_mouse_cb = std::move(cb);
}

void GlfwPlatformBackend::set_resize_callback(ResizeCallback cb) {
    m_resize_cb = std::move(cb);
}

// -----------------------------------------------------------------------------
// GLFW Callback Bridges
// -----------------------------------------------------------------------------

void GlfwPlatformBackend::cursor_pos_callback(GLFWwindow* win, double xpos, double ypos) {
    auto* self = static_cast<GlfwPlatformBackend*>(glfwGetWindowUserPointer(win));
    if (!self) return;

    MouseEvent ev = self->m_input_state.on_mouse_move(static_cast<float>(xpos), static_cast<float>(ypos));
    if (self->m_mouse_cb) {
        self->m_mouse_cb(ev);
    }
}

void GlfwPlatformBackend::mouse_button_callback(GLFWwindow* win, int button, int action, int /*mods*/) {
    auto* self = static_cast<GlfwPlatformBackend*>(glfwGetWindowUserPointer(win));
    if (!self) return;

    MouseButton mb = MouseButton::Left;
    if (button == GLFW_MOUSE_BUTTON_RIGHT) mb = MouseButton::Right;
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE) mb = MouseButton::Middle;

    InputAction ia = (action == GLFW_PRESS) ? InputAction::Press : InputAction::Release;

    MouseEvent ev = self->m_input_state.on_mouse_button(mb, ia);
    if (self->m_mouse_cb) {
        self->m_mouse_cb(ev);
    }
}

void GlfwPlatformBackend::scroll_callback(GLFWwindow* win, double xoffset, double yoffset) {
    auto* self = static_cast<GlfwPlatformBackend*>(glfwGetWindowUserPointer(win));
    if (!self) return;

    MouseEvent ev = self->m_input_state.on_mouse_scroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
    if (self->m_mouse_cb) {
        self->m_mouse_cb(ev);
    }
}

void GlfwPlatformBackend::framebuffer_size_callback(GLFWwindow* win, int width, int height) {
    auto* self = static_cast<GlfwPlatformBackend*>(glfwGetWindowUserPointer(win));
    if (!self) return;

    if (self->m_resize_cb) {
        self->m_resize_cb(width, height);
    }
}

// -----------------------------------------------------------------------------
// Window High-Level Wrapper Implementation
// -----------------------------------------------------------------------------

Window::Window(const std::string& title, int width, int height,
               std::unique_ptr<IPlatformBackend> backend) {
    if (backend) {
        m_backend = std::move(backend);
    } else {
        m_backend = std::make_unique<GlfwPlatformBackend>();
    }

    m_backend->create_window(title, width, height);
}

bool Window::is_open() const {
    return m_backend && !m_backend->should_close();
}

void Window::poll_events() {
    if (m_backend) {
        m_backend->poll_events();
    }
}

void Window::swap_buffers() {
    if (m_backend) {
        m_backend->swap_buffers();
    }
}

Vec2 Window::size() const {
    return m_backend ? m_backend->get_window_size() : Vec2(0.0f, 0.0f);
}

Vec2 Window::framebuffer_size() const {
    return m_backend ? m_backend->get_framebuffer_size() : Vec2(0.0f, 0.0f);
}

void Window::on_mouse_event(IPlatformBackend::MouseCallback cb) {
    if (m_backend) {
        m_backend->set_mouse_callback(std::move(cb));
    }
}

void Window::on_resize(IPlatformBackend::ResizeCallback cb) {
    if (m_backend) {
        m_backend->set_resize_callback(std::move(cb));
    }
}

} // namespace arin
