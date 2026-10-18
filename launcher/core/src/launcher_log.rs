//! The launcher's own log, `launcher.log` beside `launcher.json`: when it started, what it ran and
//! how each run ended, one line each with the time. It goes in a crash report's .zip
//! ([`crate::reports::bundle`]). Past [`LIMIT`] bytes it moves to `launcher.1.log` and starts again.

use std::io::Write;
use std::path::{Path, PathBuf};
use std::sync::Mutex;

/// A log bigger than this is moved aside before the next line.
pub const LIMIT: u64 = 1024 * 1024;

static FILE: Mutex<Option<PathBuf>> = Mutex::new(None);

/// Logs to `dir/launcher.log` from now on.
pub fn init(dir: &Path) {
    let _ = std::fs::create_dir_all(dir);
    *FILE.lock().unwrap() = Some(dir.join("launcher.log"));
}

/// The log's file, once [`init`] was called.
pub fn path() -> Option<PathBuf> {
    FILE.lock().unwrap().clone()
}

/// Appends `[time] line` (UTC), the home folder as `~`.
pub fn line(text: &str) {
    let Some(path) = path() else { return };
    append(&path, text, crate::discord::now_sec());
}

fn stamp(secs: u64) -> String {
    let days = (secs / 86400) as i64;
    let rest = secs % 86400;
    let z = days + 719468;
    let era = z.div_euclid(146097);
    let doe = z - era * 146097;
    let yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    let doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    let mp = (5 * doy + 2) / 153;
    let d = doy - (153 * mp + 2) / 5 + 1;
    let m = if mp < 10 { mp + 3 } else { mp - 9 };
    let y = yoe + era * 400 + i64::from(m <= 2);
    format!("{y:04}-{m:02}-{d:02} {:02}:{:02}:{:02} UTC", rest / 3600, rest / 60 % 60, rest % 60)
}

pub fn append(path: &Path, text: &str, secs: u64) {
    if std::fs::metadata(path).is_ok_and(|m| m.len() > LIMIT) {
        let _ = std::fs::rename(path, path.with_extension("1.log"));
    }
    let text = crate::reports::tidy(text, crate::reports::home().as_deref());
    if let Ok(mut f) = std::fs::OpenOptions::new().create(true).append(true).open(path) {
        let _ = writeln!(f, "[{}] {}", stamp(secs), text.trim_end());
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn appends_with_the_time_and_moves_a_big_log_aside() {
        let dir = tempfile::tempdir().unwrap();
        let path = dir.path().join("launcher.log");
        append(&path, "started 0.7.1", 1_792_238_602);
        append(&path, "the game stopped (code 3)", 1_792_238_603);
        let text = std::fs::read_to_string(&path).unwrap();
        assert_eq!(
            text,
            "[2026-10-17 12:03:22 UTC] started 0.7.1\n[2026-10-17 12:03:23 UTC] the game stopped (code 3)\n"
        );
        std::fs::write(&path, vec![b'x'; LIMIT as usize + 1]).unwrap();
        append(&path, "after", 1_792_238_604);
        assert!(dir.path().join("launcher.1.log").is_file());
        assert_eq!(std::fs::read_to_string(&path).unwrap(), "[2026-10-17 12:03:24 UTC] after\n");
    }
}
