#!/usr/bin/env python3
"""
The NTSC decomp (Lombyte's project, by Mateusz Kłysz): where this repository reads it from.

  python3 tools/ntsc_source.py path              # its checkout at the pinned commit (fetched if missing)
  python3 tools/ntsc_source.py fetch [COMMIT]    # fetch the pin, or COMMIT, into the cache
  python3 tools/ntsc_source.py pin               # the pinned url and commit
  python3 tools/ntsc_source.py follow [COMMIT]   # bump the pin to upstream HEAD (or COMMIT) and re-import

It is pinned to one commit in config/versions/us/ntsc_decomp.txt (`url commit`) and read from a
cache outside the repository: $RAC1_NTSC_DECOMP/<commit>/ (default ~/.cache/rac1-decomp/ntsc), a
shallow fetch of that one commit. Nothing of it is committed here and nothing is pushed to it.
RAC1_NTSC_DECOMP_DIR names an existing checkout to use as it is instead (someone working in the
NTSC decomp's own repository); the pin is then not checked.

Its readers: tools/names.py ntsc (its names, docs/NAMES.md), tools/version_sync.py ntsc
(config/versions/us/carry.tsv), tools/build_ntsc.sh (its own build and verification), CI (its own
progress report, uploaded as SCUS_971.99_report) and tools/lombyte.py.

`follow` is how this repository keeps up with it:

  1. resolve the new commit (upstream HEAD, or COMMIT: a full SHA-1 or a branch), fetch it;
  2. write the pin;
  3. tools/build_ntsc.sh: its setup.sh, `make elf` and `make overlays`, each ending in PASS
     (Linux, its toolchain and baserom/us/SCUS_971.99 or the disc image; skipped with --no-build
     or NTSC_SKIP_BUILD=1);
  4. tools/version_sync.py ntsc: config/versions/us/carry.tsv;
  5. commit the pin and carry.tsv: "chore(ntsc): follow the NTSC decomp to <commit>";
  6. tools/names.py ntsc, then rename; commit the names on their own:
     "chore(names): the NTSC decomp's names at <commit>";
  7. list the functions it newly matched that are open here ("from the NTSC decomp" rows that
     were not in carry.tsv before).

--no-commit leaves the changes in the working tree.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PIN = ROOT / "config/versions/us/ntsc_decomp.txt"
CARRY = ROOT / "config/versions/us/carry.tsv"
NAMES = ["config/names.tsv", "config/names_ntsc.tsv", "config/modules.tsv", "include", "src", "nonmatching"]
SHA1 = re.compile(r"[0-9a-f]{40}")
DONE = ".rac1-decomp-fetched"   # written last: a cache entry without it is incomplete


def pin() -> tuple[str, str]:
    """(url, commit) from config/versions/us/ntsc_decomp.txt."""
    for line in PIN.read_text().splitlines():
        if line.strip() and not line.startswith("#"):
            url, commit = line.split()
            if not SHA1.fullmatch(commit):
                sys.exit(f"{PIN.relative_to(ROOT)}: {commit!r} is not a full commit SHA-1")
            return url, commit
    sys.exit(f"{PIN.relative_to(ROOT)}: no `url commit` line")


def write_pin(url: str, commit: str) -> None:
    head = [l for l in PIN.read_text().splitlines() if l.startswith("#")]
    PIN.write_text("\n".join(head + [f"{url} {commit}"]) + "\n")


def cache_root() -> Path:
    return Path(os.environ.get("RAC1_NTSC_DECOMP", Path.home() / ".cache/rac1-decomp/ntsc")).expanduser()


def git(*args, cwd=None, capture=False) -> str:
    env = dict(os.environ, GIT_TERMINAL_PROMPT="0")
    r = subprocess.run(["git", *args], cwd=cwd, env=env, text=True,
                       stdout=subprocess.PIPE if capture else None)
    if r.returncode:
        sys.exit(f"*** git {' '.join(args)} failed")
    return (r.stdout or "").strip()


def fetch(commit: str | None = None, url: str | None = None) -> Path:
    """The cache entry for COMMIT (default: the pin), fetched if it is not there yet."""
    pu, pc = pin()
    url, commit = url or pu, commit or pc
    out = cache_root() / commit
    if (out / DONE).exists():
        return out
    tmp = out.with_name(f"{commit}.tmp-{os.getpid()}")
    shutil.rmtree(tmp, ignore_errors=True)
    tmp.mkdir(parents=True)
    print(f"fetching the NTSC decomp at {commit[:10]} into {out}", file=sys.stderr)
    git("init", "-q", cwd=tmp)
    git("remote", "add", "origin", url, cwd=tmp)
    git("fetch", "-q", "--depth=1", "origin", commit, cwd=tmp)
    git("-c", "advice.detachedHead=false", "checkout", "-q", "FETCH_HEAD", cwd=tmp)
    have = git("rev-parse", "HEAD", cwd=tmp, capture=True)
    if have != commit:
        sys.exit(f"*** fetched {have}, wanted {commit}")
    with (tmp / ".git/info/exclude").open("a") as f:
        f.write(f"/{DONE}\n")
    (tmp / DONE).write_text(f"{url} {commit}\n")
    try:
        tmp.rename(out)
    except OSError:            # another process got there first
        shutil.rmtree(tmp, ignore_errors=True)
        if not (out / DONE).exists():
            raise
    return out


def path() -> Path:
    """The NTSC decomp's checkout: RAC1_NTSC_DECOMP_DIR, else the pinned commit's cache entry."""
    d = os.environ.get("RAC1_NTSC_DECOMP_DIR")
    if d:
        return Path(d).expanduser().resolve()
    return fetch()


def revision(where: Path | None = None) -> str:
    """The commit of the checkout path() gives (the pin, unless RAC1_NTSC_DECOMP_DIR is set)."""
    if not os.environ.get("RAC1_NTSC_DECOMP_DIR"):
        return pin()[1]
    return git("rev-parse", "HEAD", cwd=where or path(), capture=True)


def resolve(url: str, want: str | None) -> str:
    """A full commit SHA-1 for WANT (a full SHA-1, a branch or tag; default HEAD) at URL."""
    if want and SHA1.fullmatch(want):
        return want
    ref = want or "HEAD"
    for line in git("ls-remote", url, ref, f"refs/heads/{ref}", f"refs/tags/{ref}", capture=True).splitlines():
        return line.split()[0]
    sys.exit(f"*** {url} has no ref {ref!r} (give a branch, a tag or a full commit SHA-1)")


def sh(*cmd, env=None):
    print("+ " + " ".join(cmd), flush=True)
    if subprocess.run(cmd, cwd=ROOT, env=env).returncode:
        sys.exit(f"*** failed: {' '.join(cmd)}")


def from_rows() -> set[tuple[str, ...]]:
    if not CARRY.exists():
        return set()
    rows = (l.split("\t") for l in CARRY.read_text().splitlines() if l and not l.startswith("#"))
    return {tuple(r) for r in rows if len(r) > 1 and r[1] == "from the NTSC decomp"}


def commit_paths(message: str, paths: list[str]) -> bool:
    if subprocess.run(["git", "diff", "--quiet", "--", *paths], cwd=ROOT).returncode == 0 and \
            subprocess.run(["git", "diff", "--cached", "--quiet", "--", *paths], cwd=ROOT).returncode == 0:
        return False
    git("add", "--", *paths, cwd=ROOT)
    git("commit", "-q", "-m", message, "--", *paths, cwd=ROOT)
    print(f"committed: {message}")
    return True


def follow(want: str | None, build: bool, commit: bool) -> None:
    if os.environ.get("RAC1_NTSC_DECOMP_DIR"):
        sys.exit("unset RAC1_NTSC_DECOMP_DIR: follow reads the pinned cache")
    url, old = pin()
    new = resolve(url, want)
    print(f"the NTSC decomp: {old[:10]} -> {new[:10]}")
    fetch(new, url)
    write_pin(url, new)
    before = from_rows()
    py = sys.executable
    if build and os.environ.get("NTSC_SKIP_BUILD") != "1":
        sh("bash", "tools/build_ntsc.sh")
    sh(py, "tools/version_sync.py", "ntsc")
    if commit and not commit_paths(f"chore(ntsc): follow the NTSC decomp to {new[:10]}",
                                   [str(PIN.relative_to(ROOT)), str(CARRY.relative_to(ROOT))]):
        print("nothing to commit for the pin")
    sh(py, "tools/names.py", "ntsc")
    sh(py, "tools/names.py", "rename")
    if commit and not commit_paths(f"chore(names): the NTSC decomp's names at {new[:10]}", NAMES):
        print("names unchanged")
    new_rows = sorted(from_rows() - before)
    print(f"{len(new_rows)} functions newly matched in the NTSC decomp and open here"
          f" (config/versions/us/carry.tsv, \"from the NTSC decomp\")" + (":" if new_rows else ""))
    for r in new_rows:
        print("  " + "  ".join(x for i, x in enumerate(r) if i != 1))


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("command", choices=("path", "fetch", "pin", "follow"))
    ap.add_argument("commit", nargs="?")
    ap.add_argument("--no-build", action="store_true", help="follow: skip tools/build_ntsc.sh")
    ap.add_argument("--no-commit", action="store_true", help="follow: leave the changes uncommitted")
    a = ap.parse_args()
    if a.command == "path":
        print(path())
    elif a.command == "fetch":
        url, _ = pin()
        print(fetch(resolve(url, a.commit) if a.commit else None))
    elif a.command == "pin":
        print(" ".join(pin()))
    else:
        follow(a.commit, not a.no_build, not a.no_commit)


if __name__ == "__main__":
    main()
