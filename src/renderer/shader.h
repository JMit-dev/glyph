// shader.h — internal GL shader program helper.
// Not part of the public API; used only within the renderer.
#pragma once

namespace glyph {

class Shader {
public:
    // Compile vert_src + frag_src and link. Returns false on failure (error logged).
    // Prepends the platform #version header automatically.
    bool compile(const char* vert_src, const char* frag_src);
    void destroy();

    void bind() const;

    void set_mat4(const char* name, const float* m) const;
    void set_int(const char* name, int v) const;

    unsigned int id() const { return id_; }

private:
    unsigned int id_ = 0;
};

} // namespace glyph
