#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Checks the port's includes against the module layering of port/docs/CODING_CONVENTIONS.md
(section 18).

  layering.py                   check port/ against rules.tsv; exit 1 on a new violation
  layering.py --edges           and print every edge between two modules, with its count
  layering.py --root DIR --rules FILE
                                check another tree against other rules (the tests do)

Every C and C++ file under the root is read, except the directories rules.tsv skips (build
trees, a game's hand C) and generated files (a "Do not edit" in their first two lines). Each
`#include "<dir>/..."` line is resolved as the compiler resolves it: the including file's own
directory first, then the search directories of rules.tsv. The including file and the included
file are each mapped to a module. A module may include its own files and those of any module on
a lower layer; a module on the same layer only when an `allow` row names the edge; a module on a
higher layer never.

Any other include is a violation and fails the check, unless rules.tsv lists it as a known
exception. A listed exception that no longer occurs, or has moved to another line, is a warning,
so the list stays exact and only ever shrinks. Prints `path:line: layering: message`; exits 0
when nothing is to fix (warnings included), 1 on a violation, 2 when rules.tsv is wrong.
"""
import argparse
import os
import re
import sys
from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path

SOURCE_SUFFIXES = (".c", ".cc", ".cpp", ".h", ".hpp", ".inc", ".inl", ".mm")

# A quoted include; an angle-bracket include is a system or third-party header.
INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"]+)"')

HERE = Path(__file__).resolve().parent
DEFAULT_ROOT = HERE.parent.parent
DEFAULT_RULES = HERE / "rules.tsv"


class RulesError(Exception):
    """rules.tsv is malformed, contradicts itself or misses a module; the message says where."""


@dataclass(frozen=True)
class Pattern:
    """A path prefix or a module name, in which a `*` segment stands for any one path segment."""

    text: str

    def match(self, value, whole):
        """The match of `value` (all of it when `whole`, else a prefix of it), or None."""
        parts = ("([^/]+)" if part == "*" else re.escape(part) for part in self.text.split("/"))
        return re.match("/".join(parts) + ("$" if whole else ""), value)


@dataclass(frozen=True)
class ModuleRow:
    """Paths that start with `prefix` belong to module `name`; a `*` in it takes the segment."""

    prefix: Pattern
    name: str

    def module_of(self, path):
        found = self.prefix.match(path, whole=False)
        if found is None:
            return None
        return self.name.replace("*", found.group(1)) if "*" in self.name else self.name


@dataclass
class KnownViolation:
    """An include that breaks the layers on purpose until the package named in `reason` lands."""

    path: str
    line: int
    include: str
    reason: str
    row: int
    seen_at: int = 0


@dataclass
class Rules:
    """rules.tsv, parsed: where includes are found, what is skipped, modules, layers, exceptions."""

    source: str
    search: list = field(default_factory=list)
    skip: list = field(default_factory=list)
    modules: list = field(default_factory=list)
    layers: list = field(default_factory=list)
    allow: list = field(default_factory=list)
    known: dict = field(default_factory=dict)

    def module_of(self, path):
        """The module a root-relative `path` belongs to, or None; the first matching row wins."""
        for row in self.modules:
            name = row.module_of(path)
            if name is not None:
                return name
        return None

    def skips(self, path):
        """True when the directory `path` (root-relative, ending in `/`) is not read."""
        return any(pattern.match(path, whole=False) for pattern in self.skip)

    def rank(self, module):
        """The rank of `module`'s layer: a row naming it exactly first, then a `*` row."""
        for exact in (True, False):
            for rank, pattern in self.layers:
                if ("*" not in pattern.text) == exact and pattern.match(module, whole=True):
                    return rank
        raise RulesError(f"{self.source}: no layer row names the module {module}")

    def allows(self, source, target):
        """True when module `source` may include module `target`."""
        source_rank, target_rank = self.rank(source), self.rank(target)
        if source == target or target_rank < source_rank:
            return True
        if target_rank > source_rank:
            return False
        return any(
            from_.match(source, whole=True) and to.match(target, whole=True) for from_, to in self.allow
        )


def load_rules(path):
    """Reads rules.tsv: tab-separated rows, a line starting with `#` is a comment."""
    rules = Rules(source=Path(path).name)
    for number, line in enumerate(Path(path).read_text(encoding="utf-8").splitlines(), start=1):
        if not line.strip() or line.startswith("#"):
            continue
        kind, *values = line.split("\t")
        where = f"{rules.source}:{number}"
        arity = {"search": 1, "skip": 1, "module": 2, "layer": 2, "allow": 2, "except": 3}.get(kind)
        if arity is None:
            raise RulesError(f"{where}: unknown row `{kind}`")
        if len(values) != arity or not all(value.strip() for value in values):
            raise RulesError(f"{where}: `{kind}` takes {arity} tab-separated value(s)")

        if kind == "search":
            rules.search.append(values[0])
        elif kind == "skip":
            rules.skip.append(Pattern(values[0]))
        elif kind == "module":
            prefix, name = values
            if prefix.count("*") > 1 or name.count("*") > ("*" in prefix):
                raise RulesError(f"{where}: a module row has one `*` at most, in the prefix first")
            rules.modules.append(ModuleRow(Pattern(prefix), name))
        elif kind == "layer":
            if not values[0].isdigit():
                raise RulesError(f"{where}: a layer's rank is a number")
            rules.layers.extend((int(values[0]), Pattern(name)) for name in values[1].split())
        elif kind == "allow":
            rules.allow.extend((Pattern(values[0]), Pattern(name)) for name in values[1].split())
        else:
            location, include, reason = values
            file, _, line_number = location.rpartition(":")
            if not file or not line_number.isdigit():
                raise RulesError(f"{where}: an exception starts with file:line")
            if (file, include) in rules.known:
                raise RulesError(f"{where}: {file} and {include} are listed twice")
            rules.known[(file, include)] = KnownViolation(file, int(line_number), include, reason, number)

    for from_, to in rules.allow:
        if rules.rank(to.text) > rules.rank(from_.text):
            raise RulesError(
                f"{rules.source}: allow {from_.text} -> {to.text} points up a layer; "
                "list each include as an exception instead"
            )
    return rules


def is_generated(text):
    """True when the file says in its first two lines that a tool writes it."""
    head = "\n".join(text.splitlines()[:2])
    return "Do not edit" in head or "do not edit" in head


def sources(root, rules):
    """Every C and C++ file under `root` that is read, as sorted root-relative paths."""
    found = []
    for directory, subdirectories, files in os.walk(root):
        relative = Path(directory).relative_to(root).as_posix()
        relative = "" if relative == "." else relative + "/"
        subdirectories[:] = [
            name
            for name in subdirectories
            if not name.startswith(".") and not rules.skips(f"{relative}{name}/")
        ]
        found.extend(relative + name for name in files if name.endswith(SOURCE_SUFFIXES))
    return sorted(found)


def resolve(root, including, include, rules):
    """The root-relative path the compiler would open for `include` in the file `including`."""
    for directory in [os.path.dirname(including)] + rules.search:
        path = os.path.normpath(os.path.join(directory, include)).replace(os.sep, "/")
        if (root / path).is_file():
            return path
    # Not in the tree, as a header the build generates: the include names it from the root.
    return os.path.normpath(include).replace(os.sep, "/")


@dataclass(frozen=True)
class Edge:
    """One include line, from a file of module `source` to a file of module `target`."""

    path: str
    line: int
    include: str
    source: str
    target: str


@dataclass
class Report:
    """What a run found: every edge, the violations, and the warnings about rules.tsv."""

    edges: list = field(default_factory=list)
    violations: list = field(default_factory=list)
    warnings: list = field(default_factory=list)


def check(root, rules):
    """Reads every source under `root` and sorts its includes into allowed, known and violating."""
    root = Path(root)
    report = Report()

    for path in sources(root, rules):
        text = (root / path).read_text(encoding="utf-8", errors="replace")
        if is_generated(text):
            continue
        source = rules.module_of(path)
        if source is None:
            report.violations.append(f"{path}:1: layering: no module row of {rules.source} covers this file")
            continue
        rules.rank(source)
        for number, line in enumerate(text.splitlines(), start=1):
            found = INCLUDE.match(line)
            if found is None or "/" not in found.group(1):
                continue
            include = found.group(1)
            target = rules.module_of(resolve(root, path, include, rules))
            if target is None:
                continue  # Outside every module: a third-party header.
            report.edges.append(Edge(path, number, include, source, target))
            if rules.allows(source, target):
                continue
            known = rules.known.get((path, include))
            if known is not None:
                known.seen_at = number
                continue
            report.violations.append(
                f'{path}:{number}: layering: {source} must not include {target} (#include "{include}")'
            )

    for known in rules.known.values():
        where = f"{rules.source}:{known.row}: layering (warning): {known.path}:{known.line}"
        if not known.seen_at:
            message = f'no longer breaks the layers with "{known.include}"; remove the row'
        elif known.seen_at != known.line:
            message = f'"{known.include}" is now at line {known.seen_at}; update the row'
        else:
            continue
        report.warnings.append(f"{where}: {message}")
    return report


def edge_table(rules, edges):
    """Every edge between two different modules with its include count, marked when not allowed."""
    counts = Counter((edge.source, edge.target) for edge in edges if edge.source != edge.target)
    lines = []
    for (source, target), count in sorted(counts.items()):
        mark = "" if rules.allows(source, target) else "  (against the layers)"
        lines.append(f"{source} -> {target}: {count}{mark}")
    return lines


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0].strip())
    parser.add_argument("--root", type=Path, default=DEFAULT_ROOT, help="the tree to check (port/)")
    parser.add_argument("--rules", type=Path, default=DEFAULT_RULES, help="its rules (rules.tsv here)")
    parser.add_argument("--edges", action="store_true", help="print each module edge and its count")
    args = parser.parse_args(argv)

    try:
        rules = load_rules(args.rules)
        report = check(args.root.resolve(), rules)
    except RulesError as error:
        print(f"layering: {error}", file=sys.stderr)
        return 2

    if args.edges:
        print("\n".join(edge_table(rules, report.edges)))
    for line in report.violations + report.warnings:
        print(line)
    sys.stdout.flush()
    print(
        f"layering: {len(report.violations)} to fix, {len(report.warnings)} warning(s), "
        f"{len(report.edges)} includes read (port/docs/CODING_CONVENTIONS.md, section 18)",
        file=sys.stderr,
    )
    return 1 if report.violations else 0


if __name__ == "__main__":
    sys.exit(main())
