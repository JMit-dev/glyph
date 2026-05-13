# Documentation setup

## Tooling

- **MkDocs** with the **Material** theme.
- Config: `mkdocs.yml` at the repo root.
- Source: `docs/` directory (Markdown files).
- Generated output: `site/` — gitignored, never commit this.

## Hosting

- Deployed to **GitHub Pages** at `https://jmit-dev.github.io/glyph`.
- Branch: `gh-pages` (auto-created/updated by the deploy workflow).
- GitHub repo Settings → Pages must be set to: Source = "Deploy from a branch", Branch = `gh-pages`, folder = `/ (root)`.

## CI/CD

- Workflow: `.github/workflows/docs.yml`.
- Triggers on every push to `master` and on manual `workflow_dispatch`.
- Uses `mkdocs gh-deploy --force` — pushes the built site to the `gh-pages` branch.
- Python dependency: `mkdocs-material` (installed via pip in CI).

## Structure

```
docs/
├── index.md                # Landing page
├── getting-started.md      # Build + first game walkthrough
├── api/
│   ├── index.md            # API overview table
│   ├── game.md             # Game, AppConfig, GLYPH_MAIN
│   ├── renderer.md         # Renderer
│   ├── texture.md          # Texture
│   └── math.md             # Color, Rect, ivec2 (+ glm aliases in phase 5)
└── dev/
    ├── progress.md         # Phase checklist (mirrors .claude/progress.md)
    └── conventions.md      # C++ code style guide
```

## Maintenance rules

1. **Keep `docs/dev/progress.md` in sync with `.claude/progress.md`** when phases complete. Both track the same information; progress.md is for internal AI use, the docs page is public-facing.
2. **Add an API page** for each new public header added in a phase. Update `docs/api/index.md` table.
3. **No Doxygen** — per spec §9, public headers use plain comment docstrings. API docs are written manually in Markdown.
4. Commit doc changes with `docs: ...` type, separate from code changes.

## Local preview

```bash
pip install mkdocs-material
mkdocs serve           # live-reload dev server at http://127.0.0.1:8000
mkdocs build           # builds to site/ without deploying
```
