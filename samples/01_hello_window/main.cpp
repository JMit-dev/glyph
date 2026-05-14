// 01_hello_window — verifies window, GL context, shader, and sprite batcher.
//
// Draws a 40×30 grid of 20×20 pixel tiles alternating between two procedural
// textures (checkerboard and solid colour). Both textures are batched into a
// single flush (1200 quads < kMaxQuads=2000, 2 textures < kMaxTextures=8).
#include <glyph/glyph.h>

#include <cstdint>
#include <cstdio>

class HelloWindow : public glyph::Game {
public:
    glyph::AppConfig configure() override {
        glyph::AppConfig c;
        c.title  = "Glyph: Hello Window";
        c.width  = 800;
        c.height = 600;
        return c;
    }

    void on_start() override {
        std::printf("[hello_window] window open — close to exit\n");

        // Texture A: 4×4 black-and-white checkerboard
        uint8_t checker[4 * 4 * 4];
        for (int y = 0; y < 4; ++y)
            for (int x = 0; x < 4; ++x) {
                const uint8_t v = ((x + y) % 2 == 0) ? 255 : 0;
                const int i = (y * 4 + x) * 4;
                checker[i] = v;  checker[i+1] = v;  checker[i+2] = v;  checker[i+3] = 255;
            }
        tex_checker_.create(4, 4, checker);

        // Texture B: 1×1 solid white (tinted at draw time)
        uint8_t white[4] = {255, 255, 255, 255};
        tex_solid_.create(1, 1, white);
    }

    void on_update(float dt) override { elapsed_ += dt; }

    void on_render(glyph::Renderer& r) override {
        r.clear(glyph::Color::rgba8(30, 30, 40));

        // 40 columns × 30 rows = 1200 quads, 2 textures — one batch flush.
        constexpr int   cols = 40, rows = 30, size = 20;
        constexpr float purple_r = 0.55f, purple_g = 0.27f, purple_b = 0.90f;

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                const float x = static_cast<float>(col * size);
                const float y = static_cast<float>(row * size);
                if ((col + row) % 2 == 0) {
                    r.draw_textured_quad(tex_checker_, {x, y, size, size});
                } else {
                    r.draw_textured_quad(tex_solid_,   {x, y, size, size},
                                         {purple_r, purple_g, purple_b, 1.f});
                }
            }
        }
    }

    void on_shutdown() override {
        tex_checker_.destroy();
        tex_solid_.destroy();
        std::printf("[hello_window] shut down after %.2f s\n", elapsed_);
    }

private:
    glyph::Texture tex_checker_;
    glyph::Texture tex_solid_;
    float          elapsed_ = 0.f;
};

GLYPH_MAIN(HelloWindow)
