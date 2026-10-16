# Releasing the OpenRAC Launcher

This document describes the complete procedure for releasing a new version of the OpenRAC Launcher across all supported platforms (Windows, Linux x86_64, Linux ARM64, macOS Universal) and publishing the update to the GitHub Releases updater feed and Flathub.

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
1. **Game payload build (`game` job)**:
   * **Windows** (`windows-latest`): Builds `openrac-rac1-pal.exe` and bundles standalone Python.
   * **Linux x86_64** (`ubuntu-22.04`): Builds the game against LLVM 19 libc++ (glibc 2.35 compatible).
   * **Linux ARM64** (`ubuntu-24.04-arm`): Builds natively for AArch64 and bundles `aarch64-unknown-linux-gnu` Python.
   * **macOS** (`macos-latest`): Builds the game for Apple Silicon.
2. **Installer creation (`build` & `fedora` jobs)**:
   * Packages Windows NSIS (`.exe`) and MSI.
   * Packages Linux `.deb`, `.rpm`, and `.AppImage` (both `amd64` and `arm64`).
   * Packages Fedora `.fc44.x86_64.rpm` in container.
   * Packages macOS universal `.dmg` and `.app.tar.gz`.
3. **Updater signing & manifest**:
   * Signs update archives with minisign (`TAURI_SIGNING_PRIVATE_KEY`).
   * Runs `tools/updater_manifest.py` to produce `latest.json` covering:
     * `windows-x86_64`
     * `linux-x86_64`
     * `linux-aarch64`
     * `darwin-x86_64`
     * `darwin-aarch64`
4. **Draft Release**:
   * Creates a draft release titled `OpenRAC Launcher X.Y.Z` with all installer assets and signatures.

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
   * `version` matches `X.Y.Z`.
   * Asset URLs contain dots instead of spaces (GitHub automatically converts spaces to dots in filenames: e.g. `OpenRAC.Launcher_X.Y.Z_amd64.AppImage`).

---

## 6. Updating the Flathub Package (`dev.openrac.launcher`)

Once the GitHub release is published, update the Flathub package:

### 1. Get the SHA-256 hashes of the new `.deb` packages
```bash
gh release view launcher-vX.Y.Z --json assets \
  --jq '.assets[] | select(.name | endswith(".deb")) | {name: .name, digest: .digest, url: .url}'
```

### 2. Update the Flathub manifest (`dev.openrac.launcher.yml`)
In the Flathub repository (`flathub/dev.openrac.launcher`):

* Update the `x86_64` `.deb` URL and `sha256`.
* Update the `aarch64` `.deb` URL and `sha256`.

Example snippet:
```yaml
      - type: file
        only-arches:
          - x86_64
        url: https://github.com/OpenRAC/OpenRAC/releases/download/launcher-vX.Y.Z/OpenRAC.Launcher_X.Y.Z_amd64.deb
        sha256: <NEW_AMD64_SHA256>
        dest-filename: openrac-launcher.deb

      - type: file
        only-arches:
          - aarch64
        url: https://github.com/OpenRAC/OpenRAC/releases/download/launcher-vX.Y.Z/OpenRAC.Launcher_X.Y.Z_arm64.deb
        sha256: <NEW_ARM64_SHA256>
        dest-filename: openrac-launcher.deb
```

### 3. Update AppStream metadata (`dev.openrac.launcher.metainfo.xml`)
Add the new release entry:
```xml
  <releases>
    <release version="X.Y.Z" date="YYYY-MM-DD">
      <description>
        <p>OpenRAC Launcher X.Y.Z release:</p>
        <ul>
          <li>Summary of main changes and fixes.</li>
        </ul>
      </description>
    </release>
    <!-- older releases -->
  </releases>
```

### 4. Lint and push to Flathub
```bash
flatpak run --command=flatpak-builder-lint org.flatpak.Builder manifest dev.openrac.launcher.yml

git add dev.openrac.launcher.yml dev.openrac.launcher.metainfo.xml
git commit -m "Update to X.Y.Z"
git push origin master
```

Flathub build servers automatically build and deploy the update to all Flathub and Steam Deck users within 1–2 hours.

---

## 7. Common Pitfalls & Notes

* **Spaces in filenames**: GitHub Releases replaces spaces with dots when files are uploaded. `tools/updater_manifest.py` sanitizes URLs with `.replace(" ", ".")` to prevent 404 Not Found errors in the in-app updater.
* **Minisign key formatting**: `TAURI_SIGNING_PRIVATE_KEY` secret must be trimmed of trailing newlines before passing to Tauri CLI. The CI workflow handles this automatically via a temporary file in `$RUNNER_TEMP/updater.key`.
* **Runtime EOL on Flathub**: Flathub regularly bumps GNOME/Freedesktop runtimes. Keep the manifest's `runtime-version` aligned with the active Flathub runtime (e.g. `51`).
