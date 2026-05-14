// lua_state.h — Lua 5.4 scripting state.
//
// LuaState wraps one sol::state and exposes the engine services to Lua scripts.
// sol.hpp is NOT included here — the PIMPL keeps it confined to lua_state.cpp
// and lua_bindings.cpp. Game code that needs direct sol2 access should include
// <sol/sol.hpp> separately after including this header.
//
// Usage (C++ game with embedded Lua):
//   void on_start() override {
//       lua().run_file("scripts/main.lua");
//   }
//   void on_update(float dt) override {
//       lua().on_update(dt);   // calls glyph.on_update(dt) if defined in Lua
//   }
//   void on_render(Renderer& r) override {
//       lua().on_render(r);
//   }
#pragma once

#include <memory>
#include <string>

namespace glyph {

class Audio;
class Input;
class Renderer;
class Resources;
class Scene;
class Time;

// Internal implementation detail — not part of the public API.
struct LuaStateImpl;

class LuaState {
public:
    LuaState();
    ~LuaState();   // defined in lua_state.cpp so Impl destructor is complete

    // Initialise the Lua VM, open standard libs, and register all glyph bindings.
    // Non-fatal: if this returns false the engine continues without Lua.
    bool init(Scene& scene, Resources& resources,
              Audio& audio, Input& input, Time& time);

    void shutdown();

    // Execute a Lua file. Errors are printed to stderr; execution continues.
    void run_file(const std::string& path);

    // Call glyph.on_update(dt) if defined in the loaded Lua environment.
    void on_update(float dt);

    // Call glyph.on_render() if defined in the loaded Lua environment.
    void on_render(Renderer& r);

    bool initialized() const { return impl_ != nullptr; }

    // Internal accessor for src/lua/ subsystem — requires lua_state_impl.h to use.
    LuaStateImpl* impl() { return impl_.get(); }

private:
    std::unique_ptr<LuaStateImpl> impl_;
};

} // namespace glyph
