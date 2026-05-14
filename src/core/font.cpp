// font.cpp — Font::load() and Font::measure() implementations.
#include <glyph/font.h>

#include <stb_truetype.h>

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <vector>

namespace glyph {

// ---------------------------------------------------------------------------
// Font::measure
// ---------------------------------------------------------------------------

vec2 Font::measure(std::string_view text) const {
    float w = 0.f;
    for (unsigned char c : text) {
        auto it = glyphs.find(static_cast<uint32_t>(c));
        if (it != glyphs.end())
            w += it->second.advance;
    }
    return {w, line_height};
}

// ---------------------------------------------------------------------------
// Font::load
// ---------------------------------------------------------------------------

std::shared_ptr<Font> Font::load(const std::string& ttf_path, int size_px) {
    // Read TTF bytes.
    std::ifstream f(ttf_path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) {
        std::fprintf(stderr, "[glyph] Font: cannot open %s\n", ttf_path.c_str());
        return nullptr;
    }
    const auto file_sz = static_cast<std::size_t>(f.tellg());
    f.seekg(0);
    std::vector<uint8_t> ttf(file_sz);
    f.read(reinterpret_cast<char*>(ttf.data()),
           static_cast<std::streamsize>(file_sz));

    // Bake ASCII 32–126 (95 glyphs) into a 1024×1024 single-channel bitmap.
    constexpr int kFirst  = 32;
    constexpr int kCount  = 95;   // 32..126 inclusive
    constexpr int kAtlasW = 1024;
    constexpr int kAtlasH = 1024;

    std::vector<uint8_t>   bitmap(kAtlasW * kAtlasH, 0);
    stbtt_bakedchar        baked[kCount];

    const int rows = stbtt_BakeFontBitmap(
        ttf.data(), 0, static_cast<float>(size_px),
        bitmap.data(), kAtlasW, kAtlasH,
        kFirst, kCount, baked);

    if (rows <= 0) {
        std::fprintf(stderr,
            "[glyph] Font: atlas overflow for %s at %dpx (rows=%d). "
            "Try a smaller size.\n", ttf_path.c_str(), size_px, rows);
        return nullptr;
    }

    // Convert single-channel coverage to RGBA8: white pixels with alpha = coverage.
    // The tint colour in draw_text controls the actual text colour at draw time.
    std::vector<uint8_t> rgba(kAtlasW * kAtlasH * 4);
    for (int i = 0; i < kAtlasW * kAtlasH; ++i) {
        rgba[i*4 + 0] = 255;
        rgba[i*4 + 1] = 255;
        rgba[i*4 + 2] = 255;
        rgba[i*4 + 3] = bitmap[i];
    }

    auto tex = std::make_shared<Texture>();
    if (!tex->create(kAtlasW, kAtlasH, rgba.data())) {
        std::fprintf(stderr, "[glyph] Font: texture upload failed for %s\n",
                     ttf_path.c_str());
        return nullptr;
    }

    // Extract line_height from font vertical metrics.
    stbtt_fontinfo info;
    stbtt_InitFont(&info, ttf.data(),
                   stbtt_GetFontOffsetForIndex(ttf.data(), 0));
    const float scale = stbtt_ScaleForPixelHeight(&info, static_cast<float>(size_px));
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);

    auto font = std::make_shared<Font>();
    font->atlas       = std::move(tex);
    font->size_px     = size_px;
    font->line_height = static_cast<float>(ascent - descent + line_gap) * scale;

    // Store glyph metrics.
    for (int i = 0; i < kCount; ++i) {
        const stbtt_bakedchar& bc = baked[i];
        Glyph g;
        g.uv      = { static_cast<float>(bc.x0),
                      static_cast<float>(bc.y0),
                      static_cast<float>(bc.x1 - bc.x0),
                      static_cast<float>(bc.y1 - bc.y0) };
        g.size    = { static_cast<float>(bc.x1 - bc.x0),
                      static_cast<float>(bc.y1 - bc.y0) };
        g.bearing = { bc.xoff, bc.yoff };   // yoff < 0 = glyph extends above baseline
        g.advance = bc.xadvance;
        font->glyphs[static_cast<uint32_t>(kFirst + i)] = g;
    }

    return font;
}

} // namespace glyph
