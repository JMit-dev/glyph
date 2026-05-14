// renderer.h — 2D renderer interface.
// All methods must be called from the main thread.
// Call order each frame: begin_frame() → [draw calls] → end_frame().
#pragma once

#include <glyph/camera.h>
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

    // begin_frame uses camera_.view_projection() as the VP matrix.
    void begin_frame();
    void end_frame();

    // Fill the framebuffer with a solid color.
    void clear(Color c);

    // Update the GL viewport and camera viewport_size. Call on window resize.
    void set_viewport(int w, int h);

    // Replace the active camera. Takes effect on the next begin_frame().
    void set_camera(const Camera& c);

    Camera&       camera()       { return camera_; }
    const Camera& camera() const { return camera_; }

    // Draw a textured rectangle. dest is in world-space pixel coordinates.
    // UV covers the full texture; source-rect support comes with sprite sheets.
    void draw_textured_quad(const Texture& tex, Rect dest, Color tint = {1, 1, 1, 1});

private:
    Camera                       camera_;
    std::unique_ptr<SpriteBatch> batch_;
};

} // namespace glyph
