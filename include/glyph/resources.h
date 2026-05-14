// resources.h — reference-counted asset cache.
// Resources caches loaded assets by path. Returning a shared_ptr keeps the
// asset alive; when all external references are dropped the cache entry
// expires and the asset is freed on the next unload_unused() call.
//
// All methods must be called from the main thread.
#pragma once

#include <glyph/audio.h>
#include <glyph/texture.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace glyph {

class Resources {
public:
    // Set the root directory prepended to all relative asset paths.
    // Default: "assets"
    void set_root(const std::string& path);
    const std::string& root() const { return root_; }

    // Load a PNG/JPG/etc. texture from path. Returns cached copy if already loaded.
    // Returns nullptr and logs an error if the file cannot be read or decoded.
    std::shared_ptr<Texture> texture(const std::string& path);

    // Return a Sound/Music handle for the given audio file path.
    // Sound/Music are path wrappers; actual decoding happens at play time.
    std::shared_ptr<Sound> sound(const std::string& path);
    std::shared_ptr<Music> music(const std::string& path);

    // Remove cache entries whose shared_ptrs have all expired (no live references).
    void unload_unused();

private:
    std::string resolve(const std::string& path) const;

    std::string root_ = "assets";
    std::unordered_map<std::string, std::weak_ptr<Texture>> textures_;
    std::unordered_map<std::string, std::weak_ptr<Sound>>   sounds_;
    std::unordered_map<std::string, std::weak_ptr<Music>>   musics_;
};

} // namespace glyph
