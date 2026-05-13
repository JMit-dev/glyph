// renderer.h — 2D renderer interface.
// All methods must be called from the main thread.
// Call order each frame: begin_frame() → [draw calls] → end_frame().
#pragma once

#include <glyph/math.h>

namespace glyph {

class Renderer {
public:
    // Initialize GL state. Must be called after the GL context is current.
    bool init(int viewport_w, int viewport_h);
    void shutdown();

    void begin_frame();
    void end_frame();

    // Fill the framebuffer with color c.
    void clear(Color c);

    // Update the GL viewport. Call when the window is resized.
    void set_viewport(int w, int h);
};

} // namespace glyph
