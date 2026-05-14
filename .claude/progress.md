# Build progress

Tracks position in the 23-step implementation order from `glyph_engine_spec.md` §27.

Update this file when a phase begins and when it completes. Keep notes brief.

## Current phase

**Phase 11: Built-in systems**

Status: not started

## Phase checklist

- [x] **1. CMake skeleton + SDL3 window** — `samples/01_hello_window` opens and closes cleanly → tag `v0.1.0`
- [x] **2. GL context + clear color** — verified on desktop → tag `v0.2.0`
- [x] **3. Shader + textured quad** — hardcoded vertices, one texture → tag `v0.3.0`
- [x] **4. Sprite batcher** — N sprites with M textures, batched flushes; stress-tested at 10k sprites → tag `v0.4.0`
- [x] **5. Camera + math** — mouse pan, scroll-wheel zoom → tag `v0.5.0`
- [x] **6. Resources + Texture loading** — stb_image; `samples/02_sprite` → tag `v0.6.0`
- [x] **7. Input + action mapping** — `samples/03_input` → tag `v0.7.0`
- [x] **8. Time + fixed timestep** — verified on artificial slow frame → tag `v0.8.0`
- [x] **9. Audio** — miniaudio; sound + music + volume → tag `v0.9.0`
- [x] **10. EnTT + Entity façade** — built-in components defined → tag `v0.10.0`
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

### Phase 7
Key enum values match SDL_Scancode for zero-overhead lookup. bind_axis() appends pairs (not replaces) so both WASD and arrow keys can share move_x/move_y — default bindings set both. begin_frame() placed at END of SDL_AppIterate (after swap) so events arriving before iterate are visible to on_update, then prev snapshot advances. Game::engine_set_input() is a public setter (friend struct across TUs doesn't work in C++ for locally-defined structs). Mouse pan in 03_input converts screen-pixel delta to world units by dividing by zoom.

### Phase 8
Time::tick() caps raw_dt at kMaxAccum (0.25s) BEFORE multiplying by time_scale to prevent spiral-of-death even at scale > 1. step_fixed() sets fixed_alpha when returning false (end of loop) so the alpha is correct whether 0 or N steps ran. FPS uses EMA with 0.95/0.05 split (~20 frame window), initialized on first frame to avoid NaN. on_fixed_update added as a virtual with default no-op so existing game code needs no changes.

### Phase 9
Audio::Impl uses PIMPL to keep miniaudio types out of the public header. MaSoundDeleter wraps unique_ptr to auto-stop+uninit on destruction. `play()` uses MA_SOUND_FLAG_DECODE for low-latency memory-backed playback; `play_music()` uses MA_SOUND_FLAG_STREAM for disk-streaming. Audio init failure is non-fatal (engine runs silently). `update()` called in main_entry after swap to recycle finished sound slots. Windows links miniaudio via #pragma comment(lib) in the IMPLEMENTATION block; Linux/macOS need explicit CMake libs. Sound/Music are path wrappers; actual decoding by miniaudio at play time.

### Phase 10
Entity/Scene circular dep broken by: scene.h has NO glyph component includes; components.h includes scene.h (for Entity in Transform); scene.cpp pulls in both. Entity template methods (add/get/try_get/has/remove) defined inline in scene.h AFTER Scene class so they can call scene_->registry() with complete type. EnTT must be linked to BOTH glyph (PUBLIC) AND glyph_runtime (PRIVATE) — same pattern as glm. run_systems() and render() are stubs (phase 11). Script component omits sol::table for now (added phase 16). Animator/TilemapRef use forward-declared SpriteSheet/Tilemap — shared_ptr<incomplete> is safe in C++ via type-erased deleter.
