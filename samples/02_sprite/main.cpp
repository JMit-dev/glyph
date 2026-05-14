// 02_sprite — demonstrates Resources::texture() loading a PNG from disk.
//
// Generates an 8×8 checkerboard PNG into the working directory on first run,
// loads it through Resources (verifying FileIO + stb_image), then renders it
// tiled across the screen using the sprite batcher and an animated camera.
//
// Run from the repo root or the binary's directory so the generated
// "assets/sprite.png" lands somewhere predictable.

// stb_image_write — only used in this sample to generate the test PNG.
// The engine's stb_impl.cpp defines STB_IMAGE_IMPLEMENTATION separately.
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <glyph/glyph.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>

static void write_test_png(const char* path) {
    // 16×16 four-colour checkerboard at 4×4 cell size
    constexpr int W = 16, H = 16;
    uint8_t pixels[W * H * 4];
    const uint8_t palette[4][4] = {
        {220, 80,  80,  255},   // red
        {80,  180, 80,  255},   // green
        {80,  120, 220, 255},   // blue
        {220, 200, 60,  255},   // yellow
    };
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            const int cell  = (x / 4) + (y / 4) * (W / 4);
            const int i     = (y * W + x) * 4;
            const auto& c   = palette[cell % 4];
            pixels[i]   = c[0]; pixels[i+1] = c[1];
            pixels[i+2] = c[2]; pixels[i+3] = c[3];
        }
    stbi_write_png(path, W, H, 4, pixels, W * 4);
}

class SpriteDemo : public glyph::Game {
public:
    glyph::AppConfig configure() override {
        glyph::AppConfig c;
        c.title = "Glyph: 02 Sprite";
        c.width = 800; c.height = 600;
        return c;
    }

    void on_start() override {
        const char* asset_path = "assets/sprite.png";

        // Generate the test PNG if it does not exist.
        std::filesystem::create_directories("assets");
        if (!std::filesystem::exists(asset_path)) {
            write_test_png(asset_path);
            std::printf("[02_sprite] generated %s\n", asset_path);
        }

        resources_.set_root("assets");
        sprite_ = resources_.texture("sprite.png");

        if (sprite_) {
            std::printf("[02_sprite] loaded %dx%d texture\n",
                        sprite_->width(), sprite_->height());
        } else {
            std::fprintf(stderr, "[02_sprite] ERROR: texture load failed\n");
        }
    }

    void on_update(float dt) override { elapsed_ += dt; }

    void on_render(glyph::Renderer& r) override {
        // Gentle camera drift to show the sprite renders correctly in world space.
        r.camera().position.x = std::sin(elapsed_ * 0.3f) * 40.f;
        r.camera().position.y = std::cos(elapsed_ * 0.2f) * 30.f;
        r.camera().zoom       = 1.f + std::sin(elapsed_ * 0.5f) * 0.15f;

        r.clear(glyph::Color::rgba8(25, 25, 35));

        if (!sprite_) return;

        // Tile the sprite across a 800×600 region in 64×64 pixel quads.
        constexpr float size = 64.f;
        for (float y = 0.f; y < 600.f; y += size)
            for (float x = 0.f; x < 800.f; x += size)
                r.draw_textured_quad(*sprite_, {x, y, size, size});
    }

    void on_shutdown() override {
        sprite_.reset();   // shared_ptr release; Texture destructor frees GL handle
        resources_.unload_unused();
    }

private:
    glyph::Resources             resources_;
    std::shared_ptr<glyph::Texture> sprite_;
    float                        elapsed_ = 0.f;
};

GLYPH_MAIN(SpriteDemo)
