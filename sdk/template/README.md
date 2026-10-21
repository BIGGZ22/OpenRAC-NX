# My mod

A native mod for OpenRAC's port of Ratchet & Clank. This folder is a
repository template: copy it into a repository of its own (or a folder per mod
in a repository of several, as [rac1-mods](https://github.com/OpenRAC/rac1-mods)),
then set the id, name, author and version in `openrac-mod.json` and the target
in `CMakeLists.txt`.

```sh
python3 tools/fetch_sdk.py          # the SDK sdk.lock pins, into .sdk/ (the first time it fills in the hash: commit sdk.lock)
cmake -S . -B build -G Ninja
cmake --build build
python3 .sdk/openrac-mod-sdk-*/tools/package_mod.py build/package/my_mod    # dist/my-mod-0.1.0-<platform>.zip
```

**Releasing.** Push the repository to GitHub and a tag `v0.1.0` (the
manifest's version). The workflow in `.github/workflows/build.yml` builds the
mod on Windows, Linux and macOS, and publishes on that release
`my-mod-0.1.0-<platform>.zip` for each, `my-mod-0.1.0.zip` with all of them,
and `SHA256SUMS`. Players then paste the repository's address in the
OpenRAC launcher's Mods tab (**Add from a link**): it shows the mod, downloads
the archive for their computer, checks it against `SHA256SUMS`, and offers
updates when you release a newer version. Nobody needs a compiler but you.

The interface, the manifest and the conventions are in
[docs/port/MODS.md](https://github.com/OpenRAC/OpenRAC/blob/master/docs/port/MODS.md)
(`docs/MODS.md` in the SDK).
