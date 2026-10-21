//! The window's size on a screen too small for it.
//!
//! The window opens at 1200 × 780 and cannot be made smaller than 960 × 560
//! (`tauri.conf.json`, docs/STYLE.md). A 1920 × 1080 screen at 150 % leaves
//! 1280 × 720 points and at 200 % 960 × 540: the window opened taller than the
//! screen, its bottom and its title bar's buttons out of reach, and at 200 %
//! it could not be made to fit. On such a screen it opens at what the screen
//! has room for, and its floor comes down to that as well.

/// Room kept around the window: its title bar, and some of a panel that not
/// every system leaves out of the monitor's work area (Wayland does not).
pub const MARGIN: (f64, f64) = (32.0, 48.0);

/// The size and the floor the window takes on a screen with room for `room`
/// (all in logical points), when `size` and `floor` do not fit it; `None` when
/// they do.
pub fn fit(size: (f64, f64), floor: (f64, f64), room: (f64, f64)) -> Option<((f64, f64), (f64, f64))> {
    let free = ((room.0 - MARGIN.0).max(320.0), (room.1 - MARGIN.1).max(240.0));
    if size.0 <= free.0 && size.1 <= free.1 {
        return None;
    }
    let size = (size.0.min(free.0), size.1.min(free.1));
    Some((size, (floor.0.min(size.0), floor.1.min(size.1))))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn a_screen_with_room_changes_nothing() {
        assert_eq!(fit((1200.0, 780.0), (960.0, 560.0), (1920.0, 1080.0)), None);
        assert_eq!(fit((1200.0, 780.0), (960.0, 560.0), (1536.0, 864.0)), None);
    }

    #[test]
    fn a_short_screen_gets_a_shorter_window() {
        // 1920 × 1080 at 150 %.
        assert_eq!(fit((1200.0, 780.0), (960.0, 560.0), (1280.0, 720.0)), Some(((1200.0, 672.0), (960.0, 560.0))));
        // At 200 %: the floor comes down too.
        assert_eq!(fit((1200.0, 780.0), (960.0, 560.0), (960.0, 540.0)), Some(((928.0, 492.0), (928.0, 492.0))));
    }
}
