#!/usr/bin/env python3
"""Rebuild pinned RAC5 boot C witness objects with a private trial toolchain.

Run on a native Linux filesystem (including WSL's Linux filesystem) with
lawfully obtained tools. All compiler outputs stay local; this command does
not qualify matches or publish a progress report.

Most sources compile with the profile's own GNU EE chain (``cpp``, ``cc1``,
GNU ``as``). A source whose ``config/cflags.txt`` entry names another pinned
compiler is built by that compiler instead. Two are pinned so far: SN ProDG
3.01 ``ee-gcc2953`` (GCC 2.95.3), a Windows driver that runs through WSL's
Windows interop, and the Sony EE SDK compiler ``2.9-ee-991111``, a Linux
driver the 32-bit toolchain can only read files from a Linux filesystem. Each
pin states which host it runs on.

An entry may also name another pinned assembler with ``as=NAME``. The pinned
GNU ``as`` expands a 64-bit ``dli`` with ``ori`` first and reads a static
float literal from the pool, while the retail boot does neither, and no single
assembler the project holds reproduces both the retail expansion and the
pinned chain's hazard fill on every object -- so the assembler is measured per
source exactly as the compiler is. Without a selector an object is assembled
by the pinned ``as``, which is what every pre-existing entry does.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path

import cflags
from boot_c_campaign import (ACCEPTED_SOURCES, REPLACEMENTS, accepted_replacements,
                             object_digest_matches, source_path,
                             validate_replacements)
from r5900_assembly import ASSEMBLER_SHA256, LINKER_SHA256
from c_source_policy import require_c_source

ROOT = Path(__file__).resolve().parent.parent
PROFILE = ROOT / "config/boot/compiler_trial.json"
# The hosts an extra compiler may run on: "linux" for a native driver that reads
# its inputs itself, "windows" for a driver reached through WSL's Windows
# interop, whose staging directory therefore has to be Windows-visible.
HOSTS = ("linux", "windows")


def extra_option(value: str) -> tuple[str, Path]:
    """One ``SELECTOR=DIRECTORY`` command-line value."""
    selector, separator, directory = value.partition("=")
    if not selector or not separator or not directory:
        raise argparse.ArgumentTypeError(f"expected SELECTOR=DIRECTORY, got {value!r}")
    return selector, Path(directory)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def normalized_digest(path: Path) -> str:
    return hashlib.sha256(path.read_text(encoding="utf-8")
                          .replace("\r\n", "\n").encode("utf-8")).hexdigest()


def require_private_output(path: Path) -> Path:
    resolved = path.resolve()
    if resolved.is_relative_to(ROOT) and not resolved.is_relative_to(ROOT / "build"):
        raise ValueError(f"compiler output must stay under ignored build/: {resolved}")
    return resolved


def windows_path(path: Path) -> str:
    """Windows form of a path, for the argument list of a Windows-side driver.

    Under WSL the pinned extra compiler is a Windows executable reached through
    interop; it cannot open a Linux path, so the ``-B`` directory it is given
    must be spelled the way Windows sees it. A native host returns the path
    unchanged, which is what a Windows run wants.
    """
    if os.name == "nt":
        return str(path)
    try:
        done = subprocess.run(["wslpath", "-w", str(path)], capture_output=True,
                              text=True, timeout=30, check=False)
    except (OSError, subprocess.SubprocessError):
        return str(path)
    converted = done.stdout.strip()
    return converted if done.returncode == 0 and converted else str(path)


def check_profile(cpp: Path, cc1: Path, assembler: Path) -> dict[str, object]:
    validate_replacements()
    accepted_replacements()
    profile = json.loads(PROFILE.read_text(encoding="utf-8"))
    accepted_paths = sorted(source_path(name).relative_to(ROOT).as_posix()
                            for name in ACCEPTED_SOURCES)
    if (profile["status"] != "qualified_boot_only"
            or profile["expected_objects"] != len(REPLACEMENTS)
            or profile["accepted_sources"] != accepted_paths):
        raise ValueError("compiler trial catalogue no longer matches the measured scope")
    if "expected_function_witnesses" in profile:
        raise ValueError("witness counts come from the verified boundary catalogue, not the compiler profile")
    variants = profile.get("object_sha256_alternates", {})
    measured = {"sceKernelGetSystemTimeLow__Fv", "__srefill", "fflush",
                "fread", "_multadd", "_calloc_r"}
    if (set(variants) != measured
            or any(not isinstance(variants[name], list)
                   or len(variants[name]) != 1
                   or not isinstance(variants[name][0], str)
                   or len(variants[name][0]) != 64
                   or any(char not in "0123456789abcdef"
                          for char in variants[name][0])
                   for name in measured)):
        raise ValueError("unmeasured object-hash alternative")
    patch = ROOT / profile["source"]["patch"]
    if normalized_digest(patch) != profile["source"]["patch_sha256_lf"]:
        raise ValueError("RAC5 compiler trial patch differs from pinned source")
    for label, path in (("cpp", cpp), ("cc1", cc1), ("as", assembler)):
        if digest(path) != profile["tools"][f"{label}_sha256"]:
            raise ValueError(f"{label} differs from measured RAC5 trial")
    if (profile["tools"]["sn_assembler_sha256"] != ASSEMBLER_SHA256
            or profile["tools"]["sn_linker_sha256"] != LINKER_SHA256):
        raise ValueError("SN tool pins disagree with the boot linker witness")
    if profile["flags"] != {"cpp": [], "cc1": ["-quiet", "-O2", "-G0"], "as": []}:
        raise ValueError("compiler trial flags differ from measured build")
    return profile


def check_extra_compiler(profile: dict, selector: str, bin_dir: Path | None,
                         stage_dir: Path | None = None,
                         output: Path | None = None) -> dict[str, object]:
    """Verify one pinned extra compiler and resolve the paths it is run from."""
    pinned = profile.get("extra_compilers", {}).get(selector)
    if pinned is None:
        raise ValueError(f"config/boot/compiler_trial.json pins no compiler "
                         f"named {selector}")
    if bin_dir is None:
        raise ValueError(f"source routed to {selector}; pass that compiler's "
                         f"directory with --extra-bin {selector}=DIRECTORY")
    host = pinned["host"]
    if host not in HOSTS:
        raise ValueError(f"{selector} host {host!r} is not one of {', '.join(HOSTS)}")
    tools = pinned["tools"]
    driver = bin_dir / tools["driver"]
    libexec = (bin_dir / tools["libexec"]).resolve()
    for label, path in (("driver", driver),
                        ("cpp", libexec / tools["cpp"]),
                        ("cc1", libexec / tools["cc1"])):
        if not path.is_file():
            raise ValueError(f"{selector} {label} is missing: {path}")
        if digest(path) != tools[f"{label}_sha256"]:
            raise ValueError(f"{selector} {label} differs from measured RAC5 trial")
    flags = pinned["flags"]
    if set(flags) != {"driver", "as"} or list(flags["driver"]) != ["-S"]:
        raise ValueError(f"{selector} flags differ from measured build")
    # A pin may carry a source patch. The binary it names is that build plus the
    # patch, so the patch is part of the identity and is checked like a tool: a
    # pin whose patch text moved no longer describes the binary it points at.
    if "patch" in pinned:
        patch = ROOT / pinned["patch"]["path"]
        if not patch.is_file():
            raise ValueError(f"{selector} patch is missing: {patch}")
        if normalized_digest(patch) != pinned["patch"]["sha256_lf"]:
            raise ValueError(f"{selector} patch differs from measured RAC5 trial")
    # Where the routed source is staged for that driver. A pin may name its own
    # directory; without one it stages under --output-dir, which is already a
    # native Linux path, so a Linux driver can read its own inputs there.
    if stage_dir is not None:
        stage = stage_dir
    elif "stage" in pinned:
        stage = ROOT / pinned["stage"]
    elif output is not None:
        stage = output / f"{selector}-stage"
    else:
        raise ValueError(f"{selector} needs a staging directory")
    # The driver resolves cpp/cc1 itself through -B; they are checked above so a
    # swapped front end cannot hide behind a correct driver hash.
    return {"id": pinned["id"], "host": host, "driver": driver, "libexec": libexec,
            "flags": flags, "stage": require_private_output(stage)}


def check_extra_assembler(profile: dict, selector: str,
                          bin_dir: Path | None) -> dict[str, object]:
    """Verify one pinned extra assembler and resolve the binary it names.

    An extra assembler is a plain binary the project runs directly, so unlike a
    routed compiler there is no driver, no staging and no libexec to resolve:
    the pin names one file, the caller says which directory holds it, and the
    measured flags say it is invoked the same way as the pinned GNU ``as``.
    """
    pinned = profile.get("extra_assemblers", {}).get(selector)
    if pinned is None:
        raise ValueError(f"config/boot/compiler_trial.json pins no assembler "
                         f"named {selector}")
    if bin_dir is None:
        raise ValueError(f"source routed to {selector}; pass that assembler's "
                         f"directory with --as-bin {selector}=DIRECTORY")
    host = pinned["host"]
    if host not in HOSTS:
        raise ValueError(f"{selector} host {host!r} is not one of {', '.join(HOSTS)}")
    tools = pinned["tools"]
    binary = bin_dir / tools["assembler"]
    if not binary.is_file():
        raise ValueError(f"{selector} assembler is missing: {binary}")
    if digest(binary) != tools["assembler_sha256"]:
        raise ValueError(f"{selector} assembler differs from measured RAC5 trial")
    # An assembler pin may carry flags of its own: the SDK `as` puts a `li.s`
    # float in a GP-relative lit4 pool unless it is run with -G0, and that pool
    # does not link in this campaign's script. The flag belongs to the pin, not
    # to a source line: it says how that binary was measured, not what one file
    # needs.
    flags = pinned["flags"]
    if (set(flags) != {"as"} or any(not isinstance(flag, str)
                                    or not flag.startswith("-")
                                    for flag in flags["as"])):
        raise ValueError(f"{selector} flags differ from measured build")
    return {"id": pinned["id"], "host": host, "binary": binary, "flags": flags}


def cc1_flags(profile: dict, per_source: dict[str, cflags.Entry],
              source: Path) -> list[str]:
    """Pinned-chain cc1 flags for one source, plus the flags measured for that file.

    An entry that selects another compiler carries that compiler's flag set, so
    it contributes nothing here; its object is not built by the pinned chain.
    """
    entry = cflags.for_source(per_source, source)
    return [*profile["flags"]["cc1"], *(entry.flags if entry.pinned else ())]


def compiler_flags(extra: dict, per_source: dict[str, cflags.Entry],
                   source: Path) -> list[str]:
    """Flags an extra compiler is run with for one source, in command order.

    A relative ``-I`` flag is resolved against the repository root: the driver
    runs from a staging directory, so a repository-relative include path would
    otherwise be looked up next to the staged source.
    """
    entry = cflags.for_source(per_source, source)
    flags = []
    for flag in entry.flags:
        if flag.startswith("-I") and len(flag) > 2 and not Path(flag[2:]).is_absolute():
            resolved = (ROOT / flag[2:]).resolve()
            flag = "-I" + (windows_path(resolved)
                           if extra["host"] == "windows" else str(resolved))
        flags.append(flag)
    return [*extra["flags"]["driver"], *flags]


def run(command: list[str], cwd: Path, log: Path, stdout: Path | None = None) -> None:
    if stdout is None:
        result = subprocess.run(command, cwd=cwd, capture_output=True, text=True,
                                timeout=120, check=False)
        log.write_text(result.stdout + result.stderr, encoding="utf-8")
    else:
        with stdout.open("wb") as output:
            result = subprocess.run(command, cwd=cwd, stdout=output,
                                    stderr=subprocess.PIPE, timeout=120, check=False)
        log.write_bytes(result.stderr)
    if result.returncode:
        raise ValueError(f"{Path(command[0]).name} failed; see {log}")


def compile_pinned(profile: dict, per_source: dict[str, cflags.Entry], source: Path,
                   output: Path, cpp: Path, cc1: Path, item) -> list[str]:
    """The profile's own chain: cpp, cc1, then the pinned GNU assembler."""
    preprocessed = output / f"{item.name}.i"
    assembly = output / f"{item.name}.s"
    flags = cc1_flags(profile, per_source, source)
    run([str(cpp.resolve()), *profile["flags"]["cpp"], source.name], output,
        output / f"{item.name}.cpp.log", preprocessed)
    require_c_source(preprocessed.read_text(encoding="utf-8"), f"{item.name} preprocessed input")
    run([str(cc1.resolve()), *flags, preprocessed.name,
         "-o", assembly.name], output, output / f"{item.name}.cc1.log")
    return flags


def compile_extra(extra: dict, per_source: dict[str, cflags.Entry], source: Path,
                  output: Path, item) -> list[str]:
    """A pinned extra compiler driver, staged where that driver can read it.

    A Windows driver needs a Windows-visible directory and a Windows ``-B`` path;
    a Linux driver reads a native path itself, and 32-bit ones can only read it
    from a Linux filesystem. The preprocessed input is kept and checked exactly
    as the pinned chain's is, so a macro from the driver's own include path
    cannot smuggle assembly into a routed body either.
    """
    stage = extra["stage"]
    staged = stage / item.name
    staged.mkdir(parents=True, exist_ok=True)
    (staged / source.name).write_bytes(source.read_bytes())
    assembly = staged / f"{item.name}.s"
    libexec = (windows_path(extra["libexec"]) if extra["host"] == "windows"
               else str(extra["libexec"]))
    prefix = [str(extra["driver"].resolve()), f"-B{libexec}/"]
    flags = compiler_flags(extra, per_source, source)
    pre = [flag for flag in flags if flag.startswith(("-I", "-D", "-U"))]
    run([*prefix, *pre, "-E", source.name], staged, output / f"{item.name}.cpp.log",
        output / f"{item.name}.i")
    require_c_source((output / f"{item.name}.i").read_text(encoding="utf-8"),
                     f"{item.name} preprocessed input")
    run([*prefix, *flags, source.name, "-o", assembly.name], staged,
        output / f"{item.name}.driver.log")
    if not assembly.is_file():
        raise ValueError(f"{item.name}: {extra['id']} produced no assembly")
    shutil.copyfile(assembly, output / f"{item.name}.s")
    return flags


def compile_objects(cpp: Path, cc1: Path, assembler: Path,
                    output_dir: Path, export_dir: Path | None,
                    extra_bins: dict[str, Path] | None = None,
                    extra_stages: dict[str, Path] | None = None,
                    extra_as_bins: dict[str, Path] | None = None) -> None:
    profile = check_profile(cpp, cc1, assembler)
    output = require_private_output(output_dir)
    export = require_private_output(export_dir) if export_dir is not None else None
    if output == export:
        raise ValueError("output and export directories must differ")
    output.mkdir(parents=True, exist_ok=True)
    per_source = cflags.load()
    selected = {entry.compiler for entry in per_source.values() if not entry.pinned}
    bins, stages = extra_bins or {}, extra_stages or {}
    extras = {name: check_extra_compiler(profile, name, bins.get(name),
                                         stages.get(name), output)
              for name in sorted(selected)}
    for extra in extras.values():
        extra["stage"].mkdir(parents=True, exist_ok=True)
    selected_as = {entry.assembler for entry in per_source.values()
                   if not entry.assembler_pinned}
    as_bins = extra_as_bins or {}
    alt_assemblers = {name: check_extra_assembler(profile, name, as_bins.get(name))
                      for name in sorted(selected_as)}
    manifest = {"profile": profile["id"], "status": "qualified_boot_only",
                "compilers": {name: extra["id"] for name, extra in extras.items()},
                "assemblers": {name: extra["id"]
                               for name, extra in alt_assemblers.items()},
                "objects": []}
    for item in REPLACEMENTS:
        source = source_path(item.name)
        require_c_source(source.read_text(encoding="utf-8"), item.name)
        if normalized_digest(source) != item.source_sha256:
            raise ValueError(f"C source differs from measured trial: {item.name}")
        (output / source.name).write_bytes(source.read_bytes())
        obj = output / f"{item.name}.o"
        entry = cflags.for_source(per_source, source)
        if entry.pinned:
            flags = compile_pinned(profile, per_source, source, output,
                                   cpp, cc1, item)
        else:
            flags = compile_extra(extras[entry.compiler], per_source, source,
                                  output, item)
        chosen = (assembler if entry.assembler_pinned
                  else alt_assemblers[entry.assembler]["binary"])
        chosen_flags = (() if entry.assembler_pinned
                        else alt_assemblers[entry.assembler]["flags"]["as"])
        run([str(chosen.resolve()), *chosen_flags, "-o", obj.name,
             f"{item.name}.s"], output, output / f"{item.name}.as.log")
        observed_digest = digest(obj)
        if not object_digest_matches(item, observed_digest, profile):
            raise ValueError(f"compiled object differs from measured trial: {item.name}")
        manifest["objects"].append({"name": item.name, "compiler": entry.compiler,
                                    "compiler_flags": flags,
                                    "assembler": entry.assembler,
                                    "source_sha256_lf": item.source_sha256,
                                    "object_sha256": observed_digest,
                                    "object_sha256_primary_pin": item.object_sha256})
    (output / "trial_manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8", newline="\n")
    if export is not None:
        export.mkdir(parents=True, exist_ok=True)
        for item in REPLACEMENTS:
            shutil.copyfile(output / f"{item.name}.o", export / f"{item.name}.o")
        shutil.copyfile(output / "trial_manifest.json", export / "trial_manifest.json")
    extra_count = sum(row["compiler"] != "pinned" for row in manifest["objects"])
    as_count = sum(row["assembler"] != "pinned" for row in manifest["objects"])
    print(f"{len(REPLACEMENTS)} byte-identical objects for the pinned boot C "
          f"campaign, {len(ACCEPTED_SOURCES)} accepted under the boot-only "
          f"profile, {extra_count} object(s) from another pinned compiler, "
          f"{as_count} assembled by another pinned assembler", flush=True)


def add_extra_arguments(parser: argparse.ArgumentParser) -> None:
    """The pinned-extra-compiler options, shared with the complete boot gate."""
    parser.add_argument("--extra-bin", type=extra_option, action="append",
                        default=[], metavar="SELECTOR=DIRECTORY",
                        help="directory holding a pinned extra compiler's driver; "
                             "required once a config/cflags.txt entry selects it")
    parser.add_argument("--extra-stage", type=extra_option, action="append",
                        default=[], metavar="SELECTOR=DIRECTORY",
                        help="staging directory for that driver, when the pin "
                             "does not name one")
    parser.add_argument("--as-bin", type=extra_option, action="append",
                        default=[], metavar="SELECTOR=DIRECTORY",
                        help="directory holding a pinned extra assembler's "
                             "binary; required once a config/cflags.txt entry "
                             "selects it with as=SELECTOR")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cpp", type=Path, required=True)
    parser.add_argument("--cc1", type=Path, required=True)
    parser.add_argument("--as", dest="assembler", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--export-dir", type=Path)
    add_extra_arguments(parser)
    args = parser.parse_args()
    compile_objects(args.cpp, args.cc1, args.assembler,
                    args.output_dir, args.export_dir,
                    dict(args.extra_bin), dict(args.extra_stage),
                    dict(args.as_bin))


if __name__ == "__main__":
    main()
