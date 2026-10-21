# Install from a link

Players add your mod by pasting your repository's address in the launcher's
**Mods** tab (**Add from a link**). This page is what your repository needs for
that, and what the launcher does.

## What players can paste

- `https://github.com/<owner>/<repo>` (with or without `.git` or a trailing `/`)
- `https://github.com/<owner>/<repo>/tree/<branch>`: that branch
- `https://github.com/<owner>/<repo>/releases/tag/<tag>`: that release

The repository must be **public**: the launcher reads it without an account.

## What your repository needs

- **The manifest**, where the launcher looks for it:
  - one mod: `openrac-mod.json` at the top;
  - several mods: one folder per mod at the top, each with its
    `openrac-mod.json` (as [rac1-mods](https://github.com/OpenRAC/rac1-mods));
  - or a top-level `openrac-mods.json`, `{"format": 1, "mods": ["a", "b"]}`,
    listing the folders to read (then only those).
- **For a native mod, a release** with `<id>-<version>-<platform>.zip` for
  each platform and `SHA256SUMS`: the templates' workflow makes them on a tag
  ([packaging and releases](packaging-and-releases.md)). Players never build
  anything.
- **For a script mod**, a release is optional: without one, the launcher takes
  the mod's folder from the repository itself, at its newest commit.

Both templates are laid out this way already.

## What the launcher does

1. Reads your latest release (or the one linked), the files of the repository
   at its tag, and each manifest.
2. Shows each mod before anything is installed: its name, version and author,
   the repository and its owner, the games and discs it is for, whether this
   OpenRAC can load it (and why not), and what it runs: **Runs scripts
   (sandboxed)** for a script mod, a warning that it runs code on this computer
   for a native mod.
3. On **Install**:
   - a native mod: downloads the archive for the player's platform, checks it
     against `SHA256SUMS`, and checks that the manifest inside is the one
     shown;
   - a script mod: the release's `<id>-<version>.zip` if there is one
     (checked the same way), else the repository's own files;
   - then installs it as an archive added by hand would be, switched on.
4. Remembers where it came from, for updates.

It refuses, with the reason: a mod with no archive for this platform, no
`SHA256SUMS` or a hash that differs, a manifest inside that is not the one
shown, a mod for another game.

## Updates and pinning

**Check for updates** (and once a day by itself) looks for a newer release
with an archive for the player's platform, or, for a script mod with no
releases, a newer commit. The mod's row then offers **Update**; an update keeps
its place in the list, its switch and its settings. A player can choose **Keep
this version** to never be offered one.

## Other git hosts

From other hosts the launcher installs mods without a build (script mods, data
packs) through `git`, when the player has it installed. Native mods need
releases, which it reads from GitHub today.
