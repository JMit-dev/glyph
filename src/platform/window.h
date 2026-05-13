// window.h — internal SDL3 window and GL context wrapper.
// Never include from public headers.
#pragma once

#include <glyph/math.h>

struct SDL_Window;

namespace glyph {

struct AppConfig;

class Window {
public:
    bool  create(const AppConfig& config);
    void  destroy();

    // Present the back buffer. Call once per frame after all GL draws.
    void  swap_buffers();

    // Pixel dimensions of the drawable surface (may differ from logical size on HiDPI).
    ivec2 drawable_size() const;

    SDL_Window* sdl_handle() const { return window_; }

    bool should_close() const { return should_close_; }
    void request_close() { should_close_ = true; }

private:
    SDL_Window* window_       = nullptr;
    void*       gl_context_   = nullptr;   // SDL_GLContext (typedef void*)
    bool        should_close_ = false;
};

} // namespace glyph
