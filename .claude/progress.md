# Build progress

Tracks position in the 23-step implementation order from `glyph_engine_spec.md` §27.

Update this file when a phase begins and when it completes. Keep notes brief.

## Current phase

**Phase 1: CMake skeleton + SDL3 window**

Status: not started

## Phase checklist

- [ ] **1. CMake skeleton + SDL3 window** — `samples/01_hello_window` opens and closes cleanly → tag `v0.1.0`
- [ ] **2. GL context + clear color** — verified on desktop → tag `v0.2.0`
- [ ] **3. Shader + textured quad** — hardcoded vertices, one texture → tag `v0.3.0`
- [ ] **4. Sprite batcher** — N sprites with M textures, batched flushes; stress-tested at 10k sprites → tag `v0.4.0`
- [ ] **5. Camera + math** — mouse pan, scroll-wheel zoom → tag `v0.5.0`
- [ ] **6. Resources + Texture loading** — stb_image; `samples/02_sprite` → tag `v0.6.0`
- [ ] **7. Input + action mapping** — `samples/03_input` → tag `v0.7.0`
- [ ] **8. Time + fixed timestep** — verified on artificial slow frame → tag `v0.8.0`
- [ ] **9. Audio** — miniaudio; sound + music + volume → tag `v0.9.0`
- [ ] **10. EnTT + Entity façade** — built-in components defined → tag `v0.10.0`
- [ ] **11. Built-in systems** — movement, sprite render, animator → tag `v0.11.0`
- [ ] **12. Aseprite JSON loader** — animated sprite plays → tag `v0.12.0`
- [ ] **13. Tiled TMJ loader + tilemap renderer** — `samples/04_tilemap` → tag `v0.13.0`
- [ ] **14. CollisionSystem** — spatial hash + AABB → tag `v0.14.0`
- [ ] **15. Font + text rendering** — stb_truetype atlas → tag `v0.15.0`
- [ ] **16. Lua + sol2 bindings** — core API exposed → tag `v0.16.0`
- [ ] **17. Script component + ScriptSystem** — entity scripts work → tag `v0.17.0`
- [ ] **18. Hot reload** — Lua files reload on change → tag `v0.18.0`
- [ ] **19. Emscripten web build** — `samples/02_sprite` in browser → tag `v0.19.0`
- [ ] **20. Android build** — `samples/02_sprite` on device → tag `v0.20.0`
- [ ] **21. iOS build** → tag `v0.21.0`
- [ ] **22. Lua platformer sample** — full dogfood test → tag `v1.0.0`
- [ ] **23. Documentation pass** — every public header → tag `v1.0.1`

## Phase notes

(Add brief notes here as phases complete: what was tricky, what was deferred, decisions made.)

### Phase 1
_pending_
