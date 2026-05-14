# Font & Text

**Header:** `#include <glyph/font.h>`

Loads TTF/OTF fonts and bakes an RGBA glyph atlas using `stb_truetype`. ASCII 32–126 is baked at load time. Text is rendered via `Renderer::draw_text()`.

---

## Font

```cpp
class Font {
public:
    struct Glyph {
        Rect  uv;       // pixel rect in the atlas
        vec2  size;     // glyph bitmap size in pixels
        vec2  bearing;  // offset from pen position to glyph top-left
        float advance;  // horizontal pen advance in pixels
    };

    std::shared_ptr<Texture>             atlas;
    std::unordered_map<uint32_t, Glyph>  glyphs;      // keyed by codepoint
    int                                  size_px = 0;
    float                                line_height = 0.f;

    vec2 measure(std::string_view text) const;
    static std::shared_ptr<Font> load(const std::string& ttf_path, int size_px);
};
```

`load()` bakes the ASCII range into a 1024×1024 white-on-alpha atlas and uploads it to the GPU. Returns `nullptr` on failure (file not found, or atlas too small for the requested size).

`measure()` returns `{total_advance_width, line_height}` for a string of ASCII characters.

---

## Drawing text

```cpp
// Load once (or use Resources::font() for caching).
auto font = glyph::Font::load("assets/fonts/roboto.ttf", 24);

// In on_render():
void on_render(glyph::Renderer& r) override {
    r.draw_text(*font, "Score: 100", {10, 20}, {1, 1, 0, 1}); // yellow
}
```

`pos` is the **pen origin** — left edge of the text baseline. Characters extend upward from the baseline (bearing.y is negative for most glyphs in Y-down space).

To centre text horizontally:
```cpp
auto size = font->measure("Game Over");
r.draw_text(*font, "Game Over", {(800 - size.x) * 0.5f, 300});
```

---

## Resources cache

`Resources::font()` caches fonts by `(path, size_px)` so multiple calls return the same atlas:

```cpp
auto font = res_.font("fonts/roboto.ttf", 18);
```

---

## Notes

- **Alpha blending** is enabled automatically by the engine in `Renderer::init()`.
- Non-ASCII codepoints are silently skipped in the current implementation.
- The atlas is white pixels with coverage in the alpha channel — set `tint` to control text colour.
