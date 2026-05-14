// 03_input — demonstrates keyboard movement, mouse pan, and scroll-wheel zoom.
//
// Controls:
//   WASD / arrow keys  — move the orange sprite
//   Right-mouse drag   — pan the camera
//   Scroll wheel       — zoom in/out
//   Escape             — quit
#include <glyph/glyph.h>

#include <algorithm>  // std::clamp
#include <cmath>
#include <cstdint>
#include <cstdio>

class InputDemo : public glyph::Game {
public:
    glyph::AppConfig configure() override {
        glyph::AppConfig c;
        c.title = "Glyph: 03 Input";
        c.width = 800; c.height = 600;
        return c;
    }

    void on_start() override {
        std::printf("[03_input] WASD/arrows=move  RMB-drag=pan  scroll=zoom  Esc=quit\n");

        // 16×16 orange sprite
        uint8_t sprite_px[16 * 16 * 4];
        for (int y = 0; y < 16; ++y)
            for (int x = 0; x < 16; ++x) {
                const int i = (y * 16 + x) * 4;
                // orange body with a dark border
                bool border = (x == 0 || x == 15 || y == 0 || y == 15);
                sprite_px[i+0] = border ? 180 : 255;
                sprite_px[i+1] = border ?  60 : 150;
                sprite_px[i+2] = border ?   0 :  20;
                sprite_px[i+3] = 255;
            }
        sprite_tex_.create(16, 16, sprite_px);

        // 1×1 white for drawing the world-origin crosshair
        uint8_t white[4] = {255, 255, 255, 255};
        white_tex_.create(1, 1, white);
    }

    void on_update(float dt) override {
        if (input().action_pressed("cancel")) {
            // Signal close via a flag; the window close is handled in the engine.
            std::printf("[03_input] Escape pressed — closing\n");
            // No direct close API yet; user closes the window.
        }

        // Move sprite — both WASD and arrow keys bound by default.
        constexpr float kSpeed = 200.f;
        float vx = input().axis_value("move_x");
        float vy = input().axis_value("move_y");

        // Normalise diagonal movement so it isn't faster than cardinal.
        float len = std::sqrt(vx * vx + vy * vy);
        if (len > 1.f) { vx /= len; vy /= len; }

        sprite_pos_.x += vx * kSpeed * dt;
        sprite_pos_.y += vy * kSpeed * dt;
    }

    void on_render(glyph::Renderer& r) override {
        glyph::Camera& cam = r.camera();

        // Scroll-wheel zoom (centred on current camera position).
        float wheel = input().mouse_wheel().y;
        if (wheel != 0.f)
            cam.zoom = std::clamp(cam.zoom * std::pow(1.15f, wheel), 0.1f, 20.f);

        // Right-mouse-button drag to pan.
        glyph::vec2 mouse_px = input().mouse_position();
        if (input().mouse_down(glyph::MouseButton::Right)) {
            glyph::vec2 delta = mouse_px - last_mouse_px_;
            // Convert screen-pixel delta to world-unit delta.
            cam.position -= delta / cam.zoom;
        }
        last_mouse_px_ = mouse_px;

        r.clear(glyph::Color::rgba8(20, 20, 30));

        // World-origin crosshair (thin white bars).
        glyph::Color faint{1.f, 1.f, 1.f, 0.25f};
        r.draw_textured_quad(white_tex_, {-1.f, -300.f, 2.f, 600.f}, faint);
        r.draw_textured_quad(white_tex_, {-400.f, -1.f, 800.f, 2.f}, faint);

        // Sprite (48×48 display size, centred on sprite_pos_).
        constexpr float kSize = 48.f;
        r.draw_textured_quad(sprite_tex_,
                             {sprite_pos_.x - kSize * 0.5f,
                              sprite_pos_.y - kSize * 0.5f,
                              kSize, kSize});
    }

    void on_shutdown() override {
        sprite_tex_.destroy();
        white_tex_.destroy();
    }

private:
    glyph::Texture sprite_tex_;
    glyph::Texture white_tex_;
    glyph::vec2    sprite_pos_{400.f, 300.f};
    glyph::vec2    last_mouse_px_{0.f, 0.f};
};

GLYPH_MAIN(InputDemo)
