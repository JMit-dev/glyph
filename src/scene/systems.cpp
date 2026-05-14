// systems.cpp — built-in ECS system implementations.
// Run order per spec §15.2:
//   Lifetime → Script → Animator → Movement → Collision → TransformPropagation
#include "systems.h"

#include <glyph/components.h>
#include <glyph/sprite_sheet.h>

#include <cmath>
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

void run_animator_system(entt::registry& reg, float dt) {
    reg.view<Animator, Sprite>().each([dt](Animator& anim, Sprite& sp) {
        if (!anim.sheet || !anim.playing) return;

        auto it = anim.sheet->clips.find(anim.clip);
        if (it == anim.sheet->clips.end()) return;

        const AnimClip& clip = it->second;
        const int n = static_cast<int>(clip.frame_indices.size());
        if (n == 0) return;

        // Total clip duration in seconds.
        float total = 0.f;
        for (float d : clip.durations) total += d;
        if (total <= 0.f) return;

        anim.time += dt;

        float t = anim.time;
        if (clip.loop) {
            t = std::fmod(t, total);
        } else if (t >= total) {
            anim.time = total;
            anim.playing = false;
            t = total - 1e-6f;
        }

        // Walk frame durations to find which frame t falls into.
        int fi = n - 1;
        float acc = 0.f;
        for (int i = 0; i < n; ++i) {
            acc += clip.durations[i];
            if (t < acc) { fi = i; break; }
        }

        const int frame_idx = clip.frame_indices[fi];
        if (frame_idx < static_cast<int>(anim.sheet->frames.size())) {
            sp.src     = anim.sheet->frames[frame_idx];
            sp.texture = anim.sheet->texture;
        }
    });
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
