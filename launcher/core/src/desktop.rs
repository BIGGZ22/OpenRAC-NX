//! The Linux desktop's menu entries and icons for the launcher and the games it plays: what the
//! menu lists and what the taskbar shows next to a window.
//!
//! Everything goes under the user's data folder (`~/.local/share`): icons in the `hicolor` theme
//! every desktop falls back to, `.desktop` entries in `applications/`. A file is written only when
//! its contents changed, and the desktop's caches are refreshed only after something was written,
//! so most starts do nothing here. The launcher writes these files at every start on Linux
//! (`src-tauri/src/lib.rs`, the setup step); nothing is ever removed.
//!
//! Each application gets two entries, one per id a desktop may look it up by: on Wayland the
//! window's app id is the program's name (`openrac-launcher`, `openrac-rac1-pal`), elsewhere the
//! window class names the bundle's identifier (`dev.openrac.launcher`, `dev.openrac.rac1-pal`).
//! Only one of the two shows in the menu (the other is `NoDisplay`), so nothing is listed twice.
//!
//! [`ensure_launcher`] and [`ensure_games`] read the environment (`$HOME`, `$APPIMAGE`, `$APPDIR`,
//! the program's own path) and run the cache tools; the work under them takes its folders as
//! arguments ([`Places`]), so the tests write into a temporary folder and run no program. It is
//! compiled and tested on every platform; only Linux calls it.

use std::path::{Path, PathBuf};

use crate::detect;

/// Where a desktop looks for applications' entries and icons.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Places {
    /// The icon theme every desktop falls back to: `<data>/icons/hicolor`.
    pub icons: PathBuf,
    /// The menu's entries: `<data>/applications`.
    pub applications: PathBuf,
}

impl Places {
    /// The folders under `home`'s `.local/share`. `$XDG_DATA_HOME` is not read: the launcher has
    /// always written to the default, which every desktop searches.
    pub fn in_home(home: &Path) -> Places {
        let data = home.join(".local/share");
        Places { icons: data.join("icons/hicolor"), applications: data.join("applications") }
    }

    /// The folders under `$HOME`, or None when it is not set (nothing is written then).
    pub fn from_env() -> Option<Places> {
        let home = std::env::var("HOME").ok()?;
        Some(Places::in_home(Path::new(&home)))
    }
}

/// What a pass wrote, which decides the caches [`refresh`] rebuilds.
#[derive(Debug, Clone, Copy, Default, PartialEq, Eq)]
pub struct Changed {
    /// An icon was written: the icon theme's cache is stale.
    pub icons: bool,
    /// A `.desktop` entry was written: the menu's database is stale.
    pub entries: bool,
}

/// One size of an application's icon: the theme's folder for it (`32x32`) and the PNG's bytes.
pub type Icon<'a> = (&'a str, &'a [u8]);

/// Writes `bytes` to `path` unless it already holds them; true when it wrote. A file that cannot
/// be written is skipped: the desktop then lacks an icon, the launcher still starts.
fn write_if_changed(path: &Path, bytes: &[u8]) -> bool {
    if std::fs::read(path).map(|old| old == bytes).unwrap_or(false) {
        return false;
    }
    std::fs::write(path, bytes).is_ok()
}

/// The `NoDisplay` line that keeps an entry out of the menu, or nothing.
fn no_display(hidden: bool) -> &'static str {
    if hidden {
        "NoDisplay=true\n"
    } else {
        ""
    }
}

/// Rebuilds the desktop's caches after `changed`: the icon theme's (GTK), the menu's database and
/// KDE's (`kbuildsycoca6`). A tool that is not installed is skipped; nothing to do when nothing
/// changed.
pub fn refresh(places: &Places, changed: Changed) {
    if changed.icons {
        let _ = detect::quiet(Path::new("gtk-update-icon-cache")).arg("-f").arg("-t").arg(&places.icons).status();
    }
    if changed.entries || changed.icons {
        let _ = detect::quiet(Path::new("update-desktop-database")).arg(&places.applications).status();
        let _ = detect::quiet(Path::new("kbuildsycoca6")).status();
    }
}

// ---- The launcher ---------------------------------------------------------------------------

/// The program the launcher's menu entry starts, and how it was installed.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct LauncherProgram {
    /// What the entry's `Exec` runs.
    pub path: PathBuf,
    /// A package (deb, rpm) installed it: the package lists the launcher in the menu itself, so
    /// this launcher's own entries only give the taskbar its icon and both stay hidden.
    pub packaged: bool,
}

impl LauncherProgram {
    /// The program from `appimage` (the AppImage's file, `$APPIMAGE`, used when it is a file) and
    /// `exe` (the running program). An AppImage runs from a mount that is gone once it exits
    /// (`/tmp/.mount_…`): the entry starts the AppImage file itself, else the menu would start
    /// nothing once the launcher was closed. Without one, a program under `/usr` is a package's.
    pub fn new(appimage: Option<PathBuf>, exe: PathBuf) -> LauncherProgram {
        let appimage = appimage.filter(|path| path.is_file());
        let packaged = appimage.is_none() && exe.starts_with("/usr");
        LauncherProgram { path: appimage.unwrap_or(exe), packaged }
    }

    /// The program this launcher runs as, from `$APPIMAGE` and its own path.
    pub fn from_env() -> LauncherProgram {
        let appimage = std::env::var_os("APPIMAGE").map(PathBuf::from);
        let exe = std::env::current_exe().unwrap_or_else(|_| PathBuf::from("openrac-launcher"));
        LauncherProgram::new(appimage, exe)
    }
}

/// The launcher's `.desktop` entry for the window class `wm_class`, starting `program`; `hidden`
/// keeps it out of the menu.
pub fn launcher_entry(program: &Path, wm_class: &str, hidden: bool) -> String {
    format!(
        "[Desktop Entry]\nType=Application\nName=OpenRAC Launcher\nComment=Set up, build and play the decompiled Ratchet & Clank games\nExec={}\nIcon=dev.openrac.launcher\nTerminal=false\nCategories=Game;Development;\nStartupNotify=true\nStartupWMClass={}\n{}",
        detect::desktop_exec(program),
        wm_class,
        no_display(hidden)
    )
}

/// Writes the launcher's icons (each of `icons` under both ids) and its two entries into
/// `places`: `dev.openrac.launcher` (in the menu, unless a package lists the launcher there) and
/// `openrac-launcher` (always hidden). What it wrote, for [`refresh`].
pub fn write_launcher(places: &Places, icons: &[Icon], program: &LauncherProgram) -> Changed {
    let _ = std::fs::create_dir_all(&places.applications);
    let mut changed = Changed::default();
    for (size, bytes) in icons {
        let dir = places.icons.join(size).join("apps");
        let _ = std::fs::create_dir_all(&dir);
        changed.icons |= write_if_changed(&dir.join("dev.openrac.launcher.png"), bytes);
        changed.icons |= write_if_changed(&dir.join("openrac-launcher.png"), bytes);
    }
    changed.entries |= write_if_changed(
        &places.applications.join("dev.openrac.launcher.desktop"),
        launcher_entry(&program.path, "dev.openrac.launcher", program.packaged).as_bytes(),
    );
    changed.entries |= write_if_changed(
        &places.applications.join("openrac-launcher.desktop"),
        launcher_entry(&program.path, "openrac-launcher", true).as_bytes(),
    );
    changed
}

/// The launcher's menu entries and icons for this user, kept current: written where they changed,
/// then the caches refreshed. `icons` are the bundle's own (`src-tauri/icons/`). Nothing happens
/// without `$HOME`.
pub fn ensure_launcher(icons: &[Icon]) {
    let Some(places) = Places::from_env() else { return };
    let changed = write_launcher(&places, icons, &LauncherProgram::from_env());
    refresh(&places, changed);
}

// ---- The games ------------------------------------------------------------------------------

/// The menu's name for the port's game `game_id` (a folder of `port/game/`).
pub fn game_title(game_id: &str) -> &'static str {
    match game_id {
        "rac1-pal" => "Ratchet & Clank (PAL)",
        "rac1-ntsc" => "Ratchet & Clank (NTSC)",
        "rac2-ntsc" => "Ratchet & Clank: Going Commando (NTSC)",
        "rac3-ntsc" => "Ratchet & Clank: Up Your Arsenal (NTSC)",
        "rac4-ntsc" => "Ratchet: Deadlocked (NTSC)",
        _ => "Ratchet & Clank",
    }
}

/// A game's `.desktop` entry: `title` in the menu, the icon `icon`, starting `program`, for the
/// window class `wm_class`; `hidden` keeps it out of the menu.
pub fn game_entry(title: &str, icon: &str, program: &Path, wm_class: &str, hidden: bool) -> String {
    format!(
        "[Desktop Entry]\nType=Application\nName={title}\nComment=Play {title} with OpenRAC\nExec={}\nIcon={icon}\nTerminal=false\nCategories=Game;\nStartupNotify=true\nStartupWMClass={wm_class}\n{}",
        detect::desktop_exec(program),
        no_display(hidden)
    )
}

/// Writes, into `places`, the icon and the two entries of every game of the checkout `root` that
/// has an icon (`port/game/<id>/icon.png`): `dev.openrac.<id>` in the menu and `openrac-<id>`
/// hidden, both starting the release build `port/build/release/openrac-<id>`, so the taskbar
/// shows the game's icon whether the launcher or the player started it. What it wrote, for
/// [`refresh`].
pub fn write_games(places: &Places, root: &Path) -> Changed {
    let _ = std::fs::create_dir_all(&places.applications);
    let mut changed = Changed::default();
    let Ok(entries) = std::fs::read_dir(root.join("port/game")) else { return changed };
    for entry in entries.flatten() {
        if !entry.path().is_dir() {
            continue;
        }
        let game_id = entry.file_name().to_string_lossy().to_string();
        let icon_path = entry.path().join("icon.png");
        if !icon_path.is_file() {
            continue;
        }
        let Ok(bytes) = std::fs::read(&icon_path) else { continue };

        let dir = places.icons.join("512x512/apps");
        let _ = std::fs::create_dir_all(&dir);
        let app_id = format!("dev.openrac.{game_id}");
        let prog_id = format!("openrac-{game_id}");
        changed.icons |= write_if_changed(&dir.join(format!("{app_id}.png")), &bytes);
        changed.icons |= write_if_changed(&dir.join(format!("{prog_id}.png")), &bytes);

        let program = root.join(format!("port/build/release/openrac-{game_id}"));
        let title = game_title(&game_id);
        changed.entries |= write_if_changed(
            &places.applications.join(format!("{app_id}.desktop")),
            game_entry(title, &app_id, &program, &app_id, false).as_bytes(),
        );
        changed.entries |= write_if_changed(
            &places.applications.join(format!("{prog_id}.desktop")),
            game_entry(title, &app_id, &program, &prog_id, true).as_bytes(),
        );
    }
    changed
}

/// The menu entries and icons of the checkout `root`'s games for this user, kept current (see
/// [`write_games`]). Not for the copy inside an AppImage: it is mounted afresh at each start, so
/// an entry would name a folder that is gone once the launcher exits. Nothing happens without
/// `$HOME`.
pub fn ensure_games(root: &Path) {
    if crate::appimage::appdir().is_some_and(|appdir| root.starts_with(appdir)) {
        return;
    }
    let Some(places) = Places::from_env() else { return };
    let changed = write_games(&places, root);
    refresh(&places, changed);
}

#[cfg(test)]
mod tests {
    use super::*;

    const ICONS: &[Icon] = &[("32x32", b"small"), ("512x512", b"large")];

    fn read(path: PathBuf) -> String {
        std::fs::read_to_string(&path).unwrap_or_else(|e| panic!("{}: {e}", path.display()))
    }

    #[test]
    fn the_folders_are_under_the_homes_local_share() {
        let places = Places::in_home(Path::new("/home/player"));
        assert_eq!(places.icons, Path::new("/home/player/.local/share/icons/hicolor"));
        assert_eq!(places.applications, Path::new("/home/player/.local/share/applications"));
    }

    #[test]
    fn the_launcher_gets_its_icons_and_two_entries() {
        let home = tempfile::tempdir().unwrap();
        let places = Places::in_home(home.path());
        let program = LauncherProgram { path: PathBuf::from("/opt/OpenRAC Launcher/run"), packaged: false };
        assert_eq!(write_launcher(&places, ICONS, &program), Changed { icons: true, entries: true });

        for (size, bytes) in ICONS {
            for name in ["dev.openrac.launcher.png", "openrac-launcher.png"] {
                let path = places.icons.join(size).join("apps").join(name);
                assert_eq!(std::fs::read(&path).unwrap(), *bytes, "{}", path.display());
            }
        }
        let shown = "[Desktop Entry]\nType=Application\nName=OpenRAC Launcher\n\
            Comment=Set up, build and play the decompiled Ratchet & Clank games\n\
            Exec=\"/opt/OpenRAC Launcher/run\"\nIcon=dev.openrac.launcher\nTerminal=false\n\
            Categories=Game;Development;\nStartupNotify=true\nStartupWMClass=dev.openrac.launcher\n";
        assert_eq!(read(places.applications.join("dev.openrac.launcher.desktop")), shown);
        let hidden = shown.replace("StartupWMClass=dev.openrac.launcher\n", "StartupWMClass=openrac-launcher\n")
            + "NoDisplay=true\n";
        assert_eq!(read(places.applications.join("openrac-launcher.desktop")), hidden);
    }

    #[test]
    fn nothing_is_written_again_when_nothing_changed() {
        let home = tempfile::tempdir().unwrap();
        let places = Places::in_home(home.path());
        let program = LauncherProgram { path: PathBuf::from("/opt/openrac/launcher"), packaged: false };
        write_launcher(&places, ICONS, &program);
        assert_eq!(write_launcher(&places, ICONS, &program), Changed::default());

        // A new icon is written, the entries stay.
        let newer: &[Icon] = &[("32x32", b"small, redrawn"), ("512x512", b"large")];
        assert_eq!(write_launcher(&places, newer, &program), Changed { icons: true, entries: false });
        // A program that moved changes the entries only.
        let moved = LauncherProgram { path: PathBuf::from("/opt/elsewhere/launcher"), packaged: false };
        assert_eq!(write_launcher(&places, newer, &moved), Changed { icons: false, entries: true });
        assert!(read(places.applications.join("openrac-launcher.desktop")).contains("Exec=\"/opt/elsewhere/launcher\""));
    }

    #[test]
    fn a_packaged_launcher_hides_both_entries() {
        let home = tempfile::tempdir().unwrap();
        let places = Places::in_home(home.path());
        let program = LauncherProgram { path: PathBuf::from("/usr/bin/openrac-launcher"), packaged: true };
        write_launcher(&places, ICONS, &program);
        for name in ["dev.openrac.launcher.desktop", "openrac-launcher.desktop"] {
            assert!(read(places.applications.join(name)).ends_with("\nNoDisplay=true\n"), "{name}");
        }
    }

    #[test]
    fn the_entry_starts_the_appimage_when_there_is_one() {
        let dir = tempfile::tempdir().unwrap();
        let appimage = dir.path().join("OpenRAC.AppImage");
        std::fs::write(&appimage, b"").unwrap();
        let mount = PathBuf::from("/tmp/.mount_OpenRA/usr/bin/openrac-launcher");
        assert_eq!(
            LauncherProgram::new(Some(appimage.clone()), mount.clone()),
            LauncherProgram { path: appimage, packaged: false }
        );
        // An $APPIMAGE that is not a file is not used: the program under /usr is a package's.
        let gone = dir.path().join("gone.AppImage");
        let packaged = PathBuf::from("/usr/bin/openrac-launcher");
        assert_eq!(
            LauncherProgram::new(Some(gone), packaged.clone()),
            LauncherProgram { path: packaged, packaged: true }
        );
        let built = PathBuf::from("/home/player/OpenRAC/launcher/target/release/openrac-launcher");
        assert_eq!(LauncherProgram::new(None, built.clone()), LauncherProgram { path: built, packaged: false });
    }

    #[test]
    fn each_game_with_an_icon_gets_its_icon_and_two_entries() {
        let root = tempfile::tempdir().unwrap();
        let game = root.path().join("port/game/rac1-pal");
        std::fs::create_dir_all(&game).unwrap();
        std::fs::write(game.join("icon.png"), b"rac1 icon").unwrap();
        // A game without an icon, and a file among the games, are passed over.
        std::fs::create_dir_all(root.path().join("port/game/common")).unwrap();
        std::fs::write(root.path().join("port/game/README.md"), b"").unwrap();

        let home = tempfile::tempdir().unwrap();
        let places = Places::in_home(home.path());
        assert_eq!(write_games(&places, root.path()), Changed { icons: true, entries: true });

        let apps = places.icons.join("512x512/apps");
        for name in ["dev.openrac.rac1-pal.png", "openrac-rac1-pal.png"] {
            assert_eq!(std::fs::read(apps.join(name)).unwrap(), b"rac1 icon", "{name}");
        }
        let mut written: Vec<_> = std::fs::read_dir(&places.applications)
            .unwrap()
            .map(|e| e.unwrap().file_name().to_string_lossy().to_string())
            .collect();
        written.sort();
        assert_eq!(written, ["dev.openrac.rac1-pal.desktop", "openrac-rac1-pal.desktop"]);

        let exec = detect::desktop_exec(&root.path().join("port/build/release/openrac-rac1-pal"));
        let shown = format!(
            "[Desktop Entry]\nType=Application\nName=Ratchet & Clank (PAL)\n\
             Comment=Play Ratchet & Clank (PAL) with OpenRAC\nExec={exec}\nIcon=dev.openrac.rac1-pal\n\
             Terminal=false\nCategories=Game;\nStartupNotify=true\nStartupWMClass=dev.openrac.rac1-pal\n"
        );
        assert_eq!(read(places.applications.join("dev.openrac.rac1-pal.desktop")), shown);
        let hidden = shown.replace("StartupWMClass=dev.openrac.rac1-pal\n", "StartupWMClass=openrac-rac1-pal\n")
            + "NoDisplay=true\n";
        assert_eq!(read(places.applications.join("openrac-rac1-pal.desktop")), hidden);

        assert_eq!(write_games(&places, root.path()), Changed::default());
    }

    #[test]
    fn a_checkout_without_games_writes_nothing_but_the_folder() {
        let root = tempfile::tempdir().unwrap();
        let home = tempfile::tempdir().unwrap();
        let places = Places::in_home(home.path());
        assert_eq!(write_games(&places, root.path()), Changed::default());
        assert!(places.applications.is_dir());
        assert!(!places.icons.exists());
    }

    #[test]
    fn every_game_has_a_title() {
        assert_eq!(game_title("rac2-ntsc"), "Ratchet & Clank: Going Commando (NTSC)");
        assert_eq!(game_title("rac4-ntsc"), "Ratchet: Deadlocked (NTSC)");
        assert_eq!(game_title("rac9-pal"), "Ratchet & Clank");
    }
}
