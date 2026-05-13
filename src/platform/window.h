// window.h — internal SDL3 window wrapper.
// This header is internal to the engine; never include from public headers.
#pragma once

struct SDL_Window;

namespace glyph {

struct AppConfig;

class Window {
public:
    bool create(const AppConfig& config);
    void destroy();

    SDL_Window* sdl_handle() const { return window_; }

    bool should_close() const { return should_close_; }
    void request_close() { should_close_ = true; }

private:
    SDL_Window* window_       = nullptr;
    bool        should_close_ = false;
};

} // namespace glyph
