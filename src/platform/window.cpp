// window.cpp — SDL3 window implementation.
#include "window.h"

#include <glyph/app.h>

#include <SDL3/SDL.h>

#include <cstdio>

namespace glyph {

bool Window::create(const AppConfig& config) {
    SDL_WindowFlags flags = 0;
    if (config.resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }
    if (config.fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    // We'll add SDL_WINDOW_OPENGL in phase 2 when the GL context arrives.

    window_ = SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);
    if (!window_) {
        std::fprintf(stderr, "[glyph] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

void Window::destroy() {
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

} // namespace glyph
