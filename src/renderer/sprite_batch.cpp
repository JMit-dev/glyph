// sprite_batch.cpp — SpriteBatch implementation.
#include "sprite_batch.h"

#include <glyph/texture.h>

#include "gl.h"
#include "shader.h"
#include "shaders.h"

#include <cstddef>    // offsetof
#include <cstdio>
#include <cstring>    // memcpy

namespace glyph {

bool SpriteBatch::init() {
    // --- Shader ---
    Shader s;
    if (!s.compile(shaders::kSpriteVert, shaders::kSpriteFrag)) {
        std::fprintf(stderr, "[glyph] SpriteBatch: shader compile failed\n");
        return false;
    }
    shader_ = s.id();

    // Tell the shader which GL texture unit each sampler slot maps to.
    // These bindings never change, so set them once at init.
    glUseProgram(shader_);
    for (int i = 0; i < kMaxTextures; ++i) {
        char name[24];
        std::snprintf(name, sizeof(name), "u_textures[%d]", i);
        glUniform1i(glGetUniformLocation(shader_, name), i);
    }
    glUseProgram(0);

    // --- Static index buffer (same winding for every quad) ---
    uint16_t indices[kMaxQuads * 6];
    for (int i = 0; i < kMaxQuads; ++i) {
        const int v = i * 4, idx = i * 6;
        indices[idx + 0] = static_cast<uint16_t>(v + 0);
        indices[idx + 1] = static_cast<uint16_t>(v + 1);
        indices[idx + 2] = static_cast<uint16_t>(v + 2);
        indices[idx + 3] = static_cast<uint16_t>(v + 2);
        indices[idx + 4] = static_cast<uint16_t>(v + 3);
        indices[idx + 5] = static_cast<uint16_t>(v + 0);
    }

    // --- VAO / VBO / IBO ---
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(kMaxQuads * 4 * sizeof(Vertex)),
                 nullptr, GL_DYNAMIC_DRAW);

    // position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, x)));

    // uv (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, u)));

    // color (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, r)));

    // tex_index (location 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, tex_index)));

    glGenBuffers(1, &ibo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(sizeof(indices)),
                 indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    return true;
}

void SpriteBatch::shutdown() {
    if (vbo_)    { glDeleteBuffers(1, &vbo_);       vbo_    = 0; }
    if (ibo_)    { glDeleteBuffers(1, &ibo_);       ibo_    = 0; }
    if (vao_)    { glDeleteVertexArrays(1, &vao_);  vao_    = 0; }
    if (shader_) { glDeleteProgram(shader_);        shader_ = 0; }
}

void SpriteBatch::begin(const float* vp_mat4) {
    std::memcpy(vp_, vp_mat4, 16 * sizeof(float));
    quad_count_ = 0;
    tex_count_  = 0;
    for (int i = 0; i < kMaxTextures; ++i) textures_[i] = nullptr;
}

int SpriteBatch::find_or_add_texture(const Texture& tex) {
    for (int i = 0; i < tex_count_; ++i) {
        if (textures_[i] == &tex) return i;
    }
    if (tex_count_ == kMaxTextures) return -1;  // caller must flush first
    textures_[tex_count_] = &tex;
    return tex_count_++;
}

void SpriteBatch::submit(const Texture& tex,
                         float x0, float y0, float x1, float y1,
                         float u0, float v0, float u1, float v1,
                         Color tint) {
    // Flush if we're out of texture slots or vertex space.
    int slot = find_or_add_texture(tex);
    if (slot == -1 || quad_count_ == kMaxQuads) {
        flush();
        // After flush, the batch is empty and the texture definitely fits.
        textures_[0] = &tex;
        tex_count_   = 1;
        slot         = 0;
    }

    Vertex* v = &verts_[quad_count_ * 4];
    v[0] = {x0, y0,  u0, v0,  tint.r, tint.g, tint.b, tint.a,  static_cast<float>(slot)};
    v[1] = {x1, y0,  u1, v0,  tint.r, tint.g, tint.b, tint.a,  static_cast<float>(slot)};
    v[2] = {x1, y1,  u1, v1,  tint.r, tint.g, tint.b, tint.a,  static_cast<float>(slot)};
    v[3] = {x0, y1,  u0, v1,  tint.r, tint.g, tint.b, tint.a,  static_cast<float>(slot)};
    ++quad_count_;
}

void SpriteBatch::flush() {
    if (quad_count_ == 0) return;

    glUseProgram(shader_);
    glUniformMatrix4fv(glGetUniformLocation(shader_, "u_vp"), 1, GL_FALSE, vp_);

    // Bind each active texture to its assigned GL slot.
    for (int i = 0; i < tex_count_; ++i) {
        if (textures_[i]) textures_[i]->bind(i);
    }

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    static_cast<GLsizeiptr>(quad_count_ * 4 * sizeof(Vertex)),
                    verts_);
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(quad_count_ * 6),
                   GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);

    quad_count_ = 0;
    tex_count_  = 0;
    for (int i = 0; i < kMaxTextures; ++i) textures_[i] = nullptr;
}

} // namespace glyph
