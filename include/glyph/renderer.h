// renderer.h — 2D renderer interface.
// All methods must be called from the main thread.
// Call order each frame: begin_frame() → [draw calls] → end_frame().
#pragma once

#include <glyph/camera.h>
#include <glyph/math.h>

#include <memory>
#include <string_view>

namespace glyph {

class Font;          // defined in font.h
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

    // Set the background color used to clear the framebuffer at begin_frame().
    // Default: opaque black.
    void set_clear_color(Color c) { clear_color_ = c; }

    // Manually fill the framebuffer with a solid color (rarely needed;
    // begin_frame() already clears automatically).
    void clear(Color c);

    // Update the GL viewport and camera viewport_size. Call on window resize.
    void set_viewport(int w, int h);

    // Replace the active camera. Takes effect on the next begin_frame().
    void set_camera(const Camera& c);

    Camera&       camera()       { return camera_; }
    const Camera& camera() const { return camera_; }

    // Draw a textured rectangle. dest is in world-space pixel coordinates.
    // UV covers the full texture.
    void draw_textured_quad(const Texture& tex, Rect dest, Color tint = {1, 1, 1, 1});

    // Draw with an explicit source rect in pixels (for sub-texture / sprite-sheet use).
    // If src_px has zero width or height, the full texture is used.
    void draw_textured_quad(const Texture& tex, Rect dest, Rect src_px, Color tint = {1, 1, 1, 1});

    // Draw a UTF-8 string. pos is the pen origin (left edge of baseline).
    // Glyphs outside ASCII 32–126 are silently skipped until dynamic atlas is added.
    void draw_text(const Font& font, std::string_view text, vec2 pos,
                   Color tint = {1, 1, 1, 1});

private:
    Camera                       camera_;
    Color                        clear_color_ {0.f, 0.f, 0.f, 1.f};
    std::unique_ptr<SpriteBatch> batch_;
};

} // namespace glyph
