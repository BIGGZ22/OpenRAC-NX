"""Output safety: new directories under assets/ or build/, published whole."""

import json
from pathlib import Path
import tempfile
import unittest

from extract import ROOT, output_path, publish
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
