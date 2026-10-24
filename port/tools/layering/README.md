# layering

`layering.py` checks the port's includes against the module layering of
[CODING_CONVENTIONS.md](../../docs/CODING_CONVENTIONS.md#18-layering) (section 18):
a module includes its own files and the modules below it, never the ones above.
The modules, the layers, the edges allowed within a layer and today's known
violations are data, in [rules.tsv](rules.tsv). It is standard-library Python
and needs no build.

It is not wired into CMake or CI yet: run it yourself before you commit a
change to the port's includes. A later package adds it to CI beside the
conventions check.

```sh
# From the top of OpenRAC.
python3 port/tools/layering/layering.py              # check port/; exit 1 on a new violation
python3 port/tools/layering/layering.py --edges      # and print every module edge with its count
python3 -m unittest discover -s port/tools/layering -v   # its tests
```

It prints `path:line: layering: message` for each violation and a summary on
standard error, and exits 0 when nothing is to fix, 1 on a violation, 2 when
rules.tsv is wrong (a malformed row, an `allow` that points up a layer, a
module with no layer).

## The layers

```
sdk, common  <  runtime  <  platform, assets, audio, media  <  renderer, renderer/vulkan, renderer/world
             <  scene  <  enhanced  <  game/common  <  game/<id>  <  viewer, extract, tools, android, tests
```

This is the diagram of section 18, with three readings it leaves to the
checker:

- `sdk` is the mod SDK's C ABI (`sdk/include/openrac/`, found through the
  build's include directories). It depends on nothing, so it sits at the
  bottom; the host program's mod code includes it.
- `renderer` has two submodules on its layer, `renderer/world` (the world
  renderers) and `renderer/vulkan` (the Vulkan backend). Both build on
  renderer's GL-shaped interface, and the backend also carries out
  `renderer/world/gl_world.h`; rules.tsv allows those edges and no others.
- Every `game/<id>` directory (`game/rac1`, `game/rac1-pal`, ...) is a module
  of its own on one layer. A game version includes `game/common` and below,
  not another version.

`enhanced/tests/` is read as `tests` (they are test programs);
`enhanced/tools/probes` (the probe baker) is read as `enhanced`, whose code it
drives. `viewer/` is a program's directory, so its includes of `enhanced/`
are allowed today; the library files P2-02 moves out of it to `scene/` bring
those includes with them, and that package lists them as exceptions until
P2-07 inverts the dependency.

## What it reads

Every `.c`, `.cc`, `.cpp`, `.h`, `.hpp`, `.inc`, `.inl` and `.mm` file under
`port/`, except:

- the directories rules.tsv skips: build trees (`build/`) and a game's hand C
  (`game/*/hand/`, the game's C on its way into `games/`, see
  [LAYOUT.md](../../../docs/LAYOUT.md)). A skipped file can still be
  included: `tests/*_test.c` including `../game/rac1-pal/hand/*.c` is a
  `tests -> game/rac1-pal` edge;
- hidden directories;
- generated files: "Do not edit" in the first two lines, the test the
  conventions check uses too (`game/rac1/script_binding.c`).

It reads the files on disk, not git's index, so it also checks what you have
not committed yet.

In each file it takes the quoted includes whose name has a directory,
`#include "<dir>/..."`, and resolves each as the compiler does: against the
including file's own directory first, then the `search` directories of
rules.tsv in order (the build's include directories: `port/`,
`runtime/include`, `game/common/include`, `sdk/include`). A header the tree
does not have (one the build generates) is read as a path from `port/`. The
file found is mapped to a module by the `module` rows; an include that maps
to none (a third-party header) is not an edge.

What it does not see:

- an include without a directory (`#include "settings.h"`): it is the
  including file's neighbour, or a header found through a target's own
  include directory, which only CMake knows;
- angle-bracket includes, which are system and third-party headers;
- the CMake link graph. Section 19 asks for `target_link_libraries` to match
  the layers; that is checked by review until a package teaches this tool to
  read CMake.

A line inside a comment or a disabled `#if` is read like any other: the
check is about the text of the includes.

## rules.tsv

One row per line, its columns separated by tabs; a line starting with `#` is
a comment. Paths are relative to `port/`.

| Row | Columns | Means |
|---|---|---|
| `search` | a directory | where a quoted include is looked for, after the including file's directory, in this order |
| `skip` | a path prefix | directories whose files are not read |
| `module` | a path prefix, a name | files under the prefix belong to that module. The first matching row wins, so a subdirectory comes before its parent. A `*` segment matches one directory and a `*` in the name takes its name: `game/*/` with `game/*` makes `game/rac1/` the module `game/rac1` |
| `layer` | a rank, names | the modules on one layer, separated by spaces; a lower rank is a lower layer. A name may be a `*` pattern (`game/*`); a row naming a module exactly wins over a pattern |
| `allow` | a module, names | the modules of its own layer it may include. An `allow` that points up a layer is refused: an upward include is an exception, never a rule |
| `except` | `file:line`, the include as written, a reason | a known violation, kept until the package the reason names removes it |

## Known violations

The `except` rows are the includes that break the layers today, each with the
restructure package that removes it
([PROGRESS.md](../../../docs/restructure/PROGRESS.md)):

| Edge | Includes | Removed by |
|---|---|---|
| platform -> renderer/vulkan | `platform/window.cpp` opens the Vulkan backend | no package yet |

The list only shrinks. A new include against the layers fails the check and
is fixed in the change that adds it: move the code to the right module, or
let the lower module own an interface the higher one registers with
(section 18). It is not added to the list.

When a package removes a violation, the check warns that the row no longer
occurs (`rules.tsv:N: layering (warning): ... remove the row`); remove the row
in the same commit. When an edit above a listed include moves it, the check
warns with its new line (`... is now at line N; update the row`); update the
line number. Neither warning fails the check: a change elsewhere in a file
does not break it, but the list is kept exact.

## A new module

A new module brings, in the commit that creates it (section 19): its `module`
row (before any row for a parent directory), its name in a `layer` row, and an
`allow` row for each module of its own layer it includes. A file no `module`
row covers fails the check, and a module with no layer stops it with exit 2,
so a new directory cannot slip past it.

## Tests

[test_layering.py](test_layering.py) runs the check on the real `port/`
(no violation today, every listed exception names a file that exists) and on
small made-up trees in a temporary directory: a lower layer passes and a
higher one fails, a same-layer edge needs its `allow` row, a `game/*` module
is one module per directory, a listed exception passes and warns once it is
removed or moved, skipped and generated files are not read, includes resolve
as the compiler resolves them, and malformed rules are refused.

## Changing it

A wrong finding is a bug in this tool: fix it here with a test, not by
bending the code or adding an exception. Keep it to the standard library,
and keep the format of rules.tsv described in its header and in the table
above.
