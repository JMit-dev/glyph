// texture.h — 2D GPU texture.
// All methods must be called from the main thread.
//
// Texture is a move-only RAII type: ~Texture() calls glDeleteTextures.
// Game code normally receives textures as std::shared_ptr<Texture> from
// Resources::texture() rather than constructing them directly.
#pragma once

#include <cstdint>

namespace glyph {

class Texture {
public:
    Texture() = default;
    ~Texture() { destroy(); }

    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& o) noexcept
        : id_(o.id_), w_(o.w_), h_(o.h_) { o.id_ = 0; o.w_ = 0; o.h_ = 0; }

    Texture& operator=(Texture&& o) noexcept {
        if (this != &o) {
            destroy();
            id_ = o.id_; w_ = o.w_; h_ = o.h_;
            o.id_ = 0;   o.w_ = 0;  o.h_ = 0;
        }
        return *this;
    }

    // Upload RGBA8 pixel data and create the GL texture object.
    bool create(int w, int h, const uint8_t* rgba_pixels);

    // Free the GPU texture. Safe to call on an already-destroyed texture.
    // Called automatically by the destructor.
    void destroy();

    // Bind to a GL texture slot (0–7).
    void bind(int slot = 0) const;

    int          width()  const { return w_; }
    int          height() const { return h_; }
    unsigned int id()     const { return id_; }
    bool         valid()  const { return id_ != 0; }

private:
    unsigned int id_ = 0;
    int          w_  = 0;
    int          h_  = 0;
};

} // namespace glyph
