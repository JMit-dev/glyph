// app.h — Game base class and AppConfig.
//
// Subclass Game, override the lifecycle hooks you care about, and register
// with GLYPH_MAIN(). The engine owns main() and drives the lifecycle.
#pragma once

#include <string>

namespace glyph {

class Input;
class Renderer;

struct AppConfig {
    std::string title      = "Glyph Game";
    int         width      = 1280;
    int         height     = 720;
    bool        vsync      = true;
    bool        resizable  = true;
    bool        fullscreen = false;
};

class Game {
public:
    virtual ~Game() = default;

    virtual AppConfig configure()           { return {}; }
    virtual void on_start()                 {}
    virtual void on_update(float /*dt*/)    {}
    virtual void on_render(Renderer& /*r*/) {}
    virtual void on_shutdown()              {}

protected:
    // Engine services — valid after on_start(); do not call from configure().
    Input& input();

private:
    // Engine-internal wiring — called by AppState before on_start().
    // Not for game code; use the protected accessors above.
    friend struct AppState;
    Input* input_ = nullptr;
};

} // namespace glyph

#define GLYPH_MAIN(GameClass) \
    extern "C" glyph::Game* glyph_create_game() { return new GameClass(); }
