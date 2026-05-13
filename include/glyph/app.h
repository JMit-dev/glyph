// app.h — Game base class and AppConfig.
//
// Game authors subclass Game, override the lifecycle hooks they care about,
// and register their class with GLYPH_MAIN(). The engine owns main() and
// drives the lifecycle.
#pragma once

#include <string>

namespace glyph {

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

    // Called once before window creation. Return the desired config.
    virtual AppConfig configure() { return {}; }

    // Called once after the window, GL context, and subsystems are ready.
    virtual void on_start() {}

    // Called every frame. dt is delta time in seconds.
    virtual void on_update(float /*dt*/) {}

    // Called every frame after on_update. Issue all draw calls here.
    virtual void on_render(Renderer& /*r*/) {}

    // Called once before subsystems shut down.
    virtual void on_shutdown() {}
};

} // namespace glyph

// Game authors call GLYPH_MAIN(MyGameClass) at file scope in one
// translation unit to register their game with the engine.
#define GLYPH_MAIN(GameClass)                                                   \
    extern "C" glyph::Game* glyph_create_game() { return new GameClass(); }
