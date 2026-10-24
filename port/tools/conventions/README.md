# conventions

`check_conventions.py` checks the port's C and C++ against the rules of
[CODING_CONVENTIONS.md](../../docs/CODING_CONVENTIONS.md) that a program can
count. CI runs it on every push and pull request (the "Port conventions" step
of [checks.yml](../../../.github/workflows/checks.yml)); run it yourself before
you commit.

```sh
# From the top of OpenRAC.
python3 port/tools/conventions/check_conventions.py                          # every file
python3 port/tools/conventions/check_conventions.py --since origin/master    # + new files
python3 port/tools/conventions/check_conventions.py --since origin/master --touched
python3 -m unittest discover -s port/tools/conventions                       # its tests
```

It prints `path:line: rule: message` for each finding and exits 1 when a rule
that gates fails. An advisory is printed with `(advisory)` and does not fail.

## What it checks

| Files | Rule | Gates |
|---|---|---|
| every C and C++ file in scope | line 1 is an SPDX licence line; no tab; no em or en dash | yes |
| every C and C++ file in scope | no `getenv`, `std::getenv` or `SDL_getenv` of an `OPENRAC_` name, or of a name that is not a literal, outside `common/env.cpp` and `runtime/host_env.cpp` (see [the getenv rule](#the-getenv-rule)) | yes; a stale exception is advisory |
| C++ files added since `--since` | lines 1 and 2 are the `//` licence and copyright lines; clang-format reports nothing; no `static` at namespace scope; no raw `new`, `delete`, `malloc`, `free`; no `using namespace` in a header; `extern "C"` only in a file named for a C contract, in `game/common/lib/` or in a test; no `printf` family or `std::cout`/`std::cerr` outside tests and `main` files | yes |
| C files added since `--since` | line 2 is the copyright line; clang-format reports nothing | advisory |
| lines changed since `--since` (`--touched`) | clang-format would not change them (through `git-clang-format`) | advisory; `--strict-touched` makes it gate |

In scope: every C and C++ file under `port/` that git tracks or would add,
except a game's hand C (`port/game/*/hand/`, which keeps the decompilation's
style), build trees and generated files (a "Do not edit" in their first two
lines). A `.h` is C++ when its code opens a namespace; the C contracts are the
headers of `port/runtime/include/openrac/` and
`port/game/common/include/openrac/` that do not.

The new-file rules read the code with comments and string literals blanked out,
so prose and messages may say "new" or "free". They are text patterns, not a
parser: a finding that is wrong is a bug in this tool, fixed here with a test,
not a reason to bend the code.

## The getenv rule

The port's `OPENRAC_*` variables have one door (section 16 of the
conventions): C++ reads them through `common/env.h`, C through the contract
`openrac/host_env.h`. `getenv` itself is called only by the table,
`common/env.cpp`, and the C side of the contract, `runtime/host_env.cpp`.
Anywhere else, a call of `getenv`, `std::getenv` or `SDL_getenv` is an error
when its name is a literal that starts with `OPENRAC_`, or is not a literal at
all (a computed name may be one of the port's). A variable the port does not
own (`HOME`, `SDL_VIDEO_DRIVER`) may be read by its literal name; C++ reads it
through `env::system_variable`. Vendored code (a path part named
`third_party`) is not checked, nor is anything outside the scope above.

`GETENV_EXCEPTIONS`, at the top of `check_conventions.py`, names the files
that still read `OPENRAC_` variables with `getenv` and what moves each: rac1's
host C until P2-38, and `tests/vulkan_uniform_test.cpp`, which reads its
switch to know what to expect. The package that moves a file's reads removes
its entry in the same commit series. An entry whose file reads none any more,
or is gone, is printed as a stale advisory rather than a failure, since other
work removes reads meanwhile. Never add an entry for new code.

## Raw allocation

New C++ owns its memory through containers and `std::make_unique` (section 11
of the conventions), so `new`, `delete`, `malloc` and `free` fail a new file.
`ALLOCATION_EXCEPTIONS` names the files that cannot do otherwise, each with
why: `tests/render_bench.cpp` replaces the global `operator new` and `delete`
to count a frame's allocations, and takes their memory from the C library,
since it cannot take it from the heap it counts.

## clang-format

The format rules run the clang-format it finds: `--clang-format BIN`, else
`clang-format` on `PATH`, else Xcode's (`xcrun --find clang-format`). CI uses
21.1.8 from PyPI (`pip install clang-format==21.1.8`), which also brings
`git-clang-format` for `--touched`. Without a clang-format, the format rules
are skipped with a message.

## Adding a rule

A rule belongs here when the conventions page states it and a program can
count it without false findings on the code it applies to. Add it as a
function of the file's text, with a test for what it finds and for what it must
not find, and list it in the table above and in section 2 of the conventions
page.
