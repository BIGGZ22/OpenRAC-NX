"""Per-source toolchain selection: format, validation and application.

One line may select a compiler, an assembler, both or neither, so the checks
below cover each selector on its own, the two together, and the committed file.
"""

from __future__ import annotations

import argparse
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import cflags  # noqa: E402
import compile_boot_trial  # noqa: E402

MEASURED = "src/boot/dlib_displist_direct_end__FP13DLIB_DISPLIST.c"
ROUTE_P = "src/boot/ps2fioClose.c"
ROUTE_Q = "src/boot/sceCdSyncS.c"
ROUTE_R_DLI = "src/boot/__pack_d.c"
ROUTE_R_FLOAT = "src/boot/sceKernelGetSystemTimeLow__Fv.c"
SECOND = "prodg2953"
SONY = "sdk991111"
DIVPAD = "sdk-divpad"
SDK_AS = "ee-sdk"
PS2_AS = "ps2eeas"


class CFlagsTests(unittest.TestCase):
    def test_committed_file_lists_campaign_sources_with_measured_flags(self) -> None:
        entries = cflags.load()
        for name, entry in entries.items():
            with self.subTest(name=name):
                self.assertIn(name, cflags.campaign_sources())
                self.assertTrue((cflags.ROOT / name).is_file())
                self.assertTrue(all(flag.startswith("-") for flag in entry.flags))
                self.assertIn(entry.compiler, cflags.pinned_compilers())
        self.assertEqual(entries.get(MEASURED),
                         cflags.Entry(cflags.DEFAULT_COMPILER,
                                      ("-mno-check-zero-division",)))
        self.assertEqual(entries.get(ROUTE_P),
                         cflags.Entry(SECOND, ("-O2", "-G0")))
        self.assertEqual(entries.get(ROUTE_Q),
                         cflags.Entry(SONY, ("-O2", "-G0")))

    def test_selector_names_a_pinned_second_compiler(self) -> None:
        profile = json.loads(cflags.PROFILE.read_text(encoding="utf-8"))
        self.assertIn(SECOND, profile["extra_compilers"])
        pinned = profile["extra_compilers"][SECOND]
        for key in ("driver_sha256", "cpp_sha256", "cc1_sha256"):
            self.assertEqual(len(pinned["tools"][key]), 64)
        self.assertEqual(cflags.pinned_compilers(),
                         (cflags.DEFAULT_COMPILER, "gnu-ee-trial", SECOND,
                          DIVPAD, SONY))
        self.assertTrue(cflags.Entry(SECOND, ("-O2",)).compiler != cflags.DEFAULT_COMPILER)
        self.assertTrue(cflags.DEFAULT_ENTRY.pinned)

    def test_every_pinned_compiler_states_its_host_and_provenance(self) -> None:
        profile = json.loads(cflags.PROFILE.read_text(encoding="utf-8"))
        for selector, pinned in profile["extra_compilers"].items():
            with self.subTest(selector=selector):
                # A routed object is built by a driver that must be found on the
                # host the pin names, so an unstated host may not pass silently.
                self.assertIn(pinned["host"], compile_boot_trial.HOSTS)
                self.assertEqual(pinned["flags"], {"driver": ["-S"], "as": []})
                self.assertTrue(pinned["id"].startswith("rac5-boot-"))
                for key in ("driver_sha256", "cpp_sha256", "cc1_sha256"):
                    int(pinned["tools"][key], 16)
                self.assertGreater(len(pinned["provenance"]), 80)
        self.assertEqual(profile["extra_compilers"][SECOND]["host"], "windows")
        self.assertEqual(profile["extra_compilers"][SONY]["host"], "linux")

    def test_parses_comments_blanks_and_rejects_duplicates(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "nonmatching/boot").mkdir(parents=True)
            (root / "nonmatching/boot/unit.c").write_text("int f;\n")
            file = root / "cflags.txt"
            known = frozenset({"nonmatching/boot/unit.c"})
            with patch.object(cflags, "ROOT", root), \
                 patch.object(cflags, "campaign_sources", return_value=known):
                file.write_text("# header\n\nnonmatching/boot/unit.c  -O0  # trailing\n")
                self.assertEqual(cflags.load(file),
                                 {"nonmatching/boot/unit.c":
                                  cflags.Entry(cflags.DEFAULT_COMPILER, ("-O0",))})
                file.write_text(f"nonmatching/boot/unit.c  {SECOND} -O2 -G0\n")
                self.assertEqual(cflags.load(file),
                                 {"nonmatching/boot/unit.c": cflags.Entry(SECOND, ("-O2", "-G0"))})
                file.write_text("nonmatching/boot/unit.c  -O0\nnonmatching/boot/unit.c  -O1\n")
                with self.assertRaisesRegex(ValueError, "duplicate entry"):
                    cflags.load(file)
                file.write_text("nonmatching/boot/unit.c\n")
                with self.assertRaisesRegex(ValueError, "no cc1 flags"):
                    cflags.load(file)
                file.write_text(f"nonmatching/boot/unit.c  {SECOND}\n")
                with self.assertRaisesRegex(ValueError, "no cc1 flags"):
                    cflags.load(file)

    def test_rejects_an_unpinned_selector(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "nonmatching/boot").mkdir(parents=True)
            (root / "nonmatching/boot/unit.c").write_text("int f;\n")
            file = root / "cflags.txt"
            known = frozenset({"nonmatching/boot/unit.c"})
            for line in ("nonmatching/boot/unit.c  sdk -O2 -G0\n",
                         "nonmatching/boot/unit.c  gcc2953 -O2\n",
                         "nonmatching/boot/unit.c  prodg -O2\n"):
                with self.subTest(line=line), patch.object(cflags, "ROOT", root), \
                     patch.object(cflags, "campaign_sources", return_value=known):
                    file.write_text(line)
                    with self.assertRaisesRegex(ValueError, "unknown compiler selector"):
                        cflags.load(file)

    def test_rejects_a_bare_word_after_the_selector(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "nonmatching/boot").mkdir(parents=True)
            (root / "nonmatching/boot/unit.c").write_text("int f;\n")
            file = root / "cflags.txt"
            known = frozenset({"nonmatching/boot/unit.c"})
            with patch.object(cflags, "ROOT", root), \
                 patch.object(cflags, "campaign_sources", return_value=known):
                file.write_text(f"nonmatching/boot/unit.c  {SECOND} -O2 quiet\n")
                with self.assertRaisesRegex(ValueError, "not a flag"):
                    cflags.load(file)

    def test_rejects_missing_foreign_and_unnormalised_paths(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "nonmatching/boot").mkdir(parents=True)
            (root / "nonmatching/boot/unit.c").write_text("int f;\n")
            (root / "src/levels").mkdir(parents=True)
            (root / "src/levels/other.c").write_text("int g;\n")
            file = root / "cflags.txt"
            known = frozenset({"nonmatching/boot/unit.c"})
            for line, message in (
                ("nonmatching/boot/absent.c  -O0\n", "source file does not exist"),
                ("nonmatching/boot/absent.c  prodg2953 -O0\n", "source file does not exist"),
                ("src/levels/other.c  -O0\n", "not a campaign C source"),
                ("src/levels/other.c  prodg2953 -O2\n", "not a campaign C source"),
                ("/abs/unit.c  -O0\n", "not a repository-relative C source"),
                ("../nonmatching/boot/unit.c  -O0\n", "not a repository-relative C source"),
                ("nonmatching/boot/unit.cc  -O0\n", "not a repository-relative C source"),
            ):
                with self.subTest(line=line), patch.object(cflags, "ROOT", root), \
                     patch.object(cflags, "campaign_sources", return_value=known):
                    file.write_text(line)
                    with self.assertRaisesRegex(ValueError, message):
                        cflags.load(file)

    def test_for_source_matches_only_listed_repository_sources(self) -> None:
        entries = {"nonmatching/boot/unit.c": cflags.Entry(SECOND, ("-O0",))}
        source = cflags.ROOT / "nonmatching/boot/unit.c"
        self.assertEqual(cflags.for_source(entries, source), entries[
            "nonmatching/boot/unit.c"])
        self.assertEqual(cflags.for_source(entries, Path("/tmp/elsewhere/unit.c")),
                         cflags.DEFAULT_ENTRY)
        self.assertEqual(cflags.for_source(entries, cflags.ROOT / "tools/cflags.py"),
                         cflags.DEFAULT_ENTRY)

    def test_profile_flags_are_extended_per_source(self) -> None:
        profile = {"flags": {"cc1": ["-quiet", "-O2", "-G0"]}}
        source = cflags.ROOT / MEASURED
        self.assertEqual(compile_boot_trial.cc1_flags(profile, {}, source),
                         ["-quiet", "-O2", "-G0"])
        self.assertEqual(compile_boot_trial.cc1_flags(profile, cflags.load(), source),
                         ["-quiet", "-O2", "-G0", "-mno-check-zero-division"])
        self.assertEqual(compile_boot_trial.cc1_flags(profile, cflags.load(),
                                                      cflags.ROOT / "src/boot/memcard_busy.c"),
                         ["-quiet", "-O2", "-G0"])

    def test_selector_flags_belong_to_that_compiler_alone(self) -> None:
        entries = cflags.load()
        source = cflags.ROOT / ROUTE_P
        profile = {"flags": {"cc1": ["-quiet", "-O2", "-G0"]}}
        # The pinned chain keeps its own global flags and never inherits them.
        self.assertEqual(compile_boot_trial.cc1_flags(profile, entries, source),
                         ["-quiet", "-O2", "-G0"])
        extra = {"flags": {"driver": ["-S"], "as": []}}
        self.assertEqual(compile_boot_trial.compiler_flags(extra, entries, source),
                         ["-S", "-O2", "-G0"])

    def test_repository_relative_include_flags_resolve_against_the_root(self) -> None:
        source = cflags.ROOT / "nonmatching/boot/__svfscanf.c"
        include = "third_party/newlib-1.8.2/newlib/libc/include"
        entries = {source.relative_to(cflags.ROOT).as_posix():
                   cflags.Entry(SECOND, ("-O2", "-I" + include))}
        extra = {"host": "linux", "flags": {"driver": ["-S"], "as": []}}
        self.assertEqual(
            compile_boot_trial.compiler_flags(extra, entries, source),
            ["-S", "-O2", "-I" + str((cflags.ROOT / include).resolve())])

    def test_an_unpinned_or_unprovided_second_compiler_is_rejected(self) -> None:
        profile = json.loads(compile_boot_trial.PROFILE.read_text(encoding="utf-8"))
        with self.assertRaisesRegex(ValueError, "pins no compiler"):
            compile_boot_trial.check_extra_compiler(profile, "not-pinned", Path("/nope"))
        for selector in (SECOND, SONY):
            with self.subTest(selector=selector), \
                 self.assertRaisesRegex(ValueError, f"--extra-bin {selector}="):
                compile_boot_trial.check_extra_compiler(profile, selector, None)

    def test_extra_compiler_stage_comes_from_the_pin_or_the_output_dir(self) -> None:
        def pin(host, **extra):
            return {"id": "rac5-boot-unit", "host": host, "provenance": "u" * 81,
                    "flags": {"driver": ["-S"], "as": []},
                    "tools": {"driver": "d", "libexec": ".", "cpp": "c", "cc1": "x",
                              "driver_sha256": "0" * 64, "cpp_sha256": "0" * 64,
                              "cc1_sha256": "0" * 64}, **extra}

        profile = {"extra_compilers": {"win": pin("windows", stage="build/stage-win"),
                                       "lin": pin("linux"),
                                       "bad": pin("plan9")}}
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "repo"
            root.mkdir()
            (root / "bin").mkdir()
            for name in ("d", "c", "x"):
                (root / "bin" / name).write_text("tool\n")
            output = Path(directory) / "out"
            with patch.object(compile_boot_trial, "ROOT", root), \
                 patch.object(compile_boot_trial, "digest", return_value="0" * 64):
                # A pin that names a stage keeps it; one that does not stages
                # under --output-dir, the native Linux directory in a real run.
                win = compile_boot_trial.check_extra_compiler(
                    profile, "win", root / "bin", None, output)
                lin = compile_boot_trial.check_extra_compiler(
                    profile, "lin", root / "bin", None, output)
                override = compile_boot_trial.check_extra_compiler(
                    profile, "lin", root / "bin", root / "build/elsewhere", output)
                self.assertEqual(win["stage"], root / "build/stage-win")
                self.assertEqual(lin["stage"], output / "lin-stage")
                self.assertEqual(override["stage"], root / "build/elsewhere")
                with self.assertRaisesRegex(ValueError, "under ignored build/"):
                    compile_boot_trial.check_extra_compiler(
                        profile, "lin", root / "bin", root / "elsewhere", output)
                self.assertEqual(lin["host"], "linux")
                with self.assertRaisesRegex(ValueError, "is not one of"):
                    compile_boot_trial.check_extra_compiler(
                        profile, "bad", root / "bin", None, output)

    def test_extra_bin_option_splits_a_selector_from_its_directory(self) -> None:
        self.assertEqual(compile_boot_trial.extra_option("sdk991111=/opt/sdk"),
                         ("sdk991111", Path("/opt/sdk")))
        for value in ("sdk991111", "=/opt/sdk", "sdk991111="):
            with self.subTest(value=value), \
                 self.assertRaisesRegex(Exception, "SELECTOR=DIRECTORY"):
                compile_boot_trial.extra_option(value)

    def test_campaign_sources_cover_owned_and_staged_sources(self) -> None:
        known = cflags.campaign_sources()
        self.assertIn(MEASURED, known)
        self.assertIn(ROUTE_P, known)
        self.assertIn("src/boot/memcard_busy.c", known)
        self.assertIn("src/levels/shared/func_00000100.c", known)
        self.assertNotIn("tools/cflags.py", known)

    def test_committed_file_selects_a_measured_assembler_per_source(self) -> None:
        entries = cflags.load()
        # Route R: one body needs the SDK expansion of a 64-bit `dli`, the other
        # needs Ps2EeAs to inline a `li.d` the pinned `as` loads from the pool.
        # Both keep the pinned chain's compiler and its global flags.
        self.assertEqual(entries.get(ROUTE_R_DLI),
                         cflags.Entry(cflags.DEFAULT_COMPILER, (), SDK_AS))
        self.assertEqual(entries.get(ROUTE_R_FLOAT),
                         cflags.Entry(cflags.DEFAULT_COMPILER, (), PS2_AS))
        for name, entry in entries.items():
            with self.subTest(name=name):
                self.assertIn(entry.assembler, cflags.pinned_assemblers())

    def test_assembler_selector_parses_alone_and_beside_a_compiler(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "nonmatching/boot").mkdir(parents=True)
            (root / "nonmatching/boot/unit.c").write_text("int f;\n")
            file = root / "cflags.txt"
            known = frozenset({"nonmatching/boot/unit.c"})
            with patch.object(cflags, "ROOT", root), \
                 patch.object(cflags, "campaign_sources", return_value=known):
                # An assembler selector alone is a complete entry: it says the
                # pinned chain keeps its global flags and another `as` runs.
                file.write_text(f"nonmatching/boot/unit.c  as={SDK_AS}\n")
                self.assertEqual(cflags.load(file),
                                 {"nonmatching/boot/unit.c": cflags.Entry(
                                     cflags.DEFAULT_COMPILER, (), SDK_AS)})
                file.write_text(f"nonmatching/boot/unit.c  {SECOND} as={PS2_AS} -O2 -G0\n")
                self.assertEqual(cflags.load(file),
                                 {"nonmatching/boot/unit.c": cflags.Entry(
                                     SECOND, ("-O2", "-G0"), PS2_AS)})
                # The selector order inside the entry does not change its meaning.
                file.write_text(f"nonmatching/boot/unit.c  as={PS2_AS} {SECOND} -O2 -G0\n")
                self.assertEqual(cflags.load(file),
                                 {"nonmatching/boot/unit.c": cflags.Entry(
                                     SECOND, ("-O2", "-G0"), PS2_AS)})

    def test_rejects_a_bad_assembler_selector(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "nonmatching/boot").mkdir(parents=True)
            (root / "nonmatching/boot/unit.c").write_text("int f;\n")
            file = root / "cflags.txt"
            known = frozenset({"nonmatching/boot/unit.c"})
            for line, message in (
                ("nonmatching/boot/unit.c  as=nope\n", "unknown assembler selector"),
                ("nonmatching/boot/unit.c  as=\n", "empty assembler selector"),
                (f"nonmatching/boot/unit.c  as={SDK_AS} as={PS2_AS}\n",
                 "duplicate assembler selector"),
                (f"nonmatching/boot/unit.c  -O0 as={SDK_AS}\n", "not a flag"),
                # A named compiler must state its own flags; only an assembler
                # selector may be the whole content of a line.
                (f"nonmatching/boot/unit.c  {SECOND} as={SDK_AS}\n", "no cc1 flags"),
            ):
                with self.subTest(line=line), patch.object(cflags, "ROOT", root), \
                     patch.object(cflags, "campaign_sources", return_value=known):
                    file.write_text(line)
                    with self.assertRaisesRegex(ValueError, message):
                        cflags.load(file)

    def test_every_pinned_assembler_states_its_host_and_provenance(self) -> None:
        profile = json.loads(cflags.PROFILE.read_text(encoding="utf-8"))
        for selector, pinned in profile["extra_assemblers"].items():
            with self.subTest(selector=selector):
                self.assertIn(pinned["host"], compile_boot_trial.HOSTS)
                # An assembler pin may carry flags of its own: they say how that
                # binary was measured, so the shape is fixed and the values are
                # flags, never free text.
                self.assertEqual(set(pinned["flags"]), {"as"})
                self.assertTrue(all(flag.startswith("-") for flag in pinned["flags"]["as"]))
                self.assertTrue(pinned["id"].startswith("rac5-boot-"))
                int(pinned["tools"]["assembler_sha256"], 16)
                self.assertGreater(len(pinned["provenance"]), 80)
        self.assertEqual(cflags.pinned_assemblers(),
                         (cflags.DEFAULT_ASSEMBLER, SDK_AS, PS2_AS))
        # Ps2EeAs is one binary with one identity: the pin an entry selects must
        # be the same file the boot assembly base and the campaign linker pin.
        self.assertEqual(profile["extra_assemblers"][PS2_AS]["tools"]["assembler_sha256"],
                         profile["tools"]["sn_assembler_sha256"])
        # The SDK `as` pools a `li.s` float in a GP-relative lit4 section unless
        # it is run with -G0, and that section does not link in this campaign.
        self.assertEqual(profile["extra_assemblers"][SDK_AS]["flags"]["as"], ["-G0"])
        self.assertEqual(profile["extra_assemblers"][PS2_AS]["flags"]["as"], [])
        self.assertTrue(cflags.DEFAULT_ENTRY.assembler_pinned)
        self.assertFalse(cflags.Entry(cflags.DEFAULT_COMPILER, (), SDK_AS).assembler_pinned)

    def test_an_unpinned_or_unprovided_assembler_is_rejected(self) -> None:
        profile = json.loads(cflags.PROFILE.read_text(encoding="utf-8"))
        with self.assertRaisesRegex(ValueError, "pins no assembler"):
            compile_boot_trial.check_extra_assembler(profile, "not-pinned", Path("/nope"))
        for selector in (SDK_AS, PS2_AS):
            with self.subTest(selector=selector), \
                 self.assertRaisesRegex(ValueError, f"--as-bin {selector}="):
                compile_boot_trial.check_extra_assembler(profile, selector, None)

    def test_checked_extra_assembler_verifies_the_pin(self) -> None:
        pin = {"id": "rac5-boot-unit", "host": "linux", "provenance": "u" * 81,
               "tools": {"assembler": "unit-as", "assembler_sha256": "0" * 64},
               "flags": {"as": []}}
        profile = {"extra_assemblers": {"unit": pin, "bad": {**pin, "host": "plan9"}}}
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            binary = root / "unit-as"
            binary.write_text("tool\n")
            with patch.object(compile_boot_trial, "digest", return_value="0" * 64):
                checked = compile_boot_trial.check_extra_assembler(profile, "unit", root)
                self.assertEqual(checked["binary"], binary)
                self.assertEqual(checked["host"], "linux")
            with patch.object(compile_boot_trial, "digest", return_value="1" * 64):
                with self.assertRaisesRegex(ValueError, "differs from measured"):
                    compile_boot_trial.check_extra_assembler(profile, "unit", root)
            with self.assertRaisesRegex(ValueError, "is not one of"):
                compile_boot_trial.check_extra_assembler(profile, "bad", root)
            binary.unlink()
            with self.assertRaisesRegex(ValueError, "assembler is missing"):
                compile_boot_trial.check_extra_assembler(profile, "unit", root)
            # A pin's flags are flags, not free text, and the block is `as` only.
            for broken in ({"as": ["-G0"], "cpp": []}, {"as": ["G0"]}, {"as": "-G0"}):
                with self.subTest(flags=broken), \
                     patch.object(compile_boot_trial, "digest", return_value="0" * 64):
                    binary.write_text("tool\n")
                    with self.assertRaisesRegex(ValueError, "flags differ"):
                        compile_boot_trial.check_extra_assembler(
                            {"extra_assemblers": {"unit": {**pin, "flags": broken}}},
                            "unit", root)

    def test_a_pinned_compiler_may_carry_its_source_patch(self) -> None:
        profile = json.loads(cflags.PROFILE.read_text(encoding="utf-8"))
        pin = profile["extra_compilers"][DIVPAD]
        patch = compile_boot_trial.ROOT / pin["patch"]["path"]
        self.assertTrue(patch.is_file())
        self.assertEqual(compile_boot_trial.normalized_digest(patch),
                         pin["patch"]["sha256_lf"])
        # The div pad is the whole difference from the unpatched SDK pin: same
        # driver and preprocessor, a different cc1.
        self.assertEqual(pin["tools"]["driver_sha256"],
                         profile["extra_compilers"][SONY]["tools"]["driver_sha256"])
        self.assertNotEqual(pin["tools"]["cc1_sha256"],
                            profile["extra_compilers"][SONY]["tools"]["cc1_sha256"])
        self.assertEqual(
            cflags.load()["src/boot/__ieee754_expf.c"].compiler, DIVPAD)

    def test_a_pin_whose_patch_moved_no_longer_describes_its_binary(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "repo"
            (root / "bin").mkdir(parents=True)
            (root / "tools/compiler/patches").mkdir(parents=True)
            for name in ("d", "c", "x"):
                (root / "bin" / name).write_text("tool\n")
            (root / "tools/compiler/patches/unit.patch").write_text("--- a\n+++ b\n")
            pin = {"id": "rac5-boot-unit", "host": "linux", "provenance": "u" * 81,
                   "tools": {"driver": "d", "libexec": ".", "cpp": "c", "cc1": "x",
                             "driver_sha256": "0" * 64, "cpp_sha256": "0" * 64,
                             "cc1_sha256": "0" * 64},
                   "patch": {"path": "tools/compiler/patches/unit.patch",
                             "sha256_lf": "0" * 64},
                   "flags": {"driver": ["-S"], "as": []}}
            with patch.object(compile_boot_trial, "ROOT", root), \
                 patch.object(compile_boot_trial, "digest", return_value="0" * 64):
                with self.assertRaisesRegex(ValueError, "patch differs"):
                    compile_boot_trial.check_extra_compiler(
                        {"extra_compilers": {"unit": pin}}, "unit", root / "bin")
                (root / "tools/compiler/patches/unit.patch").unlink()
                with self.assertRaisesRegex(ValueError, "patch is missing"):
                    compile_boot_trial.check_extra_compiler(
                        {"extra_compilers": {"unit": pin}}, "unit", root / "bin")

    def test_gate_arguments_carry_an_assembler_directory(self) -> None:
        parser = argparse.ArgumentParser()
        compile_boot_trial.add_extra_arguments(parser)
        parsed = parser.parse_args(["--as-bin", f"{SDK_AS}=/opt/sdk"])
        self.assertEqual(parsed.as_bin, [(SDK_AS, Path("/opt/sdk"))])
        self.assertEqual(parsed.extra_bin, [])


if __name__ == "__main__":
    unittest.main()
