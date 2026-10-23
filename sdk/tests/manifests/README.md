# The manifest test vectors

`cases.json` holds mod manifests and mod ids, each with the verdict, accepted or refused, that the
three checks of a mod's manifest give it today. The rules are those of
[MODS.md, section 3](../../../docs/port/MODS.md#3-the-manifest-openrac-modjson). Each program that
reads a manifest checks it itself, in its own language, so the rules are written three times; this
file is what holds the three to one another.

| Check | Code | Its tests of this file |
|---|---|---|
| `launcher` | [`launcher/core/src/plugins.rs`](../../../launcher/core/src/plugins.rs): serde reads the file into `Manifest`, then `Manifest::validate`; `valid_id` | `plugins::tests::shared_ids`, `plugins::tests::shared_manifests` |
| `sdk` | [`sdk/tools/package_mod.py`](../../tools/package_mod.py): `check_manifest` | `test_package_mod.SharedCases` |
| `port` | [`port/game/common/mod_manifest.cpp`](../../../port/game/common/mod_manifest.cpp): `parse_manifest`; `valid_id` | `test_shared_ids_keep_their_verdicts`, `test_shared_manifests_keep_their_verdicts` in [`port/tests/mod_manifest_test.cpp`](../../../port/tests/mod_manifest_test.cpp) |

Each test reports every case whose verdict differs, by name, with the verdict it expected.

## The file

- `validators`: the three checks, by the names the verdicts use.
- `ids`: `{"id", "valid"}`. The SDK has no id check of its own: its test puts the id into a good
  manifest and looks for an error about the id.
- `manifests`: `{"name", "valid", "why", "manifest"}`, in three groups: those all three accept,
  those all three refuse, and those they disagree on. `name` is unique and says what the case
  tries.
- `valid` is `true` or `false` when the three agree. Where they disagree it is an object with each
  one's verdict, `{"launcher": ..., "sdk": ..., "port": ...}`, and `why` says what makes them
  differ.

A verdict is only accepted or refused. Each check words its reasons its own way, and the file does
not hold them.

The SDK's check runs without the mod's folder (`root` is `None`), so the files the manifest names
are not looked for, and with the SDK's own hook tables
(`sdk/bindings/<game>.ids.lock`), so a hook no table has is refused by it alone. The launcher's
verdict is the file read as `read_manifest` reads it, then validated: a manifest serde cannot read,
a field of the wrong type for instance, is refused.

## Changing it

The verdicts are today's behaviour, found by running each check on each case. They were written
before the restructure as characterisation tests (CODING_CONVENTIONS.md, section 24.2): a refactor
of a check keeps every verdict.

- **A refactor**: no case changes. A test that fails names the rule that moved.
- **A rule changed on purpose**: change the check, then the cases whose verdict it changes, in the
  same commit, and say so in its message. When a change brings the three together, the case's
  `valid` becomes `true` or `false` and its `why` goes.
- **A new case**: add it to its group with the verdict you expect from each check, then run the
  three tests. Each one that fails names the case and the verdict it expected; correct the file to
  what the code does, and record a disagreement rather than change a check to make one go.

```sh
python3 -m unittest discover -s sdk/tools -k SharedCases
cd launcher && cargo test -p openrac-launcher-core plugins::tests::shared
cd port && cmake --build --preset release --target mod_manifest_test && ctest --preset release -R mod_manifest --output-on-failure
```

Each test finds the file from its own place in the repository, so no build needs a setting for it:
the launcher with `include_str!`, the SDK's test by its path, the port's from `__FILE__`. The SDK's
archive carries this folder under `tests/manifests/`, as it carries the rest of `tests/`; the tests
that read it stay in the repository.

## Where the checks disagree today

Of the 120 manifests, 43 get different verdicts, and one of the 16 ids. Whether to bring the checks
together, and to which rule, is for the maintainers to decide; the file records the disagreements
without settling them. In outline:

- **The SDK is the strictest about form.** Only it checks game and disc names, a hook's spelling
  and that the hook exists in its tables, a float's `step`, distinct choice values, the keys of
  `load` and the ids in it, an interface major it knows, and a mod that requires or conflicts with
  itself.
- **The launcher checks what it needs to list and switch mods on.** It takes leading zeros in
  versions and in `api`, does not check the ids in `requires` and `conflicts`, reads an empty
  `plugin` as none, and checks `api` only beside a library.
- **The port reads every number as a double** (an int default of `1.0` passes, as does `format`
  written `2.0`), refuses `..` anywhere in a library's path and a `.` part in a script's, and refuses
  limits on a setting of a type they do not belong to.
- **Python's regular expressions** let a final newline through (`$`) and take any Unicode digit
  (`\d`) in a version.
