// resources.cpp — Resources implementation.
#include <glyph/resources.h>
#include <glyph/font.h>

#include "platform/file_io.h"

#include <stb_image.h>

#include <cstdio>

namespace glyph {

void Resources::set_root(const std::string& path) {
    root_ = path;
}

std::string Resources::resolve(const std::string& path) const {
    if (path.empty()) return path;
    // Absolute paths: Unix '/' or Windows 'X:' prefix.
    if (path[0] == '/' || (path.size() > 1 && path[1] == ':')) return path;
    return root_ + "/" + path;
}

std::shared_ptr<Texture> Resources::texture(const std::string& path) {
    const std::string key = resolve(path);

    auto it = textures_.find(key);
    if (it != textures_.end())
        if (auto tex = it->second.lock()) return tex;

    const auto data = FileIO::read_binary(key);
    if (data.empty()) {
        std::fprintf(stderr, "[glyph] Resources: cannot read '%s'\n", key.c_str());
        return nullptr;
    }

    int w, h, ch;
    stbi_uc* px = stbi_load_from_memory(data.data(), static_cast<int>(data.size()),
                                        &w, &h, &ch, 4);
    if (!px) {
        std::fprintf(stderr, "[glyph] Resources: stbi_load failed for '%s': %s\n",
                     key.c_str(), stbi_failure_reason());
        return nullptr;
    }

    auto tex = std::make_shared<Texture>();
    tex->create(w, h, px);
    stbi_image_free(px);

    textures_[key] = tex;
    return tex;
}

std::shared_ptr<Sound> Resources::sound(const std::string& path) {
    const std::string key = resolve(path);

    auto it = sounds_.find(key);
    if (it != sounds_.end())
        if (auto s = it->second.lock()) return s;

    // Sound is a path wrapper; miniaudio decodes it at play time.
    auto s = std::shared_ptr<Sound>(new Sound(key));
    sounds_[key] = s;
    return s;
}

std::shared_ptr<Music> Resources::music(const std::string& path) {
    const std::string key = resolve(path);

    auto it = musics_.find(key);
    if (it != musics_.end())
        if (auto m = it->second.lock()) return m;

    auto m = std::shared_ptr<Music>(new Music(key));
    musics_[key] = m;
    return m;
}

std::shared_ptr<Font> Resources::font(const std::string& path, int size_px) {
    const std::string key = resolve(path) + "@" + std::to_string(size_px);
    auto& wp = fonts_[key];
    if (auto sp = wp.lock()) return sp;
    auto f = Font::load(resolve(path), size_px);
    wp = f;
    return f;
}

void Resources::unload_unused() {
    for (auto it = textures_.begin(); it != textures_.end(); )
        it = it->second.expired() ? textures_.erase(it) : std::next(it);
    for (auto it = sounds_.begin(); it != sounds_.end(); )
        it = it->second.expired() ? sounds_.erase(it) : std::next(it);
    for (auto it = musics_.begin(); it != musics_.end(); )
        it = it->second.expired() ? musics_.erase(it) : std::next(it);
    for (auto it = fonts_.begin(); it != fonts_.end(); )
        it = it->second.expired() ? fonts_.erase(it) : std::next(it);
}

} // namespace glyph
