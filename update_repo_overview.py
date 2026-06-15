#!/usr/bin/env python3
"""Update the "WARP Repositories" managed block in all Tinkerforge repositories
referenced by repo_overview.rst.

repo_overview.rst (in this directory) is the single source of truth for the
managed block that is duplicated into the README.rst of many sibling repos.
This script automates keeping those copies in sync.

The block is wrapped in "BEGIN WARP REPOSITORIES" / "END WARP REPOSITORIES"
comment markers. The script locates the block in each README by these markers.

Workflow:
  1. Parse the canonical managed block and the list of referenced GitHub
     repositories from repo_overview.rst.
  2. Check that every referenced repo is available locally as a git working
     copy and already contains the managed block (BEGIN/END markers).
  3. git pull (--ff-only) every available repo so we edit an up-to-date copy.
  4. Replace the existing block with the canonical one.
  5. Show a diff for every changed README and ask to confirm.
  6. git add README.rst && git commit -m "<message>" && git push.

By default the script is interactive and will ask before committing/pushing.
Use --dry-run to only report what would change without touching anything.
"""

import argparse
import difflib
import os
import re
import subprocess
import sys

GITHUB_RE = re.compile(r"github\.com/Tinkerforge/([A-Za-z0-9._-]+)")

BEGIN_MARKER = "BEGIN WARP REPOSITORIES"
END_MARKER = "END WARP REPOSITORIES"


def parse_repos(source_text):
    repos = []
    seen = set()
    for match in GITHUB_RE.finditer(source_text):
        name = match.group(1)
        if name not in seen:
            seen.add(name)
            repos.append(name)
    return repos


def extract_canonical(source_text):
    lines = source_text.split("\n")
    span = find_marker_span(lines)
    if span is None:
        raise ValueError(
            "Could not find the BEGIN/END WARP REPOSITORIES markers in the "
            "source file"
        )
    start, end = span
    return "\n".join(lines[start:end]).strip("\n")


def find_marker_span(lines):
    begin = None
    for i, line in enumerate(lines):
        if begin is None:
            if BEGIN_MARKER in line:
                begin = i
        elif END_MARKER in line:
            return (begin, i + 1)
    return None


def build_new_readme(text, canonical):
    lines = text.split("\n")
    span = find_marker_span(lines)
    if span is None:
        return None
    start, end = span

    head = "\n".join(lines[:start]).rstrip("\n")
    tail = "\n".join(lines[end:]).strip("\n")

    parts = []
    if head:
        parts.append(head)
        parts.append("")
    parts.append(canonical.strip("\n"))
    if tail:
        parts.append("")
        parts.append(tail)
    result = "\n".join(parts)
    return result + "\n"


def git(args, cwd):
    return subprocess.run(
        ["git"] + args, cwd=cwd, capture_output=True, text=True
    )


def is_git_repo(path):
    if not os.path.isdir(path):
        return False
    res = git(["rev-parse", "--is-inside-work-tree"], path)
    return res.returncode == 0 and res.stdout.strip() == "true"


def working_tree_dirty(path):
    res = git(["status", "--porcelain"], path)
    return bool(res.stdout.strip())


def file_has_uncommitted_changes(path, rel):
    res = git(["status", "--porcelain", "--", rel], path)
    return bool(res.stdout.strip())


def show_diff(name, old, new):
    diff = difflib.unified_diff(
        old.splitlines(),
        new.splitlines(),
        fromfile=f"{name}/README.rst (current)",
        tofile=f"{name}/README.rst (updated)",
        lineterm="",
    )
    printed = False
    for line in diff:
        print(line)
        printed = True
    if printed:
        print()


def confirm(prompt):
    try:
        answer = input(f"{prompt} [y/N] ").strip().lower()
    except EOFError:
        return False
    return answer in ("y", "yes")


def commit_and_push(items, args):
    print("The following repos will be committed"
          + ("" if args.no_push else " and pushed") + ":")
    for name, *_ in items:
        print(f"  - {name}")
    print()
    print("Please review the changes above (or run 'git diff' in each repo).")

    if not args.yes:
        if not confirm("Commit"
                       + ("" if args.no_push else " and push")
                       + f' with message "{args.message}"?'):
            print("Aborted. The updated README files are left in the working tree;")
            print("you can inspect them with 'git diff' and revert with "
                  "'git checkout -- README.rst' if desired.")
            return None

    print()
    print("== Commit / push ==")
    failures = []
    for name, path, readme_path in items:
        rel = os.path.relpath(readme_path, path)
        add = git(["add", rel], path)
        if add.returncode != 0:
            print(f"  - {name}: git add failed: {add.stderr.strip()}")
            failures.append(name)
            continue
        commit = git(["commit", "-m", args.message], path)
        if commit.returncode != 0:
            print(f"  - {name}: git commit failed: {commit.stderr.strip() or commit.stdout.strip()}")
            failures.append(name)
            continue
        if args.no_push:
            print(f"  - {name}: committed (push skipped)")
            continue
        push = git(["push"], path)
        if push.returncode != 0:
            print(f"  - {name}: PUSH FAILED: {push.stderr.strip()}")
            failures.append(name)
            continue
        print(f"  - {name}: committed and pushed")
    return failures


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))

    parser = argparse.ArgumentParser(
        description="Sync the WARP Repositories managed block into all referenced repos."
    )
    parser.add_argument(
        "--source",
        default=os.path.join(script_dir, "repo_overview.rst"),
        help="Path to the canonical repo_overview.rst (default: alongside this script).",
    )
    parser.add_argument(
        "--base",
        default=os.path.dirname(script_dir),
        help="Directory containing the sibling repo clones (default: parent of this repo).",
    )
    parser.add_argument(
        "--readme",
        default="README.rst",
        help="README file name to update in each repo (default: README.rst).",
    )
    parser.add_argument(
        "--message",
        default="Update README.rst",
        help='Commit message (default: "Update README.rst").',
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Report and show diffs only; do not pull, modify, commit or push.",
    )
    parser.add_argument(
        "--commit-only",
        action="store_true",
        help="Skip pull/compute/write; only commit (and push) repos whose "
             "README.rst already has uncommitted changes. Use this to resume "
             "an interrupted run at the commit/push phase.",
    )
    parser.add_argument(
        "--no-pull", action="store_true", help="Skip the git pull step."
    )
    parser.add_argument(
        "--no-push", action="store_true", help="Commit but do not push."
    )
    parser.add_argument(
        "--yes",
        action="store_true",
        help="Do not ask for confirmation before committing and pushing.",
    )
    args = parser.parse_args()

    if not os.path.isfile(args.source):
        sys.exit(f"Source file not found: {args.source}")

    with open(args.source, encoding="utf-8") as fh:
        source_text = fh.read()

    canonical = extract_canonical(source_text)
    repos = parse_repos(source_text)
    base = os.path.abspath(args.base)

    print(f"Source : {args.source}")
    print(f"Base   : {base}")
    print(f"Repos referenced in overview: {len(repos)}\n")

    # ----------------------------------------------------------------- #
    # Phase 1: discovery / availability
    # ----------------------------------------------------------------- #
    candidates = []   # (name, path, readme_path, old_text, new_text)
    skipped = []      # (name, reason)

    for name in repos:
        path = os.path.join(base, name)
        if not os.path.isdir(path):
            skipped.append((name, "directory not found"))
            continue
        if not is_git_repo(path):
            skipped.append((name, "not a git repository"))
            continue
        readme_path = os.path.join(path, args.readme)
        if not os.path.isfile(readme_path):
            skipped.append((name, f"no {args.readme}"))
            continue
        with open(readme_path, encoding="utf-8") as fh:
            old_text = fh.read()
        new_text = build_new_readme(old_text, canonical)
        if new_text is None:
            skipped.append((name, "no managed block (BEGIN/END markers) found"))
            continue
        candidates.append([name, path, readme_path, old_text, new_text])

    print("== Availability ==")
    print(f"  {len(candidates)} repo(s) contain the managed block and will be processed.")
    for name, reason in skipped:
        print(f"  - skip {name}: {reason}")
    print()

    if not candidates:
        sys.exit("Nothing to do.")

    # ----------------------------------------------------------------- #
    # Commit-only: resume an interrupted run at the commit/push phase.
    # ----------------------------------------------------------------- #
    if args.commit_only:
        to_commit = []
        clean = []
        for name, path, readme_path, _old, _new in candidates:
            rel = os.path.relpath(readme_path, path)
            if file_has_uncommitted_changes(path, rel):
                to_commit.append((name, path, readme_path))
            else:
                clean.append(name)

        print("== Commit-only ==")
        if clean:
            print(f"  {len(clean)} repo(s) have no uncommitted {args.readme} changes (skipped).")
        print(f"  {len(to_commit)} repo(s) have uncommitted {args.readme} changes.\n")

        if not to_commit:
            print("Nothing to commit.")
            return

        for name, path, readme_path in to_commit:
            rel = os.path.relpath(readme_path, path)
            print(f"--- {name}: git diff {rel} ---")
            diff = git(["--no-pager", "diff", "--", rel], path)
            if diff.stdout.strip():
                print(diff.stdout.rstrip())
            else:
                print("(staged - showing 'git diff --cached')")
                print(git(["--no-pager", "diff", "--cached", "--", rel], path).stdout.rstrip())
            print()

        if args.dry_run:
            print(f"Dry run: would commit {len(to_commit)} repo(s); nothing was committed.")
            return

        failures = commit_and_push(to_commit, args)
        print()
        if failures is None:
            return
        if failures:
            print(f"Done with {len(failures)} failure(s): {', '.join(failures)}")
            sys.exit(1)
        print("Done.")
        return

    # ----------------------------------------------------------------- #
    # Phase 2: git pull (--ff-only)
    # ----------------------------------------------------------------- #
    if not args.dry_run and not args.no_pull:
        print("== git pull --ff-only ==")
        for entry in candidates:
            name, path = entry[0], entry[1]
            if working_tree_dirty(path):
                print(f"  - {name}: working tree dirty, skipping pull")
                continue
            res = git(["pull", "--ff-only"], path)
            status = res.stdout.strip().splitlines()[-1] if res.stdout.strip() else ""
            if res.returncode == 0:
                print(f"  - {name}: {status or 'ok'}")
            else:
                print(f"  - {name}: PULL FAILED: {res.stderr.strip()}")
        print()
        # Re-read READMEs and recompute after pulling.
        for entry in candidates:
            with open(entry[2], encoding="utf-8") as fh:
                entry[3] = fh.read()
            entry[4] = build_new_readme(entry[3], canonical)

    # ----------------------------------------------------------------- #
    # Phase 3: compute changes + show diffs
    # ----------------------------------------------------------------- #
    changed = []
    unchanged = []
    for entry in candidates:
        name, _path, _readme, old_text, new_text = entry
        if new_text is None:
            skipped.append((name, "overview section disappeared after pull"))
            continue
        if new_text == old_text:
            unchanged.append(name)
        else:
            changed.append(entry)

    print("== Changes ==")
    if unchanged:
        print(f"  {len(unchanged)} already up to date: {', '.join(unchanged)}")
    print(f"  {len(changed)} need updating.\n")

    for name, _path, _readme, old_text, new_text in changed:
        show_diff(name, old_text, new_text)

    if not changed:
        print("All overview sections are already up to date.")
        return

    if args.dry_run:
        print("Dry run: no files were modified. "
              f"{len(changed)} README(s) would change.")
        return

    # ----------------------------------------------------------------- #
    # Phase 4: write files
    # ----------------------------------------------------------------- #
    for _name, _path, readme_path, _old, new_text in changed:
        with open(readme_path, "w", encoding="utf-8") as fh:
            fh.write(new_text)
    print(f"Wrote {len(changed)} README file(s).\n")

    # ----------------------------------------------------------------- #
    # Phase 5: confirm + commit + push
    # ----------------------------------------------------------------- #
    items = [(name, path, readme_path) for name, path, readme_path, _o, _n in changed]
    failures = commit_and_push(items, args)
    print()
    if failures is None:
        return
    if failures:
        print(f"Done with {len(failures)} failure(s): {', '.join(failures)}")
        sys.exit(1)
    print("Done.")


if __name__ == "__main__":
    main()
