// resources.cpp — Resources implementation.
#include <glyph/resources.h>

#include "platform/file_io.h"

#include <stb_image.h>

#include <cstdio>

namespace glyph {

void Resources::set_root(const std::string& path) {
    root_ = path;
}

std::string Resources::resolve(const std::string& path) const {
    // Treat absolute paths (Unix: '/', Windows: 'X:') as-is.
    if (path.empty()) return path;
    if (path[0] == '/' || (path.size() > 1 && path[1] == ':')) return path;
    return root_ + "/" + path;
}

std::shared_ptr<Texture> Resources::texture(const std::string& path) {
    const std::string key = resolve(path);

    // Return cached instance if still alive.
    auto it = textures_.find(key);
    if (it != textures_.end()) {
        if (auto tex = it->second.lock()) return tex;
    }

    // Load file bytes through FileIO so the same path syntax works everywhere.
    const auto data = FileIO::read_binary(key);
    if (data.empty()) {
        std::fprintf(stderr, "[glyph] Resources: cannot read '%s'\n", key.c_str());
        return nullptr;
    }

    // Decode via stb_image. Force 4 channels (RGBA) regardless of source format.
    int w, h, src_channels;
    stbi_uc* pixels = stbi_load_from_memory(
        data.data(), static_cast<int>(data.size()),
        &w, &h, &src_channels, 4);

    if (!pixels) {
        std::fprintf(stderr, "[glyph] Resources: stbi_load failed for '%s': %s\n",
                     key.c_str(), stbi_failure_reason());
        return nullptr;
    }

    auto tex = std::make_shared<Texture>();
    tex->create(w, h, pixels);
    stbi_image_free(pixels);

    textures_[key] = tex;
    return tex;
}

void Resources::unload_unused() {
    for (auto it = textures_.begin(); it != textures_.end(); ) {
        it = it->second.expired() ? textures_.erase(it) : std::next(it);
    }
}

} // namespace glyph
