// systems.cpp — built-in ECS system implementations.
// Run order per spec §15.2:
//   Lifetime → Script → Animator → Movement → Collision → TransformPropagation
#include "systems.h"

#include <glyph/components.h>
#include <glyph/sprite_sheet.h>

#include <cmath>
#include <unordered_map>
#include <unordered_set>
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

// run_script_system is defined in src/lua/script_system.cpp (requires sol2).

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

void run_collision_system(entt::registry& reg, Scene* scene, float /*dt*/,
                          const CollisionCallback& cb,
                          const CollisionCallback& lua_cb) {
    if (!cb && !lua_cb) return;  // no subscribers — skip the entire pass

    // --- Step 1: compute world-space AABBs for all BoxCollider+Transform entities ---
    // Rotation is ignored (TransformPropagation still stubbed); scale is applied.
    std::unordered_map<entt::entity, Rect> aabbs;
    {
        auto view = reg.view<BoxCollider, Transform>();
        for (auto e : view) {
            const auto& col = view.get<BoxCollider>(e);
            const auto& xf  = view.get<Transform>(e);
            aabbs[e] = {
                xf.position.x + col.bounds.x * xf.scale.x,
                xf.position.y + col.bounds.y * xf.scale.y,
                col.bounds.w  * xf.scale.x,
                col.bounds.h  * xf.scale.y
            };
        }
    }
    if (aabbs.size() < 2) return;

    // --- Step 2: build spatial hash ---
    // Cell size ≈ average sprite size; 64 px is a reasonable default.
    static constexpr float kCellSize = 64.f;

    struct CellKey {
        int x, y;
        bool operator==(const CellKey& o) const { return x == o.x && y == o.y; }
    };
    struct CellHash {
        size_t operator()(const CellKey& k) const {
            return static_cast<size_t>(k.x) * 2654435761u
                 ^ static_cast<size_t>(k.y) * 2246822519u;
        }
    };

    std::unordered_map<CellKey, std::vector<entt::entity>, CellHash> grid;
    for (auto& [e, aabb] : aabbs) {
        const int x0 = static_cast<int>(std::floor(aabb.x / kCellSize));
        const int y0 = static_cast<int>(std::floor(aabb.y / kCellSize));
        const int x1 = static_cast<int>(std::floor((aabb.x + aabb.w) / kCellSize));
        const int y1 = static_cast<int>(std::floor((aabb.y + aabb.h) / kCellSize));
        for (int cy = y0; cy <= y1; ++cy)
            for (int cx = x0; cx <= x1; ++cx)
                grid[{cx, cy}].push_back(e);
    }

    // --- Step 3: test candidate pairs ---
    // Encode pair as (min_id << 32 | max_id) so each pair is checked once.
    std::unordered_set<uint64_t> checked;

    for (auto& [cell, entities] : grid) {
        const int n = static_cast<int>(entities.size());
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                const entt::entity ea = entities[i];
                const entt::entity eb = entities[j];

                const auto ia = static_cast<uint64_t>(entt::to_integral(ea));
                const auto ib = static_cast<uint64_t>(entt::to_integral(eb));
                const uint64_t pk = ia < ib ? (ia << 32 | ib) : (ib << 32 | ia);
                if (!checked.insert(pk).second) continue;

                // Layer / mask filter (either side must see the other's layer).
                const auto& col_a = reg.get<BoxCollider>(ea);
                const auto& col_b = reg.get<BoxCollider>(eb);
                if (!(col_a.layer & col_b.mask) && !(col_b.layer & col_a.mask))
                    continue;

                // AABB narrow phase.
                if (!aabbs[ea].intersects(aabbs[eb])) continue;

                if (cb)     cb({scene, ea}, {scene, eb});
                if (lua_cb) lua_cb({scene, ea}, {scene, eb});
            }
        }
    }
}

void run_transform_propagation_system(entt::registry& /*reg*/) {
    // Parent-child world-transform resolution deferred: requires a separate
    // WorldTransform component to avoid accumulating parent offsets each frame.
}

} // namespace glyph
