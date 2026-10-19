"""Keep compiler outputs outside tracked source directories."""

from __future__ import annotations

import sys
import unittest
import json
import tempfile
from dataclasses import replace
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from compile_boot_trial import ROOT, require_private_output  # noqa: E402
import boot_c_campaign  # noqa: E402
import boot_trial_gate  # noqa: E402
import compile_boot_trial  # noqa: E402
from boot_c_campaign import source_path  # noqa: E402


class CompileBootTrialTests(unittest.TestCase):
    def test_only_measured_sn_debug_variant_is_allowed(self) -> None:
        profile = json.loads(compile_boot_trial.PROFILE.read_text())
        owner = next(item for item in boot_c_campaign.REPLACEMENTS
                     if item.name == "sceKernelGetSystemTimeLow__Fv")
        self.assertTrue(boot_c_campaign.object_digest_matches(
            owner, owner.object_sha256, profile))
        self.assertTrue(boot_c_campaign.object_digest_matches(
            owner, profile["object_sha256_alternates"][owner.name][0], profile))
        self.assertFalse(boot_c_campaign.object_digest_matches(
            owner, "0" * 64, profile))

    def test_assembly_fallback_or_partial_owner_cannot_keep_c_acceptance(self) -> None:
        rows = boot_c_campaign.REPLACEMENTS
        cases = (
            (tuple(row for row in rows if row.name != "memcard_busy"),
             "no campaign owner"),
            (tuple(replace(row, end=row.end - 8) if row.name == "memcard_busy" else row
                   for row in rows),
             "complete C substitution"),
        )
        for changed, message in cases:
            with self.subTest(changed=len(changed)), \
                 patch.object(boot_c_campaign, "REPLACEMENTS", changed):
                with self.assertRaisesRegex(ValueError, message):
                    boot_trial_gate.accepted_body_bytes()

    def test_failed_recompile_clears_a_previous_accepted_report(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            report = Path(directory) / "report.json"
            report.write_text(json.dumps({"status": "passed_boot_only", "counted_matches": 1}))
            argv = ["gate", "boot", "--cpp", "cpp", "--cc1", "cc1", "--as", "as",
                    "--assembler", "sn-as", "--linker", "sn-ld", "--output-dir", directory]
            with patch.object(boot_trial_gate, "REPORT", report), patch.object(sys, "argv", argv), \
                 patch.object(boot_trial_gate, "compile_objects", side_effect=ValueError("control failed")):
                with self.assertRaisesRegex(ValueError, "control failed"):
                    boot_trial_gate.main()
            self.assertEqual(json.loads(report.read_text())["counted_matches"], 0)
            self.assertEqual(json.loads(report.read_text())["status"], "incomplete")

    def test_changed_compiler_patch_or_flags_cannot_reuse_qualification(self) -> None:
        original = json.loads(compile_boot_trial.PROFILE.read_text())
        paths = [Path(name) for name in ("cpp", "cc1", "as")]
        def pinned(path):
            return original["tools"][f"{path.name}_sha256"]
        for case in ("compiler", "patch", "flags"):
            with self.subTest(case=case), tempfile.TemporaryDirectory() as directory:
                profile = json.loads(json.dumps(original))
                if case == "flags":
                    profile["flags"]["cc1"].append("-O0")
                file = Path(directory) / "profile.json"
                file.write_text(json.dumps(profile))
                def digest(path):
                    return "0" * 64 if case == "compiler" and path.name == "cc1" else pinned(path)
                patch_hash = "0" * 64 if case == "patch" else original["source"]["patch_sha256_lf"]
                with patch.object(compile_boot_trial, "PROFILE", file), \
                     patch.object(compile_boot_trial, "digest", side_effect=digest), \
                     patch.object(compile_boot_trial, "normalized_digest", return_value=patch_hash):
                    with self.assertRaises(ValueError):
                        compile_boot_trial.check_profile(*paths)

    def test_accepted_boot_source_and_extent_are_explicit(self) -> None:
        self.assertEqual(source_path("memcard_busy"),
                         ROOT / "src/boot/memcard_busy.c")
        self.assertEqual(source_path("fog_preupdate"),
                         ROOT / "nonmatching/boot/fog_preupdate.c")
        accepted = boot_c_campaign.accepted_replacements()
        self.assertEqual({item.name for item in accepted},
                         set(boot_c_campaign.ACCEPTED_SOURCES))
        for item in accepted:
            self.assertEqual(source_path(item.name),
                             ROOT / "src/boot" / f"{item.name}.c")
            self.assertFalse((ROOT / "nonmatching/boot" / f"{item.name}.c").exists())
        # Counted bytes are compiled bodies only, excluding any linker padding.
        matches = boot_trial_gate.accepted_matches()
        self.assertEqual(len(matches), len(accepted))
        self.assertEqual([row["owner"] for row in matches],
                         [item.name for item in accepted])
        self.assertEqual(boot_trial_gate.accepted_body_bytes(),
                         sum(row["body_bytes"] for row in matches))

    def test_verified_alignment_is_not_counted_as_c(self) -> None:
        owner = boot_c_campaign.Replacement("unit", 0x1000, 0x1010, "", "")
        row = {"symbol": "unit", "start": 0x1000, "body_end": 0x100C,
               "next_label": 0x1010, "body_bytes": 12,
               "zero_padding_bytes": 4, "exit": "jr_ra"}
        with patch.object(boot_trial_gate, "accepted_replacements",
                          return_value=(owner,)), \
             patch.object(boot_trial_gate, "catalogue_rows",
                          return_value=[row]):
            self.assertEqual(boot_trial_gate.accepted_matches()[0]["body_bytes"], 12)
            self.assertEqual(boot_trial_gate.accepted_body_bytes(), 12)

    def test_an_accepted_name_without_a_campaign_owner_keeps_no_credit(self) -> None:
        with patch.object(boot_c_campaign, "ACCEPTED_SOURCES",
                          boot_c_campaign.ACCEPTED_SOURCES | {"not_a_body"}):
            with self.assertRaisesRegex(ValueError, "no campaign owner"):
                boot_trial_gate.accepted_body_bytes()

    def test_a_promoted_source_cannot_stay_staged_in_nonmatching(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "src/boot").mkdir(parents=True)
            (root / "nonmatching/boot").mkdir(parents=True)
            for folder in ("src/boot", "nonmatching/boot"):
                (root / folder / "unit.c").write_text("int f(void) { return 0; }\n")
            owner = boot_c_campaign.Replacement("unit", 0x1000, 0x1008, "", "")
            with patch.object(boot_c_campaign, "ROOT", root), \
                 patch.object(boot_c_campaign, "REPLACEMENTS", (owner,)), \
                 patch.object(boot_c_campaign, "ACCEPTED_SOURCES", frozenset({"unit"})):
                with self.assertRaisesRegex(ValueError, "still staged"):
                    boot_c_campaign.accepted_replacements()

    def test_an_accepted_body_must_fill_its_whole_placement(self) -> None:
        rows = boot_trial_gate.catalogue_rows()
        index = next(i for i, row in enumerate(rows)
                     if row["symbol"] == "MEMCARD_Busy__Fv")
        for field, value in (("zero_padding_bytes", 4), ("exit", "call"),
                             ("body_end", rows[index]["start"] + 8)):
            changed = [dict(row) for row in rows]
            changed[index][field] = value
            with self.subTest(field=field), \
                 patch.object(boot_trial_gate, "catalogue_rows", return_value=changed):
                with self.assertRaisesRegex(ValueError, "complete C substitution"):
                    boot_trial_gate.accepted_matches()

    def test_rejects_tracked_output_and_accepts_ignored_build(self) -> None:
        with self.assertRaisesRegex(ValueError, "ignored build"):
            require_private_output(ROOT / "nonmatching/boot")
        self.assertEqual(require_private_output(ROOT / "build/boot-objects"),
                         (ROOT / "build/boot-objects").resolve())


if __name__ == "__main__":
    unittest.main()
