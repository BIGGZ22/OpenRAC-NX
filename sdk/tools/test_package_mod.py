# SPDX-License-Identifier: MIT
"""Tests for package_mod.py: the manifest's rules, an archive, joining platforms.

    python3 -m unittest discover -s sdk/tools
"""

import copy
import json
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import package_mod as pm  # noqa: E402

GOOD = {
    "format": 2,
    "id": "lock-strafe",
    "name": "Lock-strafe",
    "version": "1.0.0",
    "author": "OpenRAC contributors",
    "license": "GPL-3.0-or-later",
    "homepage": "https://github.com/OpenRAC/rac1-mods",
    "api": "1.0",
    "games": {"rac1": ["pal", "ntsc"]},
    "plugin": {"linux-x86_64": "lib/linux-x86_64/lock_strafe.so"},
    "hooks": ["Pad_ProcessReport"],
    "settings": [
        {"key": "mode", "name": "Mode", "type": "choice", "default": "lock",
         "choices": [{"value": "lock", "name": "Lock"}, {"value": "hold", "name": "Hold"}]},
        {"key": "trace", "name": "Trace", "type": "bool", "default": False},
        {"key": "every", "name": "Every", "type": "int", "default": 60, "min": 1, "max": 600},
        {"key": "scale", "name": "Scale", "type": "float", "default": 1.0, "min": 0.5, "max": 2, "step": 0.1},
        {"key": "label", "name": "Label", "type": "string", "default": "hi", "max_length": 8},
    ],
    "requires": [{"id": "base-lib", "version": ">=1.2.0"}],
    "conflicts": [{"id": "other-strafe", "reason": "both take L2 and R2"}],
    "load": {"after": ["camera-tweaks"], "before": []},
}


def changed(**kw):
    m = copy.deepcopy(GOOD)
    for k, v in kw.items():
        if v is None:
            m.pop(k, None)
        else:
            m[k] = v
    return m


class ManifestRules(unittest.TestCase):
    def test_good(self):
        self.assertEqual(pm.check_manifest(GOOD), [])

    def test_format(self):
        self.assertIn("data pack", pm.check_manifest(changed(format=1))[0])
        self.assertTrue(pm.check_manifest(changed(format=3)))

    def test_ids(self):
        for bad in ("Lock", "lock_strafe", "-x", "x-", "a--b", "", "a" * 65):
            self.assertTrue(pm.check_manifest(changed(id=bad)), bad)
        self.assertEqual(pm.check_manifest(changed(id="a1-b2")), [])

    def test_versions(self):
        for bad in ("1.0", "01.0.0", "1.0.0.0", "v1.0.0"):
            self.assertTrue(pm.check_manifest(changed(version=bad)), bad)
        self.assertEqual(pm.check_manifest(changed(version="2.10.3-beta.1")), [])

    def test_api(self):
        self.assertTrue(pm.check_manifest(changed(api=None)))
        self.assertTrue(pm.check_manifest(changed(api="1")))
        self.assertIn("this SDK knows", " ".join(pm.check_manifest(changed(api="2.0"))))
        # Data only: no plugin, no api needed.
        self.assertEqual(pm.check_manifest(changed(plugin=None, api=None)), [])

    def test_games(self):
        for bad in ({}, {"rac1": []}, {"rac1": "pal"}, {"RAC1": ["pal"]}, []):
            self.assertTrue(pm.check_manifest(changed(games=bad)), bad)

    def test_plugin_paths(self):
        for bad in ({"linux": "x.so"}, {"linux-x86_64": "/abs.so"}, {"linux-x86_64": "../x.so"},
                    {"linux-x86_64": "lib\\x.dll"}, {}):
            self.assertTrue(pm.check_manifest(changed(plugin=bad)), bad)

    def test_plugin_files_present(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            self.assertIn("not in the mod", " ".join(pm.check_manifest(GOOD, root)))
            (root / "lib/linux-x86_64").mkdir(parents=True)
            (root / "lib/linux-x86_64/lock_strafe.so").write_bytes(b"\x7fELF")
            self.assertEqual(pm.check_manifest(GOOD, root), [])

    def test_settings(self):
        def with_setting(s):
            return changed(settings=[s])
        bad = [
            {"key": "Mode", "name": "M", "type": "bool", "default": True},
            {"key": "m", "type": "bool", "default": True},
            {"key": "m", "name": "M", "type": "colour", "default": 1},
            {"key": "m", "name": "M", "type": "bool"},
            {"key": "m", "name": "M", "type": "bool", "default": 1},
            {"key": "m", "name": "M", "type": "int", "default": 1.5},
            {"key": "m", "name": "M", "type": "int", "default": 9, "min": 0, "max": 5},
            {"key": "m", "name": "M", "type": "int", "default": 1, "min": 5, "max": 0},
            {"key": "m", "name": "M", "type": "float", "default": 1.0, "step": 0},
            {"key": "m", "name": "M", "type": "choice", "default": "a", "choices": [{"value": "a", "name": "A"}]},
            {"key": "m", "name": "M", "type": "choice", "default": "c",
             "choices": [{"value": "a", "name": "A"}, {"value": "b", "name": "B"}]},
            {"key": "m", "name": "M", "type": "choice", "default": "a",
             "choices": [{"value": "a", "name": "A"}, {"value": "a", "name": "B"}]},
            {"key": "m", "name": "M", "type": "string", "default": "toolong", "max_length": 3},
        ]
        for s in bad:
            self.assertTrue(pm.check_manifest(with_setting(s)), s)
        twice = changed(settings=[GOOD["settings"][1], GOOD["settings"][1]])
        self.assertIn("a key twice", " ".join(pm.check_manifest(twice)))

    def test_keybinds_and_presets(self):
        jump = {"key": "jump", "name": "Jump", "type": "keybind", "default": "Space"}
        fire = {"key": "fire", "name": "Fire", "type": "keybind", "default": "MouseLeft,KeyF"}
        pause = {"key": "pause", "name": "Pause", "type": "keybind", "default": "Escape", "binds": 1}
        self.assertEqual(pm.check_manifest(changed(settings=[jump, fire, pause])), [])
        self.assertEqual(pm.check_manifest(changed(settings=[dict(jump, default="")])), [])
        for bad in ("Spacebar", "KeyW,KeyW", "KeyW,KeyA,KeyS", 3):
            self.assertTrue(pm.check_manifest(changed(settings=[dict(jump, default=bad)])), bad)
        self.assertTrue(pm.check_manifest(changed(settings=[dict(pause, default="KeyP,KeyO")])))
        self.assertTrue(pm.check_manifest(changed(settings=[dict(jump, binds=3)])))
        good = [{"id": "arrows", "name": "Arrows", "values": {"jump": "Numpad0", "fire": "ControlRight"}}]
        self.assertEqual(pm.check_manifest(changed(settings=[jump, fire], presets=good)), [])
        for bad in ([{"id": "x", "name": "X", "values": {"nothing": "KeyA"}}],
                    [{"id": "x", "name": "X", "values": {"jump": "Nope"}}],
                    [{"id": "X!", "name": "X", "values": {}}],
                    [{"id": "x", "name": "X", "values": {}}, {"id": "x", "name": "Y", "values": {}}],
                    {"x": 1}):
            self.assertTrue(pm.check_manifest(changed(settings=[jump, fire], presets=bad)), bad)

    def test_references(self):
        self.assertTrue(pm.check_manifest(changed(requires=[{"id": "x", "version": "1.0.0"}])))
        self.assertTrue(pm.check_manifest(changed(requires=[{"id": "X"}])))
        self.assertTrue(pm.check_manifest(changed(conflicts=[{"id": "lock-strafe"}])))
        self.assertTrue(pm.check_manifest(changed(load={"first": []})))
        self.assertTrue(pm.check_manifest(changed(homepage="http://example.com")))

    def test_hooks_by_id_or_alias(self):
        # The SDK's own table (bindings/rac1.ids.lock): ids, the names and the symbols are all aliases.
        for hooks in (["rac1.player.take_damage"], ["Hero_TakeDamage"], ["rac1.combat.get_hit", "Moby_GetHitMessage"],
                      ["func_L00_002124E8"], []):
            self.assertEqual(pm.check_manifest(changed(hooks=hooks)), [], hooks)
        for hooks in (["rac1.player.no_such_hook"], ["Hero_Nothing"], ["Rac1.Player.Take_Damage"], ["rac1."],
                      ["has space"], "Hero_TakeDamage", [3]):
            self.assertTrue(pm.check_manifest(changed(hooks=hooks)), hooks)
        self.assertIn("is not a hook point of rac1", " ".join(pm.check_manifest(changed(hooks=["rac1.x.y"]))))

    def test_hooks_of_a_game_without_a_table(self):
        # A game this SDK has no table for: the names are only checked for their form.
        other = changed(games={"rac9": ["pal"]}, hooks=["rac9.player.jump", "Hero_Jump"])
        self.assertEqual(pm.check_manifest(other), [])
        with tempfile.TemporaryDirectory() as d:
            (Path(d) / "rac1.ids.lock").write_text("# a table\nrac1.a.b\tpal:00100000\tOld_Name,func_00100000\n")
            self.assertEqual(pm.check_manifest(changed(hooks=["rac1.a.b", "Old_Name"]), bindings=Path(d)), [])
            self.assertTrue(pm.check_manifest(changed(hooks=["Hero_TakeDamage"]), bindings=Path(d)))
            self.assertIsNone(pm.hook_names("rac2", Path(d)))


class Archives(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)

    def tearDown(self):
        self.tmp.cleanup()

    def folder(self, platform, data=b"lib"):
        folder = self.root / platform
        name = {"linux": "lock_strafe.so", "macos": "lock_strafe.dylib", "windows": "lock_strafe.dll"}[
            platform.split("-")[0]]
        m = changed(plugin={platform: f"lib/{platform}/{name}"})
        (folder / "lib" / platform).mkdir(parents=True)
        (folder / "lib" / platform / name).write_bytes(data)
        (folder / pm.MANIFEST).write_text(json.dumps(m))
        (folder / "README.md").write_text("# Lock-strafe\n")
        (folder / "notes.tmp").write_text("left out")
        return folder

    def test_package(self):
        out = pm.package(self.folder("linux-x86_64"), self.root / "dist")
        self.assertEqual(out.name, "lock-strafe-1.0.0-linux-x86_64.zip")
        with zipfile.ZipFile(out) as z:
            self.assertEqual(sorted(z.namelist()),
                             ["README.md", "lib/linux-x86_64/lock_strafe.so", "openrac-mod.json"])
            self.assertEqual(z.namelist()[0], "openrac-mod.json")

    def test_package_refuses_a_missing_library(self):
        folder = self.folder("linux-x86_64")
        (folder / "lib/linux-x86_64/lock_strafe.so").unlink()
        with self.assertRaises(pm.ManifestError):
            pm.package(folder, self.root / "dist")
        self.assertFalse((self.root / "dist").exists() and any((self.root / "dist").iterdir()))

    def test_merge(self):
        a = pm.package(self.folder("linux-x86_64"), self.root / "dist")
        b = pm.package(self.folder("macos-arm64"), self.root / "dist")
        c = pm.package(self.folder("windows-x86_64"), self.root / "dist")
        out = pm.merge([b, a, c], self.root / "lock-strafe-1.0.0.zip")
        with zipfile.ZipFile(out) as z:
            m = json.loads(z.read(pm.MANIFEST))
            self.assertEqual(list(m["plugin"]), ["macos-arm64", "linux-x86_64", "windows-x86_64"])
            for p in m["plugin"].values():
                self.assertIn(p, z.namelist())
        self.assertEqual(pm.check_manifest(m), [])

    def test_merge_refuses_different_manifests(self):
        a = pm.package(self.folder("linux-x86_64"), self.root / "dist")
        folder = self.folder("macos-arm64")
        m = json.loads((folder / pm.MANIFEST).read_text())
        m["version"] = "1.0.1"
        (folder / pm.MANIFEST).write_text(json.dumps(m))
        b = pm.package(folder, self.root / "dist")
        with self.assertRaises(pm.ManifestError) as e:
            pm.merge([a, b], self.root / "out.zip")
        self.assertIn("version", str(e.exception))

    def test_merge_refuses_a_platform_twice(self):
        a = pm.package(self.folder("linux-x86_64"), self.root / "dist")
        with self.assertRaises(pm.ManifestError):
            pm.merge([a, a], self.root / "out.zip")

    def test_main(self):
        folder = self.folder("linux-x86_64")
        self.assertEqual(pm.main([str(folder), "-o", str(self.root / "d")]), 0)
        self.assertEqual(pm.main(["--check", str(folder)]), 0)
        (folder / pm.MANIFEST).write_text("{}")
        self.assertEqual(pm.main(["--check", str(folder)]), 1)



SCRIPT = {
    "format": 2,
    "id": "button-swap",
    "name": "Button swap",
    "version": "1.0.0",
    "type": "script",
    "api": "1.1",
    "scripts": ["main.luau"],
    "games": {"rac1": ["pal", "ntsc"]},
    "settings": [{"key": "pair", "name": "Pair", "type": "choice", "default": "cross-square",
                  "choices": [{"value": "cross-square", "name": "Cross, Square"},
                              {"value": "l1-r1", "name": "L1, R1"}]}],
}


def script(**kw):
    m = copy.deepcopy(SCRIPT)
    for k, v in kw.items():
        if v is None:
            m.pop(k, None)
        else:
            m[k] = v
    return m


class ScriptMods(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)

    def tearDown(self):
        self.tmp.cleanup()

    def test_rules(self):
        self.assertEqual(pm.check_manifest(SCRIPT), [])
        errors = lambda **kw: " ".join(pm.check_manifest(script(**kw)))  # noqa: E731
        self.assertIn("script mods need interface 1.1", errors(api="1.0"))
        self.assertIn("api", errors(api=None))
        self.assertIn("this SDK knows", errors(api="2.1"))
        self.assertIn("at least one", errors(scripts=[]))
        self.assertIn("at least one", errors(scripts=None))
        self.assertIn("inside the mod", errors(scripts=["../main.luau"]))
        self.assertIn("inside the mod", errors(scripts=["main.lua"]))
        self.assertIn("no library", errors(plugin={"linux-x86_64": "x.so"}))
        self.assertIn("native (the default) or script", errors(type="dll"))
        native = changed(scripts=["main.luau"])
        self.assertIn("only a script mod", " ".join(pm.check_manifest(native)))

    def folder(self):
        folder = self.root / "button-swap"
        (folder / "lib").mkdir(parents=True)
        (folder / pm.MANIFEST).write_text(json.dumps(SCRIPT))
        (folder / "main.luau").write_text("openrac.log('hi')\n")
        (folder / "lib" / "pairs.luau").write_text("return {}\n")
        (folder / "README.md").write_text("# Button swap\n")
        (folder / "LICENSE").write_text("GPL\n")
        (folder / "notes.tmp").write_text("left out")
        (folder / ".luaurc").write_text("{}")
        (folder / "build").mkdir()
        (folder / "build" / "x.luau").write_text("left out")
        return folder

    def test_package(self):
        out = pm.package(self.folder(), self.root / "dist")
        self.assertEqual(out.name, "button-swap-1.0.0.zip")
        with zipfile.ZipFile(out) as z:
            self.assertEqual(z.namelist()[0], "openrac-mod.json")
            self.assertEqual(sorted(z.namelist()),
                             ["LICENSE", "README.md", "lib/pairs.luau", "main.luau", "openrac-mod.json"])

    def test_package_refuses_a_missing_script(self):
        folder = self.folder()
        (folder / "main.luau").unlink()
        with self.assertRaises(pm.ManifestError) as e:
            pm.package(folder, self.root / "dist")
        self.assertIn("main.luau is not in the mod", str(e.exception))
        self.assertEqual(pm.main(["--check", str(folder)]), 1, "--check looks for a script mod's scripts")
        (folder / "main.luau").write_text("-- back")
        self.assertEqual(pm.main(["--check", str(folder / pm.MANIFEST)]), 0)


if __name__ == "__main__":
    unittest.main()
