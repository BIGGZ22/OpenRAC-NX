#!/usr/bin/env python3
"""
OpenRAC's extractor: sets up a game from the image of the player's own disc,
the way OpenGOAL's `extractor` does for Jak and Daxter. Standard library only.

  python3 tools/extractor.py IMAGE --game rac1 [--proj-path DIR] [steps]

Steps (all of them when none is named):
  --extract    copy the disc's files into PROJ/iso_data/GAME/ and keep the image
               there too: Ratchet & Clank reads most of its data by sector, not
               by file, so the image is linked (copied across file systems)
  --boot-only  with --extract: write out only SYSTEM.CNF and the boot executable,
               which is all the port reads beside the image; every file is still
               read and checked. Gigabytes less to write (the Android launcher's)
  --validate   refuse the image unless its boot executable is a build OpenRAC
               knows (games/GAME/game.json: serial, size and SHA-1), as
               OpenGOAL's --validate does; implied by --extract
  --decompile  turn the disc's assets into the port's formats (not available yet)
  --compile    build the native port (not available yet)

PROJ defaults to build/port in this checkout; the launcher passes its own
(INSTALL/active/GAME/data). On success iso_data/GAME/buildinfo.json records
what was extracted. On failure the last line printed is "error NNNN: ..."
with OpenGOAL's number for the same failure (ERRORS below), and the exit
status is 1 (an exit status cannot hold four digits on Linux or macOS).
Nothing is downloaded, nothing leaves the machine, and the image is never
written to.
"""
import argparse
import hashlib
import json
import os
import shutil
import sys
from pathlib import Path

from openrac_common import digest, files, games, iso9660, paths

SECTOR = iso9660.SECTOR

# Exit codes, numbered as OpenGOAL's extractor numbers the same failures
# (decompiler/extractor/extractor_util.h, ExtractorErrorCode), so that a
# player's report reads the same in both projects. 4060 is OpenRAC's own.
ERRORS = {
    4000: "the image has no boot executable (no SYSTEM.CNF, or the file it names is missing)",
    4001: "the image is not a version of this game that OpenRAC knows",
    4002: "the boot executable is not the build OpenRAC knows for this serial (a different revision, or a damaged image)",
    4011: "the image does not hold the files this build should have",
    4012: "the disc's files are not those of this build (a damaged or modified image)",
    4020: "the image is not an ISO 9660 disc image",
    4040: "the file is not a disc image (.iso)",
    4041: "the image is too small to be a PlayStation 2 disc",
    4060: "this step does not exist yet: the native port is being built (docs/port/ROADMAP.md)",
}

MIN_IMAGE_SIZE = 1_000_000_000  # OpenGOAL's is_iso_file: a PS2 DVD image is larger


class ExtractError(Exception):
    def __init__(self, code: int, detail: str = ""):
        super().__init__(code, detail)
        self.code, self.detail = code, detail

    def __str__(self) -> str:
        return f"error {self.code}: {ERRORS[self.code]}" + (f" ({self.detail})" if self.detail else "")


# --- The disc ----------------------------------------------------------------------

def files_of(image: Path) -> list[tuple[str, int, int]]:
    """(path, first sector, size) of every file on IMAGE (iso9660.files), or error 4020 with
    iso9660's reason when it is not ISO 9660 or its directories cannot be trusted."""
    try:
        return iso9660.files(image)
    except iso9660.IsoError as e:
        raise ExtractError(4020, e.detail) from e


# --- What OpenRAC knows ---------------------------------------------------------------

def builds(game: str | None = None) -> dict[str, tuple[dict, str, dict]]:
    """serial -> (game, version name, version), for GAME or every game."""
    return {v["serial"]: (g, name, v) for g, name, v in games.versions() if game in (None, g["id"])}


def check_image_file(image: Path, minimum: int) -> None:
    if not image.is_file() or image.suffix.lower() != ".iso":
        raise ExtractError(4040, str(image))
    if image.stat().st_size < minimum:
        raise ExtractError(4041, f"{image.stat().st_size} bytes")


def contents_hash(hashes: dict[str, str]) -> str:
    """One hash for a set of files, whatever order they were read in."""
    return hashlib.sha256("".join(f"{p}\0{h}\n" for p, h in sorted(hashes.items())).encode()).hexdigest()


def validate(game: str, serial: str | None, boot_sha1: str | None, hashes: dict[str, str],
             boot_sha256: str | None = None) -> tuple[dict, str, dict]:
    """The known build the disc's files are (HASHES: path -> SHA-1), or an ExtractError. A build
    game.json pins by the boot's SHA-256 alone (rac5, recorded from its project's target.json) is
    checked by that."""
    if serial is None or serial not in hashes:
        raise ExtractError(4000)
    known = builds(game)
    if serial not in known:
        other = builds().get(serial)
        if other:
            raise ExtractError(4001, f"{serial} is {other[0]['title']} ({other[2]['region']}), not {game}")
        raise ExtractError(4001, serial)
    g, name, version = known[serial]
    want = version["boot"]
    if "sha1" in want:
        if boot_sha1 != want["sha1"]:
            raise ExtractError(4002, f"{serial} SHA-1 {boot_sha1}, game.json has {want['sha1']}")
    elif boot_sha256 != want.get("sha256"):
        raise ExtractError(4002, f"{serial} SHA-256 {boot_sha256}, game.json has {want.get('sha256')}")
    # game.json may list the disc's file count and contents hash once someone
    # with the disc records them (the line print_new_build prints).
    disc = version.get("disc", {})
    if "files" in disc and disc["files"] != len(hashes):
        raise ExtractError(4011, f"{len(hashes)} files, game.json has {disc['files']}")
    if "contents" in disc and disc["contents"] != contents_hash(hashes):
        raise ExtractError(4012)
    return g, name, version


def print_new_build(serial: str | None, boot_sha1: str | None, hashes: dict[str, str]) -> None:
    """What game.json would need to know this build (OpenGOAL's log_potential_new_db_entry)."""
    print("If this is a genuine disc OpenRAC does not know yet, a maintainer can add it to game.json:")
    print(json.dumps({"serial": serial, "boot": {"sha1": boot_sha1},
                      "disc": {"files": len(hashes), "contents": contents_hash(hashes)}}, indent=2))


# --- Steps ------------------------------------------------------------------------------

def extract(image: Path, game: str, proj: Path, minimum: int = MIN_IMAGE_SIZE, boot_only: bool = False) -> dict:
    """Extract and validate IMAGE into PROJ/iso_data/GAME; returns the build info written.
    BOOT_ONLY: of the disc's files only SYSTEM.CNF and the boot executable are written."""
    check_image_file(image, minimum)
    entries = files_of(image)
    serial = iso9660.boot_serial(image)
    print(f"{image.name}: {len(entries)} files on the disc, boot executable {serial}")

    iso_data = proj / "iso_data"
    temp, final = iso_data / "_temp", iso_data / game
    if temp.exists():
        shutil.rmtree(temp)
    temp.mkdir(parents=True)
    hashes: dict[str, str] = {}
    with open(image, "rb") as f:
        for path, lba, size in entries:
            keep = not boot_only or path in (serial, "SYSTEM.CNF")
            dest = temp / path
            if keep:
                dest.parent.mkdir(parents=True, exist_ok=True)
            f.seek(lba * SECTOR)
            h, left = hashlib.sha1(), size
            with open(dest, "wb") if keep else open(os.devnull, "wb") as out:
                while left:
                    block = f.read(min(left, 1 << 23))
                    if not block:
                        raise ExtractError(4020, f"{path} runs past the end of the image")
                    if keep:
                        out.write(block)
                    h.update(block)
                    left -= len(block)
            hashes[path] = h.hexdigest()
            print(f"  {path}: {size} bytes{'' if keep else ', checked'}")

    try:
        boot = temp / serial if serial else None
        boot_sha256 = digest.sha256(boot) if boot and boot.is_file() else None
        g, name, version = validate(game, serial, hashes.get(serial or ""), hashes, boot_sha256)
    except ExtractError as e:
        if e.code in (4001, 4002, 4011, 4012):
            print_new_build(serial, hashes.get(serial or ""), hashes)
        shutil.rmtree(temp)
        raise
    print(f"validated: {g['title']} ({version['region']}, {serial}), boot executable SHA-1 matches games/{game}/game.json")

    # Most of Ratchet & Clank's data is addressed by sector, outside the file
    # system: keep the image beside the files for the decompile step.
    how = files.link_or_copy(image, temp / "disc.iso", keep_metadata=False)
    print(f"  disc.iso: {how} from {image}")
    info = [{
        "serial": serial,
        "version": f"{game}/{name}",
        "elf_sha1": hashes[serial],
        "files": len(hashes),
        "contents": contents_hash(hashes),
        "image": image.name,
    }]
    (temp / "buildinfo.json").write_text(json.dumps(info, indent=2) + "\n")
    if final.exists():
        shutil.rmtree(final)
    temp.rename(final)
    print(f"extracted to {final}")
    return info[0]


def not_yet(step: str) -> None:
    raise ExtractError(4060, f"--{step}")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("image", type=Path, help="the image (.iso) of your own disc")
    parser.add_argument("-g", "--game", required=True, choices=sorted({g["id"] for g in games.load_all()}))
    parser.add_argument("--proj-path", type=Path, default=paths.repo_root() / "build" / "port")
    parser.add_argument("-e", "--extract", action="store_true")
    parser.add_argument("--boot-only", action="store_true")
    parser.add_argument("-v", "--validate", action="store_true")
    parser.add_argument("-d", "--decompile", action="store_true")
    parser.add_argument("-c", "--compile", action="store_true")
    args = parser.parse_args(argv)
    every = not (args.extract or args.validate or args.decompile or args.compile)
    try:
        if every or args.extract or args.validate:
            extract(args.image.resolve(), args.game, args.proj_path.resolve(), boot_only=args.boot_only)
        if every or args.decompile:
            not_yet("decompile")
        if every or args.compile:
            not_yet("compile")
    except ExtractError as e:
        print(e, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
