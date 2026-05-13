// 01_hello_window — verifies window, GL context, shader, and textured quad.
// Draws a 4x4 black-and-white checkerboard scaled to 400x300 pixels.
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
        std::printf("[hello_window] window opened — close it to exit\n");

        // Build a 4×4 black-and-white checkerboard in memory.
        uint8_t pixels[4 * 4 * 4];
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                const uint8_t v = ((x + y) % 2 == 0) ? 255 : 0;
                const int i = (y * 4 + x) * 4;
                pixels[i] = v; pixels[i + 1] = v; pixels[i + 2] = v; pixels[i + 3] = 255;
            }
        }
        checkerboard_.create(4, 4, pixels);
    }

    void on_update(float dt) override {
        elapsed_ += dt;
    }

    void on_render(glyph::Renderer& r) override {
        r.clear(glyph::Color::rgba8(100, 149, 237));   // cornflower blue background
        r.draw_textured_quad(checkerboard_, {200.f, 150.f, 400.f, 300.f});
    }

    void on_shutdown() override {
        checkerboard_.destroy();
        std::printf("[hello_window] shutting down after %.2f seconds\n", elapsed_);
    }

private:
    glyph::Texture checkerboard_;
    float          elapsed_ = 0.0f;
};

GLYPH_MAIN(HelloWindow)
