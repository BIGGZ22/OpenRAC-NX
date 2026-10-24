#!/usr/bin/env bash
# Build and verify the NTSC-U version from the NTSC decomp's own sources.
#
# The NTSC decomp (Lombyte's project) is read at its pinned commit from a
# cache outside this repository (tools/ntsc_source.py path; below $NTSC_DIR).
# This script only drives its own entry points:
#
#   1. $NTSC_DIR/setup.sh --elf ... [--iso ...]
#        downloads and hash-checks its pinned toolchains into $NTSC_DIR/tools/,
#        builds its game compiler from source, creates $NTSC_DIR/.venv,
#        copies the boot ELF into $NTSC_DIR/config/us/SCUS_971.99, runs
#        `make elf` (verify-baseline.sh: "PASS: reconstructed boot ELF matches
#        retail") and, given a disc image, extracts the level overlays
#        (scripts/overlay-extract.py, about 10 minutes the first time).
#   2. make overlays   (when the overlays were extracted; ends with PASS)
#   3. python3 scripts/gen_progress_report.py   (with --progress)
#
# Usage (from anywhere; paths are relative to the rac1-decomp checkout):
#
#   tools/build_ntsc.sh              setup + boot ELF + overlays (if a disc image is found)
#   tools/build_ntsc.sh --elf-only   setup + boot ELF only
#   tools/build_ntsc.sh --progress   also write the NTSC decomp's progress report
#   tools/build_ntsc.sh --check      the NTSC decomp's ./setup.sh --check, nothing built
#
# Inputs (never committed; first one found wins):
#   NTSC_ELF   boot ELF          default baserom/us/SCUS_971.99
#   NTSC_ISO   USA disc image    default baserom/SCUS_971.99.iso, then baserom/us/SCUS_971.99.iso
#              (only needed for the overlays; the boot ELF is extracted from it
#              when NTSC_ELF is missing)
#
# Environment:
#   NTSC_DIR         the NTSC decomp checkout (default: `tools/ntsc_source.py path`,
#                    the pinned commit in ~/.cache/rac1-decomp/ntsc/<commit>)
#   NTSC_WINE_SHIM   1 (the default): the NTSC decomp's configure.py runs its SN
#                    tools (Ps2EeAs.exe, ee-gcc.exe) as `wine ...` from PATH (it
#                    has no RNC_WINE override any more), so the script writes a
#                    `wine` shim into $NTSC_DIR/tools/hostbin (gitignored) and puts
#                    it first on PATH. The shim:
#                    - runs each call in one of NTSC_WINE_SLOTS prefixes, so the
#                      parallel ninja jobs do not queue behind one wineserver;
#                    - keeps Wine headless and makes a crash exit instead of
#                      hanging in winedbg; kills a call after NTSC_WINE_TIMEOUT s;
#                    - runs Ps2EeAs.exe from a copy at a short path
#                      (NTSC_PS2EEAS_DIR) whose PE stack reservation is
#                      NTSC_PS2EEAS_STACK_MB MiB (see below).
#                    0: no shim, the `wine` already on PATH runs as is.
#   NTSC_WINE_ROOT   a user-local Wine install (a directory with usr/bin/wine or
#                    bin/wine: an extracted distro package or a portable build)
#                    for hosts without system Wine or sudo (default: `wine` on PATH)
#   NTSC_WINE_SLOTS  number of Wine prefixes in the pool (default: nproc, at most 16;
#                    each is ~400 MB, copied with --reflink=auto, so free on btrfs/xfs)
#   NTSC_WINE_POOL   where the pool's prefixes live (default: $NTSC_DIR/build/wine-pool)
#   NTSC_WINE_TIMEOUT  seconds before one PE tool call is killed (default 900)
#   NTSC_PS2EEAS_DIR  where that copy of Ps2EeAs.exe lives (default /tmp/ps2eeas-$UID).
#                    Ps2EeAs crashes ("Unhandled stack overflow") on the largest
#                    inputs (src/overlays/l17/gameplay/entities/door.c and
#                    l03/gameplay/entities/metropolis_train_locomotive.c at 4e1f255e)
#                    when its own path is about 80 characters or longer, whatever
#                    its stack size. The NTSC decomp installs it at
#                    tools/compilers/sn-compiler/ee/bin/, which inside the
#                    cache is about 120 characters on a typical host.
#   NTSC_PS2EEAS_STACK_MB  stack reservation of that copy (default 256; 0 keeps the
#                    installed 64 MiB). Only the PE header's SizeOfStackReserve
#                    differs; the objects are identical.
#   NTSC_COMPILER_JOBS  -j for building the game compiler (default 8; GCC 2.9's
#                    Makefile races at high -j, so setup.sh's all-CPUs default
#                    can fail on many-core hosts)
#
# Host requirements (setup.sh checks them): x86-64 Linux, glibc >= 2.38,
# Python >= 3.10, curl git make tar xz gcc (with -m32 support) flex, and
# Wine with 32-bit support. `ninja` comes from the NTSC decomp's .venv; when
# it is not on PATH a shim is put in tools/hostbin for setup.sh's check.
# setup.sh installs missing packages with sudo only on Debian/Ubuntu; on
# other distributions it just warns, so install them yourself there.
set -Eeuo pipefail

REPO=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)
if [[ -z "${NTSC_DIR:-}" ]]; then
    NTSC_DIR=$(cd "$REPO" && python3 tools/ntsc_source.py path) || { echo "build_ntsc: tools/ntsc_source.py path failed" >&2; exit 1; }
fi
case "$NTSC_DIR" in /*) ;; *) NTSC_DIR=$REPO/$NTSC_DIR ;; esac

MODE=all
PROGRESS=0
for arg in "$@"; do
    case "$arg" in
        --elf-only) MODE=elf ;;
        --check) MODE=check ;;
        --progress) PROGRESS=1 ;;
        -h|--help) sed -n '2,/^set -Eeuo/{/^set -Eeuo/d;s/^# \{0,1\}//;p;}' "${BASH_SOURCE[0]}"; exit 0 ;;
        *) echo "unknown option: $arg (see --help)" >&2; exit 2 ;;
    esac
done

die() { printf 'build_ntsc: %s\n' "$*" >&2; exit 1; }
say() { printf '\033[1m== %s\033[0m\n' "$*"; }

[[ -x "$NTSC_DIR/setup.sh" ]] || die "no NTSC decomp at $NTSC_DIR (see tools/ntsc_source.py)"

pick() { local f; for f in "$@"; do [[ -n "$f" && -f "$f" ]] && { printf '%s\n' "$f"; return; }; done; return 0; }
ELF=$(pick "${NTSC_ELF:-}" "$REPO/baserom/us/SCUS_971.99")
ISO=$(pick "${NTSC_ISO:-}" "$REPO/baserom/SCUS_971.99.iso" "$REPO/baserom/us/SCUS_971.99.iso")
[[ -n "${NTSC_ELF:-}" && -z "$ELF" ]] && die "NTSC_ELF=$NTSC_ELF does not exist"
[[ -n "${NTSC_ISO:-}" && -z "$ISO" ]] && die "NTSC_ISO=$NTSC_ISO does not exist"

# ------------------------------------------------------------ host shims
HOSTBIN="$NTSC_DIR/tools/hostbin"   # tools/* is gitignored in the NTSC decomp

if [[ "${NTSC_WINE_SHIM:-1}" != 0 ]]; then
    if [[ -n "${NTSC_WINE_ROOT:-}" ]]; then
        LOADER=
        for c in "$NTSC_WINE_ROOT/usr/bin/wine" "$NTSC_WINE_ROOT/bin/wine"; do
            [[ -x "$c" ]] && { LOADER=$c; break; }
        done
        [[ -n "$LOADER" ]] || die "NTSC_WINE_ROOT=$NTSC_WINE_ROOT has no usr/bin/wine or bin/wine"
    else
        # the real wine, not a shim left on PATH by an earlier run
        LOADER=$(PATH=$(printf '%s' "$PATH" | tr ':' '\n' | grep -vxF "$HOSTBIN" | paste -sd: -) command -v wine || true)
        [[ -n "$LOADER" ]] || die "no wine on PATH: install Wine with 32-bit support or set NTSC_WINE_ROOT to a user-local Wine install"
    fi
    LOADER=$(cd -- "$(dirname -- "$LOADER")" && pwd -P)/$(basename -- "$LOADER")
    POOL=${NTSC_WINE_POOL:-$NTSC_DIR/build/wine-pool}
    n=$(nproc); SLOTS=${NTSC_WINE_SLOTS:-$(( n < 16 ? n : 16 ))}
    command -v flock >/dev/null 2>&1 || die "flock (util-linux) is needed for the Wine prefix pool"
    mkdir -p "$HOSTBIN" "$POOL"
    {
        printf '#!/usr/bin/env bash\n'
        printf '# Generated by rac1-decomp tools/build_ntsc.sh; see its header.\n'
        printf 'loader=%q\npool=%q\nslots=%q\nstack_mb=%q\nlimit=%q\nshort=%q\n' \
            "$LOADER" "$POOL" "$SLOTS" "${NTSC_PS2EEAS_STACK_MB:-256}" "${NTSC_WINE_TIMEOUT:-900}" \
            "${NTSC_PS2EEAS_DIR:-/tmp/ps2eeas-$(id -u)}"
        cat <<'SHIM'
set -uo pipefail
bindir=$(dirname -- "$loader")
export WINELOADER="$loader"
[[ -x "$bindir/wineserver" ]] && export WINESERVER="$bindir/wineserver"
[[ -d "$bindir/../lib/wine" ]] && export WINEDLLPATH="$bindir/../lib/wine"
export WINEDEBUG=${WINEDEBUG:--all}
export WINEDLLOVERRIDES=${WINEDLLOVERRIDES:-mscoree,mshtml=}
unset DISPLAY WAYLAND_DISPLAY

# Ps2EeAs: run a copy at a short path (it crashes on the largest inputs when
# its own path is long), with a stack reservation of stack_mb MiB. Code and
# data are untouched (loader metadata only).
args=("$@")
if [[ ${#args[@]} -gt 0 && "${args[0]##*/}" == Ps2EeAs.exe ]]; then
    mkdir -p -- "$short"
    big="$short/as${stack_mb}.exe"
    if [[ ! -f "$big" || "${args[0]}" -nt "$big" ]]; then
        python3 - "${args[0]}" "$big.tmp.$$" "$stack_mb" <<'PATCH' && mv -f -- "$big.tmp.$$" "$big"
import struct, sys
d = bytearray(open(sys.argv[1], "rb").read())
pe = struct.unpack_from("<I", d, 0x3C)[0]
assert d[pe:pe + 4] == b"PE\0\0" and struct.unpack_from("<H", d, pe + 24)[0] == 0x10B
if int(sys.argv[3]):
    struct.pack_into("<I", d, pe + 24 + 72, int(sys.argv[3]) * 1024 * 1024)
open(sys.argv[2], "wb").write(d)
PATCH
    fi
    args[0]=$big
fi

# One prefix (and so one wineserver) per concurrent job.
slot=
for ((i = 0; i < slots; i++)); do
    exec 9>"$pool/slot-$i.lock"
    if flock -n 9; then slot=$i; break; fi
done
if [[ -z "$slot" ]]; then
    slot=$(( $$ % slots ))
    exec 9>"$pool/slot-$slot.lock"
    flock 9
fi
prefix="$pool/prefix-$slot"
if [[ ! -f "$prefix/.ready" ]]; then
    (
        flock 8
        t="$pool/template"
        if [[ ! -f "$t/.ready" ]]; then
            rm -rf -- "$t"
            export WINEPREFIX="$t"
            "$loader" wineboot -i >/dev/null 2>&1
            # headless, and a crash exits instead of waiting in winedbg
            "$loader" reg add 'HKCU\Software\Wine\Drivers' /v Graphics /d null /f >/dev/null 2>&1
            "$loader" reg add 'HKCU\Software\Wine\WineDbg' /v ShowCrashDialog /t REG_DWORD /d 0 /f >/dev/null 2>&1
            "$loader" reg add 'HKLM\Software\Microsoft\Windows NT\CurrentVersion\AeDebug' /v Debugger /d '' /f >/dev/null 2>&1
            "$loader" reg add 'HKLM\Software\Microsoft\Windows NT\CurrentVersion\AeDebug' /v Auto /d 1 /f >/dev/null 2>&1
            "${WINESERVER:-wineserver}" -w
            touch "$t/.ready"
        fi
    ) 8>"$pool/template.lock"
    rm -rf -- "$prefix"
    cp -a --reflink=auto -- "$pool/template" "$prefix"
fi
export WINEPREFIX="$prefix"
timeout -k 10 "$limit" "$loader" "${args[@]}" 9>&-
status=$?
[[ $status == 124 ]] && echo "wine shim: ${args[0]##*/} timed out after ${limit}s" >&2
exit $status
SHIM
    } > "$HOSTBIN/wine"
    chmod 0755 "$HOSTBIN/wine"
    export PATH="$HOSTBIN:$PATH"   # configure.py runs `wine`, setup.sh checks `command -v wine`
    say "Wine: $LOADER via $HOSTBIN/wine ($SLOTS prefixes in $POOL)"
fi

if ! command -v ninja >/dev/null 2>&1; then
    mkdir -p "$HOSTBIN"
    cat > "$HOSTBIN/ninja" <<EOF
#!/usr/bin/env bash
# Generated by rac1-decomp tools/build_ntsc.sh: ninja from the NTSC decomp's .venv.
exec '$NTSC_DIR/.venv/bin/ninja' "\$@"
EOF
    chmod 0755 "$HOSTBIN/ninja"
    export PATH="$HOSTBIN:$PATH"
fi
export WINEDEBUG=${WINEDEBUG:--all}

cd "$NTSC_DIR"

# --------------------------------------------------------------- check
if [[ "$MODE" == check ]]; then
    exec ./setup.sh --check
fi

# --------------------------------------------------------------- game compiler
# setup.sh builds it with --jobs = all CPUs, and GCC 2.9's Makefile races at
# high -j (flow.c before insn-flags.h exists, seen at -j32). Build it first
# with the NTSC decomp's own script at a safe job count when setup.sh would
# rebuild it (missing, or built from an older patch stack: provenance.json).
# setup.sh's own check misses a patch added since the build (0064 at 6abcf077), so
# this one also rebuilds then, before setup.sh would take the old compiler as current.
GC="tools/compilers/game-compiler"
if ! python3 - "$GC/provenance.json" patches/sce-991111b <<'EOF'
import hashlib, json, pathlib, sys
try:
    want = json.load(open(sys.argv[1])).get("patch_sha256", {})
except OSError:
    sys.exit(1)
d = pathlib.Path(sys.argv[2])
ok = want and all((d / n).is_file() and hashlib.sha256((d / n).read_bytes()).hexdigest() == h
                  for n, h in want.items())
# A patch added upstream since the build (setup.sh only rechecks the recorded ones; every
# patch in the directory is in one of build-game-compiler.py's orders).
ok = ok and all(p.name in want for p in d.glob("*.patch"))
sys.exit(0 if ok else 1)
EOF
then
    say "building the game compiler (scripts/build-game-compiler.py --jobs ${NTSC_COMPILER_JOBS:-8})"
    python3 scripts/build-game-compiler.py --output "$GC" --jobs "${NTSC_COMPILER_JOBS:-8}"
fi

# --------------------------------------------------------------- setup + boot ELF
args=()
[[ -n "$ELF" ]] && args+=(--elf "$ELF")
[[ "$MODE" == all && -n "$ISO" ]] && args+=(--iso "$ISO")
[[ -z "$ELF" && -n "$ISO" && "$MODE" == elf ]] && args+=(--iso "$ISO")
[[ ${#args[@]} -gt 0 ]] || die "no boot ELF (baserom/us/SCUS_971.99) or disc image (baserom/SCUS_971.99.iso); set NTSC_ELF or NTSC_ISO"

say "setup.sh ${args[*]}  (toolchain, make elf$([[ "$MODE" == all && -n "$ISO" ]] && echo ', overlay extraction'))"
start=$SECONDS
./setup.sh "${args[@]}"
say "boot ELF verified in $((SECONDS - start)) s: $(sha1sum build/baseline/config/us/build/SCUS_971.99)"

# --------------------------------------------------------------- overlays
if [[ "$MODE" == all ]]; then
    if [[ -d config/us/overlays/asm ]]; then
        say "make overlays"
        start=$SECONDS
        make overlays
        say "overlays verified in $((SECONDS - start)) s"
    else
        say "overlays skipped: no disc image (set NTSC_ISO)"
    fi
fi

# --------------------------------------------------------------- progress
if [[ "$PROGRESS" == 1 ]]; then
    say "progress report"
    .venv/bin/python scripts/gen_progress_report.py
fi
