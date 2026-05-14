// app.h — Game base class and AppConfig.
//
// Subclass Game, override the lifecycle hooks you care about, and register
// with GLYPH_MAIN(). The engine owns main() and drives the lifecycle.
#pragma once

#include <string>

namespace glyph {

class Input;
class Renderer;
class Time;

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

    virtual AppConfig configure()             { return {}; }
    virtual void on_start()                   {}
    virtual void on_update(float /*dt*/)      {}

    // Called at a fixed 60 Hz rate, independent of frame rate.
    // dt is always Time::kFixedDt (1/60 s). Use for physics and gameplay logic
    // that must be frame-rate independent. on_update still runs once per frame.
    virtual void on_fixed_update(float /*dt*/) {}

    virtual void on_render(Renderer& /*r*/)   {}
    virtual void on_shutdown()                {}

protected:
    // Engine services — valid after on_start(); do not call from configure().
    Input& input();
    Time&  time();

public:
    // Engine-internal wiring — called before on_start(). Not for game code.
    void engine_set_input(Input* i) { input_ = i; }
    void engine_set_time (Time*  t) { time_  = t; }

private:
    Input* input_ = nullptr;
    Time*  time_  = nullptr;
};

} // namespace glyph

#define GLYPH_MAIN(GameClass) \
    extern "C" glyph::Game* glyph_create_game() { return new GameClass(); }
