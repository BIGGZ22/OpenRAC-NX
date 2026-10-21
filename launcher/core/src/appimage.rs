//! The environment an AppImage hands to the launcher and to everything it starts.
//!
//! Every AppImage carries AppImageKit's `AppRun`, a small program that sets a
//! list of variables before the program inside it starts: `PYTHONHOME`,
//! `PYTHONPATH`, `PERLLIB`, `GST_PLUGIN_SYSTEM_PATH`, `QT_PLUGIN_PATH` and
//! `LD_LIBRARY_PATH`, all of them pointing into the AppImage, and `PATH` with
//! the AppImage's own directories in front of the system's.
//!
//! Only some of them mean anything here. This AppImage carries no Perl, no
//! GStreamer and no Qt at all, so those variables point at nothing: a Qt
//! program told to look for its platform plugin in ten directories that do not
//! exist says "Could not find the Qt platform plugin" and stops. (The Qt
//! programs a player sees that message from are their own system's, and the
//! AppImage's libraries would only make it worse.)
//!
//! linuxdeploy's GTK hook, which the AppRun runs first, adds GTK's own:
//! `GTK_PATH`, `GIO_MODULE_DIR`, `GSETTINGS_SCHEMA_DIR`, `XDG_DATA_DIRS` and
//! the others below, for the AppImage's GTK 3 and its Ubuntu 22.04 modules and
//! schemas, and `GTK_THEME=Adwaita:light` (or `:dark`) over whatever the
//! player had.
//!
//! So the launcher takes them back. What belongs to the AppImage stays in the
//! AppImage, and what the launcher starts is given the system's own
//! environment ([`sanitize`]).

use std::ffi::OsStr;
use std::path::{Path, PathBuf};
use std::process::Command;

/// The variables whose value is one path.
const ONE_PATH: [&str; 6] = [
    "PYTHONHOME",
    "GTK_DATA_PREFIX",
    "GTK_EXE_PREFIX",
    "GTK_IM_MODULE_FILE",
    "GDK_PIXBUF_MODULE_FILE",
    "GIO_MODULE_DIR",
];

/// The variables whose value is a list of paths, separated by ':'.
const PATHS: [&str; 11] = [
    "PATH",
    "LD_LIBRARY_PATH",
    "QT_PLUGIN_PATH",
    "PYTHONPATH",
    "PERLLIB",
    "GST_PLUGIN_SYSTEM_PATH",
    "GST_PLUGIN_SYSTEM_PATH_1_0",
    "GTK_PATH",
    "GI_TYPELIB_PATH",
    "GSETTINGS_SCHEMA_DIR",
    "XDG_DATA_DIRS",
];

/// The variables the launcher takes back for itself: all of them but
/// `LD_LIBRARY_PATH`, which the webview's own helper processes
/// (`WebKitWebProcess`, `WebKitNetworkProcess`) need, as they find the
/// AppImage's WebKit nowhere else.
const TAKEN_BACK: [&str; 6] =
    ["PYTHONHOME", "PYTHONPATH", "PERLLIB", "GST_PLUGIN_SYSTEM_PATH", "GST_PLUGIN_SYSTEM_PATH_1_0", "QT_PLUGIN_PATH"];

/// The folder the AppImage is mounted in, when the launcher runs from one.
pub fn appdir() -> Option<PathBuf> {
    let dir = std::env::var_os("APPDIR")?;
    (!dir.is_empty()).then(|| PathBuf::from(dir))
}

/// Whether the path `entry` is the AppImage's own.
fn inside(entry: &str, appdir: &Path) -> bool {
    Path::new(entry).starts_with(appdir)
}

/// What the variable `name` is worth to a program outside the AppImage: the
/// AppImage's own paths go, the system's stay. `None` when nothing of the
/// system's is left, and the variable goes with it.
fn outside(name: &str, value: &str, appdir: &Path) -> Option<String> {
    if PATHS.contains(&name) {
        let kept: Vec<&str> = value.split(':').filter(|entry| !entry.is_empty() && !inside(entry, appdir)).collect();
        (!kept.is_empty()).then(|| kept.join(":"))
    } else {
        (!inside(value, appdir)).then(|| value.to_string())
    }
}

/// Takes the AppImage's own variables out of this process's environment, so
/// that what the launcher starts does not get them.
pub fn clean_process_env() {
    let Some(appdir) = appdir() else { return };
    clean_with(&appdir);
}

fn clean_with(appdir: &Path) {
    for name in TAKEN_BACK {
        let Some(value) = std::env::var_os(name) else { continue };
        let value = value.to_string_lossy().into_owned();
        match outside(name, &value, appdir) {
            Some(kept) => std::env::set_var(name, kept),
            None => std::env::remove_var(name),
        }
    }
}

/// Gives a child the system's environment, not the AppImage's: the AppImage's
/// own directories go from the paths, and the variables that point at nothing
/// go entirely. The game, the tools and the Python they run find the system's
/// libraries, not the AppImage's.
pub fn sanitize(cmd: &mut Command) {
    let Some(appdir) = appdir() else { return };
    sanitize_with(cmd, &appdir);
}

fn sanitize_with(cmd: &mut Command, appdir: &Path) {
    // What the child would have: its own value where it has one, the
    // process's otherwise.
    let own: std::collections::HashMap<_, _> =
        cmd.get_envs().map(|(key, value)| (key.to_os_string(), value.map(|value| value.to_os_string()))).collect();
    for name in ONE_PATH.iter().chain(PATHS.iter()) {
        let value = match own.get(OsStr::new(name)) {
            Some(Some(value)) => value.clone(),
            Some(None) => continue,
            None => match std::env::var_os(name) {
                Some(value) => value,
                None => continue,
            },
        };
        let value = value.to_string_lossy().into_owned();
        match outside(name, &value, appdir) {
            Some(kept) => {
                cmd.env(name, kept);
            }
            None => {
                cmd.env_remove(name);
            }
        }
    }
    // The GTK hook's theme, set over the player's own: a GTK program the launcher starts (a file
    // manager, a browser) takes the player's theme again.
    let theme = match own.get(OsStr::new("GTK_THEME")) {
        Some(value) => value.clone(),
        None => std::env::var_os("GTK_THEME"),
    };
    if theme.is_some_and(|theme| theme.to_string_lossy().starts_with("Adwaita:")) {
        cmd.env_remove("GTK_THEME");
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::ffi::OsStr;

    fn mount() -> PathBuf {
        PathBuf::from("/tmp/.mount_openrac")
    }

    #[test]
    fn the_appimages_own_paths_go_and_the_systems_stay() {
        let appdir = mount();
        assert_eq!(
            outside("LD_LIBRARY_PATH", "/tmp/.mount_openrac/usr/lib/:/usr/lib", &appdir),
            Some("/usr/lib".to_string())
        );
        assert_eq!(outside("PATH", "/usr/bin:/bin", &appdir), Some("/usr/bin:/bin".to_string()));
    }

    #[test]
    fn a_variable_of_nothing_but_the_appimage_goes_entirely() {
        let appdir = mount();
        assert_eq!(outside("QT_PLUGIN_PATH", "/tmp/.mount_openrac/usr/lib/qt5/plugins/:", &appdir), None);
        assert_eq!(outside("PYTHONHOME", "/tmp/.mount_openrac/usr/", &appdir), None);
    }

    #[test]
    fn a_folder_that_only_looks_like_the_appimages_is_the_systems() {
        let appdir = mount();
        assert_eq!(
            outside("LD_LIBRARY_PATH", "/tmp/.mount_openrac2/usr/lib", &appdir),
            Some("/tmp/.mount_openrac2/usr/lib".to_string())
        );
    }

    #[test]
    fn a_child_gets_the_systems_libraries_and_not_the_appimages() {
        let appdir = mount();
        let mut cmd = Command::new("true");
        cmd.env("LD_LIBRARY_PATH", "/tmp/.mount_openrac/usr/lib/:/usr/lib");
        cmd.env("QT_PLUGIN_PATH", "/tmp/.mount_openrac/usr/lib/qt5/plugins/:");
        cmd.env("PYTHONHOME", "/tmp/.mount_openrac/usr/");
        sanitize_with(&mut cmd, &appdir);
        let envs: std::collections::HashMap<_, _> = cmd.get_envs().collect();
        assert_eq!(envs.get(OsStr::new("LD_LIBRARY_PATH")), Some(&Some(OsStr::new("/usr/lib"))));
        assert_eq!(envs.get(OsStr::new("QT_PLUGIN_PATH")), Some(&None));
        assert_eq!(envs.get(OsStr::new("PYTHONHOME")), Some(&None));
    }

    /// What linuxdeploy's GTK hook points into the AppImage goes as well, and its light theme.
    #[test]
    fn a_child_gets_the_systems_gtk_and_not_the_appimages() {
        let appdir = mount();
        let mut cmd = Command::new("true");
        cmd.env("GIO_MODULE_DIR", "/tmp/.mount_openrac//usr/lib/gio/modules");
        cmd.env("XDG_DATA_DIRS", "/tmp/.mount_openrac/usr/share/:/tmp/.mount_openrac/usr/share:/usr/share:");
        cmd.env("GSETTINGS_SCHEMA_DIR", "/tmp/.mount_openrac//usr/share/glib-2.0/schemas");
        cmd.env("GTK_THEME", "Adwaita:light");
        sanitize_with(&mut cmd, &appdir);
        let envs: std::collections::HashMap<_, _> = cmd.get_envs().collect();
        assert_eq!(envs.get(OsStr::new("GIO_MODULE_DIR")), Some(&None));
        assert_eq!(envs.get(OsStr::new("XDG_DATA_DIRS")), Some(&Some(OsStr::new("/usr/share"))));
        assert_eq!(envs.get(OsStr::new("GSETTINGS_SCHEMA_DIR")), Some(&None));
        assert_eq!(envs.get(OsStr::new("GTK_THEME")), Some(&None));
    }

    // By its full path: the child's PATH is the one being changed, and macOS has no /usr/bin/sh.
    #[cfg(unix)]
    #[test]
    fn a_child_really_runs_with_the_systems_own_environment() {
        let appdir = mount();
        let mut cmd = Command::new("/bin/sh");
        cmd.arg("-c").arg("echo \"[$LD_LIBRARY_PATH]\"; echo \"[$QT_PLUGIN_PATH]\"; echo \"[$PATH]\"");
        cmd.env("LD_LIBRARY_PATH", "/tmp/.mount_openrac/usr/lib/:/usr/lib");
        cmd.env("QT_PLUGIN_PATH", "/tmp/.mount_openrac/usr/lib/qt5/plugins/:");
        cmd.env("PATH", "/tmp/.mount_openrac/usr/bin/:/usr/bin");
        sanitize_with(&mut cmd, &appdir);
        let out = cmd.output().unwrap();
        let out = String::from_utf8_lossy(&out.stdout);
        let lines: Vec<&str> = out.lines().collect();
        assert_eq!(lines[0], "[/usr/lib]", "the AppImage's libraries are gone");
        assert_eq!(lines[1], "[]", "the variable that pointed at nothing is gone");
        assert_eq!(lines[2], "[/usr/bin]", "the system's own programs are found");
    }
}
