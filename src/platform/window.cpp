// window.cpp — SDL3 window and OpenGL context implementation.
#include "window.h"

#include <glyph/app.h>

#include <SDL3/SDL.h>

#if defined(GLYPH_PLATFORM_DESKTOP)
#  include <glad/gl.h>

// glad needs a loader that returns GLADapiproc (a function pointer type, not void*).
// SDL_GL_GetProcAddress returns SDL_FunctionPointer, which is the same underlying
// type on all our desktop targets. reinterpret_cast between function pointer types
// is explicitly blessed by the POSIX dlsym spec and works on all supported ABIs.
static GLADapiproc glad_proc_loader(const char* name) {
    return reinterpret_cast<GLADapiproc>(SDL_GL_GetProcAddress(name));
}
#endif

#include <cstdio>

namespace glyph {

bool Window::create(const AppConfig& config) {
    // Request the appropriate GL profile for the target platform.
    // Desktop: OpenGL 3.3 Core (GLSL 330). Web/mobile: GLES 3.0 (GLSL 300 es).
#if defined(__EMSCRIPTEN__)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_WindowFlags flags = SDL_WINDOW_OPENGL;
    if (config.resizable)  flags |= SDL_WINDOW_RESIZABLE;
    if (config.fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    window_ = SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);
    if (!window_) {
        std::fprintf(stderr, "[glyph] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    // SDL_GLContext is a struct pointer type; void* stores it without casting.
    gl_context_ = SDL_GL_CreateContext(window_);
    if (!gl_context_) {
        std::fprintf(stderr, "[glyph] SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        return false;
    }

    SDL_GL_MakeCurrent(window_, static_cast<SDL_GLContext>(gl_context_));

#if defined(GLYPH_PLATFORM_DESKTOP)
    const int gl_version = gladLoadGL(glad_proc_loader);
    if (!gl_version) {
        std::fprintf(stderr, "[glyph] gladLoadGL failed — GL 3.3 Core not supported?\n");
        SDL_GL_DestroyContext(static_cast<SDL_GLContext>(gl_context_));
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
        SDL_GL_DestroyContext(static_cast<SDL_GLContext>(gl_context_));
        gl_context_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

} // namespace glyph
