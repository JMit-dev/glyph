// sprite_sheet.h — Aseprite JSON sprite-sheet loader.
//
// Load a .json sidecar exported by Aseprite (Hash format preferred).
// The companion PNG is resolved relative to the JSON file path.
//
// Usage:
//   auto sheet = glyph::SpriteSheet::load("assets/player.json");
//   entity.get<glyph::Animator>().sheet = sheet;
//   entity.get<glyph::Animator>().clip  = "run";
#pragma once

#include <glyph/math.h>
#include <glyph/texture.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace glyph {

struct AnimClip {
    std::string        name;
    std::vector<int>   frame_indices;   // indices into SpriteSheet::frames
    std::vector<float> durations;       // seconds, parallel to frame_indices
    bool               loop = true;
};

class SpriteSheet {
public:
    std::shared_ptr<Texture>                  texture;
    std::vector<Rect>                         frames;   // frame rects in pixels
    std::unordered_map<std::string, AnimClip> clips;

    // Load from an Aseprite-exported JSON sidecar. Returns nullptr on failure.
    // The texture PNG is resolved relative to the JSON path via meta.image.
    static std::shared_ptr<SpriteSheet> load(const std::string& json_path);
};

} // namespace glyph
