"""Output safety: new directories under assets/ or build/, published whole."""

import io
import json
from pathlib import Path
import pickle
import sys
import tempfile
import unittest
from concurrent.futures import ProcessPoolExecutor
from unittest import mock

import extract
from extract import ROOT, output_path, publish, worker_pool
from formats import FormatError


class OutputTests(unittest.TestCase):
    def test_output_must_be_new_and_ignored(self):
        with self.assertRaises(FormatError):
            output_path(ROOT / "docs" / "extracted")
        with self.assertRaises(FormatError):
            output_path(ROOT / "assets" / ".." / "extracted")
        (ROOT / "build").mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=ROOT / "build") as temp:
            with self.assertRaises(FormatError):
                output_path(Path(temp))
            escape = Path(temp) / "escape"
            escape.symlink_to(ROOT / "docs", target_is_directory=True)
            with self.assertRaises(FormatError):
                output_path(escape / "extracted")

    def test_publish_is_all_or_nothing(self):
        (ROOT / "build").mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=ROOT / "build") as temp:
            dest = Path(temp) / "out"

            def fail(directory):
                (directory / "partial").write_text("x")
                raise FormatError("stop")

            with self.assertRaises(FormatError):
                publish(dest, fail)
            self.assertEqual(list(Path(temp).iterdir()), [])
            publish(dest, lambda directory: (directory / "done").write_text("x"))
            self.assertEqual([p.name for p in dest.iterdir()], ["done"])


class PoolTests(unittest.TestCase):
    def test_processes_where_python_has_them(self):
        with worker_pool(2) as pool:
            self.assertIsInstance(pool, ProcessPoolExecutor)

    def test_processes_of_their_own_without_sem_open(self):
        # Android's CPython: building a process pool fails for want of sem_open.
        lacking = mock.Mock(side_effect=NotImplementedError("This platform lacks sem_open"))
        with mock.patch.object(extract, "ProcessPoolExecutor", lacking):
            with worker_pool(8) as pool:
                self.assertIsInstance(pool, extract.SubprocessPool)
                # Only the level exports run there.
                with self.assertRaises(RuntimeError):
                    pool.submit(sum, [1, 2]).result()

    def test_a_worker_runs_one_export_and_pickles_its_result(self):
        stdin = io.TextIOWrapper(io.BytesIO(pickle.dumps(("export_port_title", ("disc.iso",), {"lod": 1}))))
        stdout = io.TextIOWrapper(io.BytesIO())

        def export(iso, lod):
            print("progress that must not reach the result")
            return {"level": 99, "iso": iso, "lod": lod}

        with mock.patch.object(extract, "export_port_title", export), mock.patch.object(sys, "stdin", stdin), \
                mock.patch.object(sys, "stdout", stdout), mock.patch.object(sys, "stderr", io.StringIO()):
            extract.worker()
        self.assertEqual(pickle.loads(stdout.buffer.getvalue()), {"level": 99, "iso": "disc.iso", "lod": 1})


if __name__ == "__main__":
    unittest.main()


class AdoptFlatExportTests(unittest.TestCase):
    """Levels extracted before each version had its own folder move into it, once."""

    def flat(self, root: Path, version: str) -> Path:
        parent = root / "port"
        for name in ("level_00", "level_99"):
            (parent / name).mkdir(parents=True)
            (parent / name / "manifest.json").write_text(json.dumps({"format": 3, "version": version}))
        (parent / "verified-3.json").write_text("{}")
        return parent

    def test_moves_the_same_versions_levels_and_stamps(self):
        from extract import adopt_flat_export
        with tempfile.TemporaryDirectory() as tmp:
            parent = self.flat(Path(tmp), "pal")
            self.assertTrue(adopt_flat_export(parent / "pal", "pal"))
            self.assertEqual(sorted(p.name for p in (parent / "pal").iterdir()),
                             ["level_00", "level_99", "verified-3.json"])
            self.assertEqual(sorted(p.name for p in parent.iterdir()), ["pal"])
            self.assertFalse(adopt_flat_export(parent / "pal", "pal"))  # once

    def test_leaves_another_versions_levels(self):
        from extract import adopt_flat_export
        with tempfile.TemporaryDirectory() as tmp:
            parent = self.flat(Path(tmp), "pal")
            self.assertFalse(adopt_flat_export(parent / "ntsc", "ntsc"))
            self.assertFalse((parent / "ntsc").exists())
            self.assertTrue((parent / "level_99").is_dir())

    def test_moves_godot_projects_and_packed_levels_of_pal_only(self):
        from extract import adopt_flat_export
        with tempfile.TemporaryDirectory() as tmp:
            levels = Path(tmp) / "levels"
            (levels / "level_00").mkdir(parents=True)
            (levels / "level_00" / "project.godot").write_text("edited")
            self.assertFalse(adopt_flat_export(levels / "ntsc", "ntsc", "godot"))
            self.assertTrue(adopt_flat_export(levels / "pal", "pal", "godot"))
            self.assertEqual((levels / "pal" / "level_00" / "project.godot").read_text(), "edited")
            packed = Path(tmp) / "packed"
            (packed / "level_03").mkdir(parents=True)
            (packed / "level_03" / "gameplay_pal.wad").write_bytes(b"x")
            (packed / "patches.tsv").write_text("")
            self.assertTrue(adopt_flat_export(packed / "pal", "pal", "packed"))
            self.assertEqual(sorted(p.name for p in (packed / "pal").iterdir()), ["level_03", "patches.tsv"])


def fake_godot(schema: int, split: bool = True):
    """An exporter shaped as godot(): a project per level (or one for all) with its level.json."""
    def export(out: Path, levels: list[int]) -> None:
        for i in levels:
            project = out / f"level_{i:02}" if split else out
            level = project / "levels" / f"level_{i:02}"
            level.mkdir(parents=True)
            (project / "project.godot").write_text("config_version=5\n")
            (level / "level.json").write_text(json.dumps({"schema": schema, "level": i}))
            (level / f"level_{i:02}.tscn").write_text("[gd_scene format=3]\n")
    return export


class SecondExportTests(unittest.TestCase):
    """The launcher exports into its own data folder again and again: each time must work."""

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.data = Path(self.temp.name)

    def tearDown(self):
        self.temp.cleanup()

    def scene(self, dest: Path, i: int, split: bool = True) -> Path:
        project = dest / f"level_{i:02}" if split else dest
        return project / "levels" / f"level_{i:02}" / f"level_{i:02}.tscn"

    def test_godot_twice_keeps_edits_and_adds_levels(self):
        dest = self.data / "levels" / "pal"
        schema = extract.GODOT_SCHEMA
        said = extract.export_godot(dest, [0, 1], True, True, fake_godot(schema))
        self.assertIn("one per level", said)
        self.scene(dest, 0).write_text("[gd_scene format=3]\n; moved a crate\n")
        said = extract.export_godot(dest, [0, 1, 2], True, True, fake_godot(schema))
        self.assertIn("2 level(s) kept", said)
        self.assertIn("1 added", said)
        self.assertIn("moved a crate", self.scene(dest, 0).read_text())
        self.assertTrue(self.scene(dest, 2).is_file())
        self.assertTrue((dest / "level_00" / "rc1" / "level.gd").is_file())     # scripts refreshed
        self.assertFalse((self.data / "levels" / "pal-backups").exists())

    def test_godot_after_a_format_bump_backs_up_and_replaces(self):
        dest = self.data / "levels" / "pal"
        old = extract.GODOT_SCHEMA - 1
        extract.export_godot(dest, [0, 1], True, True, fake_godot(old))
        self.scene(dest, 1).write_text("[gd_scene format=3]\n; my edit\n")
        said = extract.export_godot(dest, [0, 1], True, True, fake_godot(extract.GODOT_SCHEMA))
        self.assertIn("older extractor", said)
        self.assertEqual(extract.level_schema(self.scene(dest, 1).parent), extract.GODOT_SCHEMA)
        backups = list((self.data / "levels" / "pal-backups").iterdir())
        self.assertEqual(len(backups), 1)
        self.assertIn("my edit", (backups[0] / "level_01" / "levels" / "level_01" / "level_01.tscn").read_text())

    def test_godot_one_project_for_every_level(self):
        dest = self.data / "godot"
        extract.export_godot(dest, [0], True, False, fake_godot(extract.GODOT_SCHEMA, split=False))
        self.scene(dest, 0, split=False).write_text("; edited\n")
        extract.export_godot(dest, [0, 3], True, False, fake_godot(extract.GODOT_SCHEMA, split=False))
        self.assertIn("edited", self.scene(dest, 0, split=False).read_text())
        self.assertTrue(self.scene(dest, 3, split=False).is_file())

    def test_port_twice_and_after_a_format_bump(self):
        dest = self.data / "port" / "pal"

        def export(fmt):
            def write(out: Path) -> None:
                (out / "level_00").mkdir()
                (out / "level_00" / "manifest.json").write_text(json.dumps({"format": fmt}))
            return write

        for fmt in (2, 3, 3):
            extract.ours(output_path(dest, True, replace=True), "port")
            publish(dest, export(fmt), True, replace=True)
            self.assertEqual(json.loads((dest / "level_00" / "manifest.json").read_text())["format"], fmt)

    def test_a_folder_that_is_not_an_export_is_refused(self):
        dest = self.data / "port"
        dest.mkdir()
        (dest / "notes.txt").write_text("mine")
        with self.assertRaisesRegex(FormatError, "choose another folder"):
            extract.ours(dest, "port")
        with self.assertRaisesRegex(FormatError, "choose another folder"):
            extract.export_godot(dest, [0], True, True, fake_godot(extract.GODOT_SCHEMA))
        godot = self.data / "levels"
        extract.export_godot(godot, [0], True, True, fake_godot(extract.GODOT_SCHEMA))
        with self.assertRaisesRegex(FormatError, "Godot projects"):
            extract.ours(godot, "port")
