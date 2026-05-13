# Git workflow

These rules apply to every commit and every release tag.

## Cardinal rule: no LLM attribution

**Never include any reference to Claude, AI, LLMs, or auto-generation in commit messages, commit bodies, footers, tag messages, PR descriptions, or anywhere else in git history.**

Specifically forbidden:
- `Co-Authored-By: Claude <...>`
- `🤖 Generated with Claude Code`
- `Generated with [Claude Code](...)`
- "AI-assisted" notes
- Any emoji robot or sparkle indicating AI work

The commit history reads as if a human wrote every line. No exceptions.

## Commit frequency

**One atomic change per commit.** A commit is the smallest unit that:
- Compiles
- Doesn't break existing tests
- Makes sense on its own

If you find yourself writing "and" or "also" in a commit message, split it into multiple commits.

Examples of good atomic commits:
- Adding the `Texture` class with its loader
- Wiring sol2 bindings for `vec2`
- Fixing a UV calculation bug in the sprite batcher

Examples of bad commits (split these):
- "added texture loading and audio system" → two commits
- "implemented sprite batcher and fixed window resize" → two commits

## Commit message format

```
type: imperative one-line summary in lowercase
```

That's it. No body, no footer, no scope (we don't need it for a solo project of this size).

### Types

| Type | Use for |
|---|---|
| `feat` | New user-facing engine feature or API |
| `fix` | Bug fix |
| `refactor` | Code restructure with no behavior change |
| `perf` | Performance improvement |
| `docs` | Documentation only (markdown, header comments, spec) |
| `test` | Adding or fixing tests |
| `build` | Build system, CMake, toolchain files, dependencies |
| `chore` | Maintenance: gitignore, formatters, repo housekeeping |
| `style` | Formatting, whitespace — no logic change |

### Rules for the summary

- Imperative mood, present tense: "add", not "added" or "adds".
- Lowercase first word.
- No trailing period.
- Under 72 characters.
- One sentence — describes what the change does, not how.

### Good examples

```
feat: add sprite batcher with multi-texture support
feat: bind vec2 and Entity types to lua
fix: correct uv flip on aseprite frame import
refactor: move shader loading into Renderer
perf: cache tilemap quad data to avoid per-frame rebuild
docs: document camera coordinate conventions in header
build: add emscripten toolchain file
test: cover Rect::intersects edge cases
chore: add clang-format config
```

### Bad examples (don't do these)

```
Updated stuff                            ← no type, vague
feat: Added the sprite batcher.          ← past tense, capitalized, trailing period
feat: sprite batcher                     ← not a sentence, no verb
feat: add sprite batcher and fix bug     ← two changes in one commit
feat: implemented the sprite batcher which handles multiple textures by binding them to separate texture units and indexing them in the fragment shader  ← way too long, describes how
```

## Versioning and tags

Semantic versioning, pre-1.0 conventions while the engine is in development.

### Version scheme

- **`v0.MAJOR.MINOR`** during initial development.
- `MAJOR` bumps when a build-order phase from spec §27 completes.
- `MINOR` bumps for notable feature additions within a phase.
- `v1.0.0` ships when phase 22 (Lua platformer sample) runs successfully on all three target platform classes (desktop, web, mobile).

### Phase-to-version mapping

| Phase complete | Tag |
|---|---|
| Phase 1: CMake + window | `v0.1.0` |
| Phase 2: GL context | `v0.2.0` |
| Phase 3: textured quad | `v0.3.0` |
| Phase 4: sprite batcher | `v0.4.0` |
| ... | ... |
| Phase 22: Lua platformer sample | `v1.0.0` |

Between phases, minor bumps for noteworthy additions: `v0.4.1`, `v0.4.2`, etc.

### Tag format

Annotated tags only (`git tag -a`, not lightweight tags).

```
git tag -a v0.4.0 -m "phase 4: sprite batcher complete"
```

Tag message format: `phase N: short description` for phase tags, or `feature: short description` for minor tags. No body, no LLM mention.

### Push tags explicitly

```
git push origin main
git push origin v0.4.0
```

Or push all tags:
```
git push origin --tags
```

## What goes in `.gitignore`

```
build/
build-*/
external/*/build/
.vs/
.vscode/
.idea/
*.user
*.suo
*.o
*.obj
*.exe
*.app
*.wasm
*.js.map
.DS_Store
Thumbs.db
```

Don't gitignore `external/` — those are submodules and need to be tracked.

## Workflow summary

For every change:

1. Make the smallest change that compiles and works.
2. `git add` only the files relevant to that change.
3. `git commit -m "type: imperative summary"` — no body, no attribution.
4. When a phase completes: update `.claude/progress.md`, commit with `docs: mark phase N complete`, then `git tag -a v0.N.0 -m "phase N: ..."`.
5. Push commits and tags.
