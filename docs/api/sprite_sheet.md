# Sprite Sheets

**Header:** `#include <glyph/sprite_sheet.h>`

Loads Aseprite-exported sprite sheets (JSON + PNG). The Hash format is preferred; Array format is also supported.

---

## AnimClip

```cpp
struct AnimClip {
    std::string        name;
    std::vector<int>   frame_indices;   // indices into SpriteSheet::frames
    std::vector<float> durations;       // seconds, parallel to frame_indices
    bool               loop = true;
};
```

---

## SpriteSheet

```cpp
class SpriteSheet {
public:
    std::shared_ptr<Texture>                  texture;
    std::vector<Rect>                         frames;
    std::unordered_map<std::string, AnimClip> clips;

    static std::shared_ptr<SpriteSheet> load(const std::string& json_path);
};
```

`load()` parses the JSON sidecar and loads the companion PNG (resolved from `meta.image` relative to the JSON path). Returns `nullptr` on failure.

---

## Usage

Export from Aseprite with **File → Export Sprite Sheet**, choose JSON (Hash) output. Both the `.json` and the `.png` must be in the same directory.

```cpp
void on_start() override {
    auto sheet = glyph::SpriteSheet::load("assets/player.json");

    auto e = scene().create_entity("player");
    e.add<glyph::Transform>(glyph::Transform{{200, 300}});
    e.add<glyph::Sprite>();           // src and texture set by AnimatorSystem
    auto& anim = e.add<glyph::Animator>();
    anim.sheet   = sheet;
    anim.clip    = "run";
    anim.playing = true;
}
```

`AnimatorSystem` (run automatically by `Scene::run_systems`) advances `Animator::time` each frame, resolves the current frame from the active clip, and writes `Sprite::src` and `Sprite::texture`.

---

## Animator component

| Field | Type | Description |
|---|---|---|
| `sheet` | `shared_ptr<SpriteSheet>` | The loaded sprite sheet |
| `clip` | `string` | Active clip name (must match a key in `sheet->clips`) |
| `time` | `float` | Playback time in seconds; reset to 0 when switching clips |
| `playing` | `bool` | Pauses when false; set to false automatically at end of non-looping clip |
| `loop` | `bool` | Overridden by `AnimClip::loop` — the clip's own loop flag is used |

!!! note
    Reset `Animator::time` to `0.f` when switching clips.
