# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Tests for tools/sources.py: which versions have a source project, where its checkout and tree
are, what a sync records, and what status prints. A synthetic games/ tree and a fake git: no
checkout, no network."""
import contextlib
import io
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import sources  # noqa: E402

SOURCE = {"name": "a-decomp", "repo": "https://example.invalid/a.git", "branch": "main",
          "checkout": "~/src/a-decomp", "commit": "1" * 40, "date": "2026-01-01",
          "license": "MIT", "exclude": ["asm/", "tools/x.py"]}


class FakeGit:
    """subprocess.run for `git -C REPO ...`: answers from a table of argument tuples (after the
    repository) and records each call."""

    def __init__(self, answers: dict[tuple[str, ...], bytes]):
        self.answers, self.calls = answers, []

    def __call__(self, cmd, **kwargs):
        assert cmd[:2] == ["git", "-C"], cmd
        self.calls.append(tuple(cmd[3:]))
        out = self.answers[tuple(cmd[3:])]
        return subprocess.CompletedProcess(cmd, 0, out, b"")


class SourcesTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = Path(tmp.name)
        for game, versions in (("a", {"pal": {"source": SOURCE, "dir": "games/a"}, "ntsc": {}}),
                               ("b", {"ntsc": {"source": dict(SOURCE, name="b-decomp")}})):
            path = self.root / "games" / game / "game.json"
            path.parent.mkdir(parents=True)
            path.write_text(json.dumps({"id": game, "versions": versions}), encoding="utf-8")
        patcher = mock.patch.object(sources, "ROOT", self.root)
        patcher.start()
        self.addCleanup(patcher.stop)

    def test_manifests_lists_the_versions_with_a_source(self):
        found = sources.manifests()
        self.assertEqual(list(found), ["a/pal", "b/ntsc"])
        path, game = found["a/pal"]
        self.assertEqual(path, self.root / "games/a/game.json")
        self.assertEqual(game["id"], "a")

    def test_version_dir_takes_dir_or_the_default(self):
        found = sources.manifests()
        self.assertEqual(sources.version_dir(found["a/pal"][1], "pal"), self.root / "games/a")
        self.assertEqual(sources.version_dir(found["b/ntsc"][1], "ntsc"), self.root / "games/b/ntsc")

    def test_checkout_is_the_environment_else_the_manifest(self):
        with mock.patch.dict(os.environ, {"OPENRAC_SOURCE_A_PAL": "/elsewhere"}):
            self.assertEqual(sources.checkout("a/pal", SOURCE), Path("/elsewhere"))
        with mock.patch.dict(os.environ, {"OPENRAC_SOURCE_A_PAL": ""}):
            self.assertEqual(sources.checkout("a/pal", SOURCE), Path(os.path.expanduser("~/src/a-decomp")))

    def test_excluded(self):
        self.assertTrue(sources.excluded("asm/x.s", SOURCE["exclude"]))
        # The folder's own entry is not excluded by "asm/": only what is in it.
        self.assertFalse(sources.excluded("asm", SOURCE["exclude"]))
        self.assertTrue(sources.excluded("tools/x.py", SOURCE["exclude"]))
        self.assertFalse(sources.excluded("asmx/y.s", SOURCE["exclude"]))
        self.assertFalse(sources.excluded("tools/x.pyc", SOURCE["exclude"]))

    def test_record_writes_the_commit_and_its_date(self):
        path, game = sources.manifests()["a/pal"]
        git = FakeGit({("log", "-1", "--format=%cs", "2" * 40): b"2026-10-01\n"})
        with mock.patch("subprocess.run", git), mock.patch("time.strftime", return_value="2026-10-23"):
            sources.record(path, game, "pal", "2" * 40, Path("/repo"))
        written = json.loads(path.read_text(encoding="utf-8"))["versions"]["pal"]["source"]
        self.assertEqual((written["commit"], written["date"], written["imported"]),
                         ("2" * 40, "2026-10-01", "2026-10-23"))
        self.assertTrue(path.read_text(encoding="utf-8").endswith("}\n"))

    def test_sync_at_the_recorded_commit_does_nothing(self):
        git = FakeGit({("rev-parse", "HEAD"): ("1" * 40 + "\n").encode()})
        out = io.StringIO()
        with mock.patch("subprocess.run", git), contextlib.redirect_stdout(out):
            sources.do_sync("a/pal", None)
        self.assertEqual(out.getvalue(), f"a/pal: already at {'1' * 10}\n")
        self.assertEqual(git.calls, [("rev-parse", "HEAD")])

    def test_import_refuses_a_tree_that_is_not_empty(self):
        (self.root / "games/a/main.c").write_text("")
        with mock.patch("subprocess.run", FakeGit({})), self.assertRaises(SystemExit) as stop:
            sources.do_import("a/pal", None)
        self.assertEqual(str(stop.exception), "games/a is not empty: use sync")

    def test_status(self):
        checkout = self.root / "checkout"
        (checkout / ".git").mkdir(parents=True)
        git = FakeGit({("rev-list", "--count", f"{'1' * 40}..HEAD"): b"7\n"})
        out = io.StringIO()
        env = {"OPENRAC_SOURCE_A_PAL": str(checkout), "OPENRAC_SOURCE_B_NTSC": str(self.root / "none")}
        with mock.patch.dict(os.environ, env), mock.patch("subprocess.run", git), contextlib.redirect_stdout(out):
            sources.do_status()
        self.assertEqual(out.getvalue(),
                         f"a/pal: a-decomp at {'1' * 10} (2026-01-01); checkout {checkout} is 7 commits past it\n"
                         f"b/ntsc: no checkout at {self.root / 'none'}\n")


if __name__ == "__main__":
    unittest.main()
