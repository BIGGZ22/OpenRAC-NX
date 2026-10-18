"""The index gate catches accidentally tracked game inputs."""

from __future__ import annotations

import sys
import unittest
import tempfile
from types import SimpleNamespace
from unittest.mock import patch
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from check_repository import forbidden_reason  # noqa: E402
import check_repository  # noqa: E402


class RepositoryTests(unittest.TestCase):
    def test_scratch_is_private_but_named_source_forms_are_allowed(self) -> None:
        for name in ("scratch/probe.py", "nonmatching/boot/_scratch/form.c",
                     "nonmatching/boot/_scratch2/report.txt", "scratch\\variant.c"):
            with self.subTest(name=name):
                self.assertEqual(forbidden_reason(name), "private scratch experiment directory")
        self.assertIsNone(forbidden_reason("nonmatching/boot/alternatives/MEMCARD_Test__Fi-form-01.c"))

    def test_tracked_hybrid_source_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / "src/boot/hybrid.c"
            path.parent.mkdir(parents=True)
            path.write_text('void f(void) { __asm__("syscall"); }\n')
            with patch.object(check_repository, "ROOT", root), patch(
                "subprocess.run", return_value=SimpleNamespace(stdout=b"src/boot/hybrid.c\0")
            ):
                problems = check_repository.check()
            self.assertEqual(len(problems), 1)
            self.assertIn("hybrid.c", problems[0])
            self.assertIn("not C evidence", problems[0])

    def test_private_paths(self) -> None:
        self.assertIsNotNone(forbidden_reason("baserom/SCUS_976.15"))
        self.assertIsNotNone(forbidden_reason("src/retail.elf"))
        self.assertIsNotNone(forbidden_reason(".env.local"))
        self.assertIsNone(forbidden_reason("src/boot/func_00100000.c"))
