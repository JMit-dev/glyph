# Tilemaps

**Header:** `#include <glyph/tilemap.h>`

Loads [Tiled](https://www.mapeditor.org/) `.tmj` maps. Supports tile layers, object layers, embedded and external (`.tsj`) tilesets.

!!! note "Limitations"
    Only plain-integer tile data arrays are supported. Base64 / zlib-compressed tile data is not yet implemented. Tile flip flags (bits 29–31 of a GID) are silently stripped.

---

## Types

### TiledProperty

```cpp
struct TiledProperty {
    std::string value;   // always stored as string

    int   as_int()   const;
    float as_float() const;
    bool  as_bool()  const;
};
```

### TiledObject

```cpp
struct TiledObject {
    int         id;
    std::string name;
    std::string type;    // "class" in Tiled ≥1.9, "type" in older versions
    Rect        bounds;
    std::unordered_map<std::string, TiledProperty> properties;
};
```

### TileLayer / ObjectLayer

```cpp
struct TileLayer {
    std::string           name;
    ivec2                 size;       // width × height in tiles
    std::vector<uint32_t> gids;       // row-major; 0 = empty tile
    bool                  visible  = true;
    float                 opacity  = 1.f;
};

struct ObjectLayer {
    std::string              name;
    std::vector<TiledObject> objects;
};
```

### Tileset

```cpp
struct Tileset {
    std::shared_ptr<Texture> texture;
    int tile_w, tile_h;
    int columns;
    int first_gid;
    int tile_count;
};
```

---

## Tilemap

```cpp
class Tilemap {
public:
    int map_w, map_h;    // map size in tiles
    int tile_w, tile_h;  // tile size in pixels

    std::vector<Tileset>     tilesets;
    std::vector<TileLayer>   tile_layers;
    std::vector<ObjectLayer> object_layers;

    static std::shared_ptr<Tilemap> load(const std::string& tmj_path);

    Rect     tile_uv(uint32_t gid) const;   // pixel source rect for a GID
    Tileset* tileset_for(uint32_t gid);      // owning tileset for a GID
};
```

`load()` resolves tileset images and external `.tsj` files relative to the TMJ path. Returns `nullptr` on failure.

---

## Usage

```cpp
void on_start() override {
    auto map = glyph::Tilemap::load("assets/map.tmj");

    auto e = scene().create_entity("map");
    e.add<glyph::Transform>();   // position = world offset of the map
    e.add<glyph::TilemapRef>(glyph::TilemapRef{map});
}
```

`Scene::render()` draws all tile layers automatically before sprites. The entity's `Transform::position` offsets the entire map in world space.

---

## Tiled export settings

- **File → Export Sprite Sheet**: use the TMJ (JSON) format, not XML.
- Tileset images must be placed in the same directory as the `.tmj` (or the same directory as the `.tsj` for external tilesets).
- Tile layer encoding: leave at **CSV** (plain integer array). Base64/zlib is not supported.
