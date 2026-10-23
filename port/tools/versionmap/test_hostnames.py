# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""hostnames.py on a small made-up game: two versions sharing host C, a map, a generated folder."""
import io
import json
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from types import SimpleNamespace

import hostnames

SHARED = """#include "rac1.h"
/* func_00100000 in a comment is not used */
#if !RAC1_OMIT(func_00100200)
void func_00100200(void) { GREF(int, RAC1_ADDR(0x00150000u)) = 1; }
#endif
void func_00100100(void) {
    func_00100300__game();
    GREF(int, OPENRAC_LDATA(0, RAC1_LADDR(00, 0x00160000u))) = 2;
    GREF(int, 0x00150004u) = 3;
}
"""

TABLE = """#ifndef OPENRAC_RAC1_VERSION_H
#define OPENRAC_RAC1_VERSION_H
#define RAC1_OMIT_func_00100200 1
/* ---- generated below ---- */
#define func_00100100 game_FUN_00100080
#define func_00100999 game_FUN_00100999
#endif
"""

MAP = {"from": "x/a", "to": "x/b", "gp": [None, None], "programs": {
    "boot": {"functions": {"00100100": ["00100080", "same", 8, 8], "00100300": ["00100280", "near", 8, 8]},
             "data": {"00150000": "00140000"}},
    "level:00": {"functions": {}, "data": {"00160000": "0015FF00"}},
}}

FUNCTIONS_C = """static const openrac_fn_entry k_exe[] = {
    {0x00100080u, (openrac_host_fn)game_FUN_00100080, "FUN_00100080"},
    {0x00100280u, (openrac_host_fn)game_FUN_00100280, "FUN_00100280"},
};
"""


class HostNames(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        root = Path(self.tmp.name)
        (root / "game" / "shared").mkdir(parents=True)
        (root / "game" / "shared" / "a.c").write_text(SHARED)
        self.version = root / "game" / "v"
        (self.version / "host").mkdir(parents=True)
        (self.version / "host" / "rac1_version.h").write_text(TABLE)
        (self.version / "hostgen.json").write_text(json.dumps({"host_shared": ["../shared"], "source": "nowhere"}))
        self.map = root / "map.json"
        self.map.write_text(json.dumps(MAP))
        self.gen = root / "gen"
        self.gen.mkdir()
        (self.gen / "functions.c").write_text(FUNCTIONS_C)
        (self.gen / "stubs.c").write_text("")

    def tearDown(self):
        self.tmp.cleanup()

    def run_cmd(self, cmd, **kw):
        args = SimpleNamespace(version=str(self.version), map=kw.get("map"), gen=kw.get("gen"), verbose=False)
        out = io.StringIO()
        with redirect_stdout(out):
            code = hostnames.cmd_check(args) if cmd == "check" else hostnames.cmd_write(args)
        return code, out.getvalue()

    def test_check_finds_missing_unused_and_loose(self):
        code, out = self.run_cmd("check")
        self.assertEqual(code, 1)
        self.assertIn("missing: RAC1_ADDR_L00_0x00160000u", out)
        self.assertIn("missing: func_00100300__game", out)
        self.assertIn("unused: func_00100999", out)
        self.assertIn("0x00150004u outside RAC1_ADDR", out)
        # The omitted function's names and addresses are not asked for; comments are not read.
        self.assertNotIn("func_00100200", out)
        self.assertNotIn("RAC1_ADDR_0x00150000u", out)
        self.assertNotIn("func_00100000", out)

    def test_write_fills_from_the_map_and_keeps_entries(self):
        code, out = self.run_cmd("write", map=str(self.map), gen=str(self.gen))
        self.assertEqual(code, 0)
        table = (self.version / "host" / "rac1_version.h").read_text()
        self.assertIn("#define func_00100100 game_FUN_00100080", table)
        self.assertIn("#define func_00100300__game game_FUN_00100280__game", table)
        self.assertIn("#define RAC1_ADDR_L00_0x00160000u 0x0015FF00u", table)
        self.assertNotIn("func_00100999", table)
        self.assertIn("#define RAC1_OMIT_func_00100200 1", table)
        # With the loose literal wrapped, the table is complete and agrees with the map.
        shared = self.version.parent / "shared" / "a.c"
        shared.write_text(shared.read_text().replace("0x00150004u", "RAC1_ADDR(0x00150000u) + 4"))
        self.run_cmd("write", map=str(self.map), gen=str(self.gen))
        code, out = self.run_cmd("check", map=str(self.map), gen=str(self.gen))
        self.assertEqual(code, 0, out)
        self.assertIn("0 problems, 0 not as the map says", out)

    def test_a_disagreeing_entry_is_reported_not_changed(self):
        self.run_cmd("write", map=str(self.map), gen=str(self.gen))
        path = self.version / "host" / "rac1_version.h"
        path.write_text(path.read_text().replace("0x0015FF00u", "0x0015FF04u"))
        code, out = self.run_cmd("check", map=str(self.map), gen=str(self.gen))
        self.assertIn("RAC1_ADDR_L00_0x00160000u = 0x0015FF04u, the map says 0x0015FF00u", out)
        self.assertIn("0x0015FF04u", path.read_text())


if __name__ == "__main__":
    unittest.main()
