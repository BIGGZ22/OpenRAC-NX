"""priority.py on a tiny made-up program: two levels, a hook, a vtbl and a few weak functions.

    python3 -m unittest port/tools/hostgen/test_priority.py      (no Clang needed)
"""
import json
import struct
import tempfile
import unittest
from pathlib import Path

import priority

LOW = 0x15F000


def jal(target: int) -> int:
    return (3 << 26) | ((target >> 2) & 0x3FFFFFF)


def write(path: Path, text) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if isinstance(text, bytes):
        path.write_bytes(text)
    else:
        path.write_text(text)


class Fixture:
    """main -> frame -> engine (calls the hook stub) ; level 0's copy of engine calls a stub of
    its own there; level 1's vtbl points at a functional candidate."""

    def __init__(self, root: Path):
        self.root = root
        gen, src = root / "gen", root / "src"
        self.cfg = {"title": "Tiny", "roots": ["func_00100000", "func_L00_00200000"],
                    "level_hooks": ["func_00100100"], "candidates": "functional.tsv",
                    "host_functions": ["func_00100300"], "skip": ["src/libgcc"],
                    "relocate": [LOW, 0x250000], "_dir": str(root)}
        write(gen / "report.json", json.dumps({
            "call_graph": {
                "func_00100000": ["func_L00_00200000", "func_00100300", "func_00100400"],
                "func_L00_00200000": ["func_L00_00200100"],
                "func_L00_00200100": ["func_00100100", "func_00100200"],
            },
            "candidates_used": ["func_00100200", "func_L01_00210000", "func_00100300"],
            "arguments_filled_with_zero": {"func_L00_00200100": ["func_00100200 argument 2"]},
            "call_site_mismatches": {"func_L00_00200100": ["func_00100100: 1 integer arguments for 0"],
                                     "func_00100000": ["func_00100200: 0 float arguments for 1"]},
            "stubbed": {},
        }))
        exe = [(0x00100000, "func_00100000"), (0x00100100, "func_00100100"),
               (0x00100200, "func_00100200"), (0x00100300, "func_00100300"),
               (0x00100400, "func_00100400")]
        l0 = [(0x00200000, "func_L00_00200000"), (0x00200100, "func_L00_00200100"),
              (0x00200200, "func_L00_00200200")]
        l1 = [(0x00300000, "func_L00_00200000"), (0x00300100, "func_L00_00200100"),
              (0x00300200, "func_L01_00210000")]

        def table(name, rows):
            body = "".join(f'    {{0x{a:08X}u, (openrac_host_fn){n}, "{n}"}},\n' for a, n in rows)
            return f"static const openrac_fn_entry k_{name}[] = {{\n{body}}};\n"
        write(gen / "functions.c", table("exe", exe) + table("overlay_00", l0) + table("overlay_01", l1))
        write(gen / "stubs.c", 'void func_L00_00200200(void) { openrac_guest_missing("func_L00_00200200"); }\n'
                               'void func_00100400(void) { openrac_guest_missing("func_00100400"); }\n'
                               'void func_00100500(void) { openrac_guest_missing("func_00100500"); }\n')
        write(gen / "prep/src/game/camera.c",
              "/* hostgen: func_00100200 from a candidate, not the matched C */\n"
              'extern float func_00100300_hc1(int),func_00100500(float) __asm__("func_00100300");\n')
        write(src / "functional.tsv", "# name\tfile\nfunc_00100200\tf/a.c\nfunc_L01_00210000\tf/b.c\n"
                                      "func_00100300\tf/c.c\n")
        write(src / "f/a.c", "/* functional: high. same result */\nint func_00100200(int a, int b) { return a; }\n")
        write(src / "f/b.c", "/* functional: low. a guess */\nvoid func_L01_00210000(void) {}\n")
        write(src / "f/c.c", "/* functional: low. */\nvoid func_00100300(void) {}\n")
        write(src / "src/game/camera.c", 'INCLUDE_ASM("asm", func_L00_00200200);\nvoid func_00100200(int a) {\n}\n')
        write(src / "src/libgcc/x.c", 'INCLUDE_ASM("asm", func_00100400);\n')
        # Level dumps: level 0's engine copy calls its own function where the hook is; level 1's
        # vtbl names its class's update.
        for level, base, text, vtbl in [
                (0, 0x00200000, {0x100: [0, jal(0x00200200), 0]}, [0xFFFFFFFF]),
                (1, 0x00300000, {}, [7, 0x00300200, 0, 0xFFFFFFFF])]:
            words = [0] * (0x300 // 4)
            for off, ws in text.items():
                words[off // 4:off // 4 + len(ws)] = ws
            d = root / "dump" / f"level_{level:02d}"
            write(d / "text.bin", struct.pack(f"<{len(words)}I", *words))
            write(d / "vtbl.bin", struct.pack(f"<{len(vtbl)}I", *vtbl))
            write(d / "manifest.json", json.dumps({"entry_point": base, "records": [
                {"name": "vtbl", "address": LOW, "bytes": 4 * len(vtbl)},
                {"name": "text", "address": base, "bytes": 4 * len(words)}]}))


class PriorityTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        root = Path(self.tmp.name)
        self.fx = Fixture(root)
        self.result = priority.analyse(self.fx.cfg, root / "gen", root / "src", root / "dump", None)
        self.rows = {r["name"]: r for r in self.result["rows"]}

    def tearDown(self):
        self.tmp.cleanup()

    def test_hook_resolved_per_level(self):
        # Only level 0's copy of the engine function calls the stub at the hook.
        row = self.rows["func_L00_00200200"]
        self.assertEqual(row["weakness"], ["no C"])
        self.assertEqual(row["levels"], [0])
        self.assertIn("camera", row["hot"])
        self.assertEqual(row["frame_depth"], 2)

    def test_vtbl_root(self):
        row = self.rows["func_L01_00210000"]
        self.assertEqual(row["weakness"], ["functional: low"])
        self.assertEqual(row["levels"], [1])
        self.assertIn("vtbl", row["via"])

    def test_kinds(self):
        self.assertEqual(self.rows["func_00100200"]["weakness"], ["functional: high", "mislabelled declaration"])
        self.assertEqual(self.rows["func_00100200"]["mislabelled"], ["func_00100500 -> func_00100300"])
        self.assertEqual(self.rows["func_L00_00200100"]["weakness"], ["zero-filled args"])
        # Extra arguments the callee never reads are not a weakness; host and libgcc are left out.
        self.assertNotIn("func_00100100", self.rows)
        self.assertEqual(self.rows["func_00100200"]["mismatched_callers"],
                         ["func_00100000 (0 float arguments for 1)"])
        self.assertNotIn("func_00100300", self.rows)
        self.assertNotIn("func_00100400", self.rows)
        self.assertIn("func_00100500", self.result["unreached"])

    def test_order(self):
        names = [r["name"] for r in self.result["rows"]]
        # Reached in both levels first, then hot, then the rest by weakness.
        self.assertLess(names.index("func_00100200"), names.index("func_L01_00210000"))
        self.assertLess(names.index("func_L00_00200200"), names.index("func_L01_00210000"))
        text = priority.markdown(self.result, "Tiny")
        self.assertIn("| func_00100200 | functional: high, mislabelled declaration | all +title |", text)
        self.assertEqual(self.rows["func_00100200"]["tier"], "Sonnet")
        self.assertEqual(priority.tier(["unverifiable"], 40), "Opus")


if __name__ == "__main__":
    unittest.main()
