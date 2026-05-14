// renderer.h — 2D renderer interface.
// All methods must be called from the main thread.
// Call order each frame: begin_frame() → [draw calls] → end_frame().
#pragma once

#include <glyph/math.h>

#include <memory>

namespace glyph {

class SpriteBatch;   // internal; defined in src/renderer/sprite_batch.h
class Texture;

class Renderer {
public:
    Renderer();
    ~Renderer();   // defined in renderer.cpp after SpriteBatch is complete

    // Initialize GL state. Must be called after the GL context is current.
    bool init(int viewport_w, int viewport_h);
    void shutdown();

    void begin_frame();
    void end_frame();

    // Fill the framebuffer with a solid color.
    void clear(Color c);

    // Update the GL viewport and recalculate the ortho projection.
    // Call when the window is resized.
    void set_viewport(int w, int h);

    // Draw a textured rectangle. dest is in screen pixels (y-down, origin top-left).
    // UV covers the full texture; source-rect support arrives with the sprite sheet loader.
    void draw_textured_quad(const Texture& tex, Rect dest, Color tint = {1, 1, 1, 1});

private:
    std::unique_ptr<SpriteBatch> batch_;
    float ortho_[16] = {};
};

} // namespace glyph
