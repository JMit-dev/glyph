// systems.h — internal declarations for built-in ECS systems.
// Called by Scene::run_systems() in the order defined by spec §15.2.
#pragma once

#include <entt/entt.hpp>

namespace glyph {

void run_lifetime_system(entt::registry& reg, float dt);
void run_script_system(entt::registry& reg, float dt);
void run_animator_system(entt::registry& reg, float dt);
void run_movement_system(entt::registry& reg, float dt);
void run_collision_system(entt::registry& reg, float dt);
void run_transform_propagation_system(entt::registry& reg);

} // namespace glyph
