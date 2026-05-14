// 04_tilemap — demonstrates Tilemap::load() and Scene tilemap rendering.
//
// Generates a 4-tile tileset PNG and a TMJ map file at startup (assets/),
// then loads and renders the map via the ECS TilemapRef component.
//
// Controls:
//   WASD / arrow keys  — scroll the map
//   Scroll wheel       — zoom in / out
//   Escape             — quit

// stb_image_write — only used here to generate the test tileset PNG.
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <glyph/glyph.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>

// ---------------------------------------------------------------------------
// Asset generation helpers
// ---------------------------------------------------------------------------

// Generate a 64×16 tileset PNG: 4 tiles of 16×16 pixels each.
//   Tile 1 (GID 1): grass  — green
//   Tile 2 (GID 2): dirt   — brown
//   Tile 3 (GID 3): stone  — grey
//   Tile 4 (GID 4): water  — blue
static void write_tileset_png(const char* path) {
    constexpr int TW = 16, TH = 16, TILES = 4;
    constexpr int W = TW * TILES, H = TH;
    uint8_t pixels[W * H * 4];

    const uint8_t colours[TILES][4] = {
        { 80, 180,  80, 255},   // grass
        {160, 120,  80, 255},   // dirt
        {140, 140, 140, 255},   // stone
        { 60, 120, 200, 255},   // water
    };

    for (int t = 0; t < TILES; ++t) {
        for (int y = 0; y < TH; ++y) {
            for (int x = 0; x < TW; ++x) {
                // Slight shading: darken edges by 20 points to give depth.
                const bool edge = (x == 0 || y == 0 || x == TW-1 || y == TH-1);
                const int px = (t * TW + x) + y * W;
                for (int c = 0; c < 3; ++c)
                    pixels[px * 4 + c] = edge
                        ? static_cast<uint8_t>(colours[t][c] > 20 ? colours[t][c] - 20 : 0)
                        : colours[t][c];
                pixels[px * 4 + 3] = 255;
            }
        }
    }
    stbi_write_png(path, W, H, 4, pixels, W * 4);
}

// Write a minimal Tiled TMJ file (inline tileset, one tile layer).
static void write_map_tmj(const char* path) {
    constexpr int MAP_W = 30, MAP_H = 20;

    std::ostringstream data;
    for (int row = 0; row < MAP_H; ++row) {
        for (int col = 0; col < MAP_W; ++col) {
            // Simple pattern: row 0-4=grass, 5-9=dirt, etc.; water border
            int tile;
            const bool border = (row == 0 || col == 0 || row == MAP_H-1 || col == MAP_W-1);
            if (border)                         tile = 4;  // water
            else if ((row + col) % 7 == 0)     tile = 3;  // stone
            else if (row < MAP_H / 2)          tile = 1;  // grass
            else                               tile = 2;  // dirt

            data << tile;
            if (row < MAP_H-1 || col < MAP_W-1) data << ',';
        }
    }

    std::ofstream f(path);
    f << "{\n"
      << "  \"width\":"      << MAP_W << ",\n"
      << "  \"height\":"     << MAP_H << ",\n"
      << "  \"tilewidth\":16,\n"
      << "  \"tileheight\":16,\n"
      << "  \"tilesets\":[{\n"
      << "    \"firstgid\":1,\n"
      << "    \"name\":\"tiles\",\n"
      << "    \"image\":\"tiles.png\",\n"
      << "    \"imagewidth\":64,\n"
      << "    \"imageheight\":16,\n"
      << "    \"tilewidth\":16,\n"
      << "    \"tileheight\":16,\n"
      << "    \"columns\":4,\n"
      << "    \"tilecount\":4\n"
      << "  }],\n"
      << "  \"layers\":[{\n"
      << "    \"type\":\"tilelayer\",\n"
      << "    \"name\":\"Ground\",\n"
      << "    \"width\":"  << MAP_W << ",\n"
      << "    \"height\":" << MAP_H << ",\n"
      << "    \"visible\":true,\n"
      << "    \"opacity\":1.0,\n"
      << "    \"data\":[" << data.str() << "]\n"
      << "  }]\n"
      << "}\n";
}

// ---------------------------------------------------------------------------
// Game
// ---------------------------------------------------------------------------

class TilemapDemo : public glyph::Game {
public:
    glyph::AppConfig configure() override {
        glyph::AppConfig c;
        c.title  = "Glyph: 04 Tilemap";
        c.width  = 800;
        c.height = 600;
        return c;
    }

    void on_start() override {
        std::printf("[04_tilemap] WASD/arrows=scroll  scroll-wheel=zoom  Esc=quit\n");

        // Generate assets on first run.
        std::filesystem::create_directories("assets");
        if (!std::filesystem::exists("assets/tiles.png"))
            write_tileset_png("assets/tiles.png");
        if (!std::filesystem::exists("assets/map.tmj"))
            write_map_tmj("assets/map.tmj");

        // Load the tilemap.
        auto map = glyph::Tilemap::load("assets/map.tmj");
        if (!map) {
            std::fprintf(stderr, "[04_tilemap] Failed to load map!\n");
            return;
        }
        std::printf("[04_tilemap] Map loaded: %dx%d tiles, %d tile layer(s)\n",
                    map->map_w, map->map_h,
                    static_cast<int>(map->tile_layers.size()));

        // Map entity — position (0,0) puts the top-left tile at the world origin.
        auto map_e = scene().create_entity("map");
        map_e.add<glyph::Transform>();
        map_e.add<glyph::TilemapRef>(glyph::TilemapRef{map});

        // Camera entity.
        auto cam_e = scene().create_entity("camera");
        auto& cam2d = cam_e.add<glyph::Camera2D>();
        cam2d.value.position = {
            static_cast<float>(map->map_w * map->tile_w) * 0.5f,
            static_cast<float>(map->map_h * map->tile_h) * 0.5f
        };
        cam2d.primary = true;
    }

    void on_update(float dt) override {
        // Scroll camera via WASD / arrows.
        constexpr float kSpeed = 200.f;
        auto cam_e = scene().find("camera");
        if (!cam_e.valid()) return;

        auto& cam2d = cam_e.get<glyph::Camera2D>();
        const float ax = input().axis_value("move_x");
        const float ay = input().axis_value("move_y");
        cam2d.value.position.x += ax * kSpeed * dt;
        cam2d.value.position.y += ay * kSpeed * dt;

        // Zoom with scroll wheel.
        const float wheel = input().mouse_wheel().y;
        if (wheel != 0.f) {
            cam2d.value.zoom *= (wheel > 0.f ? 1.1f : 1.f / 1.1f);
            cam2d.value.zoom  = std::max(0.1f, std::min(cam2d.value.zoom, 10.f));
        }
    }
};

GLYPH_MAIN(TilemapDemo)
