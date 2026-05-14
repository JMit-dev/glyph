// lua_state.cpp — LuaState implementation.
#include "lua_state_impl.h"

#include <cstdio>
#include <string>

namespace glyph {

LuaState::LuaState()  = default;
LuaState::~LuaState() = default;   // Impl destructor resolved here

bool LuaState::init(Scene& scene, Resources& resources,
                    Audio& audio, Input& input, Time& time) {
    impl_ = std::make_unique<LuaStateImpl>();
    impl_->scene     = &scene;
    impl_->resources = &resources;
    impl_->audio     = &audio;
    impl_->input     = &input;
    impl_->time      = &time;

    register_lua_bindings(*impl_);

    return true;
}

void LuaState::shutdown() {
    impl_.reset();
}

void LuaState::run_file(const std::string& path) {
    if (!impl_) return;
    auto result = impl_->lua.safe_script_file(path,
        [](lua_State*, sol::protected_function_result pfr) { return pfr; });
    if (!result.valid()) {
        sol::error err = result;
        std::fprintf(stderr, "[glyph] Lua error in %s: %s\n",
                     path.c_str(), err.what());
    }
}

void LuaState::on_update(float dt) {
    if (!impl_) return;
    impl_->dt = dt;
    sol::protected_function fn = impl_->lua["glyph"]["on_update"];
    if (!fn.valid()) return;
    auto r = fn(dt);
    if (!r.valid()) {
        sol::error err = r;
        std::fprintf(stderr, "[glyph] Lua on_update error: %s\n", err.what());
    }
}

void LuaState::on_render(Renderer& /*r*/) {
    if (!impl_) return;
    sol::protected_function fn = impl_->lua["glyph"]["on_render"];
    if (!fn.valid()) return;
    // Renderer bindings added in a later phase; on_render is hooked for future use.
    auto result = fn();
    if (!result.valid()) {
        sol::error err = result;
        std::fprintf(stderr, "[glyph] Lua on_render error: %s\n", err.what());
    }
}

} // namespace glyph
