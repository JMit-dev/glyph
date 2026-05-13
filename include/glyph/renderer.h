// renderer.h — 2D renderer interface.
// All methods must be called from the main thread.
// Call order each frame: begin_frame() → [draw calls] → end_frame().
#pragma once

#include <glyph/math.h>

namespace glyph {

class Texture;

class Renderer {
public:
    // Initialize GL state. Must be called after the GL context is current.
    bool init(int viewport_w, int viewport_h);
    void shutdown();

    void begin_frame();
    void end_frame();

    // Fill the framebuffer with color c.
    void clear(Color c);

    // Update the GL viewport and projection. Call when the window is resized.
    void set_viewport(int w, int h);

    // Draw a single textured quad. dest is in pixel/screen coordinates (y-down).
    void draw_textured_quad(const Texture& tex, Rect dest, Color tint = {1, 1, 1, 1});

private:
    // GL object handles — primitive types only; no GL headers in the public API.
    unsigned int shader_   = 0;   // GL program
    unsigned int vao_      = 0;
    unsigned int vbo_      = 0;
    unsigned int ibo_      = 0;
    float        ortho_[16] = {};
};

} // namespace glyph
