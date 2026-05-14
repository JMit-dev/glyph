// script_system.cpp — ScriptSystem implementation (requires sol2).
//
// On the first update of an entity with a Script component, the system:
//   1. Loads (and caches) the Lua module from scripts/entities/<name>.lua
//   2. Creates a per-entity self table with the class as its __index
//   3. Stores self.entity = Entity handle
//   4. Calls self:on_start()
// Every subsequent frame it calls self:on_update(dt).
// on_collision(other) is called by the CollisionSystem via LuaState::init().
#include "lua_state_impl.h"

#include <glyph/components.h>
#include <glyph/scene.h>

#include <any>
#include <cstdio>

namespace glyph {

void run_script_system(entt::registry& reg, Scene* scene, LuaState* lua_state, float dt) {
    if (!lua_state) return;
    LuaStateImpl* impl = lua_state->impl();
    if (!impl) return;

    auto view = reg.view<Script>();
    for (auto e : view) {
        auto& script = view.get<Script>(e);
        if (script.lua_module.empty()) continue;

        // --- First frame: initialise the self table ---
        if (!script.self.has_value()) {
            sol::table class_tbl = impl->load_script_module(script.lua_module);
            if (!class_tbl.valid()) continue;

            // self = setmetatable({}, {__index = class_tbl})
            sol::table self = impl->lua.create_table();
            sol::table mt   = impl->lua.create_table();
            mt[sol::meta_function::index] = class_tbl;
            self[sol::metatable_key]      = mt;
            self["entity"]                = Entity{scene, e};

            script.self = self;

            sol::protected_function on_start = class_tbl["on_start"];
            if (on_start.valid()) {
                auto r = on_start(self);
                if (!r.valid()) {
                    sol::error err = r;
                    std::fprintf(stderr, "[glyph] Script '%s' on_start: %s\n",
                                 script.lua_module.c_str(), err.what());
                }
            }
        }

        // --- Every frame: call on_update ---
        sol::table& self = std::any_cast<sol::table&>(script.self);
        sol::protected_function on_update = self["on_update"];
        if (on_update.valid()) {
            auto r = on_update(self, dt);
            if (!r.valid()) {
                sol::error err = r;
                std::fprintf(stderr, "[glyph] Script '%s' on_update: %s\n",
                             script.lua_module.c_str(), err.what());
            }
        }
    }
}

} // namespace glyph
