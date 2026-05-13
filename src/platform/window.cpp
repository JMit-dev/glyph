// window.cpp — SDL3 window and OpenGL context implementation.
#include "window.h"

#include <glyph/app.h>

#include <SDL3/SDL.h>

// glad must be included after the GL context is current; the actual
// gladLoadGL() call happens inside create() after SDL_GL_MakeCurrent.
#if defined(GLYPH_PLATFORM_DESKTOP)
#  include <glad/gl.h>
#endif

#include <cstdio>
#include <cstring>

namespace glyph {

bool Window::create(const AppConfig& config) {
    // Request GL 3.3 Core before creating the window. SDL reads these
    // attributes during SDL_CreateWindow when SDL_WINDOW_OPENGL is set.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_WindowFlags flags = SDL_WINDOW_OPENGL;
    if (config.resizable)  flags |= SDL_WINDOW_RESIZABLE;
    if (config.fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    window_ = SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);
    if (!window_) {
        std::fprintf(stderr, "[glyph] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    gl_context_ = SDL_GL_CreateContext(window_);
    if (!gl_context_) {
        std::fprintf(stderr, "[glyph] SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        return false;
    }

    SDL_GL_MakeCurrent(window_, gl_context_);

#if defined(GLYPH_PLATFORM_DESKTOP)
    // Load all GL 3.3 Core function pointers via glad2.
    // SDL_GL_GetProcAddress returns SDL_FunctionPointer (a function pointer type),
    // but glad needs void* (a data pointer). They are the same size on all
    // platforms we target; memcpy is the pedantically-correct conversion.
    const int gl_version = gladLoadGL([](const char* name) -> void* {
        SDL_FunctionPointer fn = SDL_GL_GetProcAddress(name);
        void* ptr;
        std::memcpy(&ptr, &fn, sizeof(ptr));
        return ptr;
    });
    if (!gl_version) {
        std::fprintf(stderr, "[glyph] gladLoadGL failed — GL 3.3 Core not supported?\n");
        SDL_GL_DestroyContext(gl_context_);
        SDL_DestroyWindow(window_);
        gl_context_ = nullptr;
        window_     = nullptr;
        return false;
    }
#endif

    SDL_GL_SetSwapInterval(config.vsync ? 1 : 0);

    return true;
}

void Window::swap_buffers() {
    SDL_GL_SwapWindow(window_);
}

ivec2 Window::drawable_size() const {
    ivec2 s;
    SDL_GetWindowSizeInPixels(window_, &s.x, &s.y);
    return s;
}

void Window::destroy() {
    if (gl_context_) {
        SDL_GL_DestroyContext(gl_context_);
        gl_context_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

} // namespace glyph
