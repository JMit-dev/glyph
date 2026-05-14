// renderer.cpp — Renderer implementation.
#include <glyph/renderer.h>
#include <glyph/texture.h>

#include "gl.h"
#include "sprite_batch.h"

#include <glm/gtc/type_ptr.hpp>   // glm::value_ptr

#include <cstdio>

namespace glyph {

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
    mat4 vp = camera_.view_projection();
    batch_->begin(glm::value_ptr(vp));
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
    camera_.viewport_size = {static_cast<float>(w), static_cast<float>(h)};
}

void Renderer::set_camera(const Camera& c) {
    camera_ = c;
}

void Renderer::draw_textured_quad(const Texture& tex, Rect dest, Color tint) {
    batch_->submit(tex,
                   dest.x, dest.y, dest.x + dest.w, dest.y + dest.h,
                   0.f, 0.f, 1.f, 1.f,
                   tint);
}

void Renderer::draw_textured_quad(const Texture& tex, Rect dest, Rect src_px, Color tint) {
    float u0 = 0.f, v0 = 0.f, u1 = 1.f, v1 = 1.f;
    if (src_px.w > 0.f && src_px.h > 0.f) {
        const float iw = 1.f / static_cast<float>(tex.width());
        const float ih = 1.f / static_cast<float>(tex.height());
        u0 = src_px.x * iw;        v0 = src_px.y * ih;
        u1 = (src_px.x + src_px.w) * iw;
        v1 = (src_px.y + src_px.h) * ih;
    }
    batch_->submit(tex,
                   dest.x, dest.y, dest.x + dest.w, dest.y + dest.h,
                   u0, v0, u1, v1,
                   tint);
}

} // namespace glyph
