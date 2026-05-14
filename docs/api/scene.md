# Scene & Entity

**Headers:** `#include <glyph/scene.h>` · `#include <glyph/components.h>`

The ECS layer. [EnTT](https://github.com/skypjack/entt) drives the registry internally; `Entity` and `Scene` provide a friendlier façade. Access the scene via `scene()` inside any `Game` lifecycle hook.

---

## Entity

A value-typed handle: a `Scene*` plus an `entt::entity` id. Pass by value. Default-constructed entity is invalid.

```cpp
glyph::Entity e = scene().create_entity("player");
e.add<glyph::Transform>(glyph::Transform{{400, 300}});
e.add<glyph::Sprite>(glyph::Sprite{res_.texture("player.png")});

// Retrieve
auto& t = e.get<glyph::Transform>();
t.position.x += 100.f;

// Query
if (e.has<glyph::Velocity>()) { /* ... */ }

// Optional (returns nullptr if missing)
if (auto* v = e.try_get<glyph::Velocity>()) v->value.x = 50.f;

// Remove
e.remove<glyph::Velocity>();

// Destroy
e.destroy();
```

### Methods

| Method | Description |
|---|---|
| `valid() → bool` | True if the entity exists in the scene |
| `destroy()` | Remove the entity and all its components |
| `add<T>(args...)` | Attach component T (returns `T&`) |
| `get<T>()` | Get component T (UB if missing — use `has` first) |
| `try_get<T>()` | Get component T or nullptr |
| `has<T>()` | True if the entity has component T |
| `remove<T>()` | Detach component T |

---

## Scene

```cpp
void on_start() override {
    auto e = scene().create_entity("camera");
    e.add<glyph::Camera2D>();

    auto player = scene().create_entity("player");
    player.add<glyph::Transform>(glyph::Transform{{400, 300}});
    player.add<glyph::Velocity>();
}

void on_update(float dt) override {
    // Iterate all entities with Velocity and Transform
    scene().each<glyph::Velocity, glyph::Transform>(
        [](glyph::Entity e, glyph::Velocity& vel, glyph::Transform& xf) {
            xf.position += vel.value * dt;   // manual movement for now
        });
}
```

### Methods

| Method | Description |
|---|---|
| `create_entity()` | Create an anonymous entity |
| `create_entity(name)` | Create entity with a `Name` component |
| `find(name)` | First entity with matching `Name`; invalid if not found |
| `each<Comps...>(fn)` | Iterate entities with all listed components |
| `run_systems(dt)` | Run built-in systems (Lifetime, Movement, Collision, Animator) |
| `on_collision(fn)` | Register a `void(Entity, Entity)` callback fired for each overlapping `BoxCollider` pair |
| `render(r)` | Find primary Camera2D, sort sprites by layer, submit to batcher |
| `clear()` | Destroy all entities |
| `registry()` | Raw `entt::registry&` escape hatch |

---

## Built-in components (`components.h`)

| Struct | Fields | Notes |
|---|---|---|
| `Name` | `value: string` | Used by `scene().find()` |
| `Transform` | `position`, `rotation`, `scale`, `parent` | Y-down, radians CW |
| `Sprite` | `texture`, `src`, `tint`, `layer`, `origin`, `visible` | Rendered by `Scene::render()` |
| `Velocity` | `value: vec2` | World units per second; applied by `MovementSystem` each frame |
| `Animator` | `sheet`, `clip`, `time`, `playing`, `loop` | Driven by `AnimatorSystem` (phase 12) |
| `BoxCollider` | `bounds`, `layer`, `mask`, `is_trigger` | Used by CollisionSystem (phase 14) |
| `Camera2D` | `value: Camera`, `primary: bool` | First primary used for rendering |
| `TilemapRef` | `map: shared_ptr<Tilemap>` | Rendered by Scene (phase 13) |
| `Script` | `lua_module: string` | Lua entity script; driven by `ScriptSystem` (phase 17) |
| `Lifetime` | `seconds: float` | Destroyed when ≤ 0 by `LifetimeSystem` each frame |

---

---

## Collision detection

`BoxCollider` defines an axis-aligned bounding box in local space. `CollisionSystem` runs each frame via `Scene::run_systems()` and fires the registered callback for every overlapping pair that passes the layer/mask filter.

```cpp
void on_start() override {
    scene().on_collision([](glyph::Entity a, glyph::Entity b) {
        // called once per overlapping pair per frame
    });

    auto bullet = scene().create_entity("bullet");
    auto& col = bullet.add<glyph::BoxCollider>();
    col.bounds    = {-4, -4, 8, 8};   // local-space AABB, centred on origin
    col.layer     = 0x02;              // bullet layer
    col.mask      = 0x01;              // detects enemy layer
    col.is_trigger = true;             // fire event, no physics resolution
}
```

| Field | Meaning |
|---|---|
| `bounds` | Local-space AABB relative to `Transform::position` |
| `layer` | Bitmask — which layer this collider is on |
| `mask` | Bitmask — which layers this collider detects |
| `is_trigger` | Fires events but does not push entities apart |

A collision is reported when `(a.layer & b.mask) || (b.layer & a.mask)` and the world-space AABBs overlap.

---

## Transform hierarchy

Set `Transform::parent` to another entity to form a parent-child relationship. `TransformPropagationSystem` is stubbed for now — correct implementation requires a separate world-transform store to avoid accumulating parent offsets each frame. `Transform::position` is always world-space until then. A null/invalid `parent` means no parent.

---

## EnTT escape hatch

For advanced use, the raw registry is accessible:

```cpp
auto& reg = scene().registry();
auto view = reg.view<glyph::Transform, glyph::Sprite>();
```
