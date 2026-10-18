# Releasing the OpenRAC Launcher

This document describes the complete procedure for releasing a new version of the OpenRAC Launcher across all supported platforms (Windows, Linux x86_64, Linux ARM64, macOS Universal) and publishing the update to the GitHub Releases updater feed.

---

## 1. Pre-Release Verification

Before tagging a release, ensure all checks and unit tests pass locally:

```bash
# Launcher tests and lints
cd launcher
yarn install --immutable
yarn verify
cargo fmt --all --check
cargo test -p openrac-launcher-core
cd ..

# Repository tools and editor tests
python3 -m unittest discover -s tools
python3 -m unittest discover -s editor
```

Ensure your working directory is clean (`git status`).

---

## 2. Version Bump

Update the version number across the four configuration files:

1. **`launcher/package.json`**:
   ```json
   "version": "X.Y.Z"
   ```
2. **`launcher/src-tauri/tauri.conf.json`**:
   ```json
   "version": "X.Y.Z"
   ```
3. **`launcher/Cargo.toml`**:
   ```toml
   [workspace.package]
   version = "X.Y.Z"
   ```
4. **Update `launcher/Cargo.lock`**:
   ```bash
   cd launcher && cargo check && cd ..
   ```

Verify the diff with `git diff launcher/`.

---

## 3. Commit and Tag

Commit the version bump following Conventional Commits format and create the release tag:

```bash
git add launcher/package.json launcher/src-tauri/tauri.conf.json launcher/Cargo.toml launcher/Cargo.lock
git commit -m "chore(launcher): X.Y.Z"

# Tag must match the launcher-v* pattern
git tag launcher-vX.Y.Z
```

---

## 4. Push and CI Automation

Push the commit to `master` and the tag to GitHub:

```bash
git push origin master
git push origin launcher-vX.Y.Z
```

### What GitHub Actions does automatically (`.github/workflows/launcher.yml`):

1. **One `build` job per system** (a matrix row), each building the game and the copy of
   OpenRAC the installer carries straight into the launcher's resources, then the installer:
   - **Windows** (`windows-latest`): `openrac-rac1-pal.exe` with a standalone Python; NSIS
     (`.exe`) and MSI.
   - **Linux x86_64** (`ubuntu-22.04`): the game against LLVM 19's libc++ (glibc 2.35
     compatible); `.deb`, `.rpm` (zstd) and `.AppImage`.
   - **Linux ARM64** (`ubuntu-24.04-arm`): the same for `aarch64`.
   - **Fedora** (a `fedora:44` container): the game with Fedora's compiler; `.fc44.x86_64.rpm`.
   - **macOS** (`macos-latest`): the game for Apple silicon; a universal `.dmg` and `.app.tar.gz`.
   - **Android** (`ubuntu-latest`): the port with the runner's NDK (`tools/build_android.py
--no-apk`), then the debug APK, signed with the launcher's Android key (the
     `ANDROID_KEYSTORE`, `ANDROID_KEYSTORE_PASSWORD`, `ANDROID_KEY_ALIAS` and
     `ANDROID_KEY_PASSWORD` secrets; without them, the runner's debug key).

   Kept between runs: hostgen's output (skipped when its inputs are unchanged), ccache, the
   Rust dependencies (Swatinem/rust-cache) and Gradle's.
   Each row signs its update archives (and the APK) with minisign (`TAURI_SIGNING_PRIVATE_KEY`).

2. **Release job** (on a tag), after every row:
   - Runs `tools/updater_manifest.py` to produce `latest.json` covering `windows-x86_64`,
     `linux-x86_64`, `linux-aarch64`, `darwin-x86_64`, `darwin-aarch64` and `android-aarch64`
     (read by the Android launcher's own updater, `launcher/core/src/update.rs`).
   - Creates a draft release titled `OpenRAC Launcher X.Y.Z` with all installer assets and signatures.

Monitor the CI status:

```bash
gh run list --workflow=launcher.yml --limit 3
```

---

## 5. Publishing the Release on GitHub

Once all CI jobs complete:

1. **Inspect the draft release**:
   ```bash
   gh release view launcher-vX.Y.Z
   ```
2. **Publish the draft as Latest**:
   ```bash
   gh release edit launcher-vX.Y.Z --draft=false --latest
   ```
3. **Verify the live updater manifest**:
   ```bash
   curl -sSfL "https://github.com/OpenRAC/OpenRAC/releases/latest/download/latest.json"
   ```
   Check that:
   - `version` matches `X.Y.Z`.
   - Asset URLs contain dots instead of spaces (GitHub automatically converts spaces to dots in filenames: e.g. `OpenRAC.Launcher_X.Y.Z_amd64.AppImage`).

---

## 6. Common Pitfalls & Notes

- **Spaces in filenames**: GitHub Releases replaces spaces with dots when files are uploaded. `tools/updater_manifest.py` sanitizes URLs with `.replace(" ", ".")` to prevent 404 Not Found errors in the in-app updater.
- **The Android key**: Android installs an update only over an APK signed with the same key, so
  the keystore in `ANDROID_KEYSTORE` (base64 of a `.jks`, made once with
  `keytool -genkeypair -v -keystore openrac.jks -keyalg RSA -keysize 4096 -validity 10000 -alias openrac`)
  must never change: keep a copy offline. An APK built without it (a fork's run) installs, but
  does not update an installed launcher, nor update to one.
- **Minisign key formatting**: `TAURI_SIGNING_PRIVATE_KEY` secret must be trimmed of trailing newlines before passing to Tauri CLI. The CI workflow handles this automatically via a temporary file in `$RUNNER_TEMP/updater.key`.
