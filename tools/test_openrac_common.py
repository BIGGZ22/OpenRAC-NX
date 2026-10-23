# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""tools/openrac_common on synthetic data only, never a real disc:
python3 -m unittest discover -s tools

Two kinds of test. Each helper's own behaviour, with expected values from its documentation
(ECMA-119 for the image, hashlib for the digests). And characterisation: the existing copy each
helper was written from runs on the same synthetic input as the helper, and both must give the
same result, so a caller moved onto the package later behaves as it did. Those copies are
imported from where they live today (tools/, port/tools/hostgen, port/tools/versionmap,
sdk/tools, editor, port/enhanced/tools/probes); once their callers have moved and a copy is
gone, its comparison goes with it.
"""
import contextlib
import hashlib
import io
import json
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from openrac_common import digest, files, games, iso9660, paths, proc, tables

ROOT = Path(__file__).resolve().parent.parent

# The existing copies' folders go after tools/ on the path, so a tools/ module of the same name
# (tools/port.py beside editor/port.py) is the one imported.
for folder in ("port/tools/hostgen", "port/tools/versionmap", "sdk/tools", "editor",
               "port/enhanced/tools/probes"):
    if str(ROOT / folder) not in sys.path:
        sys.path.append(str(ROOT / folder))

import bake_all  # noqa: E402  port/enhanced/tools/probes
import carry_functions  # noqa: E402  port/tools/versionmap
import disc  # noqa: E402  editor
import fcheck_gate  # noqa: E402
import gen_binding  # noqa: E402  sdk/tools
import hostgen  # noqa: E402  port/tools/hostgen
import no_c  # noqa: E402  port/tools/hostgen
import openrac  # noqa: E402
import versionmap  # noqa: E402  port/tools/versionmap

SECTOR = iso9660.SECTOR


# ---- A synthetic ISO 9660 image ----

def record(name: bytes, lba: int, size: int, directory: bool = False) -> bytes:
    """One directory record (ECMA-119 9.1): its length, extent and size both-endian, its flags
    and its name, padded to an even length."""
    length = 33 + len(name) + (1 - len(name) % 2)
    out = bytearray(length)
    out[0] = length
    struct.pack_into("<I", out, 2, lba)
    struct.pack_into(">I", out, 6, lba)
    struct.pack_into("<I", out, 10, size)
    struct.pack_into(">I", out, 14, size)
    out[25] = iso9660.FLAG_DIRECTORY if directory else 0
    out[32] = len(name)
    out[33:33 + len(name)] = name
    return bytes(out)


class ImageBuilder:
    """Lays out a tree of {name: bytes or a nested dict} as an ISO 9660 image: the volume
    descriptor at sector 16, then files and directories after it, a directory's records packed
    into sectors without crossing one, as the format requires."""

    def __init__(self):
        self.sectors: dict[int, bytes] = {}
        self.next_lba = 18

    def allocate(self, size: int) -> int:
        lba = self.next_lba
        self.next_lba += max(1, (size + SECTOR - 1) // SECTOR)
        return lba

    def directory(self, tree: dict, parent: int | None) -> tuple[int, int]:
        """Places TREE's files and subdirectories, then TREE itself; (its sector, its size)."""
        children = []
        for name, item in tree.items():
            if isinstance(item, dict):
                children.append((name.encode(), item, True))
            else:
                lba = self.allocate(len(item))
                self.sectors[lba] = item
                children.append((name.encode() + b";1", (lba, len(item)), False))
        # A directory's sectors are known only after its records are; reserve its place by size.
        names = [b"\0", b"\1"] + [name for name, _item, _is_dir in children]
        sectors_needed = self.sectors_for([record(n, 0, 0) for n in names])
        lba = self.allocate(sectors_needed * SECTOR)
        size = sectors_needed * SECTOR
        records = [record(b"\0", lba, size, True), record(b"\1", parent or lba, size, True)]
        for name, item, is_dir in children:
            if is_dir:
                records.append(record(name, *self.directory(item, lba), True))
            else:
                records.append(record(name, *item))
        self.sectors[lba] = self.pack(records)
        return lba, size

    @staticmethod
    def sectors_for(records: list[bytes]) -> int:
        return len(ImageBuilder.pack(records)) // SECTOR

    @staticmethod
    def pack(records: list[bytes]) -> bytes:
        """RECORDS in sectors, zero-padded where the next would cross a sector boundary."""
        out = bytearray()
        for item in records:
            if len(out) % SECTOR + len(item) > SECTOR:
                out += bytes(SECTOR - len(out) % SECTOR)
            out += item
        return bytes(out) + bytes(-len(out) % SECTOR)

    def build(self, tree: dict) -> bytes:
        root_lba, root_size = self.directory(tree, None)
        image = bytearray(self.next_lba * SECTOR)
        pvd = bytearray(SECTOR)
        pvd[0:7] = b"\x01CD001\x01"
        struct.pack_into("<H", pvd, 128, SECTOR)
        struct.pack_into(">H", pvd, 130, SECTOR)
        pvd[156:156 + 34] = record(b"\0", root_lba, root_size, True)
        image[16 * SECTOR:17 * SECTOR] = pvd
        image[17 * SECTOR] = 0xFF  # the set terminator (ECMA-119 8.3)
        for lba, data in self.sectors.items():
            image[lba * SECTOR:lba * SECTOR + len(data)] = data
        return bytes(image)


def build_image(tree: dict) -> bytes:
    return ImageBuilder().build(tree)


CNF = b"BOOT2 = cdrom0:\\SCES_999.99;1\r\nVER = 1.00\r\nVMODE = PAL\r\n"
BOOT = b"\x7fELF" + bytes(range(256)) * 20

# Enough files in the root that its records fill more than one sector.
MANY = {f"FILE{n:03d}.BIN": bytes([n]) * (n + 1) for n in range(60)}

TREE = {
    "SYSTEM.CNF": CNF,
    "SCES_999.99": BOOT,
    "DATA": {"LEVEL0.WAD": b"level zero", "SUB": {"DEEP.BIN": b"deep"}, "EMPTY": {}},
    **MANY,
}


class TempDir(unittest.TestCase):
    """A test with a fresh temporary folder in self.dir."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.dir = Path(self.tmp.name)

    def tearDown(self):
        self.tmp.cleanup()

    def write(self, name: str, data: bytes | str) -> Path:
        path = self.dir / name
        path.parent.mkdir(parents=True, exist_ok=True)
        if isinstance(data, str):
            path.write_text(data, encoding="utf-8")
        else:
            path.write_bytes(data)
        return path


# ---- paths ----

class PathsTests(TempDir):
    def test_finds_this_checkout_from_the_package_and_from_a_script(self):
        self.assertEqual(paths.repo_root(), ROOT)
        self.assertEqual(paths.repo_root(__file__), ROOT)
        self.assertEqual(paths.repo_root(ROOT), ROOT)

    def test_matches_the_hand_written_roots(self):
        # tools/*.py take theirs from repo_root since P1-05; these are still written by hand.
        for module in (versionmap, gen_binding):
            with self.subTest(module=module.__name__):
                self.assertEqual(paths.repo_root(module.__file__), module.ROOT)

    def test_a_checkout_needs_games_and_agents_md(self):
        (self.dir / "a/games").mkdir(parents=True)
        (self.dir / "a/b/c").mkdir(parents=True)
        with self.assertRaises(FileNotFoundError):
            paths.repo_root(self.dir / "a/b/c")
        (self.dir / "a/AGENTS.md").write_text("")
        self.assertEqual(paths.repo_root(self.dir / "a/b/c"), (self.dir / "a").resolve())

    def test_the_launchers_bundled_copy_is_a_root(self):
        (self.dir / "bundle/games").mkdir(parents=True)
        (self.dir / "bundle/tools").mkdir()
        (self.dir / "bundle/STANDALONE").write_text("")
        self.assertEqual(paths.repo_root(self.dir / "bundle/tools/openrac.py"), (self.dir / "bundle").resolve())

    def test_agents_md_without_games_is_not_a_root(self):
        (self.dir / "x/y").mkdir(parents=True)
        (self.dir / "x/AGENTS.md").write_text("")
        with self.assertRaises(FileNotFoundError):
            paths.repo_root(self.dir / "x/y")


# ---- games ----

RAC8 = {"id": "rac8", "title": "Eight", "versions": {
    "pal": {"serial": "SCES_888.88", "region": "PAL", "dir": "games/rac8", "source": {"checkout": "~/x"},
            "xmap": {"code": []}},
    "ntsc": {"serial": "SCUS_888.88", "region": "NTSC-U"},
}}
RAC9 = {"id": "rac9", "title": "Nine", "versions": {
    "ntsc": {"serial": "SCUS_999.99", "region": "NTSC-U", "source": {"checkout": "~/y"}},
}}


class GamesTests(TempDir):
    def setUp(self):
        super().setUp()
        (self.dir / "AGENTS.md").write_text("")
        # rac9 is written first: the order comes from the names, not from the file system.
        for game in (RAC9, RAC8):
            self.write(f"games/{game['id']}/game.json", json.dumps(game))
        self.games_dir = self.dir / "games"

    def test_loads_one_manifest(self):
        self.assertEqual(games.load("rac9", self.dir), RAC9)
        with self.assertRaises(FileNotFoundError):
            games.load("rac7", self.dir)

    def test_every_version_in_game_order(self):
        self.assertEqual([(g["id"], name) for g, name, _v in games.versions(self.dir)],
                         [("rac8", "pal"), ("rac8", "ntsc"), ("rac9", "ntsc")])

    def test_version_dir_takes_dir_or_the_default(self):
        self.assertEqual(games.version_dir(RAC8, "pal"), "games/rac8")
        self.assertEqual(games.version_dir(RAC8, "ntsc"), "games/rac8/ntsc")

    def test_matches_versionmap(self):
        with mock.patch.object(versionmap, "ROOT", self.dir):
            self.assertEqual(games.load("rac8", self.dir)["versions"]["ntsc"], versionmap.version("rac8/ntsc"))


# ---- tables ----

# A table with every convention and quirk: the comment header, a comment line inside the rows,
# an indented "#" (data), blank and whitespace-only lines, spaces in cells, short and long rows.
LIBRARIES = (
    "# The library table.\n"
    "#\n"
    "# symbol\tlibrary\tname\tport\tnotes\tsignature (optional)\n"
    "func_00100000\tlibc\tstrlen\thost\t\tsize_t strlen(const char *s)\n"
    "\n"
    "func_00100010\tlibc\t memcpy \thost\tthe shared one\n"
    "   \n"
    "func_00100020\tgame\tTween\thook\n"
    "# a comment between rows\n"
    "func_00100030\tgame\tWrapped\twrap\tnote\n"
    "  # indented\tis\tdata\thost\n"
    "func_00100040\tgame\tHooked\t hook \n"
    "short\trow\n"
    "func_00100050\tgame\tLong\tskip\tn\tsig\textra\n"
)
LIBRARY_API = (
    "# name\tlibrary\tsignature\n"
    "strlen\tlibc\tsize_t strlen(const char *)\n"
    "memcpy\tlibc\tvoid *memcpy(void *, const void *, size_t)\n"
    "lonely\n"
)
NO_C = (
    "# a comment\n"
    "name\tclass\tkind\tc\treachable\tevidence\n"
    "func_a\ta\tpadding\tnone\tnothing\tby hand: kept\n"
    "func_b\tc\tcode\tnone\tcalled\tthe assembly\n"
    "func_c\tb\thost\tnone\tnothing\tby hand: also kept\textra\n"
    "func_d\ta\tpadding\tnone\tnothing\tby hand: short\n"
)


class TablesTests(TempDir):
    def setUp(self):
        super().setUp()
        self.libraries = self.write("libraries.tsv", LIBRARIES)
        self.api = self.write("api.tsv", LIBRARY_API)
        self.no_c = self.write("no_c.tsv", NO_C)

    def test_rows_keep_their_cells_as_written(self):
        rows = tables.read_tsv(self.libraries)
        self.assertEqual(rows[1], ["func_00100010", "libc", " memcpy ", "host", "the shared one"])
        self.assertIn(["  # indented", "is", "data", "host"], rows)
        self.assertEqual(len(rows), 8)

    def test_strip_and_columns(self):
        rows = tables.read_tsv(self.libraries, ["symbol", "library", "name", "port"], strip=True)
        self.assertEqual(rows[1], {"symbol": "func_00100010", "library": "libc", "name": "memcpy", "port": "host"})
        self.assertEqual(rows[6], {"symbol": "short", "library": "row", "name": "", "port": ""})

    def test_an_uncommented_header_line_is_skipped_with_columns(self):
        rows = tables.read_tsv(self.no_c, no_c.COLUMNS)
        self.assertEqual([r["name"] for r in rows], ["func_a", "func_b", "func_c", "func_d"])
        self.assertEqual(tables.read_tsv(self.no_c)[0][0], "name")

    def test_a_missing_table_is_empty_unless_required(self):
        self.assertEqual(tables.read_tsv(self.dir / "none.tsv"), [])
        self.assertEqual(tables.header(self.dir / "none.tsv"), [])
        with self.assertRaises(FileNotFoundError):
            tables.read_tsv(self.dir / "none.tsv", required=True)

    def test_header_reads_the_comment_line_with_tabs(self):
        self.assertEqual(tables.header(self.libraries),
                         ["symbol", "library", "name", "port", "notes", "signature (optional)"])
        self.assertEqual(tables.header(self.no_c), [])

    def test_matches_carry_functions_read_tsv(self):
        for path in (self.libraries, self.api, self.no_c, self.dir / "none.tsv"):
            with self.subTest(path=path.name):
                self.assertEqual(tables.read_tsv(path), carry_functions.read_tsv(path))

    def test_matches_no_c_read_tsv_names(self):
        for column in (0, 1):
            self.assertEqual([row[column] for row in tables.read_tsv(self.libraries, strip=True)],
                             no_c.read_tsv_names(self.libraries, column))

    def test_matches_no_c_keep_rows(self):
        rows = tables.read_tsv(self.no_c)
        ours = {row[0]: dict(zip(no_c.COLUMNS, row)) for row in rows[1:]
                if len(row) == len(no_c.COLUMNS) and row[5].startswith("by hand:")}
        self.assertEqual(ours, no_c.keep_rows(self.no_c))

    def test_matches_hostgen_library_readers(self):
        cfg = {"libraries": self.libraries.name, "library_api": self.api.name}
        api = {row[0]: row[2] for row in tables.read_tsv(self.api, strip=True) if len(row) >= 3}
        self.assertEqual(api, hostgen.library_api(self.dir, cfg))
        columns = ["symbol", "library", "name", "port", "notes", "signature"]
        rows = tables.read_tsv(self.libraries, columns, strip=True)
        host = {r["symbol"]: (r["name"], r["signature"] or api.get(r["name"], "")) for r in rows if r["port"] == "host"}
        self.assertEqual(host, hostgen.host_table(self.dir, cfg))
        self.assertEqual({r["symbol"] for r in rows if r["port"] in ("wrap", "hook")}, hostgen.wrapped(self.dir, cfg))
        self.assertEqual({r["symbol"] for r in rows if r["port"] == "wrap"}, hostgen.wrapped_by_host(self.dir, cfg))

    def test_matches_gen_binding_hook_rows(self):
        path = ROOT / "port" / "game" / "rac8-pal" / "libraries.tsv"
        expected = gen_binding.hook_rows("rac8", ["pal"], {path: LIBRARIES})
        rows = tables.read_tsv(self.libraries, ["symbol", "library", "name", "port"], strip=True)
        self.assertEqual({"pal": {r["name"]: r["symbol"] for r in rows if r["port"] == "hook"}}, expected)

    def test_reads_this_checkouts_libraries_as_carry_functions_does(self):
        for path in sorted((ROOT / "port" / "game").glob("*/libraries.tsv")):
            with self.subTest(path=str(path.relative_to(ROOT))):
                self.assertEqual(tables.read_tsv(path), carry_functions.read_tsv(path))
                self.assertEqual(tables.header(path)[:3], ["symbol", "library", "name"]
                                 if path.parent.name != "common" else ["name", "library", "signature"])


# ---- digest ----

class DigestTests(TempDir):
    def setUp(self):
        super().setUp()
        # Past one block, and not a multiple of it, so the last short block counts.
        self.data = hashlib.sha256(b"seed").digest() * (3 * digest.BLOCK_BYTES // 32 + 5)
        self.path = self.write("blob.bin", self.data)

    def test_digests_are_hashlibs(self):
        self.assertEqual(digest.sha1(self.path), hashlib.sha1(self.data).hexdigest())
        self.assertEqual(digest.sha256(self.path), hashlib.sha256(self.data).hexdigest())
        self.assertEqual(digest.file_digest(self.path, "md5"), hashlib.md5(self.data).hexdigest())

    def test_an_empty_file(self):
        empty = self.write("empty", b"")
        self.assertEqual(digest.sha1(empty), hashlib.sha1(b"").hexdigest())

    def test_matches_the_existing_copies(self):
        self.assertEqual(digest.sha256(self.path), bake_all.sha256(self.path))
        full = openrac.checksums(self.path, True)
        self.assertEqual((digest.sha1(self.path), digest.sha256(self.path), digest.file_digest(self.path, "md5")),
                         (full["sha1"], full["sha256"], full["md5"]))


# ---- files ----

class FilesTests(TempDir):
    def setUp(self):
        super().setUp()
        self.source = self.write("source.bin", b"contents")
        os.utime(self.source, (1_000_000_000, 1_000_000_000))

    def test_links_where_it_can(self):
        dest = self.dir / "linked.bin"
        self.assertEqual(files.link_or_copy(self.source, dest), "linked")
        self.assertEqual(dest.stat().st_ino, self.source.stat().st_ino)

    def test_copies_when_the_link_fails(self):
        with mock.patch("os.link", side_effect=OSError("cross-device link")):
            self.assertEqual(files.link_or_copy(self.source, self.dir / "kept.bin"), "copied")
            self.assertEqual(files.link_or_copy(self.source, self.dir / "plain.bin", keep_metadata=False), "copied")
        self.assertEqual((self.dir / "kept.bin").read_bytes(), b"contents")
        self.assertEqual((self.dir / "kept.bin").stat().st_mtime, 1_000_000_000)
        self.assertNotEqual((self.dir / "plain.bin").stat().st_mtime, 1_000_000_000)

    def test_copy_tree_keeps_what_keep_accepts(self):
        for name in ("a.py", "test_a.py", "sub/b.py", "sub/c.tsv", "sub/__pycache__/b.pyc", "deep/er/d.py"):
            self.write(f"src/{name}", name)
        keep = lambda rel: not rel.name.startswith("test_") and rel.suffix in (".py", ".pyc", ".tsv")  # noqa: E731
        files.copy_tree(self.dir / "src", self.dir / "ours", keep)
        copied = sorted((str(p.relative_to(self.dir / "ours")), p.read_bytes())
                        for p in (self.dir / "ours").rglob("*") if p.is_file())
        self.assertEqual(copied, [(name, name.encode()) for name in ("a.py", "deep/er/d.py", "sub/b.py", "sub/c.tsv")])


# ---- proc ----

class ProcTests(TempDir):
    def test_run_echoes_the_command_as_strings(self):
        # As tools/build_android.py's own run did before it moved onto this.
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            proc.run([Path(sys.executable), "-c", "pass"], cwd=self.dir, echo=True)
        self.assertEqual(out.getvalue(), f"+ {sys.executable} -c pass\n")

    def test_run_checks_the_exit_status(self):
        with self.assertRaises(subprocess.CalledProcessError):
            proc.run([sys.executable, "-c", "raise SystemExit(3)"])
        self.assertEqual(proc.run([sys.executable, "-c", "raise SystemExit(3)"], check=False).returncode, 3)

    def test_run_captures_text_and_replaces_bad_bytes(self):
        done = proc.run([sys.executable, "-c", "import sys; sys.stdout.buffer.write(b'ok \\xff')"], capture=True)
        self.assertEqual(done.stdout, "ok �")

    @unittest.skipUnless(shutil.which("git"), "git is not installed")
    def test_git_reads_and_fcheck_gates_commit(self):
        repo = self.dir / "repo"
        repo.mkdir()
        # The setup ignores the user's git configuration (signing, hooks); the reads under test do not.
        env = dict(os.environ, GIT_CONFIG_GLOBAL=os.devnull, GIT_CONFIG_NOSYSTEM="1",
                   GIT_AUTHOR_NAME="t", GIT_AUTHOR_EMAIL="t@example.com",
                   GIT_COMMITTER_NAME="t", GIT_COMMITTER_EMAIL="t@example.com")
        subprocess.run(["git", "init", "-q", str(repo)], check=True, env=env)
        (repo / "tools").mkdir()
        (repo / "tools/fcheck.py").write_text("x = 1\n")
        subprocess.run(["git", "-C", str(repo), "add", "."], check=True, env=env)
        subprocess.run(["git", "-C", str(repo), "commit", "-q", "-m", "one"], check=True, env=env)

        self.assertEqual(len(proc.git(repo, "rev-parse", "HEAD")), 40)
        self.assertEqual(proc.git(repo, "show", "HEAD:tools/fcheck.py", binary=True), b"x = 1\n")
        with self.assertRaises(subprocess.CalledProcessError):
            proc.git(repo, "rev-parse", "no-such-ref")

        # fcheck_gate's own git does not check, and an unknown ref gives nothing.
        (repo / "tools/fcheck.py").write_text("x = 2\n")
        dirty = proc.git(repo, "status", "--porcelain", "--", "tools/fcheck.py", check=False)
        ours = proc.git(repo, "rev-parse", "--short=8", "HEAD", check=False) + ("+dirty" if dirty else "")
        self.assertEqual(ours, fcheck_gate.fcheck_commit(repo))
        self.assertTrue(ours.endswith("+dirty"))
        self.assertEqual(proc.git(repo, "rev-parse", "--verify", "-q", "no-such-ref", check=False), "")


# ---- iso9660 ----

class IsoTests(TempDir):
    def setUp(self):
        super().setUp()
        self.image = self.write("any name.iso", build_image(TREE))

    def test_lists_every_file_with_its_path(self):
        listed = iso9660.files(self.image)
        names = [path for path, _lba, _size in listed]
        self.assertEqual(names[:2], ["SYSTEM.CNF", "SCES_999.99"])
        self.assertIn("DATA/SUB/DEEP.BIN", names)
        self.assertEqual(len(listed), 2 + 2 + len(MANY))
        with open(self.image, "rb") as f:
            for path, lba, size in listed:
                f.seek(lba * SECTOR)
                expected = TREE[path] if "/" not in path else {"DATA/LEVEL0.WAD": b"level zero",
                                                               "DATA/SUB/DEEP.BIN": b"deep"}[path]
                self.assertEqual(f.read(size), expected)

    def test_the_root_spans_two_sectors(self):
        with open(self.image, "rb") as f:
            self.assertGreater(iso9660.root_directory(f)[1], SECTOR)

    def test_finds_reads_and_names_the_boot(self):
        self.assertEqual(iso9660.boot_serial(self.image), "SCES_999.99")
        self.assertEqual(iso9660.read(self.image, "SCES_999.99"), BOOT)
        self.assertEqual(iso9660.read(self.image, "data/sub/deep.bin"), b"deep")
        with self.assertRaises(FileNotFoundError):
            iso9660.read(self.image, "NOPE.BIN")

    def test_parse_boot2(self):
        self.assertEqual(iso9660.parse_boot2("BOOT2 = cdrom0:\\SCUS_971.99;1\nVER = 1.00"), "SCUS_971.99")
        self.assertEqual(iso9660.parse_boot2("BOOT2=cdrom0:\\DIR\\SLES_123.45;1"), "SLES_123.45")
        self.assertIsNone(iso9660.parse_boot2("BOOT = cdrom:\\PSX.EXE;1"))

    def test_no_ps2_disc(self):
        blank = self.write("blank.iso", bytes(20 * SECTOR))
        no_cnf = self.write("no_cnf.iso", build_image({"README.TXT": b"hello"}))
        no_boot2 = self.write("no_boot2.iso", build_image({"SYSTEM.CNF": b"VER = 1.00\r\n"}))
        for image in (blank, no_cnf, no_boot2):
            with self.subTest(image=image.name):
                self.assertIsNone(iso9660.boot_serial(image))
        with self.assertRaises(iso9660.IsoError) as caught:
            iso9660.files(blank)
        self.assertEqual(caught.exception.detail, "")

    def test_refuses_what_it_cannot_trust(self):
        cases = {
            "space.iso": ({"A B.BIN": b"x"}, "a file name OpenRAC will not write: 'A B.BIN'"),
            "dots.iso": ({"..": {}}, "a file name OpenRAC will not write: '..'"),
            "deep.iso": (self.nested(iso9660.MAX_DEPTH + 1), "directories nested too deep"),
        }
        for name, (tree, detail) in cases.items():
            with self.subTest(image=name):
                image = self.write(name, build_image(tree))
                with self.assertRaises(iso9660.IsoError) as caught:
                    iso9660.files(image)
                self.assertEqual(caught.exception.detail, detail)
        deepest = self.write("deepest.iso", build_image(self.nested(iso9660.MAX_DEPTH)))
        self.assertEqual(iso9660.files(deepest)[0][0], "D/" * iso9660.MAX_DEPTH + "F.BIN")

    @staticmethod
    def nested(levels: int) -> dict:
        """A file LEVELS directories down: D/D/.../F.BIN."""
        tree: dict = {"F.BIN": b"f"}
        for _ in range(levels):
            tree = {"D": tree}
        return tree

    def test_a_damaged_root_is_refused(self):
        image = bytearray(self.image.read_bytes())
        struct.pack_into("<I", image, 16 * SECTOR + 156 + 10, (iso9660.MAX_ROOT_SECTORS + 1) * SECTOR)
        damaged = self.write("damaged.iso", bytes(image))
        with self.assertRaises(iso9660.IsoError) as caught:
            iso9660.files(damaged)
        self.assertEqual(caught.exception.detail, "the root directory is damaged")

    def test_a_record_cut_short_is_refused(self):
        # The root's own and parent records are 34 bytes each; end the root 20 bytes into the third.
        image = bytearray(self.image.read_bytes())
        struct.pack_into("<I", image, 16 * SECTOR + 156 + 10, 2 * 34 + 20)
        cut = self.write("cut.iso", bytes(image))
        for read in (iso9660.files, iso9660.boot_serial):
            with self.subTest(read=read.__name__), self.assertRaises(iso9660.IsoError) as caught:
                read(cut)
            self.assertEqual(caught.exception.detail, "a directory record is cut short")

    def test_matches_the_editors_disc_reader(self):
        flat = self.write("flat.iso", build_image({"SYSTEM.CNF": CNF, "SCES_999.99": BOOT, **MANY}))
        with disc.Disc(flat) as image:
            theirs = [(r["name"], r["lba"], r["bytes"]) for r in image.files()]
            boot = disc.BOOT.search(CNF).group(1).decode()
        self.assertEqual(iso9660.files(flat), theirs)
        self.assertEqual(iso9660.boot_serial(flat), boot)


if __name__ == "__main__":
    unittest.main()
