"""Output safety: new directories under assets/ or build/, published whole."""

import json
from pathlib import Path
import tempfile
import unittest
from concurrent.futures import ProcessPoolExecutor, ThreadPoolExecutor
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

    def test_one_thread_without_sem_open(self):
        # Android's CPython: building a process pool fails for want of sem_open.
        lacking = mock.Mock(side_effect=NotImplementedError("This platform lacks sem_open"))
        with mock.patch.object(extract, "ProcessPoolExecutor", lacking):
            with worker_pool(8) as pool:
                self.assertIsInstance(pool, ThreadPoolExecutor)
                self.assertEqual(pool.submit(sum, [1, 2]).result(), 3)


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
