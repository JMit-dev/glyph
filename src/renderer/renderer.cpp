// renderer.cpp — Renderer implementation.
#include <glyph/renderer.h>
#include <glyph/texture.h>

#include "gl.h"
#include "sprite_batch.h"

#include <cstdio>

namespace glyph {

// Build a column-major orthographic projection that maps pixel-space
// (origin top-left, y-down) to NDC. Recomputed on set_viewport().
static void make_ortho(float* m16, float w, float h) {
    m16[ 0] =  2.f / w;  m16[ 1] = 0;          m16[ 2] = 0;  m16[ 3] = 0;
    m16[ 4] = 0;          m16[ 5] = -2.f / h;   m16[ 6] = 0;  m16[ 7] = 0;
    m16[ 8] = 0;          m16[ 9] = 0;           m16[10] = 1;  m16[11] = 0;
    m16[12] = -1.f;       m16[13] =  1.f;        m16[14] = 0;  m16[15] = 1;
}

Renderer::Renderer()  = default;
Renderer::~Renderer() = default;   // unique_ptr<SpriteBatch> destructor resolves here

bool Renderer::init(int viewport_w, int viewport_h) {
    batch_ = std::make_unique<SpriteBatch>();
    if (!batch_->init()) {
        std::fprintf(stderr, "[glyph] Renderer: SpriteBatch init failed\n");
        return false;
    }
    set_viewport(viewport_w, viewport_h);
    return true;
}

void Renderer::shutdown() {
    if (batch_) {
        batch_->shutdown();
        batch_.reset();
    }
}

void Renderer::begin_frame() {
    batch_->begin(ortho_);
}

void Renderer::end_frame() {
    batch_->flush();
}

void Renderer::clear(Color c) {
    glClearColor(c.r, c.g, c.b, c.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::set_viewport(int w, int h) {
    glViewport(0, 0, w, h);
    make_ortho(ortho_, static_cast<float>(w), static_cast<float>(h));
}

void Renderer::draw_textured_quad(const Texture& tex, Rect dest, Color tint) {
    batch_->submit(tex,
                   dest.x, dest.y, dest.x + dest.w, dest.y + dest.h,
                   0.f, 0.f, 1.f, 1.f,
                   tint);
}

} // namespace glyph
