# Build progress

Tracks position in the 23-step implementation order from `glyph_engine_spec.md` §27.

Update this file when a phase begins and when it completes. Keep notes brief.

## Current phase

**Phase 7: Input + action mapping**

Status: not started

## Phase checklist

- [x] **1. CMake skeleton + SDL3 window** — `samples/01_hello_window` opens and closes cleanly → tag `v0.1.0`
- [x] **2. GL context + clear color** — verified on desktop → tag `v0.2.0`
- [x] **3. Shader + textured quad** — hardcoded vertices, one texture → tag `v0.3.0`
- [x] **4. Sprite batcher** — N sprites with M textures, batched flushes; stress-tested at 10k sprites → tag `v0.4.0`
- [x] **5. Camera + math** — mouse pan, scroll-wheel zoom → tag `v0.5.0`
- [x] **6. Resources + Texture loading** — stb_image; `samples/02_sprite` → tag `v0.6.0`
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
SDL3 callback-based main pattern used instead of traditional while loop — works identically across desktop, web, and mobile without restructuring at phase 19. SDL_Delay(16) placeholder removed in phase 2 when vsync took over.

### Phase 2
glad2 loaded inside Window::create() after SDL_GL_MakeCurrent — keeps platform GL details confined to src/platform/. SDL_FunctionPointer → void* conversion done via memcpy (pedantically correct). Platform compile definitions (GLYPH_PLATFORM_DESKTOP etc.) propagated to all targets via target_compile_definitions PUBLIC. ivec2 defined as a plain struct for now; becomes a glm alias in phase 5.

### Phase 3
GLSL source strings carry no #version — platform header injected at compile time via glShaderSource two-source array (#version 330 core on desktop, #version 300 es on GLES). #ifdef GL_ES guards precision qualifiers. Renderer stores GL handles (unsigned int) directly as private members to avoid leaking internal types into the public header. Ortho matrix computed without glm; will be replaced when glm arrives in phase 5. LNK4098 warning from SDL/MSVC runtime mix is harmless — address if it causes runtime issues.

### Phase 4
SpriteBatch owns shader/VAO/VBO/IBO; Renderer delegates everything to it via unique_ptr<SpriteBatch> (forward-declared to keep internal type out of public header; ~Renderer() defined in renderer.cpp so unique_ptr can see the complete type). Fragment shader uses a switch on int(v_tex_index) rather than dynamic sampler array indexing — GLES 3.0 spec allows it but some drivers reject it. Static IBO (pre-built quad index pattern, GL_STATIC_DRAW). Index type uint16_t — safe up to kMaxQuads=2000 (max vertex index 7999 < 65535).

### Phase 5
glm added as a submodule at 1.0.1. Must be linked to BOTH glyph (PUBLIC) AND glyph_runtime (PRIVATE) — glyph_runtime compiles main_entry.cpp which includes renderer.h → camera.h → math.h → glm/glm.hpp. Camera stores VP in terms of position/zoom/rotation; view_projection() builds glm::ortho with bottom=pos.y+hh, top=pos.y-hh for y-down convention. Mouse pan/zoom deferred to phase 7 (Input system); phase 5 verifies math with time-based camera animation.

### Phase 6
`core` in .gitignore was catching src/core/ — fixed to `/core` (root-anchored). stb_image IMPLEMENTATION defined in stb_impl.cpp (one TU only); all other files include stb_image.h without IMPLEMENTATION. Texture made move-only RAII (destructor calls glDeleteTextures; copy deleted). Resources uses weak_ptr cache — entries expire naturally when shared_ptrs are released. 02_sprite generates its own test PNG via stb_image_write at startup (no binary in repo); _CRT_SECURE_NO_WARNINGS suppresses MSVC sprintf warning from stb internals.
