// tilemap.cpp — Tilemap::load() and query helpers.
#include <glyph/tilemap.h>

#include <nlohmann/json.hpp>
#include <stb_image.h>

#include <cstdio>
#include <cstdlib>   // std::stoi / std::stof
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace glyph {

// ---------------------------------------------------------------------------
// TiledProperty helpers
// ---------------------------------------------------------------------------

int   TiledProperty::as_int()   const { return std::stoi(value); }
float TiledProperty::as_float() const { return std::stof(value); }
bool  TiledProperty::as_bool()  const { return value == "true"; }

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<Texture> load_texture(const std::filesystem::path& path) {
    int w, h, ch;
    uint8_t* px = stbi_load(path.string().c_str(), &w, &h, &ch, 4);
    if (!px) {
        std::fprintf(stderr, "[glyph] Tilemap: cannot load image %s\n",
                     path.string().c_str());
        return nullptr;
    }
    auto tex = std::make_shared<Texture>();
    if (!tex->create(w, h, px)) {
        stbi_image_free(px);
        std::fprintf(stderr, "[glyph] Tilemap: texture upload failed for %s\n",
                     path.string().c_str());
        return nullptr;
    }
    stbi_image_free(px);
    return tex;
}

// Parse an embedded or external tileset JSON object into a Tileset.
// base_dir is the directory of the TMJ (or TSJ) file.
static Tileset parse_tileset_data(const nlohmann::json& j,
                                  const std::filesystem::path& base_dir) {
    Tileset ts;
    ts.tile_w    = j.value("tilewidth",  16);
    ts.tile_h    = j.value("tileheight", 16);
    ts.columns   = j.value("columns",    1);
    ts.tile_count = j.value("tilecount", 0);

    if (j.contains("image")) {
        const std::string img = j["image"].get<std::string>();
        ts.texture = load_texture(base_dir / img);
    }
    return ts;
}

// Parse Tiled property array into a map.
static std::unordered_map<std::string, TiledProperty>
parse_properties(const nlohmann::json& arr) {
    std::unordered_map<std::string, TiledProperty> out;
    if (!arr.is_array()) return out;
    for (const auto& p : arr) {
        TiledProperty tp;
        const auto& v = p["value"];
        if (v.is_string())          tp.value = v.get<std::string>();
        else if (v.is_boolean())    tp.value = v.get<bool>() ? "true" : "false";
        else if (v.is_number_integer()) tp.value = std::to_string(v.get<int>());
        else if (v.is_number_float())   tp.value = std::to_string(v.get<double>());
        out[p["name"].get<std::string>()] = std::move(tp);
    }
    return out;
}

// ---------------------------------------------------------------------------
// Tilemap::load
// ---------------------------------------------------------------------------

std::shared_ptr<Tilemap> Tilemap::load(const std::string& tmj_path) {
    std::ifstream f(tmj_path);
    if (!f.is_open()) {
        std::fprintf(stderr, "[glyph] Tilemap: cannot open %s\n", tmj_path.c_str());
        return nullptr;
    }

    nlohmann::json j;
    try { f >> j; }
    catch (const std::exception& e) {
        std::fprintf(stderr, "[glyph] Tilemap: JSON parse error in %s: %s\n",
                     tmj_path.c_str(), e.what());
        return nullptr;
    }

    const auto base_dir = std::filesystem::path(tmj_path).parent_path();
    auto map = std::make_shared<Tilemap>();

    map->map_w  = j.value("width",      0);
    map->map_h  = j.value("height",     0);
    map->tile_w = j.value("tilewidth",  16);
    map->tile_h = j.value("tileheight", 16);

    // --- Tilesets ---
    for (const auto& ts_entry : j.value("tilesets", nlohmann::json::array())) {
        Tileset ts;

        if (ts_entry.contains("source")) {
            // External tileset — load .tsj sidecar.
            const auto tsj_path = base_dir / ts_entry["source"].get<std::string>();
            std::ifstream tsj_f(tsj_path);
            if (!tsj_f.is_open()) {
                std::fprintf(stderr, "[glyph] Tilemap: cannot open tileset %s\n",
                             tsj_path.string().c_str());
                continue;
            }
            nlohmann::json tsj;
            try { tsj_f >> tsj; }
            catch (...) {
                std::fprintf(stderr, "[glyph] Tilemap: bad JSON in tileset %s\n",
                             tsj_path.string().c_str());
                continue;
            }
            ts = parse_tileset_data(tsj, tsj_path.parent_path());
        } else {
            ts = parse_tileset_data(ts_entry, base_dir);
        }

        ts.first_gid = ts_entry.value("firstgid", 1);
        map->tilesets.push_back(std::move(ts));
    }

    // --- Layers ---
    for (const auto& layer : j.value("layers", nlohmann::json::array())) {
        const std::string layer_type = layer.value("type", "");

        if (layer_type == "tilelayer") {
            TileLayer tl;
            tl.name    = layer.value("name", "");
            tl.size    = { layer.value("width", 0), layer.value("height", 0) };
            tl.visible = layer.value("visible", true);
            tl.opacity = layer.value("opacity", 1.f);

            const auto& data = layer["data"];
            if (!data.is_array()) {
                std::fprintf(stderr,
                    "[glyph] Tilemap: layer '%s' uses unsupported encoding "
                    "(only plain integer arrays are supported)\n",
                    tl.name.c_str());
                continue;
            }
            tl.gids.reserve(data.size());
            for (const auto& v : data)
                tl.gids.push_back(v.get<uint32_t>());

            map->tile_layers.push_back(std::move(tl));

        } else if (layer_type == "objectgroup") {
            ObjectLayer ol;
            ol.name = layer.value("name", "");

            for (const auto& obj : layer.value("objects", nlohmann::json::array())) {
                TiledObject to;
                to.id     = obj.value("id",   0);
                to.name   = obj.value("name", "");
                // Tiled <1.9 uses "type", ≥1.9 uses "class"; check both.
                to.type   = obj.contains("class") ? obj["class"].get<std::string>()
                                                   : obj.value("type", "");
                to.bounds = { obj.value("x", 0.f), obj.value("y", 0.f),
                              obj.value("width", 0.f), obj.value("height", 0.f) };

                if (obj.contains("properties"))
                    to.properties = parse_properties(obj["properties"]);

                ol.objects.push_back(std::move(to));
            }
            map->object_layers.push_back(std::move(ol));
        }
        // Other layer types (image, group) are silently ignored.
    }

    return map;
}

// ---------------------------------------------------------------------------
// Query helpers
// ---------------------------------------------------------------------------

Rect Tilemap::tile_uv(uint32_t gid) const {
    const uint32_t clean = gid & 0x1FFFFFFFu;  // strip flip bits
    if (clean == 0) return {};

    const Tileset* ts = nullptr;
    for (const auto& t : tilesets) {
        if (t.first_gid <= static_cast<int>(clean) &&
            (!ts || t.first_gid > ts->first_gid))
            ts = &t;
    }
    if (!ts || ts->columns == 0) return {};

    const uint32_t lid = clean - static_cast<uint32_t>(ts->first_gid);
    return {
        static_cast<float>(static_cast<int>(lid % static_cast<uint32_t>(ts->columns)) * ts->tile_w),
        static_cast<float>(static_cast<int>(lid / static_cast<uint32_t>(ts->columns)) * ts->tile_h),
        static_cast<float>(ts->tile_w),
        static_cast<float>(ts->tile_h)
    };
}

Tileset* Tilemap::tileset_for(uint32_t gid) {
    const uint32_t clean = gid & 0x1FFFFFFFu;
    Tileset* result = nullptr;
    for (auto& ts : tilesets) {
        if (ts.first_gid <= static_cast<int>(clean) &&
            (!result || ts.first_gid > result->first_gid))
            result = &ts;
    }
    return result;
}

} // namespace glyph
