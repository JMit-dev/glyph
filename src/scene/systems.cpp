// systems.cpp — built-in ECS system implementations.
// Run order per spec §15.2:
//   Lifetime → Script → Animator → Movement → Collision → TransformPropagation
#include "systems.h"

#include <glyph/components.h>

#include <vector>

namespace glyph {

void run_lifetime_system(entt::registry& reg, float dt) {
    std::vector<entt::entity> dead;
    auto view = reg.view<Lifetime>();
    for (auto e : view) {
        view.get<Lifetime>(e).seconds -= dt;
        if (view.get<Lifetime>(e).seconds <= 0.f)
            dead.push_back(e);
    }
    reg.destroy(dead.begin(), dead.end());
}

void run_script_system(entt::registry& /*reg*/, float /*dt*/) {
    // Lua scripting not available until phase 16.
}

void run_animator_system(entt::registry& /*reg*/, float /*dt*/) {
    // SpriteSheet / AnimClip not available until phase 12.
}

void run_movement_system(entt::registry& reg, float dt) {
    reg.view<Velocity, Transform>().each([dt](Velocity& vel, Transform& xf) {
        xf.position += vel.value * dt;
    });
}

void run_collision_system(entt::registry& /*reg*/, float /*dt*/) {
    // Spatial-hash AABB collision deferred to phase 14.
}

void run_transform_propagation_system(entt::registry& /*reg*/) {
    // Parent-child world-transform resolution deferred: requires a separate
    // WorldTransform component to avoid accumulating parent offsets each frame.
}

} // namespace glyph
