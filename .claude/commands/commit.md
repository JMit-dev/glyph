---
description: Stage and commit current changes following project rules
---

Look at the changes currently staged or modified in the working directory. Then:

1. If multiple unrelated changes are present, stop and tell me which ones — I'll split them. Don't commit a batch of unrelated edits in one go.
2. If the changes form one atomic unit, propose a single commit message following `.claude/git.md`:
   - `type: imperative one-line summary in lowercase`
   - Pick the right type from: feat, fix, refactor, perf, docs, test, build, chore, style
   - Under 72 characters, no period
3. Show me the proposed message. Do not execute the commit yet.
4. After I confirm, run:
   ```
   git add <relevant files>
   git commit -m "<the message>"
   ```
5. **Never** add `Co-Authored-By`, "Generated with Claude Code", robot emoji, or any LLM attribution. The commit must look like a human wrote it.

If a phase from `.claude/progress.md` was completed by this change:
- Also update `progress.md` to check the box and add a brief phase note.
- After committing the code change, commit the progress update separately as `docs: mark phase N complete`.
- Then suggest the version tag command: `git tag -a v0.N.0 -m "phase N: <description>"`.
