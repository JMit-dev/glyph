// 01_hello_window — verifies window, GL context, sprite batcher, and camera.
//
// Draws a 40×30 grid of 20×20 world-pixel tiles (1200 quads, 2 textures,
// one batch flush). The camera slowly pans and oscillates zoom over time,
// demonstrating that view_projection() and the ortho projection are correct.
// Mouse pan / scroll-wheel zoom wired up properly in phase 7 (Input system).
#include <glyph/glyph.h>

#include <cmath>
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

        // Texture A: 4×4 checkerboard
        uint8_t checker[4 * 4 * 4];
        for (int y = 0; y < 4; ++y)
            for (int x = 0; x < 4; ++x) {
                const uint8_t v = ((x + y) % 2 == 0) ? 255 : 0;
                const int i = (y * 4 + x) * 4;
                checker[i] = v; checker[i+1] = v; checker[i+2] = v; checker[i+3] = 255;
            }
        tex_checker_.create(4, 4, checker);

        // Texture B: 1×1 solid white (tinted purple at draw time)
        uint8_t white[4] = {255, 255, 255, 255};
        tex_solid_.create(1, 1, white);
    }

    void on_update(float dt) override { elapsed_ += dt; }

    void on_render(glyph::Renderer& r) override {
        // Animate the camera: slow pan + zoom oscillation.
        // Mouse pan and scroll-wheel zoom arrive in phase 7.
        glyph::Camera& cam = r.camera();
        cam.position.x = std::sin(elapsed_ * 0.4f) * 120.f;
        cam.position.y = std::cos(elapsed_ * 0.3f) *  80.f;
        cam.zoom       = 1.f + std::sin(elapsed_ * 0.6f) * 0.4f;

        r.clear(glyph::Color::rgba8(30, 30, 40));

        // 40 × 30 = 1200 quads, 2 textures — fits in one batch flush.
        constexpr int   cols = 40, rows = 30, size = 20;
        for (int row = 0; row < rows; ++row)
            for (int col = 0; col < cols; ++col) {
                const float x = static_cast<float>(col * size);
                const float y = static_cast<float>(row * size);
                if ((col + row) % 2 == 0) {
                    r.draw_textured_quad(tex_checker_, {x, y, size, size});
                } else {
                    r.draw_textured_quad(tex_solid_, {x, y, size, size},
                                         glyph::Color::rgba8(140, 70, 230));
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
