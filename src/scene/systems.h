// systems.h — internal declarations for built-in ECS systems.
// Called by Scene::run_systems() in the order defined by spec §15.2.
#pragma once

#include <glyph/scene.h>   // Entity, CollisionCallback

#include <entt/entt.hpp>

namespace glyph {

class LuaState;

void run_lifetime_system(entt::registry& reg, float dt);
// Implemented in src/lua/script_system.cpp — requires sol2.
void run_script_system(entt::registry& reg, Scene* scene, LuaState* lua, float dt);
void run_animator_system(entt::registry& reg, float dt);
void run_movement_system(entt::registry& reg, float dt);
void run_collision_system(entt::registry& reg, Scene* scene, float dt,
                          const CollisionCallback& cb,
                          const CollisionCallback& lua_cb);
void run_transform_propagation_system(entt::registry& reg);

} // namespace glyph
