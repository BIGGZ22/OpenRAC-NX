#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
The layout of OpenRAC's own C++ (port/docs/CODING_CONVENTIONS.md, sections 5 and 12).

  python3 tools/cpp_style.py                    list the files that need formatting; exit 1 if any
  python3 tools/cpp_style.py --fix              rewrite them
  python3 tools/cpp_style.py [--fix] PATH...    only these files, or the C++ under these folders
  python3 tools/cpp_style.py --comments [PATH...]
                                                list control blocks with no comment above them
                                                (report only: it never changes a file, never fails)
  python3 tools/cpp_style.py --report-only      the check, but always exit 0 (CI uses it for now)
  python3 tools/cpp_style.py --trees            print the folders it covers

Two passes. First clang-format with port/.clang-format (the one the port's CI pins), which
places braces, wraps lines and sorts includes. Then the blank lines clang-format has no option
for: a control block (if, for, while, switch, do, try) gets a blank line before it and after its
closing brace, except when it is the first or the last thing in the block around it or sits
between `}` and its `else`. Comment lines right above a block stay with it, so the blank line
goes above them. Between the two passes nothing else is touched: comments are never rewrapped.

Scope: the C++ that git tracks (or would track) under port/ and sdk/. Never games/ (the
decompilations), a game's hand C (port/game/*/hand/), a generated file (a "Do not edit" in its
first two lines), or a region a file fences with `// clang-format off` and `// clang-format on`.
Plain C (.c) and Objective-C++ (.mm) are left to clang-format's own check in
port/tools/conventions.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path
from typing import List, Optional, Sequence, Tuple

ROOT = Path(__file__).resolve().parent.parent
STYLE = ROOT / "port" / ".clang-format"

# The folders (relative to the repository root) whose C++ this tool covers.
TREES = ("port", "sdk")
# Objective-C++ (.mm) is left out: the port's .clang-format says `Language: Cpp`, which clang-format
# refuses for it.
SUFFIXES = {".cpp", ".cc", ".hpp", ".inl", ".h"}
# A path with one of these parts is never covered: a game's own C, build output, vendored code.
SKIPPED_PARTS = {"hand", "build", "third_party", "node_modules"}

# A line that begins a control block, and a line that carries one on (`} else {`, `} while (x);`).
CONTROL = re.compile(r"^\s*(?:if|for|while|switch|do|try)\b")
CARRIES_ON = re.compile(r"^\s*\}\s*(?:else|catch|while)\b")
# A line after which a block needs no blank line above it: it opens a block or is a label.
OPENING = re.compile(r"(?:\{|(?<!:):)$")
# A line a blank line is never put before: the end of the enclosing block, a label, a directive.
ENDS_BLOCK = re.compile(r"^\s*(?:\}|case\b|default\b|public:|private:|protected:|#)")
# A plain null check, for --comments: `if (name)`, `if (!name)`, `if (name == nullptr)`.
NULL_CHECK = re.compile(r"^\s*if \((?:!)?(\w+)(?: [!=]= nullptr)?\) \{")

FORMAT_OFF = re.compile(r"//\s*clang-format off\b")
FORMAT_ON = re.compile(r"//\s*clang-format on\b")


def find_clang_format() -> Optional[List[str]]:
    """The command that runs clang-format: $CLANG_FORMAT, one on PATH, or Xcode's on macOS.

    CI pins clang-format 21.1.8; Xcode's 21.0.0 lays out every port file the same way
    (CODING_CONVENTIONS.md, section 2). When two versions disagree the pinned one decides."""
    env = os.environ.get("CLANG_FORMAT")
    if env:
        return [env]

    found = shutil.which("clang-format")
    if found:
        return [found]

    xcrun = shutil.which("xcrun")
    if xcrun:
        probe = subprocess.run([xcrun, "--find", "clang-format"], capture_output=True, text=True)
        path = probe.stdout.strip()
        if probe.returncode == 0 and path:
            return [path]

    return None


def is_generated(text: str) -> bool:
    """True when one of the first two lines says the file is generated."""
    return any("do not edit" in line.lower() for line in text.split("\n", 2)[:2])


def in_scope(path: Path) -> bool:
    """Whether the tool may touch this file: a C++ source under a covered tree, outside games/."""
    try:
        parts = path.resolve().relative_to(ROOT).parts
    except ValueError:
        return False

    if not parts or parts[0] not in TREES or path.suffix not in SUFFIXES:
        return False

    return not SKIPPED_PARTS.intersection(parts[:-1])


def tracked(folder: Path) -> List[Path]:
    """The files git tracks or would track (untracked, not ignored) under a folder."""
    out = subprocess.run(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard", "--", str(folder)],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=True,
    ).stdout

    return [ROOT / line for line in out.split("\n") if line]


def sources(paths: Sequence[Path]) -> List[Path]:
    """The files to look at: the given files, or the covered files under the given folders."""
    roots = [p.resolve() for p in paths] or [ROOT / tree for tree in TREES]
    found = set()

    for root in roots:
        candidates = tracked(root) if root.is_dir() else [root]
        found.update(p for p in candidates if p.is_file() and in_scope(p))

    return sorted(found)


def code_of(line: str) -> str:
    """A line's code only: string and character literals emptied, the // comment cut, trimmed."""
    code = re.sub(r'"(?:\\.|[^"\\])*"', '""', line)
    code = re.sub(r"'(?:\\.|[^'\\])+'", "''", code)

    return code.split("//", 1)[0].strip()


def scan(lines: Sequence[str]) -> List[Optional[str]]:
    """Each line's code, or None for a line the spacing pass must leave alone.

    None marks the inside of a block comment or a raw string, a preprocessor directive with its
    continuation lines, and everything between `// clang-format off` and `// clang-format on`."""
    result: List[Optional[str]] = []
    in_comment = False
    raw_end: Optional[str] = None
    in_directive = False
    fenced = False

    for line in lines:
        stripped = line.strip()

        if FORMAT_OFF.search(stripped):
            fenced = True
        elif FORMAT_ON.search(stripped):
            fenced = False
            result.append(None)
            continue

        if fenced or in_directive:
            in_directive = in_directive and stripped.endswith("\\")
            result.append(None)
            continue

        if in_comment:
            in_comment = "*/" not in stripped
            result.append(None)
            continue

        if raw_end is not None:
            if raw_end in stripped:
                raw_end = None
            result.append(None)
            continue

        if stripped.startswith("#"):
            in_directive = stripped.endswith("\\")
            result.append(None)
            continue

        if stripped.startswith("/*"):
            in_comment = "*/" not in stripped
            result.append(None)
            continue

        raw = re.search(r'R"([^(\s]*)\(', stripped)
        if raw and f"){raw.group(1)}\"" not in stripped[raw.end():]:
            raw_end = f"){raw.group(1)}\""
            result.append(None)
            continue

        result.append(code_of(line))

    return result


def starts_block(line: str) -> bool:
    """True when the line begins a control block (not the `else` or `while` that carries one on)."""
    return bool(CONTROL.match(line)) and not CARRIES_ON.match(line)


def space_blocks(text: str) -> str:
    """The text with a blank line before each control block and after its closing brace."""
    lines = text.split("\n")
    codes = scan(lines)
    out: List[str] = []

    # One entry per open brace: does it belong to a control block?
    braces: List[bool] = []
    parens = 0
    # A control statement has begun and its `{` has not come yet (a wrapped condition).
    pending = False
    # The previous line closed a control block that nothing carries on.
    blank_due = False

    for line, code in zip(lines, codes):
        if code is None:
            out.append(line)
            blank_due = False
            continue

        stripped = line.strip()
        carries_on = bool(CARRIES_ON.match(line))
        begins = starts_block(line)

        if blank_due and stripped and not carries_on and not ENDS_BLOCK.match(line):
            out.append("")

        blank_due = False

        if begins:
            # The comment lines right above a block belong to it: the blank line goes above them.
            top = len(out)

            while top > 0 and out[top - 1].strip().startswith("//"):
                top -= 1

            above = out[top - 1].strip() if top > 0 else ""

            if above and not OPENING.search(code_of(above)) and not above.startswith("#"):
                out.insert(top, "")

        out.append(line)

        if begins or carries_on:
            pending = True

        closed_control = False

        for char in code:
            if char == "(":
                parens += 1
            elif char == ")":
                parens = max(parens - 1, 0)
            elif char == "{":
                braces.append(pending and parens == 0)
                pending = pending and parens != 0
                closed_control = False
            elif char == "}" and braces:
                closed_control = braces.pop()
            elif char == ";" and parens == 0:
                # A control statement with no body (`while (x);`) has no brace to wait for.
                pending = False

        blank_due = closed_control and not pending

    return "\n".join(out)


def uncommented(text: str) -> List[int]:
    """Line numbers (1-based) of control blocks with no comment on the line above.

    A plain null check of what the line before assigned is the one exception: `x = find();` and
    then `if (!x) {`."""
    lines = text.split("\n")
    codes = scan(lines)
    found = []

    for number, (line, code) in enumerate(zip(lines, codes)):
        if code is None or not starts_block(line):
            continue

        above = lines[number - 1].strip() if number > 0 else ""

        if above.startswith("//"):
            continue

        check = NULL_CHECK.match(line)

        if check and re.search(rf"\b{check.group(1)}\b\s*=[^=]", above):
            continue

        found.append(number + 1)

    return found


def formatted(command: List[str], path: Path) -> str:
    """The file as it should be: clang-format first, then the blank lines around control blocks."""
    done = subprocess.run(
        command + [f"--style=file:{STYLE}", str(path)], capture_output=True, check=True
    )

    return space_blocks(done.stdout.decode("utf-8"))


def relative(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def report_comments(files: Sequence[Path]) -> None:
    """Prints each control block without a comment above it, and the total."""
    total = 0

    for path in files:
        for number in uncommented(path.read_text(encoding="utf-8")):
            print(f"{relative(path)}:{number}: control block without a comment above it")
            total += 1

    print(f"{total} control blocks without a comment above them in {len(files)} files")


def run_layout(files: Sequence[Path], fix: bool, report_only: bool) -> int:
    """Checks (or with fix, rewrites) each file's layout. Returns the exit status."""
    command = find_clang_format()

    if not command:
        print("clang-format not found: pip install clang-format==21.1.8, or set CLANG_FORMAT", file=sys.stderr)
        return 2

    changed: List[Tuple[Path, str]] = []

    for path in files:
        original = path.read_text(encoding="utf-8")

        # A generated file is never rewritten, whatever the tool would make of it.
        if is_generated(original):
            continue

        try:
            text = formatted(command, path)
        except subprocess.CalledProcessError as error:
            print(f"clang-format failed on {relative(path)}: {error.stderr.decode('utf-8').strip()}", file=sys.stderr)
            return 2

        if text != original:
            changed.append((path, text))

    if fix:
        for path, text in changed:
            path.write_text(text, encoding="utf-8")

        print(f"formatted {len(changed)} of {len(files)} files")
        return 0

    for path, _ in changed:
        print(f"needs formatting: {relative(path)}")

    print(f"{len(changed)} of {len(files)} files need formatting: python3 tools/cpp_style.py --fix PATH")

    return 1 if changed and not report_only else 0


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--fix", action="store_true", help="rewrite the files in place")
    parser.add_argument("--comments", action="store_true", help="list control blocks with no comment above them")
    parser.add_argument("--report-only", action="store_true", help="report what would change but exit 0")
    parser.add_argument("--trees", action="store_true", help="print the folders this tool covers")
    parser.add_argument("paths", nargs="*", type=Path, help="files or folders (default: every covered tree)")
    args = parser.parse_args(argv)

    if args.trees:
        for tree in TREES:
            print(f"{tree}/")

        return 0

    files = sources(args.paths)

    if args.comments:
        report_comments(files)
        return 0

    return run_layout(files, args.fix, args.report_only)


if __name__ == "__main__":
    sys.exit(main())
