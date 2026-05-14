// camera.h — 2D orthographic camera.
// Converts between world space (pixel units, y-down) and screen pixels.
// Pass to Renderer::set_camera() before issuing draw calls each frame.
#pragma once

#include <glyph/math.h>

namespace glyph {

// How the logical viewport fits when the window aspect ratio differs.
enum class FitMode { Stretch, Letterbox, PixelPerfect };

class Camera {
public:
    vec2  position{0.f, 0.f};      // world position at the center of the view
    float zoom     = 1.f;           // >1 = zoomed in, <1 = zoomed out
    float rotation = 0.f;           // radians, clockwise positive (y-down convention)
    vec2  viewport_size{1280, 720}; // logical resolution in world units

    // Combined orthographic × view matrix. Upload this to the renderer each frame.
    mat4 view_projection() const;

    // Convert a screen pixel position (top-left origin) to world space.
    vec2 screen_to_world(vec2 screen_px) const;

    // Convert a world position to screen pixel coordinates.
    vec2 world_to_screen(vec2 world_pos) const;
};

} // namespace glyph
