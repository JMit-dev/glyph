// texture.cpp — GL texture implementation.
#include <glyph/texture.h>

#include "gl.h"

#include <cstdio>

namespace glyph {

bool Texture::create(int w, int h, const uint8_t* rgba_pixels) {
    if (id_) destroy();

    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D, id_);

    // Nearest-neighbor filtering preserves pixel art edges.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_pixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    w_ = w;
    h_ = h;
    return true;
}

void Texture::destroy() {
    if (id_) {
        glDeleteTextures(1, &id_);
        id_ = 0;
    }
    w_ = h_ = 0;
}

void Texture::bind(int slot) const {
    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + slot));
    glBindTexture(GL_TEXTURE_2D, id_);
}

} // namespace glyph
