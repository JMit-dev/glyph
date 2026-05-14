// tilemap.h — Tiled TMJ tilemap loader and data types.
//
// Supports a practical subset of the Tiled TMJ format:
//   - Tile layers with plain-integer data arrays
//   - Object layers with typed properties
//   - Embedded and external (.tsj) tilesets
//   - Horizontal/vertical flip bits in GIDs are silently stripped
//
// Base64 / zlib-compressed tile data is not supported.
#pragma once

#include <glyph/math.h>
#include <glyph/texture.h>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace glyph {

// ---------------------------------------------------------------------------

struct TiledProperty {
    std::string value;  // always stored as string; parse with as_int/float/bool

    int   as_int()   const;
    float as_float() const;
    bool  as_bool()  const;
};

struct TiledObject {
    int         id   = 0;
    std::string name;
    std::string type;   // object class / type field (Tiled ≥1.9: "class")
    Rect        bounds;
    std::unordered_map<std::string, TiledProperty> properties;
};

struct ObjectLayer {
    std::string             name;
    std::vector<TiledObject> objects;
};

struct TileLayer {
    std::string          name;
    ivec2                size;       // width × height in tiles
    std::vector<uint32_t> gids;     // row-major; 0 = empty tile
    bool                 visible  = true;
    float                opacity  = 1.f;
};

struct Tileset {
    std::shared_ptr<Texture> texture;
    int tile_w     = 0;
    int tile_h     = 0;
    int columns    = 0;
    int first_gid  = 1;
    int tile_count = 0;
};

// ---------------------------------------------------------------------------

class Tilemap {
public:
    int map_w  = 0, map_h  = 0;   // map dimensions in tiles
    int tile_w = 0, tile_h = 0;   // tile dimensions in pixels

    std::vector<Tileset>     tilesets;
    std::vector<TileLayer>   tile_layers;
    std::vector<ObjectLayer> object_layers;

    // Load a Tiled .tmj file. Companion assets are resolved relative to tmj_path.
    // Returns nullptr on failure.
    static std::shared_ptr<Tilemap> load(const std::string& tmj_path);

    // Source rect in pixels for a given global tile ID.
    // Returns {} (zero rect) for GID 0 or unknown.
    Rect tile_uv(uint32_t gid) const;

    // Tileset that owns a given global tile ID, or nullptr.
    Tileset* tileset_for(uint32_t gid);
};

} // namespace glyph
