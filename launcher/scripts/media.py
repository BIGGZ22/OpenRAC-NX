#!/usr/bin/env python3
"""The launcher's game pictures and thumbnails (public/img), made with ffmpeg.

    python3 scripts/media.py still RECORDING GAME --at SECONDS
    python3 scripts/media.py thumbs

`still` writes a game's picture, public/img/<name>-bg.webp, from the frame at that second of a
recording of your own play, and its thumbnail. GAME is OpenRAC's id (rac1 to rac6); <name> is the
site's name for the game (rac1, gc, uya, deadlocked, size-matters, sac), so the files match
src/lib/media.ts.

`thumbs` makes every game's thumbnail, <name>-thumb.webp, and its tile picture, <name>-tile.webp,
from its picture: the game list shows the pictures 64 by 44 and Android's Games tab a few hundred
pixels wide, and a picture up to 3840 wide decodes to tens of MB where those take one or two.
Run it after a picture is added or replaced.

Record the game yourself, from your own disc: docs/STYLE.md says what to write in
THIRD_PARTY_NOTICES.md before it ships. Needs ffmpeg with libwebp.
"""

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

NAMES = {"rac1": "rac1", "rac2": "gc", "rac3": "uya", "rac4": "deadlocked", "rac5": "size-matters", "rac6": "sac"}
IMG = Path(__file__).resolve().parent.parent / "public" / "img"
# The game list's picture is 64 by 44: three times that stays sharp on any screen.
THUMB = (192, 132)
# Android's game tiles are at most about 900 pixels wide on the screen: 1280 stays sharp there.
TILE_WIDTH = 1280


def still_args(recording: Path, out: Path, at: float, width: int) -> list[str]:
    """ffmpeg's arguments for the picture: the frame at AT, WIDTH wide."""
    return [
        "ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
        "-ss", str(at), "-i", str(recording), "-frames:v", "1",
        "-vf", f"scale={width}:-2:flags=lanczos",
        "-c:v", "libwebp", "-quality", "85",
        str(out),
    ]  # fmt: skip


def thumb_args(picture: Path, out: Path) -> list[str]:
    """ffmpeg's arguments for a thumbnail: the picture filling THUMB, cut to it from the middle."""
    w, h = THUMB
    return [
        "ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-i", str(picture),
        "-vf", f"scale={w}:{h}:force_original_aspect_ratio=increase:flags=lanczos,crop={w}:{h}",
        "-frames:v", "1", "-c:v", "libwebp", "-quality", "80",
        str(out),
    ]  # fmt: skip


def tile_args(picture: Path, out: Path) -> list[str]:
    """ffmpeg's arguments for a tile picture: the whole picture, TILE_WIDTH wide (never wider)."""
    return [
        "ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-i", str(picture),
        "-vf", f"scale='min({TILE_WIDTH},iw)':-2:flags=lanczos",
        "-frames:v", "1", "-c:v", "libwebp", "-quality", "82",
        str(out),
    ]  # fmt: skip


def thumb(img: Path, name: str) -> Path | None:
    """Writes NAME's thumbnail and tile picture from its picture; None when there is no picture yet."""
    picture = img / f"{name}-bg.webp"
    if not picture.is_file():
        return None
    subprocess.run(tile_args(picture, img / f"{name}-tile.webp"), check=True)
    out = img / f"{name}-thumb.webp"
    subprocess.run(thumb_args(picture, out), check=True)
    return out


def still(a: argparse.Namespace) -> int:
    if not a.recording.is_file():
        print(f"no recording at {a.recording}", file=sys.stderr)
        return 1
    name = NAMES[a.game]
    a.out.mkdir(parents=True, exist_ok=True)
    picture = a.out / f"{name}-bg.webp"
    subprocess.run(still_args(a.recording, picture, a.at, 1600), check=True)
    print(f"{picture}: {picture.stat().st_size / 1024:.0f} KB")
    small = thumb(a.out, name)
    if small:
        print(f"{small}: {small.stat().st_size / 1024:.0f} KB")
    return 0


def thumbs(a: argparse.Namespace) -> int:
    for game, name in NAMES.items():
        small = thumb(a.out, name)
        print(f"{game}: {small} ({small.stat().st_size / 1024:.0f} KB)" if small else f"{game}: no {name}-bg.webp yet")
    return 0


def main(argv: list[str]) -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="command", required=True)
    c = sub.add_parser("still", help="a game's picture from a recording of your own play")
    c.add_argument("recording", type=Path, help="a video of your own play (mp4, mkv, webm…)")
    c.add_argument("game", choices=sorted(NAMES), help="OpenRAC's id for the game")
    c.add_argument("--at", type=float, required=True, help="the second of the frame to take")
    c.add_argument("--out", type=Path, default=IMG, help="where to write (default public/img)")
    t = sub.add_parser("thumbs", help="every game's thumbnail from its picture")
    t.add_argument("--out", type=Path, default=IMG, help="where the pictures are (default public/img)")
    a = p.parse_args(argv)

    if not shutil.which("ffmpeg"):
        print("media.py needs ffmpeg (with libwebp) on the PATH", file=sys.stderr)
        return 1
    return still(a) if a.command == "still" else thumbs(a)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
