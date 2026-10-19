#!/usr/bin/env python3
"""The launcher's game pictures, clips and thumbnails (public/img), made with ffmpeg.

    python3 scripts/media.py clip RECORDING GAME --start SECONDS [--length 6] [--still SECONDS]
    python3 scripts/media.py thumbs

`clip` makes a game's clip from a recording of your own play: public/img/<name>-clip.webp, an
animated WebP that loops (the game page plays it while the pointer is on the game's band). With
--still it also writes the frame at that second as the game's picture, <name>-bg.webp, and its
thumbnail. GAME is OpenRAC's id (rac1 to rac5); <name> is the site's name for the game (rac1, gc,
uya, deadlocked, size-matters), so the files match src/lib/media.ts.

`thumbs` makes every game's thumbnail, <name>-thumb.webp, from its picture: the game list shows
the pictures 64 by 44, and a thumbnail is a few KB to decode where a picture is several MB.
Run it after a picture is added or replaced.

Record the game yourself, from your own disc: docs/STYLE.md says what makes a good clip and what
to write in THIRD_PARTY_NOTICES.md before it ships. Needs ffmpeg with libwebp.
"""

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

NAMES = {"rac1": "rac1", "rac2": "gc", "rac3": "uya", "rac4": "deadlocked", "rac5": "size-matters"}
IMG = Path(__file__).resolve().parent.parent / "public" / "img"
# A clip above this is slow to show on hover and heavy in the installer.
BUDGET = 3 * 1024 * 1024
# The game list's picture is 64 by 44: three times that stays sharp on any screen.
THUMB = (192, 132)


def clip_args(recording: Path, out: Path, start: float, length: float, width: int, fps: int, quality: int) -> list[str]:
    """ffmpeg's arguments for the clip: no sound, WIDTH wide at FPS, looping for ever."""
    return [
        "ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
        "-ss", str(start), "-t", str(length), "-i", str(recording),
        "-vf", f"fps={fps},scale={width}:-2:flags=lanczos",
        "-an", "-c:v", "libwebp_anim", "-loop", "0",
        "-quality", str(quality), "-compression_level", "6", "-preset", "picture",
        str(out),
    ]  # fmt: skip


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


def thumb(img: Path, name: str) -> Path | None:
    """Writes NAME's thumbnail from its picture; None when there is no picture yet."""
    picture = img / f"{name}-bg.webp"
    if not picture.is_file():
        return None
    out = img / f"{name}-thumb.webp"
    subprocess.run(thumb_args(picture, out), check=True)
    return out


def clip(a: argparse.Namespace) -> int:
    if not a.recording.is_file():
        print(f"no recording at {a.recording}", file=sys.stderr)
        return 1
    if not 1 <= a.length <= 12:
        print("a clip lasts from 1 to 12 seconds: it loops while the pointer is on the band", file=sys.stderr)
        return 1
    name = NAMES[a.game]
    a.out.mkdir(parents=True, exist_ok=True)
    out = a.out / f"{name}-clip.webp"
    subprocess.run(clip_args(a.recording, out, a.start, a.length, a.width, a.fps, a.quality), check=True)
    size = out.stat().st_size
    print(f"{out}: {size / 1024 / 1024:.1f} MB")
    if size > BUDGET:
        print(
            f"  over {BUDGET // 1024 // 1024} MB: try a shorter --length, a lower --fps or --quality",
            file=sys.stderr,
        )
    if a.still is not None:
        still = a.out / f"{name}-bg.webp"
        subprocess.run(still_args(a.recording, still, a.still, 1600), check=True)
        print(f"{still}: {still.stat().st_size / 1024:.0f} KB")
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
    c = sub.add_parser("clip", help="a game's clip (and picture) from a recording of your own play")
    c.add_argument("recording", type=Path, help="a video of your own play (mp4, mkv, webm…)")
    c.add_argument("game", choices=sorted(NAMES), help="OpenRAC's id for the game")
    c.add_argument("--start", type=float, required=True, help="where the clip starts, in seconds")
    c.add_argument("--length", type=float, default=6, help="how long it lasts (default 6 s)")
    c.add_argument("--width", type=int, default=960, help="its width in pixels (default 960)")
    c.add_argument("--fps", type=int, default=20, help="frames per second (default 20)")
    c.add_argument("--quality", type=int, default=70, help="WebP quality, 0 to 100 (default 70)")
    c.add_argument("--still", type=float, help="also write the game's picture from the frame at this second")
    c.add_argument("--out", type=Path, default=IMG, help="where to write (default public/img)")
    t = sub.add_parser("thumbs", help="every game's thumbnail from its picture")
    t.add_argument("--out", type=Path, default=IMG, help="where the pictures are (default public/img)")
    a = p.parse_args(argv)

    if not shutil.which("ffmpeg"):
        print("media.py needs ffmpeg (with libwebp) on the PATH", file=sys.stderr)
        return 1
    return clip(a) if a.command == "clip" else thumbs(a)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
