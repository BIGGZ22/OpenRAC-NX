//! The webview's own environment, set before the window opens.
//!
//! The launcher's window is a WebKitGTK webview, and two of WebKitGTK's
//! settings draw nothing on some systems:
//!
//! - The DMA-BUF renderer draws a blank or flickering window under Wayland
//!   with NVIDIA's own driver. It is turned off there, and only there.
//! - The accelerated compositing mode draws nothing at all on a Steam Deck:
//!   the window opens empty, and the launcher has to be started with
//!   `WEBKIT_DISABLE_COMPOSITING_MODE=1` for anything to show. A Deck is
//!   told apart by its own file (`/etc/os-release` says `steamos`) and by
//!   gamescope, which says so in the environment of the programs it runs.

use std::path::Path;

/// The variables the webview gets, when the player has not set them.
pub fn webkit_env() -> Vec<(&'static str, &'static str)> {
    env_for(wayland(), nvidia(), steamos(), under_gamescope())
}

/// The variables the webview gets, given what the system is.
fn env_for(wayland: bool, nvidia: bool, steamos: bool, gamescope: bool) -> Vec<(&'static str, &'static str)> {
    let mut vars = Vec::new();
    if wayland && nvidia {
        vars.push(("WEBKIT_DISABLE_DMABUF_RENDERER", "1"));
    }
    if steamos || gamescope {
        vars.push(("WEBKIT_DISABLE_COMPOSITING_MODE", "1"));
    }
    vars
}

/// Whether the system is SteamOS: `/etc/os-release` says so.
fn steamos() -> bool {
    std::fs::read_to_string("/etc/os-release").map(|text| is_steamos(&text)).unwrap_or(false)
}

fn is_steamos(release: &str) -> bool {
    release.lines().any(|line| {
        let mut parts = line.splitn(2, '=');
        parts.next() == Some("ID") && parts.next().map(|value| value.trim().trim_matches('"')) == Some("steamos")
    })
}

/// Whether the programs run under gamescope, which says so in the environment.
fn under_gamescope() -> bool {
    std::env::var_os("GAMESCOPE_WAYLAND_DISPLAY").is_some()
}

fn wayland() -> bool {
    std::env::var_os("WAYLAND_DISPLAY").is_some()
}

fn nvidia() -> bool {
    Path::new("/proc/driver/nvidia/version").exists()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn the_decks_webview_draws_without_the_accelerated_compositing_mode() {
        let vars = env_for(true, false, true, false);
        assert_eq!(vars, vec![("WEBKIT_DISABLE_COMPOSITING_MODE", "1")]);
        let vars = env_for(false, false, false, true);
        assert_eq!(vars, vec![("WEBKIT_DISABLE_COMPOSITING_MODE", "1")]);
    }

    #[test]
    fn an_ordinary_desktop_gets_nothing_turned_off() {
        assert!(env_for(false, false, false, false).is_empty());
        assert!(env_for(true, false, false, false).is_empty());
    }

    #[test]
    fn the_dmabuf_renderer_goes_on_wayland_with_nvidia() {
        let vars = env_for(true, true, false, false);
        assert_eq!(vars, vec![("WEBKIT_DISABLE_DMABUF_RENDERER", "1")]);
        assert!(env_for(false, true, false, false).is_empty());
    }

    #[test]
    fn steamos_is_told_apart_by_its_own_release_file() {
        assert!(is_steamos("NAME=\"SteamOS\"\nID=steamos\nID_LIKE=arch\n"));
        assert!(is_steamos("ID=\"steamos\"\n"));
        assert!(!is_steamos("NAME=\"Fedora Linux\"\nID=fedora\nVERSION_ID=42\n"));
        assert!(!is_steamos("ID=steamosx\n"));
    }
}
