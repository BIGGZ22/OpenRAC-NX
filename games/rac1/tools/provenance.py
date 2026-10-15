#!/usr/bin/env python3
"""
Provenance guard for the movie code (docs/MOVIE.md, CONTRIBUTING.md "Sources").

The movie code in retail is Sony's ezmpeg sample. On 2026-09-30 it was reverted
to assembly because it had been compiled from that sample's leaked source. It
may only be redone from the retail assembly alone. Two checks back that up:

  sony_sample(text)         names that only exist in the sample's source or
                            headers (the sample itself, its helper macros);
                            tools/integrate.py and tools/apply_candidate.py
                            refuse a candidate that mentions one.
  movie_provenance(func)    a function landing in src/game/movie/ needs a row in
                            config/movie_provenance.tsv saying where its C
                            came from ("asm only" for a function written from
                            the retail assembly alone). apply_candidate.py
                            refuses without one.
"""
import re
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
SONY_SAMPLE = re.compile(r"ezmpeg|ezcontainer|\bUncAddr\b", re.I)
PROVENANCE = REPO / "config" / "movie_provenance.tsv"


def sony_sample(text: str):
    """The first name in TEXT (comments included) that belongs to Sony's sample, or None."""
    m = SONY_SAMPLE.search(text)
    return m.group(0) if m else None


def is_movie_file(src: Path) -> bool:
    return "movie" in Path(src).parts


def movie_provenance(func: str) -> bool:
    if not PROVENANCE.exists():
        return False
    for line in PROVENANCE.read_text().splitlines():
        cols = line.split("\t")
        if line and not line.startswith("#") and cols[0] == func:
            return True
    return False
