#!/usr/bin/env bash
# Regenerate asm/ from your own baserom. asm/ is NOT in git: it is the
# disassembly of the retail executable (instruction bytes included), which
# the repo must never contain -- see LEGAL.md.
#
#   bash tools/setup_asm.sh
#
# Needs baserom/SCES_509.16 (your own dump) and the pinned packages from
# requirements.txt. Output is byte-identical to what the build and the audit
# tools were verified against; this script checks the inputs that decide
# that (baserom hash, splat/spimdisasm versions) before touching anything.
set -euo pipefail
cd "$(dirname "$0")/.."

if [ -d "$PWD/.venv/bin" ]; then
  export PATH="$PWD/.venv/bin:$PATH"
elif [ -d "$PWD/../../.venv/bin" ]; then
  export PATH="$PWD/../../.venv/bin:$PATH"
fi

PYTHON="python"
if ! command -v python >/dev/null 2>&1; then
  PYTHON="python3"
fi

want_sha1=79956931bd62fafd8d20fa2eae796dbaf2e15e83
[ -f baserom/SCES_509.16 ] || { echo "missing baserom/SCES_509.16 (dump your own copy; see LEGAL.md)"; exit 1; }
have_sha1=$("$PYTHON" -c "import hashlib;print(hashlib.sha1(open('baserom/SCES_509.16','rb').read()).hexdigest())")
[ "$have_sha1" = "$want_sha1" ] || { echo "baserom/SCES_509.16 sha1 $have_sha1, expected $want_sha1 (PAL v2.00)"; exit 1; }

check_deps() {
  "$PYTHON" - <<'PY'
import importlib.metadata as m, sys
want = {"splat64": "0.50.0", "spimdisasm": "1.42.4", "rabbitizer": "1.16.2"}
bad = {k: (m.version(k) if k in {d.metadata["Name"] for d in m.distributions()} else None, v)
       for k, v in want.items()}
bad = {k: v for k, v in bad.items() if v[0] != v[1]}
if bad:
    sys.exit(1)
PY
}

if ! check_deps 2>/dev/null; then
  echo "Setting up local Python virtual environment for splat..."
  [ -d .venv ] || "$PYTHON" -m venv .venv
  .venv/bin/pip install -q -r requirements.txt
  export PATH="$PWD/.venv/bin:$PATH"
  PYTHON="$PWD/.venv/bin/python"
fi

rm -rf asm
# Existing src/*.c and include/ are left untouched: config/splat.yaml sets
# generate_asm_macros_files: False, and splat never overwrites a .c file.
"$PYTHON" -m splat split config/splat.yaml
# Same post-processing the committed asm used to carry.
"$PYTHON" tools/fix_vu0_macro.py asm
"$PYTHON" tools/sn_regnames.py asm
"$PYTHON" tools/fix_denormal_floats.py asm
"$PYTHON" tools/organize_asm.py
echo "asm/ ready ($(find asm -type f -name '*.s' | wc -l) files)"

# Level code overlays (docs/OVERLAYS.md): asm/overlays/<name>.s for every
# distinct shared/level function, from baserom/overlays/ (assumes
# `python3 tools/overlays.py dump` has already populated it -- that needs
# baserom/SCES_509.16.iso, so it is not repeated here).
if [ -d baserom/overlays ]; then
    "$PYTHON" tools/overlay_asm.py
else
    echo "baserom/overlays missing: skipping asm/overlays (run tools/overlays.py dump first)"
fi
