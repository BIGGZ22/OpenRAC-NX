#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""A game's binding for mods, generated from one table (docs/port/MODS.md, section 11).

    python3 sdk/tools/gen_binding.py [GAME ...] [--catalogue FILE] [--check]

reads sdk/bindings/<game>.json (rac1 when no game is named) and writes:

  port/game/<game>/script_binding.c   the port's side for script mods: for each hook point,
                                           OPENRAC_SCRIPT_LINKS C functions of its signature that hand
                                           their arguments to the script runtime, and the fields
                                           (port/game/common/include/openrac/script_binding.h)
  sdk/include/openrac/games/<game>.hpp     the hook points as C++ types, for the SDK's C++ layer
  sdk/types/<game>.d.luau                  the `openrac` global's types, for an editor's autocomplete

The behaviour catalogue (the level editor's moby classes, their update functions and pvars) is its
other input: the table's "catalogue" (a path from the top of OpenRAC), or --catalogue FILE. Each
class with an update function becomes a hook point Moby_<Class>_Update, and its pvars the fields
openrac.pvars(moby) gives for a moby of that class. Its format:

  {"format": 1, "game": "rac1",
   "moby": {"class": "0xAA", "class_type": "i16", "pvars": "0x78"},   where a moby keeps its class
                                                                     number and its pvars' address
   "classes": [{"name": "Crate", "class": 500,
                "update": {"pal": "func_L05_002D48B8", "ntsc": "FUN_L05_..."},
                "signature": "void(u32)", "params": ["moby"],        (these two are the default)
                "pvars": [{"name": "timer", "type": "f32", "offset": "0x10",
                           "readonly": false, "about": "..."}],
                "about": "..."}]}

--check writes nothing: it fails when an output differs from what the table gives, or when the
table's hook points (with the catalogue's) are not exactly the hook rows of every disc's
port/game/<game>-<disc>/libraries.tsv. Stdlib only.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LINKS = 8  # OPENRAC_SCRIPT_LINKS (script_binding.h)

KINDS = {
    # signature kind: (C type, OPENRAC_SV_*, value field, Luau type)
    "i8": ("int8_t", "OPENRAC_SV_I8", "i", "number"),
    "u8": ("uint8_t", "OPENRAC_SV_U8", "i", "number"),
    "i16": ("int16_t", "OPENRAC_SV_I16", "i", "number"),
    "u16": ("uint16_t", "OPENRAC_SV_U16", "i", "number"),
    "i32": ("int32_t", "OPENRAC_SV_I32", "i", "number"),
    "u32": ("uint32_t", "OPENRAC_SV_U32", "i", "number"),
    "i64": ("int64_t", "OPENRAC_SV_I64", "i", "number"),
    "u64": ("uint64_t", "OPENRAC_SV_U64", "i", "number"),
    "f32": ("float", "OPENRAC_SV_F32", "f", "number"),
}
FIELD_KINDS = {"i8", "u8", "i16", "u16", "i32", "u32", "f32"}
NAME = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


class BindingError(Exception):
    pass


def parse_signature(sig: str) -> tuple[str, list[str]]:
    m = re.fullmatch(r"(void|[iu](?:8|16|32|64)|f32)\(([a-z0-9,]*)\)", sig)
    if not m:
        raise BindingError(f"signature {sig!r} is not of the form void(i32,f32)")
    params = [p for p in m.group(2).split(",") if p]
    for p in params:
        if p not in KINDS:
            raise BindingError(f"signature {sig!r}: {p!r} is not a kind ({', '.join(KINDS)})")
    return m.group(1), params


def number(text: str | int) -> int:
    return text if isinstance(text, int) else int(str(text), 0)


def load(game: str, catalogue: Path | None = None, table_path: Path | None = None) -> dict:
    """The table, checked, with the catalogue's hook points and classes merged in."""
    path = table_path or ROOT / "sdk" / "bindings" / f"{game}.json"
    table = json.loads(path.read_text())
    game = game or table.get("game", "")
    if table.get("format") != 1 or not game or table.get("game") != game:
        raise BindingError(f"{path}: format 1 for game {game} expected")
    discs = table["discs"]
    hooks = []
    for h in table["hooks"]:
        hooks.append(dict(h, origin="table"))
    classes = []
    moby = None
    cat_path = catalogue or (ROOT / table["catalogue"] if table.get("catalogue") else None)
    if cat_path is not None:
        cat = json.loads(Path(cat_path).read_text())
        if cat.get("format") != 1 or cat.get("game") != game:
            raise BindingError(f"{cat_path}: a catalogue of format 1 for game {game} expected")
        moby = cat.get("moby")
        for c in cat.get("classes", []):
            if not NAME.match(c["name"]):
                raise BindingError(f"{cat_path}: class name {c['name']!r} is not an identifier")
            pvars = []
            for p in c.get("pvars", []):
                if p["type"] not in FIELD_KINDS or not NAME.match(p["name"]):
                    raise BindingError(f"{cat_path}: {c['name']}.{p.get('name')}: a name and one of {sorted(FIELD_KINDS)}")
                pvars.append({"name": p["name"], "type": p["type"], "offset": number(p["offset"]),
                              "readonly": bool(p.get("readonly")), "about": p.get("about", "")})
            classes.append({"name": c["name"], "class": number(c["class"]), "pvars": pvars, "about": c.get("about", "")})
            if c.get("update"):
                hooks.append({"name": f"Moby_{c['name']}_Update", "signature": c.get("signature", "void(u32)"),
                              "params": c.get("params", ["moby"]), "symbols": c["update"],
                              "about": c.get("about") or f"The update of a {c['name']} moby, each tick.",
                              "origin": "catalogue"})
        if classes and not moby:
            raise BindingError(f"{cat_path}: classes but no \"moby\" layout (class, class_type, pvars)")
    names = set()
    for h in hooks:
        if not NAME.match(h["name"]) or h["name"] in names:
            raise BindingError(f"hook point {h['name']!r}: an identifier, once")
        names.add(h["name"])
        ret, params = parse_signature(h["signature"])
        h["ret"], h["kinds"] = ret, params
        h["params"] = h.get("params") or [f"a{i}" for i in range(len(params))]
        if len(h["params"]) != len(params) or not all(NAME.match(p) for p in h["params"]):
            raise BindingError(f"hook point {h['name']}: one parameter name per argument")
    bases = {k: {d: number(v[d]) for d in discs if d in v} for k, v in table.get("bases", {}).items()}
    fields = []
    for f in table.get("fields", []):
        if f["type"] not in FIELD_KINDS or not all(NAME.match(p) for p in f["name"].split(".")):
            raise BindingError(f"field {f['name']!r}: dotted identifiers, of {sorted(FIELD_KINDS)}")
        if "base" in f:
            base = bases[f["base"]]
            addresses = [base[d] + number(f.get("offset", 0)) if d in base else 0 for d in discs]
        else:
            addresses = [number(f["address"][d]) if d in f["address"] else 0 for d in discs]
        fields.append({"name": f["name"], "type": f["type"], "readonly": bool(f.get("readonly")),
                       "addresses": addresses, "about": f.get("about", "")})
    names = [f["name"] for f in fields]
    for n in names:
        if any(o.startswith(n + ".") for o in names):
            raise BindingError(f"field {n} is also a table of fields")
    return {"game": game, "discs": discs, "hooks": hooks, "fields": fields, "classes": classes, "moby": moby}


# ---- The outputs ----

HEAD_C = "/* Generated from sdk/bindings/{game}.json by sdk/tools/gen_binding.py. Do not edit. */\n"


def c_binding(b: dict) -> str:
    g = b["game"]
    out = [HEAD_C.format(game=g),
           "/* SPDX-License-Identifier: GPL-3.0-or-later */",
           "/* The script mods' side of the game's hook points and its fields (docs/port/MODS.md, section 11;",
           " * openrac/script_binding.h). */",
           "#include <stddef.h>",
           "#include <stdint.h>",
           "",
           '#include "openrac/script_binding.h"',
           "",
           f"static const openrac_script_point k_points[{max(1, len(b['hooks']))}];",
           ""]
    for i, h in enumerate(b["hooks"]):
        n = h["name"]
        ret = "void" if h["ret"] == "void" else KINDS[h["ret"]][0]
        ctypes = [KINDS[k][0] for k in h["kinds"]]
        params = ", ".join(f"{t} {p}" for t, p in zip(ctypes, h["params"])) or "void"
        fn_t = f"{n}_fn"
        out.append(f"/* {n} {h['signature']}: {h.get('about', '')} */")
        out.append(f"typedef {ret} (*{fn_t})({', '.join(ctypes) or 'void'});")
        out.append(f"static openrac_script_link k_{n}_links[OPENRAC_SCRIPT_LINKS];")
        kinds = ", ".join(KINDS[k][1] for k in h["kinds"]) or "OPENRAC_SV_VOID"
        out.append(f"static const uint8_t k_{n}_kinds[] = {{{kinds}}};")
        names = ", ".join(f'"{p}"' for p in h["params"]) or "NULL"
        out.append(f"static const char* const k_{n}_names[] = {{{names}}};")
        # The call of an original with the values.
        args = ", ".join(f"({t})v[{j}].{KINDS[k][2]}" for j, (t, k) in enumerate(zip(ctypes, h["kinds"])))
        out.append(f"static void {n}_call(void* original, const openrac_script_value* v, openrac_script_value* r) {{")
        if not h["kinds"]:
            out.append("    (void)v;")
        if h["ret"] == "void":
            out.append("    (void)r;")
            out.append(f"    (({fn_t})original)({args});")
        else:
            out.append(f"    r->{KINDS[h['ret']][2]} = (({fn_t})original)({args});")
        out.append("}")
        for k in range(LINKS):
            out.append(f"static {ret} {n}_{k}({params}) {{")
            nv = max(1, len(h["kinds"]))
            out.append(f"    openrac_script_value v[{nv}] = {{{{0, 0.0}}}};")
            for j, (p, kind) in enumerate(zip(h["params"], h["kinds"])):
                field = KINDS[kind][2]
                cast = "(double)" if field == "f" else "(int64_t)"
                out.append(f"    v[{j}].{field} = {cast}{p};")
            out.append("    openrac_script_value r = {0, 0.0};")
            call = ", ".join(h["params"])
            out.append(f"    if (!openrac_script_dispatch(&k_points[{i}], &k_{n}_links[{k}], v, &r)) {{")
            if h["ret"] == "void":
                out.append(f"        (({fn_t})k_{n}_links[{k}].original)({call});")
                out.append("        return;")
                out.append("    }")
            else:
                out.append(f"        return (({fn_t})k_{n}_links[{k}].original)({call});")
                out.append("    }")
                out.append(f"    return ({ret})r.{KINDS[h['ret']][2]};")
            out.append("}")
        thunks = ", ".join(f"(void*){n}_{k}" for k in range(LINKS))
        out.append(f"static void* const k_{n}_thunks[OPENRAC_SCRIPT_LINKS] = {{{thunks}}};")
        out.append("")
    out.append(f"static const openrac_script_point k_points[{max(1, len(b['hooks']))}] = {{")
    for h in b["hooks"]:
        n = h["name"]
        rk = "OPENRAC_SV_VOID" if h["ret"] == "void" else KINDS[h["ret"]][1]
        out.append(f'    {{"{n}", "{h["signature"]}", {len(h["kinds"])}, k_{n}_kinds, k_{n}_names, {rk}, k_{n}_links, '
                   f"k_{n}_thunks, {n}_call}},")
    if not b["hooks"]:
        out.append("    {NULL, NULL, 0, NULL, NULL, 0, NULL, NULL, NULL},")
    out.append("};")
    out.append("")
    discs = ", ".join(f'"{d}"' for d in b["discs"])
    out.append(f"static const char* const k_discs[] = {{{discs}}};")
    for i, f in enumerate(b["fields"]):
        addrs = ", ".join(f"0x{a:08X}u" for a in f["addresses"])
        out.append(f"static const uint32_t k_field_{i}[] = {{{addrs}}}; /* {f['name']} */")
    out.append(f"static const openrac_script_field k_fields[{max(1, len(b['fields']))}] = {{")
    for i, f in enumerate(b["fields"]):
        out.append(f'    {{"{f["name"]}", {KINDS[f["type"]][1]}, {1 if f["readonly"] else 0}, k_field_{i}}},')
    if not b["fields"]:
        out.append("    {NULL, 0, 0, NULL},")
    out.append("};")
    out.append("")
    # Classes from the catalogue: openrac.pvars(moby).
    if b["classes"]:
        m = b["moby"]
        for i, c in enumerate(b["classes"]):
            out.append(f"static const openrac_script_pvar k_class_{i}_pvars[{max(1, len(c['pvars']))}] = {{")
            for p in c["pvars"]:
                out.append(f'    {{"{p["name"]}", {KINDS[p["type"]][1]}, {1 if p["readonly"] else 0}, 0x{p["offset"]:X}u}},')
            if not c["pvars"]:
                out.append("    {NULL, 0, 0, 0},")
            out.append("};")
        out.append(f"static const openrac_script_class k_classes[] = {{")
        for i, c in enumerate(b["classes"]):
            out.append(f'    {{"{c["name"]}", {c["class"]}, {len(c["pvars"])}, k_class_{i}_pvars}},')
        out.append("};")
        out.append(f"static const openrac_script_moby k_moby = {{0x{number(m['class']):X}u, {KINDS[m.get('class_type', 'i16')][1]}, "
                   f"0x{number(m['pvars']):X}u, {len(b['classes'])}, k_classes}};")
        moby = "&k_moby"
    else:
        moby = "NULL"
    out.append("static const openrac_script_binding k_binding = {")
    out.append(f'    "{g}", {len(b["discs"])}, k_discs, {len(b["hooks"])}, k_points, {len(b["fields"])}, k_fields, {moby},')
    out.append("};")
    out.append("")
    out.append("const openrac_script_binding* openrac_game_script_binding(void) {")
    out.append("    return &k_binding;")
    out.append("}")
    return "\n".join(out) + "\n"


def cpp_header(b: dict) -> str:
    g = b["game"]
    out = [f"// Generated from sdk/bindings/{g}.json by sdk/tools/gen_binding.py. Do not edit.",
           "// SPDX-License-Identifier: GPL-3.0-or-later",
           "//",
           f"// {g}'s hook points as C++ types, for the SDK's C++ layer (openrac/mod.hpp, docs/port/MODS.md):",
           "// each one's name and signature as openrac_api.hook takes them, and the type of a function of it.",
           "#pragma once",
           "",
           "#include <cstdint>",
           "",
           f"namespace openrac::{g} {{",
           ""]
    for h in b["hooks"]:
        ret = "void" if h["ret"] == "void" else KINDS[h["ret"]][0]
        args = ", ".join(KINDS[k][0] for k in h["kinds"])
        if h.get("about"):
            out.append(f"// {h['about']}")
        out.append(f"struct {h['name']} {{")
        out.append(f"    static constexpr const char* name = \"{h['name']}\";")
        out.append(f"    static constexpr const char* signature = \"{h['signature']}\";")
        out.append(f"    using fn = {ret} (*)({args});")
        out.append("};")
        out.append("")
    out.append(f"}}  // namespace openrac::{g}")
    return "\n".join(out) + "\n"


def luau_types(b: dict) -> str:
    g = b["game"]
    out = [f"-- Generated from sdk/bindings/{g}.json by sdk/tools/gen_binding.py. Do not edit.",
           "-- SPDX-License-Identifier: GPL-3.0-or-later",
           "--",
           f"-- The `openrac` global as a script mod for {g} sees it (docs/port/MODS.md, section 11): put this",
           "-- file where your editor's Luau language server reads definitions (luau-lsp: --definitions).",
           ""]
    hooks = b["hooks"]
    if hooks:
        out.append("type HookPoint = " + " | ".join(f'"{h["name"]}"' for h in hooks))
    else:
        out.append("type HookPoint = string")
    out.append('type MemoryType = "u8" | "i8" | "u16" | "i16" | "u32" | "i32" | "f32"')
    out.append("")

    def tree(fields):
        root: dict = {}
        for f in fields:
            node = root
            parts = f["name"].split(".")
            for p in parts[:-1]:
                node = node.setdefault(p, {})
            node[parts[-1]] = f
        return root

    def emit(node, indent):
        lines = []
        for k, v in node.items():
            pad = "    " * indent
            if "type" in v and isinstance(v.get("type"), str) and "addresses" in v:
                words = (["read-only"] if v["readonly"] else []) + ([v["about"]] if v["about"] else [])
                note = f" -- {'; '.join(words)}" if words else ""
                lines.append(f"{pad}{k}: number,{note}")
            else:
                lines.append(f"{pad}{k}: {{")
                lines += emit(v, indent + 1)
                lines.append(f"{pad}}},")
        return lines

    out.append("type Fields = {")
    out += emit(tree(b["fields"]), 1)
    out.append("}")
    out.append("")
    for h in hooks:
        params = ", ".join(f"{p}: number" for p in h["params"])
        ret = "()" if h["ret"] == "void" else "number"
        out.append(f"-- {h['name']} {h['signature']}: {h.get('about', '')}")
        out.append(f"type {h['name']}Fn = ({params}) -> {ret}")
    out.append("")
    out.append("declare openrac: {")
    out.append("    game: string,")
    out.append(f'    version: {" | ".join(chr(34) + d + chr(34) for d in b["discs"])},')
    out.append("    platform: string,")
    out.append("    api: string,")
    out.append("    log: (...any) -> (),")
    out.append("    warn: (...any) -> (),")
    out.append("    setting: (key: string) -> any,")
    out.append("    hook: (name: HookPoint, fn: (original: (...number) -> ...number, ...number) -> ...number) -> (),")
    out.append("    before: (name: HookPoint, fn: (...number) -> ()) -> (),")
    out.append("    after: (name: HookPoint, fn: (...number) -> ()) -> (),")
    out.append("    on_frame: (fn: () -> ()) -> (),")
    out.append("    on_pad: (fn: (port: number, report: buffer) -> ()) -> (),")
    out.append("    read: (kind: MemoryType, address: number) -> number,")
    out.append("    write: (kind: MemoryType, address: number, value: number) -> (),")
    out.append("    fields: Fields,")
    if b["classes"]:
        out.append("    pvars: (moby: number) -> { [string]: number }?,")
    out.append("}")
    return "\n".join(out) + "\n"


def outputs(b: dict) -> dict[Path, str]:
    g = b["game"]
    return {
        ROOT / "port" / "game" / g / "script_binding.c": c_binding(b),
        ROOT / "sdk" / "include" / "openrac" / "games" / f"{g}.hpp": cpp_header(b),
        ROOT / "sdk" / "types" / f"{g}.d.luau": luau_types(b),
    }


def hook_rows(game: str, discs: list[str]) -> dict[str, dict[str, str]]:
    """disc -> {name: symbol} of the hook rows of port/game/<game>-<disc>/libraries.tsv."""
    out = {}
    for d in discs:
        rows = {}
        path = ROOT / "port" / "game" / f"{game}-{d}" / "libraries.tsv"
        for line in path.read_text().splitlines():
            cols = line.split("\t")
            if not line.startswith("#") and len(cols) >= 4 and cols[3].strip() == "hook":
                rows[cols[2].strip()] = cols[0].strip()
        out[d] = rows
    return out


def check_rows(b: dict) -> list[str]:
    problems = []
    rows = hook_rows(b["game"], b["discs"])
    for d in b["discs"]:
        want = {h["name"]: h["symbols"].get(d) for h in b["hooks"] if h["symbols"].get(d)}
        have = rows[d]
        for name, sym in sorted(want.items()):
            if have.get(name) != sym:
                problems.append(f"{b['game']}-{d}/libraries.tsv: no hook row {sym} {name}")
        for name, sym in sorted(have.items()):
            if name not in want:
                problems.append(f"{b['game']}-{d}/libraries.tsv: hook row {sym} {name} is not in sdk/bindings/{b['game']}.json")
    return problems


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("games", nargs="*", default=["rac1"])
    ap.add_argument("--catalogue", type=Path, help="the behaviour catalogue (one game only)")
    ap.add_argument("--check", action="store_true", help="write nothing; fail when anything differs")
    ap.add_argument("--table", type=Path, help="another table (a test's): only its C binding, to --c")
    ap.add_argument("--c", type=Path, help="with --table: where its C binding goes")
    args = ap.parse_args(argv)
    if args.catalogue and len(args.games) != 1:
        ap.error("--catalogue goes with one game")
    if args.table:
        if not args.c:
            ap.error("--table needs --c")
        try:
            text = c_binding(load("", args.catalogue, args.table))
        except (BindingError, KeyError, ValueError) as e:
            print(f"gen_binding: {args.table}: {e}", file=sys.stderr)
            return 1
        if args.check:
            if not args.c.is_file() or args.c.read_text() != text:
                print(f"gen_binding: {args.c} is not what {args.table} gives", file=sys.stderr)
                return 1
            return 0
        args.c.write_text(text)
        return 0
    failed = False
    for game in args.games:
        try:
            b = load(game, args.catalogue)
        except (BindingError, KeyError, ValueError) as e:
            print(f"gen_binding: {game}: {e}", file=sys.stderr)
            return 1
        for path, text in outputs(b).items():
            rel = path.relative_to(ROOT)
            if args.check:
                if not path.is_file() or path.read_text() != text:
                    print(f"gen_binding: {rel} is not what sdk/bindings/{game}.json gives (run gen_binding.py)", file=sys.stderr)
                    failed = True
            else:
                path.parent.mkdir(parents=True, exist_ok=True)
                if not path.is_file() or path.read_text() != text:
                    path.write_text(text)
                    print(rel)
        for p in check_rows(b):
            print(f"gen_binding: {p}", file=sys.stderr)
            failed = True
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
