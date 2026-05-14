// lua_bindings.cpp — sol2 bindings for the glyph Lua API.
//
// All engine types are registered on the "glyph" table. Mirrors the C++ API
// but stays handle-flat: no inheritance hierarchies, no raw engine pointers
// exposed directly to Lua scripts.
#include "lua_state_impl.h"

#include <glyph/audio.h>
#include <glyph/camera.h>
#include <glyph/components.h>
#include <glyph/input.h>
#include <glyph/math.h>
#include <glyph/resources.h>
#include <glyph/scene.h>
#include <glyph/time.h>

#include <string>

namespace glyph {

void register_lua_bindings(LuaStateImpl& impl) {
    sol::state& L = impl.lua;

    L.open_libraries(sol::lib::base, sol::lib::math,
                     sol::lib::string, sol::lib::table,
                     sol::lib::io, sol::lib::package);

    auto g = L.create_named_table("glyph");

    // -----------------------------------------------------------------------
    // Math types
    // -----------------------------------------------------------------------

    g.new_usertype<vec2>("vec2",
        sol::constructors<vec2(), vec2(float, float)>(),
        "x", &vec2::x,
        "y", &vec2::y,
        sol::meta_function::addition,
            [](const vec2& a, const vec2& b) { return a + b; },
        sol::meta_function::subtraction,
            [](const vec2& a, const vec2& b) { return a - b; },
        sol::meta_function::multiplication, sol::overload(
            [](const vec2& v, float s)  { return v * s; },
            [](float s,  const vec2& v) { return s * v; }),
        sol::meta_function::to_string,
            [](const vec2& v) {
                return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
            }
    );

    g.new_usertype<Color>("Color",
        sol::factories(
            []()                                       { return Color{}; },
            [](float r, float gv, float b, float a)   { return Color{r, gv, b, a}; }),
        "r", &Color::r, "g", &Color::g, "b", &Color::b, "a", &Color::a
    );

    // -----------------------------------------------------------------------
    // Components (mutable so Lua can write transform, velocity, etc.)
    // -----------------------------------------------------------------------

    g.new_usertype<Transform>("Transform",
        "position", &Transform::position,
        "rotation", &Transform::rotation,
        "scale",    &Transform::scale
    );

    g.new_usertype<Velocity>("Velocity",
        "value", &Velocity::value
    );

    g.new_usertype<Sprite>("Sprite",
        "tint",    &Sprite::tint,
        "layer",   &Sprite::layer,
        "visible", &Sprite::visible
    );

    // -----------------------------------------------------------------------
    // Entity handle
    // -----------------------------------------------------------------------

    g.new_usertype<Entity>("Entity",
        "valid",   &Entity::valid,
        "destroy", &Entity::destroy,

        // Component getters — return nil if component not present.
        "get_transform", [](Entity& e) -> Transform* { return e.try_get<Transform>(); },
        "get_velocity",  [](Entity& e) -> Velocity*  { return e.try_get<Velocity>();  },
        "get_sprite",    [](Entity& e) -> Sprite*    { return e.try_get<Sprite>();    },

        // Component adders.
        "add_transform", [](Entity& e, sol::optional<vec2> pos) -> Transform& {
            return e.add<Transform>(Transform{pos.value_or(vec2{0,0})});
        },
        "add_velocity",  [](Entity& e, sol::optional<vec2> vel) -> Velocity& {
            return e.add<Velocity>(Velocity{vel.value_or(vec2{0,0})});
        },
        "add_sprite",    [&impl](Entity& e, const std::string& path) -> Sprite& {
            return e.add<Sprite>(Sprite{impl.resources->texture(path)});
        }
    );

    // -----------------------------------------------------------------------
    // Scene
    // -----------------------------------------------------------------------

    g["create_entity"] = [&impl]() {
        return impl.scene->create_entity();
    };
    g["create_entity_named"] = [&impl](const std::string& name) {
        return impl.scene->create_entity(name);
    };
    g["find"] = [&impl](const std::string& name) {
        return impl.scene->find(name);
    };

    // -----------------------------------------------------------------------
    // Input
    // -----------------------------------------------------------------------

    g["key_down"]    = [&impl](int k) {
        return impl.input->key_down(static_cast<Key>(k));
    };
    g["action_down"] = [&impl](const std::string& a) {
        return impl.input->action_down(a);
    };
    g["axis"] = [&impl](const std::string& a) {
        return impl.input->axis_value(a);
    };

    // Key constants table — mirrors glyph::Key enum values.
    auto keys = g.create_named("Key");
    auto kv = [&](const char* name, Key k) {
        keys[name] = static_cast<int>(k);
    };
    kv("W", Key::W); kv("A", Key::A); kv("S", Key::S); kv("D", Key::D);
    kv("Up", Key::Up); kv("Down", Key::Down);
    kv("Left", Key::Left); kv("Right", Key::Right);
    kv("Space", Key::Space); kv("Return", Key::Return);
    kv("Escape", Key::Escape);
    kv("LShift", Key::LShift); kv("LCtrl", Key::LCtrl);
    kv("F1", Key::F1); kv("F2", Key::F2); kv("F3", Key::F3); kv("F4", Key::F4);

    // -----------------------------------------------------------------------
    // Audio
    // -----------------------------------------------------------------------

    g["play_sound"] = [&impl](const std::string& path,
                               sol::optional<float> vol) {
        auto s = impl.resources->sound(path);
        impl.audio->play(s, vol.value_or(1.f));
    };
    g["play_music"] = [&impl](const std::string& path,
                               sol::optional<float> vol) {
        auto m = impl.resources->music(path);
        impl.audio->play_music(m, vol.value_or(1.f));
    };
    g["stop_music"] = [&impl]() { impl.audio->stop_music(); };

    // -----------------------------------------------------------------------
    // Resources
    // -----------------------------------------------------------------------

    g["load_texture"] = [&impl](const std::string& path) {
        impl.resources->texture(path);   // preload into cache; discard ref
    };

    // -----------------------------------------------------------------------
    // Time
    // -----------------------------------------------------------------------

    // glyph.dt is read each frame via a property so it stays current.
    g["dt"]      = sol::property([&impl]() { return impl.dt; });
    g["elapsed"] = [&impl]() { return impl.time->elapsed(); };
}

} // namespace glyph
