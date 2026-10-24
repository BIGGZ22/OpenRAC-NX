# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""A game's profile (profiles/<game>-<version>.json) and what it finds in that game's checkout: a
function's C, its retail assembly, and the commands that compile and check one candidate.

A profile is data. Its keys:

  name        "rac3/pal": what the command line takes
  checkout    [game, version] for openrac_common.games.source_checkout: where the sister project
              is ($OPENRAC_SOURCE_<GAME>_<VERSION>, else game.json's "checkout")
  env         variables the commands need, used only where the environment does not set them
  python      interpreters to try for the project's own tools, the first that exists wins
  sources     globs of the C files that may hold the function (its definition or INCLUDE_ASM)
  asm         globs of the splat .s that may hold it; word_order "le-bytes" or "word" (score.py)
  cpp         the host preprocessor command that makes base.c readable by decomp-permuter
  compile     steps (argv lists) that turn one candidate into {out}, an object holding the function
  check       the project's own checker for a candidate; exact is the regex its output must match
              (search, multiline) for EXACT. Without check, exact is matched against compile's output
  weights     decomp-permuter randomization weights for this game, on top of its defaults
  flags       a command printing the compiler flags of {source}, the file the function is in, once
              per run: the project's own rule, never a copy of it; its output is {flags}
  start       commands run once before a run (a container for the compiler); each idempotent
  shims       {command: program}: small scripts put first on PATH (a `wine` that runs wibo)
  compile_may_fail  run every compile step even when one fails (a checker that exits 1 on a miss)
  check_in_place    the checker reads only the project's own source: the candidate is put in the
              source file for the length of the check, under a lock, and the file restored after
  prelude     text a file starts with when it does not have it already (the `#include "common.h"`
              a checker adds to a standalone draft)
  rename      when retail names the function otherwise (another release's twin): {"table": a .tsv
              in the checkout, "from": column, "to": column, "where": {column: value}}, or {"sub": [regex,
              replacement]} for a rule on the name itself
  notes       free text: what the profile stands on, for the README

Placeholders in commands: {func}, {snippet} (the candidate as the checker takes it: the draft's
declarations, then the function), {decls}, {out}, {tmp} (a directory of the worker's own),
{python}, {checkout}, {cache} (upstream.cache_dir(), where the workers' directories are), {source} and {srcdir} (the function's file and its folder), {spliced}
(a copy of that file with the candidate in place, in {tmp}), {flags}, the variables of env by name ({UYA_TARGET}), and ~ for the home folder. An argument that is
only {flags} becomes one argument per flag;
sources and asm take {func} and the variables too.
"""
from __future__ import annotations

import json
import os
import re
import shlex
import shutil
import subprocess
from dataclasses import dataclass, field
from pathlib import Path

from openrac_common import games, tables

PROFILES = Path(__file__).resolve().parent / "profiles"


@dataclass
class Profile:
    name: str
    checkout_of: tuple[str, str]
    compile: list[list[str]]
    exact: str
    sources: list[str]
    asm: list[str]
    cpp: list[str] = field(default_factory=lambda: ["gcc", "-E", "-P"])
    word_order: str = "le-bytes"
    check: list[str] | None = None
    env: dict[str, str] = field(default_factory=dict)
    python: list[str] = field(default_factory=lambda: ["python3"])
    weights: dict[str, float] = field(default_factory=dict)
    flags: list[str] | None = None
    start: list[list[str]] = field(default_factory=list)
    shims: dict[str, str] = field(default_factory=dict)
    compile_may_fail: bool = False
    check_in_place: bool = False
    prelude: str = ""
    rename: dict | None = None
    notes: str = ""
    root: Path | None = None

    @property
    def checkout(self) -> Path:
        if self.root is None:
            path = games.source_checkout(*self.checkout_of)
            if path is None:
                raise LookupError(f"{self.name}: game.json names no source project for {'/'.join(self.checkout_of)}")
            self.root = path
        return self.root

    def interpreter(self) -> str:
        for candidate in self.python:
            path = Path(os.path.expanduser(candidate))
            if path.is_file() or shutil.which(candidate):
                return str(path) if path.is_file() else candidate
        return "python3"

    def environment(self) -> dict[str, str]:
        env = dict(os.environ)
        for name, value in self.env.items():
            env.setdefault(name, os.path.expanduser(value))
        if self.shims:
            from . import upstream
            shims = upstream.cache_dir() / "shims" / self.name.replace("/", "-")
            shims.mkdir(parents=True, exist_ok=True)
            for command, target in self.shims.items():
                path = shims / command
                text = f'#!/bin/sh\nexec {shlex.quote(os.path.expanduser(target))} "$@"\n'
                if not path.is_file() or path.read_text() != text:
                    path.write_text(text)
                    path.chmod(0o755)
            env["PATH"] = f"{shims}{os.pathsep}{env.get('PATH', '')}"
        return env

    def expand(self, argv: list[str], values: dict[str, str]) -> list[str]:
        """ARGV with its placeholders filled: VALUES, {python}, {checkout}, and the variables of
        environment() by name ({UYA_TARGET})."""
        from . import upstream
        values = {**self.environment(), "python": self.interpreter(), "checkout": str(self.checkout),
                  "cache": str(upstream.cache_dir()), **values}
        out = []
        for a in argv:
            if a == "{flags}":   # alone, the flags are several arguments
                out.extend(shlex.split(values.get("flags", "")))
            else:
                out.append(os.path.expanduser(a.format(**values)))
        return out

    def globs(self, patterns: list[str], func: str) -> list[Path]:
        out = []
        for pattern in self.expand(patterns, {"func": func}):
            out.extend(sorted(self.checkout.glob(pattern)))
        return out

    def exact_re(self, func: str) -> re.Pattern:
        return re.compile(self.exact.replace("{func}", re.escape(func)), re.M)


def names() -> list[str]:
    return sorted(load_file(p).name for p in PROFILES.glob("*.json"))


def load_file(path: Path) -> Profile:
    data = json.loads(Path(path).read_text())
    data["checkout_of"] = tuple(data.pop("checkout"))
    data.pop("about", None)
    return Profile(**data)


def load(name: str) -> Profile:
    """The profile called NAME ("rac4/ntsc", or its file stem "rac4-ntsc"). Raises KeyError."""
    for path in PROFILES.glob("*.json"):
        profile = load_file(path)
        if name in (profile.name, path.stem):
            return profile
    raise KeyError(f"no permuter profile {name!r}; there are: {', '.join(names())}")


def definition(text: str, func: str) -> tuple[int, int] | None:
    """(start, end) of FUNC's definition in C TEXT, from the start of its line to its closing
    brace, or None. A prototype or a call is not a definition."""
    pattern = re.compile(r"^(?!\s*(?:extern|return)\b)[A-Za-z_][^;{}()\n]*?\**\s*\b" + re.escape(func)
                         + r"\s*\([^;{}]*\)\s*(?:__attribute__\s*\(\(.*?\)\)\s*)?\{", re.M | re.S)
    for m in pattern.finditer(text):
        depth, i = 0, m.end() - 1
        while i < len(text):
            depth += {"{": 1, "}": -1}.get(text[i], 0)
            if depth == 0:
                return m.start(), i + 1
            i += 1
    return None


def stub(text: str, func: str) -> tuple[int, int] | None:
    """(start, end) of the INCLUDE_ASM (or GLOBAL_ASM pragma) line that stands for FUNC, or None."""
    m = re.search(r"^[ \t]*(?:INCLUDE_ASM|INCLUDE_RODATA|#pragma GLOBAL_ASM)\b[^\n]*\b" + re.escape(func)
                  + r"\b[^\n]*$", text, re.M)
    return (m.start(), m.end()) if m else None


def find_source(profile: Profile, func: str) -> Path:
    """The C file that defines FUNC or includes its assembly. Raises FileNotFoundError."""
    hits = []
    for path in profile.globs(profile.sources, func):
        text = path.read_text(errors="replace")
        if func in text and (definition(text, func) or stub(text, func)):
            hits.append(path)
    if not hits:
        raise FileNotFoundError(f"{func}: no file in {', '.join(profile.sources)} defines it or includes its asm")
    # a file under src/ beats a draft elsewhere (its context is the build's), then a definition a stub
    hits.sort(key=lambda p: ("src" not in p.relative_to(profile.checkout).parts,
                             definition(p.read_text(errors="replace"), func) is None))
    return hits[0]


def retail_name(profile: Profile, func: str) -> str:
    """What the retail assembly calls FUNC: FUNC, or its row's name in the profile's rename table.
    Raises KeyError when the table has no row for it."""
    if not profile.rename:
        return func
    r = profile.rename
    if "sub" in r:
        return re.sub(r["sub"][0], r["sub"][1], func)
    for row in tables.read_tsv(profile.checkout / r["table"]):
        if len(row) > max(r["from"], r["to"]) and row[r["from"]] == func and all(
                row[int(c)] == profile.expand([v], {})[0] for c, v in r.get("where", {}).items()):
            return row[r["to"]]
    raise KeyError(f"{func} has no row in {r['table']}")


def find_asm(profile: Profile, func: str) -> Path:
    """The .s holding FUNC's retail instructions (FUNC as retail_name gives it). Raises
    FileNotFoundError."""
    for path in profile.globs(profile.asm, func):
        if func in path.name or re.search(r"\b(?:[ga]label|nonmatching)\s+" + re.escape(func) + r"\b",
                                          path.read_text(errors="replace")):
            return path
    raise FileNotFoundError(f"{func}: no assembly in {', '.join(profile.asm)} (is the project's asm set up?)")


def split_draft(text: str, func: str) -> tuple[str, str]:
    """(declarations, function) of a draft: everything before FUNC's definition, and the definition.
    Raises ValueError when the draft does not define FUNC."""
    span = definition(text, func)
    if not span:
        raise ValueError(f"the draft has no definition of {func}")
    return text[:span[0]], text[span[0]:span[1]]


def strip_top_level_asm(text: str) -> str:
    """TEXT without what pycparser cannot read at the top level: `__asm__(...);` statements, and the
    `__asm__("name")` label of a declaration (the declaration stays)."""
    out, i, depth = [], 0, 0
    pattern = re.compile(r"\b(?:__asm__|asm)\b(?:\s+(?:__volatile__|volatile))?\s*\(")
    while i < len(text):
        m = pattern.search(text, i)
        if not m:
            out.append(text[i:])
            break
        before = text[i:m.start()]
        depth += before.count("{") - before.count("}")
        out.append(before)
        j, level = m.end(), 1
        while j < len(text) and level:
            level += {"(": 1, ")": -1}.get(text[j], 0)
            j += 1
        if depth:
            out.append(text[m.start():j])
        elif not "".join(out).rstrip() or "".join(out).rstrip()[-1] in ";}":
            while j < len(text) and text[j] in " \t\n;":   # a statement of its own: drop it whole
                j += 1
                if text[j - 1] == ";":
                    break
        i = j
    return "".join(out)


def base_source(profile: Profile, func: str, draft: str | None, strip_other_fns) -> tuple[str, str]:
    """(base.c, the draft's declarations): FUNC's source file with the draft in FUNC's place,
    preprocessed by the profile's cpp with what pycparser cannot read defined away, every other
    function reduced to its prototype (decomp-permuter's strip_other_fns). Without a draft, the
    function as the file has it. Raises RuntimeError when the preprocessor fails."""
    path = find_source(profile, func)
    text = path.read_text(errors="replace")
    span = definition(text, func) or stub(text, func)
    decls = ""
    if draft is not None:
        decls, _ = split_draft(draft, func)
        text = text[:span[0]] + draft + text[span[1]:]
    if profile.prelude and profile.prelude.strip() not in text:
        text = profile.prelude + text
    cmd = profile.expand(profile.cpp, {"func": func, "source": str(path), "srcdir": str(path.parent)}) + [
        "-D__attribute__(x)=", "-D__extension__=", "-D__inline__=inline", "-D__restrict=", "-"]
    done = subprocess.run(cmd, input=text, capture_output=True, text=True, cwd=path.parent,
                          env=profile.environment())
    if done.returncode:
        raise RuntimeError(f"preprocessing {path} failed:\n{done.stderr[-3000:]}")
    body = strip_top_level_asm(done.stdout)
    return strip_other_fns(body, func), decls


def function_text(source: str, func: str) -> str:
    """FUNC's definition in a candidate decomp-permuter wrote. Raises ValueError."""
    span = definition(source, func)
    if not span:
        raise ValueError(f"the candidate has no definition of {func}")
    return source[span[0]:span[1]]
