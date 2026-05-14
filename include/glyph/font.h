// font.h — TTF font loader and glyph atlas.
//
// Font::load() bakes ASCII 32–126 into a 1024×1024 RGBA atlas texture at load
// time using stb_truetype. The atlas stores coverage in the alpha channel so
// any tint colour can be applied at draw time.
//
// Usage:
//   auto font = res_.font("assets/fonts/roboto.ttf", 24);
//   renderer().draw_text(*font, "Hello!", {10, 40}, glyph::Color{1,1,1,1});
#pragma once

#include <glyph/math.h>
#include <glyph/texture.h>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace glyph {

class Font {
public:
    struct Glyph {
        Rect  uv;       // pixel rect in the atlas (pass to draw_textured_quad as src_px)
        vec2  size;     // glyph bitmap size in pixels
        vec2  bearing;  // offset from pen position to glyph top-left (bearing.y < 0 = above baseline)
        float advance;  // horizontal pen advance in pixels
    };

    std::shared_ptr<Texture>              atlas;
    std::unordered_map<uint32_t, Glyph>   glyphs;   // keyed by codepoint
    int                                   size_px = 0;
    float                                 line_height = 0.f;

    // Total advance width × line_height of text in world pixels.
    vec2 measure(std::string_view text) const;

    // Load a TTF/OTF file and bake ASCII glyphs. Returns nullptr on failure.
    static std::shared_ptr<Font> load(const std::string& ttf_path, int size_px);
};

} // namespace glyph
