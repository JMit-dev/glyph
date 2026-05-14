// lua_state_impl.h — LuaStateImpl definition (internal to src/lua/ only).
#pragma once

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include <sol/sol.hpp>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#include <glyph/lua_state.h>

namespace glyph {

class Audio;
class Input;
class Resources;
class Scene;
class Time;

struct LuaStateImpl {
    sol::state lua;
    Scene*     scene     = nullptr;
    Resources* resources = nullptr;
    Audio*     audio     = nullptr;
    Input*     input     = nullptr;
    Time*      time      = nullptr;
    float      dt        = 0.f;
};

// Defined in lua_bindings.cpp.
void register_lua_bindings(LuaStateImpl& impl);

} // namespace glyph
