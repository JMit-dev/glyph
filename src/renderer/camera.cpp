// camera.cpp — Camera implementation.
#include <glyph/camera.h>

#include <glm/gtc/matrix_transform.hpp>

namespace glyph {

mat4 Camera::view_projection() const {
    const float hw = viewport_size.x * 0.5f / zoom;
    const float hh = viewport_size.y * 0.5f / zoom;

    // glm::ortho(left, right, bottom, top):
    //   bottom → NDC -1 (lower on screen)
    //   top    → NDC +1 (upper on screen)
    //
    // For y-down pixel space: smaller world Y = upper on screen = NDC +1.
    // So  top    = position.y - hh  (smaller y, upper on screen)
    //     bottom = position.y + hh  (larger  y, lower on screen)
    mat4 proj = glm::ortho(
        position.x - hw,   // left
        position.x + hw,   // right
        position.y + hh,   // bottom  (y-down: larger y = lower on screen)
        position.y - hh    // top     (y-down: smaller y = upper on screen)
    );

    if (rotation == 0.f) return proj;

    // Rotate the view around the camera's own position (clockwise = positive).
    // The ortho already centres on position, so we just rotate in-place.
    mat4 rot = glm::rotate(mat4(1.f), rotation, vec3(0.f, 0.f, 1.f));
    return proj * rot;
}

vec2 Camera::screen_to_world(vec2 screen_px) const {
    // Normalise to [-1, 1], then invert the ortho.
    const float hw = viewport_size.x * 0.5f / zoom;
    const float hh = viewport_size.y * 0.5f / zoom;

    // NDC: x left→right [-1,1], y top→bottom flipped to [+1,-1]
    const float ndc_x =  (screen_px.x / viewport_size.x) * 2.f - 1.f;
    const float ndc_y = -(screen_px.y / viewport_size.y) * 2.f + 1.f;

    return {
        position.x + ndc_x * hw,
        position.y - ndc_y * hh   // flip y back to world (y-down)
    };
}

vec2 Camera::world_to_screen(vec2 world_pos) const {
    const float hw = viewport_size.x * 0.5f / zoom;
    const float hh = viewport_size.y * 0.5f / zoom;

    const float ndc_x = (world_pos.x - position.x) / hw;
    const float ndc_y = (world_pos.y - position.y) / hh;   // +y = down in world

    return {
         (ndc_x + 1.f) * 0.5f * viewport_size.x,
        (-ndc_y + 1.f) * 0.5f * viewport_size.y   // flip to screen y-down
    };
}

} // namespace glyph
