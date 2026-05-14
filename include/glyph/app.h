// app.h — Game base class and AppConfig.
//
// Subclass Game, override the lifecycle hooks you care about, and register
// with GLYPH_MAIN(). The engine owns main() and drives the lifecycle.
#pragma once

#include <string>

namespace glyph {

class Audio;
class Input;
class LuaState;
class Renderer;
class Resources;
class Scene;
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

    virtual AppConfig configure()              { return {}; }
    virtual void on_start()                    {}
    virtual void on_update(float /*dt*/)       {}
    virtual void on_fixed_update(float /*dt*/) {}
    virtual void on_render(Renderer& /*r*/)    {}
    virtual void on_shutdown()                 {}

protected:
    // Engine services — valid after on_start(); do not call from configure().
    Audio&     audio();
    Input&     input();
    LuaState&  lua();
    Resources& resources();
    Scene&     scene();
    Time&      time();

public:
    // Engine-internal wiring — called before on_start(). Not for game code.
    void engine_set_audio    (Audio*     a) { audio_     = a; }
    void engine_set_input    (Input*     i) { input_     = i; }
    void engine_set_lua      (LuaState*  l) { lua_       = l; }
    void engine_set_resources(Resources* r) { resources_ = r; }
    void engine_set_scene    (Scene*     s) { scene_     = s; }
    void engine_set_time     (Time*      t) { time_      = t; }

private:
    Audio*     audio_     = nullptr;
    Input*     input_     = nullptr;
    LuaState*  lua_       = nullptr;
    Resources* resources_ = nullptr;
    Scene*     scene_     = nullptr;
    Time*      time_      = nullptr;
};

} // namespace glyph

#define GLYPH_MAIN(GameClass) \
    extern "C" glyph::Game* glyph_create_game() { return new GameClass(); }
