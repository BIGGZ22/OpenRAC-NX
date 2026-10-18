#!/usr/bin/env python3
"""Per-source compiler, assembler and cc1 flags measured against the retail boot.

``config/cflags.txt`` holds one entry per line: the repository-relative path of
a campaign C source, optional selectors, then the flags measured for that file.
A source without an entry compiles with the profile's global flags and its own
chain, so its pinned object stays valid without requalification.

Two selectors may precede the flags, each at most once and each a bare token in
the first flag position. A plain token names one of the compilers the boot
profile pins besides its own chain; ``as=NAME`` names one of the assemblers it
pins besides the pinned GNU ``as``. An entry without a compiler selector extends
the pinned chain's global flags; an entry with one carries the complete flag set
for the selected compiler, so a reader never has to combine two places to know
how that file was built. An entry must name an existing C source of a repository
compiler campaign and must carry either a flag or an assembler selector, so a
mistyped path, an unknown selector, a duplicate or a flag on a file no gate
compiles fails instead of being ignored silently.
"""

from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parent.parent
CFLAGS = ROOT / "config/cflags.txt"
PROFILE = ROOT / "config/boot/compiler_trial.json"
# C sources belong to a campaign when a gate compiles them or when they are
# staged for review in one of these directories.
CAMPAIGN_DIRS = ("src/boot", "nonmatching/boot", "src/levels", "nonmatching/levels")


@dataclass(frozen=True)
class Entry:
    """One ``config/cflags.txt`` line: which toolchain, which flags for that file."""

    compiler: str
    flags: tuple[str, ...]
    assembler: str = "pinned"

    @property
    def pinned(self) -> bool:
        """True when the entry asks for the profile's own compiler chain."""
        return self.compiler == DEFAULT_COMPILER

    @property
    def assembler_pinned(self) -> bool:
        """True when the entry asks for the profile's own GNU assembler."""
        return self.assembler == DEFAULT_ASSEMBLER


# The profile's own chain. A source with no entry, and an entry with no
# selector, both mean this compiler and this assembler.
DEFAULT_COMPILER = "pinned"
DEFAULT_ASSEMBLER = "pinned"
DEFAULT_ENTRY = Entry(DEFAULT_COMPILER, ())


def campaign_sources() -> frozenset[str]:
    """Repository-relative C sources the campaigns compile or stage."""
    from boot_c_campaign import REPLACEMENTS, source_path

    names = {source_path(item.name).relative_to(ROOT).as_posix()
             for item in REPLACEMENTS}
    level = json.loads((ROOT / "config/levels/compiler_trial.json")
                       .read_text(encoding="utf-8"))
    for row in (level["accepted"], level["additional_accepted"],
                level["frontend_accepted"],
                *level["provisional_controls"]):
        names.add(row["source"])
    for folder in CAMPAIGN_DIRS:
        base = ROOT / folder
        if base.is_dir():
            names.update(path.relative_to(ROOT).as_posix()
                         for path in base.glob("*.c"))
    return frozenset(names)


def pinned_compilers() -> tuple[str, ...]:
    """Every compiler selector a line may name: the profile chain and its extras."""
    profile = json.loads(PROFILE.read_text(encoding="utf-8"))
    return (DEFAULT_COMPILER, *sorted(profile.get("extra_compilers", {})))


def pinned_assemblers() -> tuple[str, ...]:
    """Every assembler selector a line may name: the pinned ``as`` and its extras."""
    profile = json.loads(PROFILE.read_text(encoding="utf-8"))
    return (DEFAULT_ASSEMBLER, *sorted(profile.get("extra_assemblers", {})))


def load(path: Path = CFLAGS) -> dict[str, Entry]:
    """Parse and validate the per-source compiler/flag file."""
    known = campaign_sources()
    selectors = pinned_compilers()
    assemblers = pinned_assemblers()
    entries: dict[str, Entry] = {}
    problems: list[str] = []
    for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        entry = line.split("#", 1)[0].strip()
        if not entry:
            continue
        fields = entry.split()
        name, rest = fields[0].replace("\\", "/"), fields[1:]
        if name in entries:
            problems.append(f"line {number}: {name}: duplicate entry")
            continue
        if (PurePosixPath(name).is_absolute() or Path(name).is_absolute()
                or ".." in PurePosixPath(name).parts
                or not name.endswith(".c")):
            problems.append(f"line {number}: {name}: not a repository-relative C source")
            continue
        if not (ROOT / name).is_file():
            problems.append(f"line {number}: {name}: source file does not exist")
            continue
        if name not in known:
            problems.append(f"line {number}: {name}: not a campaign C source")
            continue
        # Leading bare tokens are selectors -- "as=NAME" names an assembler, any
        # other one names a compiler -- and flags always start with "-".
        compiler, assembler = DEFAULT_COMPILER, DEFAULT_ASSEMBLER
        problem = ""
        while rest and not rest[0].startswith("-"):
            token, rest = rest[0], rest[1:]
            if token.startswith("as="):
                chosen = token[len("as="):]
                if not chosen:
                    problem = "empty assembler selector"
                elif assembler != DEFAULT_ASSEMBLER:
                    problem = f"duplicate assembler selector {chosen}"
                elif chosen not in assemblers:
                    problem = (f"unknown assembler selector {chosen}; pinned "
                               f"selectors are {', '.join(assemblers)}")
                else:
                    assembler = chosen
            elif compiler != DEFAULT_COMPILER:
                problem = f"duplicate compiler selector {token}"
            elif token not in selectors:
                problem = (f"unknown compiler selector {token}; pinned selectors "
                           f"are {', '.join(selectors)}")
            else:
                compiler = token
            if problem:
                break
        if problem:
            problems.append(f"line {number}: {name}: {problem}")
            continue
        # An entry without a flag is meaningful only when an assembler selector
        # is its whole content: moving the object to another `as` while the
        # pinned chain keeps its global flags. Naming a compiler without a flag
        # would claim that compiler's complete flag set is empty, which is a
        # mistyped line rather than a measurement.
        if not rest and (compiler != DEFAULT_COMPILER or assembler == DEFAULT_ASSEMBLER):
            problems.append(f"line {number}: {name}: no cc1 flags")
            continue
        if not all(flag.startswith("-") for flag in rest):
            problems.append(f"line {number}: {name}: not a flag: "
                            f"{next(f for f in rest if not f.startswith('-'))}")
            continue
        entries[name] = Entry(compiler, tuple(rest), assembler)
    if problems:
        raise ValueError("config/cflags.txt: " + "; ".join(problems))
    return entries


def for_source(entries: dict[str, Entry], source: Path) -> Entry:
    """Compiler and flags measured for one source file; the profile chain by default."""
    try:
        name = Path(source).resolve().relative_to(ROOT).as_posix()
    except ValueError:
        return DEFAULT_ENTRY
    return entries.get(name, DEFAULT_ENTRY)
