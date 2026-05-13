// texture.h — 2D GPU texture.
// All methods must be called from the main thread.
#pragma once

#include <cstdint>

namespace glyph {

class Texture {
public:
    // Upload RGBA8 pixel data and create the GL texture.
    // Nearest-neighbor filtering; suitable for pixel art.
    bool create(int w, int h, const uint8_t* rgba_pixels);
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
