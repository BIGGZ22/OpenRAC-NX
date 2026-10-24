#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Checks the port's C and C++ against the rules of port/docs/CODING_CONVENTIONS.md
that a program can count.

  check_conventions.py                      every file: the rules that hold everywhere
  check_conventions.py --since REF          and the new-code rules on files added since REF
  check_conventions.py --since REF --touched
                                            and reports clang-format's changes on the lines
                                            changed since REF (advisory unless --strict-touched)

Three kinds of rule (CODING_CONVENTIONS.md, "What the machines check"):

- every file in scope: the first line is an SPDX licence line; no tab; no em or en dash;
  getenv, std::getenv or SDL_getenv reads no OPENRAC_ variable and no name that is not a
  literal outside common/env.cpp and runtime/host_env.cpp (section 16), except in the files
  GETENV_EXCEPTIONS names until their package moves the reads;
- a C++ file added since REF: clang-format reports nothing; the first two lines are the
  `//` licence and copyright lines; no `static` at namespace scope; no raw new, delete,
  malloc or free, except in the files ALLOCATION_EXCEPTIONS names; no `using namespace` in a
  header; `extern "C"` only in a file named for a C contract; no printf family or std::cout
  outside tests and programs' main files;
- a C file added since REF: the copyright line and clang-format, reported as advisories.
  The port's C is on its way into games/ and the gate for it is the one above.

Scope: every C and C++ file git tracks (or is about to add) under port/, except a game's
hand C (port/game/*/hand/), build trees and generated files (a "Do not edit" in the first two
lines). Prints `path:line: rule: message` and exits 1 when a gating rule fails.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

SOURCE_SUFFIXES = (".c", ".cc", ".cpp", ".h", ".hpp", ".inl", ".mm")
C_SUFFIXES = (".c",)
CPP_SUFFIXES = (".cc", ".cpp", ".hpp", ".mm")
HEADER_SUFFIXES = (".h", ".hpp", ".inl")

# The directories whose headers are the C contracts (CODING_CONVENTIONS.md, "The C boundary").
CONTRACT_DIRS = (
    "port/runtime/include/openrac",
    "port/game/common/include/openrac",
)

SPDX = re.compile(r"^(?://|/\*) SPDX-License-Identifier: \S+")
CPP_LICENCE = re.compile(r"^// SPDX-License-Identifier: \S+$")
COPYRIGHT = re.compile(r"^(?://|/\*| \*) Copyright \(c\) \d{4} the OpenRAC contributors( \*/)?$")
CPP_COPYRIGHT = re.compile(r"^// Copyright \(c\) \d{4} the OpenRAC contributors$")
DASHES = re.compile("[\u2013\u2014]")

STATIC_AT_NAMESPACE = re.compile(r"^static\b(?!_assert)")
RAW_NEW = re.compile(r"(?<![\w.>])new\b\s*(?:\(|\[|[A-Za-z_:])")
RAW_DELETE = re.compile(r"(?<!=)(?<!= )\bdelete\b\s*(?:\[\s*\]\s*)?[A-Za-z_(]")
C_ALLOCATION = re.compile(r"(?<![\w.>:])(?:std::|::)?(malloc|calloc|realloc|free)\s*\(")
USING_NAMESPACE = re.compile(r"^\s*using\s+namespace\b")
# Matched in code whose literals are blanked: `extern "C"` reads `extern ""` there.
EXTERN_C = re.compile(r'\bextern\s+""')
CONSOLE_OUTPUT = re.compile(
    r"(?<![\w.>:])(?:std::)?(printf|fprintf|puts|fputs)\s*\(|\bstd::(cout|cerr|clog)\b"
)
# A call of the C library's or SDL's getenv, up to its first argument; not a member, not fegetenv.
GETENV_CALL = re.compile(r"(?<![\w.>:])(?:std::|::)?(?:SDL_)?getenv\s*\(\s*")

# The files that read the environment for the port (CODING_CONVENTIONS.md, section 16): the table
# of OPENRAC_* variables, and the C entry point to it (openrac/host_env.h).
GETENV_READERS = (
    "port/common/env.cpp",
    "port/runtime/host_env.cpp",
)

# The files that still read OPENRAC_* variables with getenv, each with what moves its reads onto
# the table. When a package moves a file's reads, it removes the file's entry; an entry whose file
# reads none any more is reported as stale, a warning, since other work removes reads meanwhile.
GETENV_EXCEPTIONS = {
    "port/game/rac1/host/boot.c": "P2-38 reads it through openrac/host_env.h",
    "port/game/rac1/host/collision.c": "P2-38 reads it through openrac/host_env.h",
    "port/game/rac1/host/hand.c": "P2-38 reads it through openrac/host_env.h",
    "port/game/rac1/host/ui2d.c": "P2-38 reads it through openrac/host_env.h",
    "port/tests/vulkan_uniform_test.cpp": "a test that reads the switch to know what to expect",
}


# The files that may allocate raw memory, each with why it has to: a program that counts the heap
# replaces operator new and delete, which cannot take their memory from the heap they count.
ALLOCATION_EXCEPTIONS = {
    "port/tests/render_bench.cpp": "replaces operator new and delete to count a frame's allocations",
}


@dataclass(frozen=True)
class Finding:
    """One broken rule: where, which rule, what to do; `gating` ones fail the check."""

    path: str
    line: int
    rule: str
    message: str
    gating: bool = True

    def __str__(self):
        kind = "" if self.gating else " (advisory)"
        return f"{self.path}:{self.line}: {self.rule}{kind}: {self.message}"


def in_scope(path):
    """True for a port C or C++ file the conventions cover (a repository-relative path)."""
    if not path.startswith("port/") or not path.endswith(SOURCE_SUFFIXES):
        return False
    parts = path.split("/")

    # A game's hand C follows the games' style: it is on its way into games/.
    if len(parts) > 3 and parts[1] == "game" and parts[3] == "hand":
        return False

    # Build trees and fetched dependencies are not the port's source.
    return not any(part in ("build", "_deps") for part in parts)


def is_generated(text):
    """True when the file says in its first two lines that a tool writes it."""
    head = "\n".join(text.splitlines()[:2])
    return "Do not edit" in head or "do not edit" in head


def strip_comments_and_strings(text, keep_columns=False):
    """Blanks out comments and string and character literals, keeping every newline.

    The new-code rules look for words such as `new` and `free`, which prose and messages
    use too. A literal becomes its two quotes; with `keep_columns`, comments become spaces and a
    literal keeps its length (its quotes around spaces), so an offset in the result is the same
    offset in `text`.
    """
    out = []
    n = 0
    length = len(text)
    while n < length:
        two = text[n : n + 2]

        # A line comment runs to the end of the line.
        if two == "//":
            end = text.find("\n", n)
            end = length if end < 0 else end
            if keep_columns:
                out.append(" " * (end - n))
            n = end
            continue

        # A block comment keeps its newlines, so line numbers stay right.
        if two == "/*":
            end = text.find("*/", n + 2)
            end = length if end < 0 else end + 2
            if keep_columns:
                out.append(re.sub(r"[^\n]", " ", text[n:end]))
            else:
                out.append("\n" * text.count("\n", n, end))
            n = end
            continue

        # A literal: skip to its closing quote, past escaped characters.
        if text[n] in "\"'":
            quote = text[n]
            start = n
            n += 1
            while n < length and text[n] != quote and text[n] != "\n":
                n += 2 if text[n] == "\\" else 1
            if keep_columns:
                # The character that ended it (the quote, or the newline of an unclosed one) stays.
                end = text[n] if n < length else ""
                out.append(quote + " " * (min(n, length) - start - 1) + end)
            else:
                out.append(quote + quote)
            n += 1
            continue

        out.append(text[n])
        n += 1

    return "".join(out)


def opens_namespace(text):
    """True when the code (not a comment) opens a namespace: only C++ can."""
    return re.search(r"^\s*namespace\b", strip_comments_and_strings(text), re.M) is not None


def is_cpp(path, text):
    """True for C++: a C++ suffix, or a `.h` that opens a namespace."""
    if path.endswith(CPP_SUFFIXES):
        return True
    return path.endswith(".h") and opens_namespace(text)


def may_define_c_contract(path, contract_stems):
    """True when `path` may hold `extern "C"`: named for a contract, a library, or a test."""
    parts = path.split("/")
    stem = Path(path).stem
    if stem in contract_stems:
        return True

    # game_lib.h collects the library replacements: each file of game/common/lib is one.
    if path.startswith("port/game/common/lib/"):
        return True

    # Tests stand in for the game's C with C functions of their own.
    return "tests" in parts


def is_program_or_test(path):
    """True for a test or a program's main file, which may write to stdout and stderr."""
    parts = path.split("/")
    name = parts[-1]
    return "tests" in parts or name.startswith("main.") or Path(name).stem.endswith("_main")


def check_every_file(path, text):
    """The rules that hold for every file in scope, old or new."""
    findings = []
    lines = text.splitlines()

    if not lines or not SPDX.match(lines[0]):
        findings.append(Finding(path, 1, "licence", "the first line is the SPDX licence line"))

    for number, line in enumerate(lines, 1):
        if "\t" in line:
            findings.append(Finding(path, number, "tab", "indent with spaces, never a tab"))
        if DASHES.search(line):
            findings.append(
                Finding(path, number, "dash", "no em or en dash: a comma, colon or parentheses")
            )

    return findings


def getenv_rule_applies(path):
    """True when the getenv rule covers `path`: not a reader of the table, nor vendored code."""
    if path in GETENV_READERS:
        return False

    # third_party.c and the like compile vendored libraries, which read what they read.
    return not any(Path(part).stem == "third_party" for part in path.split("/"))


def getenv_reads(path, text):
    """The getenv calls of `text` that read an OPENRAC_ variable, or a name that is not a literal.

    The call is found in the code with comments and literals blanked to spaces, and its literal
    argument read from `text` at the same offset.
    """
    # C reaches the table through its contract (CODING_CONVENTIONS.md, section 14.3).
    door = "common/env.h" if is_cpp(path, text) else "openrac/host_env.h"

    findings = []
    code = strip_comments_and_strings(text, keep_columns=True)
    for call in GETENV_CALL.finditer(code):
        line = code.count("\n", 0, call.start()) + 1
        start = call.end()

        # A literal name: only the port's own variables are the table's.
        if text.startswith('"', start):
            end = text.find('"', start + 1)
            name = text[start + 1 : end if end >= 0 else len(text)]
            if name.startswith("OPENRAC_"):
                findings.append(Finding(path, line, "getenv", f"read {name} through {door}"))
            continue

        # A computed name may be one of the port's variables, which only the table reads.
        message = f"a name that is not a literal: OPENRAC_ variables through {door}"
        findings.append(Finding(path, line, "getenv", message))
    return findings


def check_getenv(texts, exceptions):
    """The getenv rule over every file of `texts` (path to text), and the stale `exceptions`.

    A file `exceptions` names keeps its reads; an entry whose file reads none (or is gone) is an
    advisory, so that the work that moved the reads removes it.
    """
    findings = []
    excused = set()
    for path, text in sorted(texts.items()):
        if not getenv_rule_applies(path) or is_generated(text):
            continue
        reads = getenv_reads(path, text)

        # A known exception keeps its reads until its package moves them.
        if path in exceptions:
            if reads:
                excused.add(path)
            continue

        findings += reads

    for path in sorted(set(exceptions) - excused):
        message = "stale: it reads no OPENRAC_ variable now, drop its GETENV_EXCEPTIONS entry"
        findings.append(Finding(path, 1, "getenv", message, gating=False))
    return findings


def check_new_cpp(path, text, contract_stems):
    """The rules for a C++ file that did not exist at the base of the change."""
    findings = []
    lines = text.splitlines()

    # The licence block is two `//` lines (CODING_CONVENTIONS.md, "The project's constraints").
    if len(lines) < 2 or not CPP_LICENCE.match(lines[0]) or not CPP_COPYRIGHT.match(lines[1]):
        findings.append(
            Finding(path, 1, "licence", "the first two lines are the `//` SPDX and copyright lines")
        )

    code = strip_comments_and_strings(text).splitlines()
    header = path.endswith(HEADER_SUFFIXES)
    contract_ok = may_define_c_contract(path, contract_stems)
    output_ok = is_program_or_test(path)
    allocation_ok = path in ALLOCATION_EXCEPTIONS

    for number, line in enumerate(code, 1):
        if STATIC_AT_NAMESPACE.match(line):
            findings.append(
                Finding(path, number, "static", "file-local code goes in an anonymous namespace")
            )
        if not allocation_ok and (RAW_NEW.search(line) or RAW_DELETE.search(line)):
            findings.append(
                Finding(path, number, "new-delete", "own it: std::make_unique or a container")
            )
        if not allocation_ok and C_ALLOCATION.search(line):
            findings.append(
                Finding(path, number, "malloc-free", "own it: a container or an RAII wrapper")
            )
        if header and USING_NAMESPACE.match(line):
            findings.append(Finding(path, number, "using-namespace", "never in a header"))
        if not contract_ok and EXTERN_C.search(line):
            findings.append(
                Finding(path, number, "extern-c", 'extern "C" only in the file named for its contract')
            )
        if not output_ok and CONSOLE_OUTPUT.search(line):
            findings.append(
                Finding(path, number, "logging", "write through common/log.h, not to the console")
            )

    return findings


def check_new_c(path, text):
    """The advisories for a C file that did not exist at the base of the change."""
    lines = text.splitlines()
    if len(lines) >= 2 and COPYRIGHT.match(lines[1]):
        return []
    return [
        Finding(
            path, 2, "licence", "the second line is the copyright line", gating=False
        )
    ]


def git(root, *args):
    """Runs git in `root` and returns its output, a line per entry."""
    result = subprocess.run(
        ["git", *args], cwd=root, check=True, capture_output=True, text=True
    )
    return [line for line in result.stdout.splitlines() if line]


def tracked_files(root):
    """Every file git tracks under port/, and the ones not yet added that it does not ignore."""
    tracked = git(root, "ls-files", "--", "port")
    untracked = git(root, "ls-files", "--others", "--exclude-standard", "--", "port")
    return sorted(set(tracked) | set(untracked))


def added_files(root, since):
    """The files under port/ that exist now and did not at commit `since`."""
    added = git(root, "diff", "--name-only", "--diff-filter=A", since, "--", "port")
    untracked = git(root, "ls-files", "--others", "--exclude-standard", "--", "port")
    return set(added) | set(untracked)


def contract_stems(root):
    """The names of the C contract headers, without `.h`.

    The contract directories also hold C++ headers of the runtime (memory.h, elf.h); those open
    a namespace and are not contracts.
    """
    stems = set()
    for directory in CONTRACT_DIRS:
        folder = root / directory
        if not folder.is_dir():
            continue
        for entry in folder.glob("*.h"):
            if not opens_namespace(entry.read_text(encoding="utf-8", errors="replace")):
                stems.add(entry.stem)
    return stems


def find_clang_format(wanted):
    """The clang-format to run: `wanted`, or clang-format on PATH, or Xcode's; None if none."""
    if wanted:
        found = shutil.which(wanted)
        return [found] if found else None
    found = shutil.which("clang-format")
    if found:
        return [found]

    # Xcode's clang-format is not on PATH; git-clang-format needs its full path.
    if sys.platform == "darwin" and shutil.which("xcrun"):
        result = subprocess.run(["xcrun", "--find", "clang-format"], capture_output=True, text=True)
        if result.returncode == 0 and result.stdout.strip():
            return [result.stdout.strip()]
    return None


def clang_format_findings(root, clang_format, paths, gating):
    """Runs clang-format's own check on `paths`; one finding per file it would change."""
    findings = []
    for path in paths:
        result = subprocess.run(
            [*clang_format, "--dry-run", "-Werror", path],
            cwd=root,
            capture_output=True,
            text=True,
        )
        if result.returncode != 0:
            first = re.search(r":(\d+):\d+: (?:error|warning)", result.stderr)
            line = int(first.group(1)) if first else 1
            findings.append(
                Finding(path, line, "format", "run clang-format -i on it", gating=gating)
            )
    return findings


def touched_line_findings(root, clang_format, since, paths, gating):
    """Reports what git-clang-format would change on the lines changed since `since`."""
    if not paths:
        return []

    # git-clang-format ships beside clang-format (LLVM, Homebrew's llvm, PyPI's clang-format).
    binary = clang_format[-1]
    tool = shutil.which("git-clang-format") or str(Path(binary).with_name("git-clang-format"))
    if not Path(tool).exists():
        print("touched lines: git-clang-format not found, skipped", file=sys.stderr)
        return []

    command = [tool, "--binary", binary, "--diff", "--quiet", since, "--"] + sorted(paths)
    result = subprocess.run(command, cwd=root, capture_output=True, text=True)

    # It exits 1 when it has changes to show; anything else is its own failure.
    if result.returncode not in (0, 1):
        print(f"touched lines: git-clang-format failed: {result.stderr.strip()}", file=sys.stderr)
        return []

    findings = []
    current = None
    for line in result.stdout.splitlines():
        if line.startswith("+++ b/"):
            current = line[len("+++ b/") :]
        elif line.startswith("@@") and current:
            start = re.search(r"\+(\d+)", line)
            findings.append(
                Finding(
                    current,
                    int(start.group(1)) if start else 1,
                    "format",
                    "clang-format would change this changed line",
                    gating=gating,
                )
            )
    return findings


def run(
    root,
    since=None,
    touched=False,
    strict_touched=False,
    clang_format=None,
    getenv_exceptions=GETENV_EXCEPTIONS,
):
    """Checks the tree at `root` and returns the findings.

    `getenv_exceptions` maps the files that may still read OPENRAC_ variables with getenv to why.
    """
    files = [path for path in tracked_files(root) if in_scope(path)]
    texts = {}
    for path in files:
        full = root / path

        # A file git still tracks but the working tree has deleted is not checked.
        if full.is_file():
            texts[path] = full.read_text(encoding="utf-8", errors="replace")

    findings = []
    for path, text in texts.items():
        if not is_generated(text):
            findings += check_every_file(path, text)
    findings += check_getenv(texts, getenv_exceptions)

    if since is None:
        return findings

    stems = contract_stems(root)
    added = {path for path in added_files(root, since) if path in texts}
    added = {path for path in added if not is_generated(texts[path])}
    new_cpp = sorted(path for path in added if is_cpp(path, texts[path]))
    new_c = sorted(path for path in added if path not in new_cpp)

    for path in new_cpp:
        findings += check_new_cpp(path, texts[path], stems)
    for path in new_c:
        findings += check_new_c(path, texts[path])

    tool = find_clang_format(clang_format)
    if tool is None:
        print("clang-format not found: the format rules were not checked", file=sys.stderr)
        return findings

    findings += clang_format_findings(root, tool, new_cpp, gating=True)
    findings += clang_format_findings(root, tool, new_c, gating=False)

    # Lines changed in files that existed before: advisory until --strict-touched.
    if touched:
        changed = git(root, "diff", "--name-only", "--diff-filter=M", since, "--", "port")
        changed = [path for path in changed if path in texts and not is_generated(texts[path])]
        findings += touched_line_findings(root, tool, since, changed, gating=strict_touched)

    return findings


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0].strip())
    parser.add_argument("--since", metavar="REF", help="apply the new-code rules to files added since REF")
    parser.add_argument("--touched", action="store_true", help="report formatting on lines changed since REF")
    parser.add_argument("--strict-touched", action="store_true", help="fail on those, too")
    parser.add_argument("--clang-format", metavar="BIN", help="the clang-format to run")
    args = parser.parse_args(argv)

    if (args.touched or args.strict_touched) and not args.since:
        parser.error("--touched needs --since")

    here = Path(__file__).resolve().parent
    root = Path(git(here, "rev-parse", "--show-toplevel")[0])
    os.chdir(root)

    findings = run(
        root,
        since=args.since,
        touched=args.touched or args.strict_touched,
        strict_touched=args.strict_touched,
        clang_format=args.clang_format,
    )
    for finding in sorted(findings, key=lambda f: (f.path, f.line, f.rule)):
        print(finding)

    failed = [finding for finding in findings if finding.gating]
    advisories = len(findings) - len(failed)
    print(
        f"conventions: {len(failed)} to fix, {advisories} advisory "
        "(port/docs/CODING_CONVENTIONS.md)",
        file=sys.stderr,
    )
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
