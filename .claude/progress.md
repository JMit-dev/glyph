# Build progress

Tracks position in the 23-step implementation order from `glyph_engine_spec.md` §27.

Update this file when a phase begins and when it completes. Keep notes brief.

## Current phase

**Phase 20: Android build**

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
- [x] **11. Built-in systems** — movement, sprite render, animator → tag `v0.11.0`
- [x] **12. Aseprite JSON loader** — animated sprite plays → tag `v0.12.0`
- [x] **13. Tiled TMJ loader + tilemap renderer** — `samples/04_tilemap` → tag `v0.13.0`
- [x] **14. CollisionSystem** — spatial hash + AABB → tag `v0.14.0`
- [x] **15. Font + text rendering** — stb_truetype atlas → tag `v0.15.0`
- [x] **16. Lua + sol2 bindings** — core API exposed → tag `v0.16.0`
- [x] **17. Script component + ScriptSystem** — entity scripts work → tag `v0.17.0`
- [x] **18. Hot reload** — Lua files reload on change → tag `v0.18.0`
- [x] **19. Emscripten web build** — `samples/02_sprite` in browser → tag `v0.19.0`
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

### Phase 19
window.cpp: SDL_GL_CONTEXT_PROFILE_ES + MAJOR=3 MINOR=0 on __EMSCRIPTEN__ (WebGL2 = GLES 3.0), GL 3.3 Core on desktop. Lua54: kept as C (NOT compiled as C++) — keeps C linkage for luaL_*/lua_* symbols; Emscripten emulates setjmp/longjmp via -enable-emscripten-sjlj so Lua's default error path works. sol2 updated from v3.3.0 → develop tip (c1f95a77) to fix sol::optional<T&> bug with Clang 18+ (Emscripten 5.x). project() LANGUAGES changed from CXX to "C CXX" — required so CMake initializes C compiler rules upfront; without it, Emscripten's toolchain fails to set CMAKE_C_COMPILE_OBJECT during the Generate step (silent bug, not a configure-time error). Generator must be Ninja on Windows — MinGW Makefiles cannot invoke emcc.bat as a compiler. build_web.sh updated to auto-detect Ninja and error on Windows without it. glyph target: -fexceptions on Emscripten. glyph_add_executable: outputs .html into build_web/web/<name>/; link flags: USE_WEBGL2, FULL_ES3, MIN/MAX_WEBGL_VERSION=2, ALLOW_MEMORY_GROWTH, STACK_SIZE=2mb, INITIAL_MEMORY=64mb, NO_EXIT_RUNTIME, FORCE_FILESYSTEM, -fexceptions. Audio: miniaudio detects __EMSCRIPTEN__ and uses webaudio backend. Hot reload: guarded by GLYPH_PLATFORM_DESKTOP, no-op on web. All 4 samples build to .html/.js/.wasm triplets.

### Phase 18
lua_hotreload.cpp polls std::filesystem::last_write_time every 250ms (std::chrono::steady_clock timer in LuaStateImpl::last_reload_check). Entity script modules: mtime recorded in module_mtimes on first load_script_module(); on change: evict from module_cache, reload, patch all live entity self metatables (__index → new class table) preserving per-entity state. Global scripts (run_file paths): tracked in global_scripts/global_mtimes; on change: re-execute with safe_script_file. All hot reload code guarded by #ifdef GLYPH_PLATFORM_DESKTOP — no-op on web/mobile. std::find used to avoid duplicate path registration in global_scripts.

### Phase 17
ScriptSystem in src/lua/script_system.cpp (requires sol2 — split from systems.cpp). First-frame init: loads class table via load_script_module (cached in LuaStateImpl::module_cache after first load), creates self = setmetatable({}, {__index=class}), sets self.entity = Entity handle, calls class:on_start(self). Every frame: calls self:on_update(dt) via sol::protected_function. on_collision forwarded via LuaState::init() registering scene.set_lua_collision_handler — calls self:on_collision(other) on both entities in each collision pair. Scene::set_lua() stores LuaState*; run_script_system and run_collision_system receive it from scene.lua_. CollisionSystem now takes two callbacks (C++ + Lua); fires both. Script::self stores sol::table as std::any; any_cast<sol::table&> avoids copy. Module path convention: scripts/entities/<name>.lua.

### Phase 16
Lua 5.4.7 and sol2 v3.3.0 added as submodules. Lua compiled as lua54 static library via file(GLOB) excluding lua.c, luac.c, and onelua.c (amalgamation that caused duplicate symbols). sol2 headers PRIVATE to glyph (sol.hpp not in any public header). LuaState uses PIMPL — LuaStateImpl is a namespace-level struct (not nested) to avoid MSVC's restriction on defining private nested classes externally. Script::self changed from (future sol::table) to std::any to keep components.h free of sol.hpp. Color binding uses sol::factories (Color is an aggregate; sol::constructors doesn't apply). Resources and LuaState added as engine services in AppState and Game. LuaState::on_update/on_render called in frame loop before C++ game callbacks. Bindings: vec2, Color, Transform, Velocity, Sprite, Entity, Scene, Input, Audio, Resources, Time, Key constants.

### Phase 15
STB_TRUETYPE_IMPLEMENTATION added to stb_impl.cpp alongside STB_IMAGE_IMPLEMENTATION. Font::load() bakes ASCII 32–126 into a 1024×1024 single-channel bitmap via stbtt_BakeFontBitmap, converts to RGBA8 (white+alpha) for the sprite shader, extracts line_height from stbtt_GetFontVMetrics. bearing.y (stb yoff) is negative for above-baseline glyphs — pos.y + bearing.y gives correct Y-down glyph top. Resources::font(path, size_px) caches by "path@size" key. Renderer::draw_text() iterates UTF-8 chars, advances pen_x by advance. glEnable(GL_BLEND) + glBlendFunc added to Renderer::init() — required for font alpha and transparent sprites. resources.h includes font.h directly (forward-decl not viable due to shared_ptr<Font> member).

### Phase 14
CollisionCallback alias added to scene.h. Scene::on_collision(cb) stores a single subscriber (last registration wins; per-entity Lua hooks come in phase 17). run_collision_system takes Scene* to construct Entity handles for the callback. Spatial hash cell size = 64 px. World AABB = Transform::position + BoxCollider::bounds * Transform::scale (rotation ignored, TransformPropagation still stubbed). Pairs encoded as (min_id<<32|max_id) in unordered_set to avoid duplicate checking. Layer/mask: collision fires if (a.layer & b.mask)||(b.layer & a.mask) != 0. Early-out if no callback registered. IDE false positive on scene.cpp — MSVC compiles clean. Also fixed: begin_frame() now calls glClearColor+glClear each frame.

### Phase 13
Tilemap::load() supports embedded and external (.tsj) tilesets; plain-integer tile data arrays only (Base64/zlib not supported — document this). Top 3 GID bits (flip flags) masked out silently. Tiled ≥1.9 "class" field checked before legacy "type" for TiledObject type. tileset_for() scans for highest first_gid ≤ gid. tile_uv() computes pixel rect without calling tileset_for (avoids const issue). Scene::render() draws TilemapRef entities before sprites using Transform::position as world offset; layer opacity applied as tint alpha. Fix: const Tilemap& → Tilemap& in scene.cpp since tileset_for is non-const. 04_tilemap sample generates tileset PNG + TMJ at startup, renders 30×20 map with WASD pan and scroll zoom.

### Phase 12
nlohmann/json added as submodule at external/json; single_include path added to glyph PRIVATE includes. SpriteSheet::load() handles both Aseprite Hash and Array JSON formats; ordered_json used for Hash format to preserve frame export order. Durations read from each frame entry (ms → seconds). AnimatorSystem implemented: time += dt, fmod for looping, walks frame durations to find current frame, writes sp.src and sp.texture each tick. TransformPropagation and Collision remain stubbed.

### Phase 11
LifetimeSystem and MovementSystem implemented in src/scene/systems.cpp; ScriptSystem, AnimatorSystem, CollisionSystem, TransformPropagationSystem are stubs (phases 16, 12, 14, and deferred respectively). Scene::render() finds the primary Camera2D, calls set_camera()+begin_frame() to re-upload the VP, then collects Sprite+Transform pairs, stable-sorts by layer, and submits each to the batcher via the new draw_textured_quad(tex, dest, src_px, tint) overload. Sprite::origin (pivot) and Transform::scale are applied; rotation in the batcher requires per-vertex positions — deferred. TransformPropagationSystem stubbed: correct implementation needs a separate WorldTransform component to avoid accumulating parent offsets each frame.

### Phase 10
Entity/Scene circular dep broken by: scene.h has NO glyph component includes; components.h includes scene.h (for Entity in Transform); scene.cpp pulls in both. Entity template methods (add/get/try_get/has/remove) defined inline in scene.h AFTER Scene class so they can call scene_->registry() with complete type. EnTT must be linked to BOTH glyph (PUBLIC) AND glyph_runtime (PRIVATE) — same pattern as glm. run_systems() and render() are stubs (phase 11). Script component omits sol::table for now (added phase 16). Animator/TilemapRef use forward-declared SpriteSheet/Tilemap — shared_ptr<incomplete> is safe in C++ via type-erased deleter.
