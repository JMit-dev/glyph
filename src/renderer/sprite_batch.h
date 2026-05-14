// sprite_batch.h — multi-texture sprite batcher.
// Internal to the renderer; not part of the public API.
//
// Algorithm (spec §11.3):
//   1. Accumulate quad vertices into a CPU-side buffer.
//   2. Track up to kMaxTextures distinct textures; assign each an index slot.
//   3. Flush (one glDrawElements) when the vertex buffer is full or the
//      texture list is full. Also flushed explicitly by end_frame().
#pragma once

#include <glyph/math.h>

#include <cstdint>

namespace glyph {

class Texture;

class SpriteBatch {
public:
    struct Vertex {
        float x, y;        // position (pixel-space; transformed by u_vp)
        float u, v;        // UV [0, 1]
        float r, g, b, a;  // linear-space color
        float tex_index;   // slot index into u_textures[]
    };

    // kMaxQuads drives buffer size. kMaxTextures must stay ≤ 8 for GLES 3 portability.
    static constexpr int kMaxQuads    = 2000;
    static constexpr int kMaxTextures = 8;

    // compile shader, build VAO/VBO/IBO. Call after GL context is current.
    bool init();
    void shutdown();

    // Set the view-projection matrix used for the whole frame.
    void begin(const float* vp_mat4_col_major);

    // Queue one quad. May trigger an internal flush if the buffer or texture
    // list is full.
    void submit(const Texture& tex,
                float x0, float y0, float x1, float y1,
                float u0, float v0, float u1, float v1,
                Color tint);

    // Upload CPU buffer to GPU and issue glDrawElements. Resets the batch.
    void flush();

private:
    int find_or_add_texture(const Texture& tex);

    Vertex         verts_[kMaxQuads * 4];
    int            quad_count_ = 0;

    const Texture* textures_[kMaxTextures] = {};
    int            tex_count_  = 0;

    float          vp_[16] = {};

    unsigned int   shader_ = 0;
    unsigned int   vao_    = 0;
    unsigned int   vbo_    = 0;
    unsigned int   ibo_    = 0;
};

} // namespace glyph
