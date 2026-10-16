#!/usr/bin/env bash
# Does a candidate do what the retail function does? Builds it in its file and
# relocates it (tools/equiv.py --behave, in the container), then runs both in
# an R5900 interpreter on random state on the host (tools/behave, Rust).
#
#   bash tools/behave.sh FUNC CANDIDATE.c [--trials N] [--seed S] [--show N]
set -euo pipefail
repo=$(cd "$(dirname "$0")/.." && pwd)
name=$1 cand=$2
shift 2
out=$(bash "$repo/tools/docker/run.sh" python tools/equiv.py "$name" "$cand" --behave | tail -1)
manifest=${out##*behave manifest }
if [ "$manifest" = "$out" ]; then
  echo "$out" >&2
  exit 1
fi
bin="$repo/tools/behave/target/release/behave"
[ -x "$bin" ] || cargo build --release --quiet --manifest-path "$repo/tools/behave/Cargo.toml"
echo "$name:"
"$bin" "$repo/$manifest" "$@"
