# Packaging and releases

Players install a mod from a `.zip`, or from your repository's address, which
downloads the `.zip` from a release. The SDK makes the archives and the
templates' workflow publishes them, so releasing is pushing a tag.

## The archives

| Archive | What is in it | Made by |
|---|---|---|
| `<id>-<version>-<platform>.zip` | a native mod built for one platform (`windows-x86_64`, `linux-x86_64`, `macos-arm64`, ...) | `package_mod.py build/package/<target>` after a build |
| `<id>-<version>.zip` | a native mod for every platform built, joined; or a script mod (for all platforms) | `package_mod.py --merge ...`; `package_mod.py <folder>` for a script mod |
| `SHA256SUMS` | the SHA-256 of every archive, as `sha256sum` writes it | the workflow |

Inside, the manifest is at the top of the archive (or of its one top folder),
with a native mod's libraries under `lib/<platform>/`, plus `README*` and
`LICENSE*`.

```sh
python3 <SDK>/tools/package_mod.py build/package/my_mod -o dist              # dist/my-mod-0.1.0-macos-arm64.zip
python3 <SDK>/tools/package_mod.py --merge dist/my-mod-0.1.0-*.zip -o dist/my-mod-0.1.0.zip
python3 <SDK>/tools/package_mod.py path/to/script-mod -o dist                # dist/button-swap-1.0.0.zip
python3 <SDK>/tools/package_mod.py --check openrac-mod.json                  # the manifest only
```

`package_mod.py` refuses a manifest the launcher would refuse, so a mod that
packages also installs.

## The workflow

Both templates (`template/` and `template-script/`) carry
`.github/workflows/build.yml`. On every push it:

1. builds and tests native mods (if the repository has a top
   `CMakeLists.txt`) on Windows (MSVC), Linux (x86_64, aarch64) and macOS
   (arm64, x86_64), against the SDK `sdk.lock` pins;
2. packages each platform's build, and every script mod;
3. joins each native mod's platforms, and writes `SHA256SUMS`;
4. keeps them as the run's artifacts.

On a **tag** it also publishes them on that tag's GitHub release.

## Releasing

1. Set `version` in `openrac-mod.json` (and in your `OPENRAC_MOD_CPP` or
   `OPENRAC_MOD_DEFINE` line for a native mod: the game checks they agree).
2. Commit, then tag and push:

   ```sh
   git tag v1.2.0
   git push origin v1.2.0
   ```

   In a repository of several mods released apart, tag `<id>-v<version>`
   (`lock-strafe-v1.2.0`): that release carries only that mod's archives, and a
   `SHA256SUMS` listing them. `v<version>` releases every mod.
3. When the workflow finishes, the release has the archives and `SHA256SUMS`,
   and players' launchers offer the update.

The launcher installs only an archive that `SHA256SUMS` lists with the same
hash, so do not upload archives to a release by hand.

## The SDK your repository builds with

`sdk.lock` names the SDK release (version, address, SHA-256), and
`tools/fetch_sdk.py` downloads it into `.sdk/` and checks it. A new template's
lock has no hash yet: the first fetch takes it from the release and writes it
in; commit `sdk.lock` then. To move to a newer SDK, change its version, address
and hash (each SDK release's notes give the hash).
