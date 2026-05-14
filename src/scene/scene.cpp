// scene.cpp — Scene non-template method implementations.
#include <glyph/scene.h>
#include <glyph/components.h>
#include <glyph/renderer.h>
#include <glyph/tilemap.h>

#include "systems.h"

#include <algorithm>
#include <vector>

namespace glyph {

Entity Scene::create_entity() {
    return {this, registry_.create()};
}

Entity Scene::create_entity(const std::string& name) {
    Entity e = create_entity();
    e.add<Name>(Name{name});
    return e;
}

Entity Scene::find(const std::string& name) {
    auto view = registry_.view<Name>();
    for (auto e : view) {
        if (view.get<Name>(e).value == name)
            return {this, e};
    }
    return {};
}

void Scene::run_systems(float dt) {
    run_lifetime_system(registry_, dt);
    run_script_system(registry_, dt);
    run_animator_system(registry_, dt);
    run_movement_system(registry_, dt);
    run_collision_system(registry_, this, dt, collision_cb_);
    run_transform_propagation_system(registry_);
}

void Scene::render(Renderer& r) {
    // 1. Find the primary Camera2D; re-start the batch with its VP if found.
    //    begin_frame() was already called in main_entry, so calling it again here
    //    just overwrites the VP before any quads are queued — safe and intentional.
    {
        auto view = registry_.view<Camera2D>();
        for (auto e : view) {
            const auto& cam = view.get<Camera2D>(e);
            if (cam.primary) {
                r.set_camera(cam.value);
                r.begin_frame();
                break;
            }
        }
    }

    // 2. Render TilemapRef entities (background — drawn before sprites).
    {
        auto tv = registry_.view<TilemapRef, Transform>();
        for (auto e : tv) {
            const auto& tmref = tv.get<TilemapRef>(e);
            const auto& xf    = tv.get<Transform>(e);
            if (!tmref.map) continue;

            Tilemap& map = *tmref.map;
            for (const auto& layer : map.tile_layers) {
                if (!layer.visible) continue;
                for (int row = 0; row < layer.size.y; ++row) {
                    for (int col = 0; col < layer.size.x; ++col) {
                        const uint32_t raw = layer.gids[row * layer.size.x + col];
                        const uint32_t gid = raw & 0x1FFFFFFFu;
                        if (gid == 0) continue;

                        const Tileset* ts = map.tileset_for(gid);
                        if (!ts || !ts->texture) continue;

                        const Rect src  = map.tile_uv(gid);
                        const Rect dest = {
                            xf.position.x + static_cast<float>(col * map.tile_w),
                            xf.position.y + static_cast<float>(row * map.tile_h),
                            static_cast<float>(map.tile_w),
                            static_cast<float>(map.tile_h)
                        };
                        r.draw_textured_quad(*ts->texture, dest, src,
                                             {1.f, 1.f, 1.f, layer.opacity});
                    }
                }
            }
        }
    }

    // 3. Collect visible Sprite+Transform pairs and sort by layer (stable = insertion
    //    order preserved within the same layer).
    struct DrawEntry {
        int              layer;
        const Sprite*    sp;
        const Transform* xf;
    };
    std::vector<DrawEntry> entries;

    auto sv = registry_.view<Sprite, Transform>();
    for (auto e : sv) {
        const auto& sp = sv.get<Sprite>(e);
        const auto& xf = sv.get<Transform>(e);
        if (sp.visible && sp.texture && sp.texture->valid())
            entries.push_back({sp.layer, &sp, &xf});
    }

    std::stable_sort(entries.begin(), entries.end(),
        [](const DrawEntry& a, const DrawEntry& b) { return a.layer < b.layer; });

    // 3. Submit each sprite to the batcher.
    for (const auto& d : entries) {
        const Sprite&    sp  = *d.sp;
        const Transform& xf  = *d.xf;
        const Texture&   tex = *sp.texture;

        const bool has_src = sp.src.w > 0.f && sp.src.h > 0.f;
        const float sw = (has_src ? sp.src.w : static_cast<float>(tex.width()))  * xf.scale.x;
        const float sh = (has_src ? sp.src.h : static_cast<float>(tex.height())) * xf.scale.y;

        // Shift top-left so that Transform::position lands at the pivot (origin).
        const Rect dest = {
            xf.position.x - sp.origin.x * sw,
            xf.position.y - sp.origin.y * sh,
            sw, sh
        };

        r.draw_textured_quad(tex, dest, sp.src, sp.tint);
    }

    // TilemapRef rendering deferred to phase 13.
}

void Scene::clear() {
    registry_.clear();
}

} // namespace glyph
