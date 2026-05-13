// renderer.cpp — Renderer implementation.
#include <glyph/renderer.h>

#include "gl.h"

namespace glyph {

bool Renderer::init(int viewport_w, int viewport_h) {
    // glad has already loaded all function pointers by the time this is called.
    set_viewport(viewport_w, viewport_h);
    return true;
}

void Renderer::shutdown() {}

void Renderer::begin_frame() {
    // Future: reset batcher state, bind default framebuffer.
}

void Renderer::end_frame() {
    // Future: flush any remaining batcher submissions.
}

void Renderer::clear(Color c) {
    glClearColor(c.r, c.g, c.b, c.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::set_viewport(int w, int h) {
    glViewport(0, 0, w, h);
}

} // namespace glyph
