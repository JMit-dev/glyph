// renderer.cpp — Renderer implementation.
#include <glyph/renderer.h>
#include <glyph/texture.h>

#include "gl.h"
#include "shader.h"
#include "shaders.h"

#include <cstddef>   // offsetof
#include <cstdio>

namespace glyph {

// Per-vertex data uploaded to the GPU each draw call.
struct QuadVertex {
    float x, y;         // position (pixel-space)
    float u, v;         // UV coords [0, 1]
    float r, g, b, a;   // linear-space color
};

// Build a column-major orthographic projection matrix that maps pixel-space
// (origin top-left, y-down) to NDC. Stored directly in the shader uniform.
static void make_ortho(float* m16, float w, float h) {
    // Scale and translate: x ∈ [0, w] → [-1, 1], y ∈ [0, h] → [1, -1]
    m16[ 0] =  2.f / w; m16[ 1] = 0;        m16[ 2] = 0; m16[ 3] = 0;
    m16[ 4] = 0;        m16[ 5] = -2.f / h;  m16[ 6] = 0; m16[ 7] = 0;
    m16[ 8] = 0;        m16[ 9] = 0;         m16[10] = 1; m16[11] = 0;
    m16[12] = -1.f;     m16[13] =  1.f;      m16[14] = 0; m16[15] = 1;
}

bool Renderer::init(int viewport_w, int viewport_h) {
    // --- Shader ---
    Shader s;
    if (!s.compile(shaders::kSpriteVert, shaders::kSpriteFrag)) {
        std::fprintf(stderr, "[glyph] Renderer: sprite shader failed to compile\n");
        return false;
    }
    shader_ = s.id();

    // --- Quad geometry (4 verts, 6 indices, updated each draw call) ---
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(QuadVertex), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex),
        reinterpret_cast<void*>(offsetof(QuadVertex, x)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex),
        reinterpret_cast<void*>(offsetof(QuadVertex, u)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVertex),
        reinterpret_cast<void*>(offsetof(QuadVertex, r)));

    glGenBuffers(1, &ibo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    static const unsigned short kIndices[6] = {0, 1, 2, 2, 3, 0};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    set_viewport(viewport_w, viewport_h);
    return true;
}

void Renderer::shutdown() {
    if (vbo_)    { glDeleteBuffers(1, &vbo_);            vbo_    = 0; }
    if (ibo_)    { glDeleteBuffers(1, &ibo_);            ibo_    = 0; }
    if (vao_)    { glDeleteVertexArrays(1, &vao_);       vao_    = 0; }
    if (shader_) { glDeleteProgram(shader_);             shader_ = 0; }
}

void Renderer::begin_frame() {}

void Renderer::end_frame() {}

void Renderer::clear(Color c) {
    glClearColor(c.r, c.g, c.b, c.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::set_viewport(int w, int h) {
    glViewport(0, 0, w, h);
    make_ortho(ortho_, static_cast<float>(w), static_cast<float>(h));
}

void Renderer::draw_textured_quad(const Texture& tex, Rect dest, Color tint) {
    const float x0 = dest.x,          y0 = dest.y;
    const float x1 = dest.x + dest.w, y1 = dest.y + dest.h;

    const QuadVertex verts[4] = {
        {x0, y0,  0.f, 0.f,  tint.r, tint.g, tint.b, tint.a},
        {x1, y0,  1.f, 0.f,  tint.r, tint.g, tint.b, tint.a},
        {x1, y1,  1.f, 1.f,  tint.r, tint.g, tint.b, tint.a},
        {x0, y1,  0.f, 1.f,  tint.r, tint.g, tint.b, tint.a},
    };

    glUseProgram(shader_);
    glUniformMatrix4fv(glGetUniformLocation(shader_, "u_vp"), 1, GL_FALSE, ortho_);
    glUniform1i(glGetUniformLocation(shader_, "u_texture"), 0);

    tex.bind(0);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}

} // namespace glyph
