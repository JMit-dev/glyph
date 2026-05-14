// components.h — built-in ECS component structs.
//
// All components are plain data structs. Systems in scene/systems.cpp read and
// write them; game code can also read/write them directly via Entity::get<T>().
//
// Forward-declared types (SpriteSheet, Tilemap) are complete in their own
// headers which are included once those phases are built.
#pragma once

#include <glyph/camera.h>
#include <glyph/math.h>
#include <glyph/scene.h>    // for Entity type used in Transform::parent
#include <glyph/texture.h>

#include <any>
#include <cstdint>
#include <memory>
#include <string>

namespace glyph {

class SpriteSheet;   // phase 12 — include sprite_sheet.h for full definition
class Tilemap;       // phase 13 — include tilemap.h for full definition

// ---------------------------------------------------------------------------

struct Name {
    std::string value;
};

struct Transform {
    vec2   position {0.f, 0.f};
    float  rotation = 0.f;         // radians, clockwise positive (y-down)
    vec2   scale    {1.f, 1.f};
    Entity parent;                  // invalid entity = no parent
};

struct Sprite {
    std::shared_ptr<Texture> texture;
    Rect  src    {};                // source rect in pixels; {0,0,0,0} = whole texture
    Color tint   {1, 1, 1, 1};
    int   layer  = 0;              // lower drawn first; same-layer order is insertion
    vec2  origin {0.5f, 0.5f};    // pivot point in [0,1] — 0.5 = centre
    bool  visible = true;
};

struct Animator {
    std::shared_ptr<SpriteSheet> sheet;
    std::string clip;
    float time    = 0.f;
    bool  playing = true;
    bool  loop    = true;
};

struct Velocity {
    vec2 value;                    // world units per second
};

struct BoxCollider {
    Rect     bounds;               // axis-aligned, in local space relative to Transform
    uint32_t layer = 1;
    uint32_t mask  = 0xFFFFFFFF;   // which layers this collider interacts with
    bool     is_trigger = false;   // triggers fire events but don't resolve physics
};

struct TilemapRef {
    std::shared_ptr<Tilemap> map;
};

struct Camera2D {
    Camera value;
    bool   primary = true;         // the first primary camera found is used for rendering
};

struct Script {
    std::string lua_module;   // module name under scripts/entities/
    std::any    self;         // holds sol::table; set by ScriptSystem on first update
};

struct Lifetime {
    float seconds;                 // entity is destroyed when this reaches <= 0
};

} // namespace glyph
