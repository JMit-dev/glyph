# Glyph

A code-first, cross-platform 2D game engine. C++17 core, Lua 5.4 scripting. Targets desktop, web (WebAssembly), and mobile from one codebase.

## Where things live

- **`glyph_engine_spec.md`** — the full architecture spec. Treat it as the source of truth. If something in this file or in `.claude/` contradicts the spec, the spec wins, but flag it so we can reconcile.
- **`.claude/conventions.md`** — C++ code style and file organization rules. Follow these on every file you touch.
- **`.claude/git.md`** — git commit format, tagging, and versioning rules. Follow these on every commit.
- **`.claude/progress.md`** — which implementation phase we're on. Update it when phases complete.

## How to work on this project

1. **Read the spec section relevant to the task before writing code.** The spec is structured by subsystem (§5–§19). Don't guess at the architecture.
2. **Stay in the current phase.** `progress.md` names the active phase from the 23-step build order in spec §27. Don't jump ahead. If you finish the phase, update `progress.md` and stop — don't auto-start the next one.
3. **One atomic change per commit.** See `.claude/git.md`. Don't batch unrelated edits.
4. **No LLM attribution in commits, ever.** No `Co-Authored-By: Claude`, no "Generated with..." footer, nothing. See `.claude/git.md`.
5. **Ask before deviating from the spec.** If a spec decision turns out to be wrong or unworkable, surface it — don't silently substitute.
6. **No new dependencies without asking.** The dependency list in spec §2 is intentional.

## Project state

This is a solo-developer project. There is no team to coordinate with. Bias toward direct, minimal code over enterprise patterns. Comments explain *why*, not *what*.

## When in doubt

Ask. A clarifying question is cheaper than reverting a wrong direction.
