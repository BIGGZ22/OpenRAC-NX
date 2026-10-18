//! A game's logs and crash reports, for players to send when the game stops or misbehaves.
//!
//! The native game writes them in its data folder (`<install>/active/<game>/data`, the port's
//! `--data`): `openrac-<id>.log` is the latest session, `openrac-<id>.1.log` ... `.5.log` the
//! ones before it, and `crashes/crash-<date>_<time>.txt` a report for each time it stopped on a
//! fault (port/common/log.h, port/common/crash.h). This module lists them, writes the short text
//! a player pastes in an issue ([`copy_text`]) and the .zip they attach ([`bundle`]), with the
//! player's home folder as `~` in both.

use std::io::{Read, Seek, SeekFrom, Write};
use std::path::{Path, PathBuf};
use std::time::UNIX_EPOCH;

use serde::Serialize;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
#[serde(rename_all = "camelCase")]
pub enum ReportKind {
    Crash,
    Session,
}

/// One file of the list.
#[derive(Debug, Clone, PartialEq, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct ReportEntry {
    pub kind: ReportKind,
    /// Its path in the data folder, `/`-separated: `crashes/crash-...txt`, `openrac-rac1-pal.log`.
    pub file: String,
    /// The program that wrote it: `openrac-rac1-pal`.
    pub program: Option<String>,
    /// When it happened (a crash) or began (a session), seconds since 1970; else the file's time.
    pub time: u64,
    /// How long the session ran, in seconds, when its log says when it began.
    pub duration: Option<u64>,
    /// A crash's reason, in the port's words (shown under "Details").
    pub reason: Option<String>,
    /// The level being played: the crash's, or the last one a session's log names.
    pub level: Option<u32>,
    /// That level's planet, for the games whose planets the launcher knows.
    pub planet: Option<String>,
    /// A session that ended in a crash.
    pub crashed: bool,
    /// The session's id (when it began, as its log's header and its crash reports write it).
    pub session: Option<String>,
    pub size: u64,
}

// ---- Paths and privacy ----------------------------------------------------------------

/// The player's home folder, as `~` replaces it.
pub fn home() -> Option<String> {
    let var = if cfg!(windows) { "USERPROFILE" } else { "HOME" };
    std::env::var(var).ok().map(|h| h.trim_end_matches(['/', '\\']).to_string()).filter(|h| h.len() > 1)
}

/// `text` with `home` as `~` wherever it is a folder of its own (not `/Users/ann` in `/Users/anna`).
pub fn tidy(text: &str, home: Option<&str>) -> String {
    let Some(home) = home.filter(|h| !h.is_empty()) else { return text.to_string() };
    let mut out = String::with_capacity(text.len());
    let mut rest = text;
    while let Some(at) = rest.find(home) {
        let after = &rest[at + home.len()..];
        let ends = after
            .chars()
            .next()
            .is_none_or(|c| matches!(c, '/' | '\\' | ' ' | '"' | '\'' | ':' | ',' | ')' | '\n' | '\r'));
        out.push_str(&rest[..at]);
        out.push_str(if ends { "~" } else { home });
        rest = after;
    }
    out.push_str(rest);
    out
}

/// The file of the list named `file` in `data`, refused unless it is one this module lists
/// (a session log in the folder itself, or a report in `crashes/`).
pub fn resolve(data: &Path, file: &str) -> Result<PathBuf, String> {
    let ok_log = |name: &str| name.starts_with("openrac-") && name.ends_with(".log") && !name.contains(['/', '\\']);
    let ok_crash = |name: &str| name.starts_with("crash-") && name.ends_with(".txt") && !name.contains(['/', '\\']);
    let path = match file.split_once('/') {
        Some(("crashes", name)) if ok_crash(name) => data.join("crashes").join(name),
        None if ok_log(file) => data.join(file),
        _ => return Err(format!("{file} is not a log or a crash report")),
    };
    if path.is_file() {
        Ok(path)
    } else {
        Err(format!("{file} is not there any more"))
    }
}

/// Where `file` is, if it has the shape of a file this module may delete for GAME: a session log
/// `openrac-<game>*.log` in the data folder itself, or `crashes/crash-*.txt` (a report of any
/// program: the folder is the game's). The folder and the name; nothing is read from disk.
fn shape<'a>(data: &Path, game: &str, file: &'a str) -> Option<(PathBuf, &'a str)> {
    let stem = format!("openrac-{game}");
    let plain = |name: &str| !name.contains(['/', '\\']) && name != "." && name != "..";
    match file.split_once('/') {
        Some(("crashes", name)) if plain(name) && name.starts_with("crash-") && name.ends_with(".txt") => {
            Some((data.join("crashes"), name))
        }
        None if plain(file)
            && file.strip_prefix(&stem).is_some_and(|rest| rest.starts_with(['-', '.']))
            && file.ends_with(".log") =>
        {
            Some((data.to_path_buf(), file))
        }
        _ => None,
    }
}

/// Deletes the log or crash report `file` of GAME from `data`, and nothing else: only a plain file
/// (never a link, which could point anywhere) directly in the data folder or its `crashes/` folder,
/// itself not a link out of it. One that is already gone is not an error: the list the player
/// saw was old.
pub fn delete(data: &Path, game: &str, file: &str) -> Result<(), String> {
    let refuse = || format!("{file} is not a log or a crash report of this game");
    let (dir, name) = shape(data, game, file).ok_or_else(refuse)?;
    let path = dir.join(name);
    let inside = match (dir.canonicalize(), data.canonicalize()) {
        (Ok(d), Ok(root)) => d == root || d.parent() == Some(root.as_path()),
        (Err(e), _) if e.kind() == std::io::ErrorKind::NotFound => return Ok(()),
        _ => false,
    };
    if !inside {
        return Err(refuse());
    }
    match std::fs::symlink_metadata(&path) {
        Err(e) if e.kind() == std::io::ErrorKind::NotFound => Ok(()),
        Ok(m) if m.is_file() => std::fs::remove_file(&path).map_err(|e| format!("{file} could not be deleted: {e}")),
        _ => Err(refuse()),
    }
}

/// Deletes every log and crash report of GAME in `data` (what [`list`] shows, no more); returns
/// how many files went.
pub fn delete_all(data: &Path, game: &str) -> Result<usize, String> {
    let mut count = 0;
    for e in list(data) {
        // A listed file of another program's name is left alone: not this game's to delete.
        if shape(data, game, &e.file).is_some() {
            delete(data, game, &e.file)?;
            count += 1;
        }
    }
    Ok(count)
}

// ---- Times ----------------------------------------------------------------------------

fn days_from_civil(y: i64, m: i64, d: i64) -> i64 {
    let y = if m <= 2 { y - 1 } else { y };
    let era = if y >= 0 { y } else { y - 399 } / 400;
    let yoe = y - era * 400;
    let mp = (m + 9) % 12;
    let doy = (153 * mp + 2) / 5 + d - 1;
    let doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    era * 146097 + doe - 719468
}

/// "2026-10-17 14:03:22 +0200" (the port's way of writing a time) as seconds since 1970.
pub fn parse_time(text: &str) -> Option<u64> {
    let text = text.trim();
    let num = |a: usize, b: usize| text.get(a..b)?.parse::<i64>().ok();
    let (y, mo, d, h, mi, s) = (num(0, 4)?, num(5, 7)?, num(8, 10)?, num(11, 13)?, num(14, 16)?, num(17, 19)?);
    let mut secs = days_from_civil(y, mo, d) * 86400 + h * 3600 + mi * 60 + s;
    if let (Some(sign), Some(oh), Some(om)) = (text.get(20..21), num(21, 23), num(23, 25)) {
        let offset = oh * 3600 + om * 60;
        secs -= if sign == "-" { -offset } else { offset };
    }
    u64::try_from(secs).ok()
}

fn modified(path: &Path) -> u64 {
    std::fs::metadata(path)
        .and_then(|m| m.modified())
        .ok()
        .and_then(|t| t.duration_since(UNIX_EPOCH).ok())
        .map_or(0, |d| d.as_secs())
}

// ---- Reading the files ----------------------------------------------------------------

/// The first `n` bytes of a file, as text.
fn head(path: &Path, n: u64) -> String {
    let mut bytes = Vec::new();
    if let Ok(f) = std::fs::File::open(path) {
        let _ = f.take(n).read_to_end(&mut bytes);
    }
    String::from_utf8_lossy(&bytes).into_owned()
}

/// The last `n` bytes of a file, as text, from a line's start.
fn tail(path: &Path, n: u64) -> String {
    let mut bytes = Vec::new();
    if let Ok(mut f) = std::fs::File::open(path) {
        let len = f.metadata().map_or(0, |m| m.len());
        let from = len.saturating_sub(n);
        if f.seek(SeekFrom::Start(from)).is_ok() {
            let _ = f.read_to_end(&mut bytes);
        }
        if from > 0 {
            if let Some(nl) = bytes.iter().position(|&b| b == b'\n') {
                bytes.drain(..=nl);
            }
        }
    }
    String::from_utf8_lossy(&bytes).into_owned()
}

/// The value of a `key   value` line of a session header, or a `Key:     value` line of a report.
fn field<'a>(text: &'a str, key: &str) -> Option<&'a str> {
    text.lines().find_map(|l| l.strip_prefix(key)).map(str::trim)
}

fn game_of(program: &str) -> Option<&str> {
    program.strip_prefix("openrac-").map(|rest| rest.split('-').next().unwrap_or(rest))
}

/// A level's planet, for the games whose planets the launcher knows (rac1).
pub fn planet(program: Option<&str>, level: u32) -> Option<String> {
    match program.and_then(game_of) {
        Some("rac1") => crate::saves::RAC1_PLANETS.get(level as usize).map(|p| p.to_string()),
        _ => None,
    }
}

/// "level 3, game mode 0, frame 3000, ..." -> 3.
fn level_in(game_line: &str) -> Option<u32> {
    game_line.strip_prefix("level ")?.split([',', ' ']).next()?.parse().ok()
}

/// What a crash report says.
struct Crash {
    reason: Option<String>,
    when: Option<u64>,
    session: Option<String>,
    program: Option<String>,
    level: Option<u32>,
}

fn read_crash(path: &Path) -> Crash {
    let text = head(path, 16 * 1024);
    let when_line = field(&text, "When:").unwrap_or_default();
    let session = when_line.split_once("(the session began ").map(|(_, s)| s.trim_end_matches(')').to_string());
    let program = field(&text, "Program:").and_then(|p| p.split(',').next()).map(str::to_string);
    Crash {
        reason: field(&text, "Reason:").map(str::to_string),
        when: parse_time(when_line),
        session,
        program,
        level: field(&text, "Game:").and_then(level_in),
    }
}

/// What a session log says: its program and start (header), the last level and whether it crashed (tail).
struct Session {
    program: Option<String>,
    session: Option<String>,
    level: Option<u32>,
    crashed: bool,
}

fn read_session(path: &Path) -> Session {
    let top = head(path, 8 * 1024);
    let session = field(&top, "session ").map(str::to_string);
    let program = field(&top, "game ").and_then(|g| g.split(':').next()).map(|id| format!("openrac-{}", id.trim()));
    let end = tail(path, 256 * 1024);
    let level =
        end.lines().rev().find_map(|l| l.split("now playing level ").nth(1).and_then(|n| n.trim().parse().ok()));
    Session { program, session, level, crashed: end.contains("==== the program stopped") }
}

/// The program a session log is named after: `openrac-rac1-pal.2.log` -> `openrac-rac1-pal`.
fn program_of_log(name: &str) -> Option<String> {
    let stem = name.strip_suffix(".log")?;
    let stem = match stem.rsplit_once('.') {
        Some((base, n)) if n.chars().all(|c| c.is_ascii_digit()) => base,
        _ => stem,
    };
    Some(stem.to_string())
}

/// Every session log and crash report in `data`, the newest first.
pub fn list(data: &Path) -> Vec<ReportEntry> {
    let mut out = Vec::new();
    if let Ok(dir) = std::fs::read_dir(data) {
        for e in dir.flatten() {
            let name = e.file_name().to_string_lossy().into_owned();
            if !(name.starts_with("openrac-") && name.ends_with(".log")) {
                continue;
            }
            let path = e.path();
            let s = read_session(&path);
            let ended = modified(&path);
            let started = s.session.as_deref().and_then(parse_time);
            let program = s.program.or_else(|| program_of_log(&name));
            out.push(ReportEntry {
                kind: ReportKind::Session,
                file: name,
                planet: s.level.and_then(|l| planet(program.as_deref(), l)),
                program,
                time: started.unwrap_or(ended),
                duration: started.map(|t| ended.saturating_sub(t)),
                reason: None,
                level: s.level,
                crashed: s.crashed,
                session: s.session,
                size: e.metadata().map_or(0, |m| m.len()),
            });
        }
    }
    if let Ok(dir) = std::fs::read_dir(data.join("crashes")) {
        for e in dir.flatten() {
            let name = e.file_name().to_string_lossy().into_owned();
            if !(name.starts_with("crash-") && name.ends_with(".txt")) {
                continue;
            }
            let path = e.path();
            let c = read_crash(&path);
            out.push(ReportEntry {
                kind: ReportKind::Crash,
                file: format!("crashes/{name}"),
                planet: c.level.and_then(|l| planet(c.program.as_deref(), l)),
                program: c.program,
                time: c.when.unwrap_or_else(|| modified(&path)),
                duration: None,
                reason: c.reason,
                level: c.level,
                crashed: true,
                session: c.session,
                size: e.metadata().map_or(0, |m| m.len()),
            });
        }
    }
    // The newest first; a crash before the session it ended (same time: the crash is the news).
    out.sort_by(|a, b| b.time.cmp(&a.time).then((a.kind == ReportKind::Session).cmp(&(b.kind == ReportKind::Session))));
    out
}

/// The newest crash report written at or after `since` (seconds since 1970), if any.
pub fn latest_crash(data: &Path, since: u64) -> Option<ReportEntry> {
    list(data).into_iter().find(|e| e.kind == ReportKind::Crash && e.time >= since)
}

/// The session log that a crash report belongs to, and the crash report of a session log.
fn partner(data: &Path, entry: &ReportEntry) -> Option<PathBuf> {
    let session = entry.session.as_deref()?;
    let want = match entry.kind {
        ReportKind::Crash => ReportKind::Session,
        ReportKind::Session => ReportKind::Crash,
    };
    list(data)
        .into_iter()
        .find(|e| e.kind == want && e.session.as_deref() == Some(session))
        .and_then(|e| resolve(data, &e.file).ok())
}

fn entry_for(data: &Path, file: &str) -> Result<ReportEntry, String> {
    resolve(data, file)?;
    list(data).into_iter().find(|e| e.file == file).ok_or_else(|| format!("{file} is not there any more"))
}

// ---- The text a player pastes ---------------------------------------------------------

/// The short text for an issue or a Discord message: what happened, where, the versions and the
/// top of the backtrace. The full files go in the .zip ([`bundle`]).
pub fn copy_text(data: &Path, file: &str, launcher_version: &str) -> Result<String, String> {
    let entry = entry_for(data, file)?;
    let path = resolve(data, file)?;
    let mut out = String::new();
    let place = |e: &ReportEntry| match (e.level, &e.planet) {
        (Some(l), Some(p)) => format!("{p} (level {l})"),
        (Some(l), None) => format!("level {l}"),
        _ => "the title or the menus".to_string(),
    };
    // The session's header: a crash report's own copy has the graphics and settings lines, which
    // the log writes later in its own words.
    let session_text = match entry.kind {
        ReportKind::Crash => {
            let own = tail(&path, 16 * 1024);
            match own.split_once("\nThe session:\n") {
                Some((_, header)) => header.to_string(),
                None => partner(data, &entry).map(|p| head(&p, 8 * 1024)).unwrap_or_default(),
            }
        }
        ReportKind::Session => head(&path, 8 * 1024),
    };
    match entry.kind {
        ReportKind::Crash => {
            let text = head(&path, 64 * 1024);
            out.push_str("**OpenRAC: the game stopped unexpectedly**\n\n");
            out.push_str(&format!("- What happened: {}\n", entry.reason.as_deref().unwrap_or("unknown")));
            out.push_str(&format!("- Where: {}\n", place(&entry)));
            if let Some(game) = field(&text, "Game:") {
                out.push_str(&format!("- Game state: {game}\n"));
            }
            if let Some(when) = field(&text, "When:") {
                out.push_str(&format!("- When: {}\n", when.split(" (").next().unwrap_or(when)));
            }
            if let Some(program) = field(&text, "Program:") {
                out.push_str(&format!("- Program: {program}\n"));
            }
            push_versions(&mut out, &session_text, launcher_version);
            // The top of the backtrace, without the report's own frames.
            let frames: Vec<&str> = text
                .lines()
                .skip_while(|l| !l.starts_with("Backtrace"))
                .skip(1)
                .take_while(|l| !l.trim().is_empty() && !l.starts_with('('))
                .filter(|l| !is_handler_frame(l))
                .take(12)
                .collect();
            if !frames.is_empty() {
                out.push_str("\n```\n");
                for f in frames {
                    out.push_str(&squeeze(f));
                    out.push('\n');
                }
                out.push_str("```\n");
            }
        }
        ReportKind::Session => {
            out.push_str("**OpenRAC: a game session's log**\n\n");
            out.push_str(&format!("- Session: {}\n", entry.session.as_deref().unwrap_or("unknown")));
            out.push_str(&format!("- Last place: {}\n", place(&entry)));
            out.push_str(&format!("- Ended in a crash: {}\n", if entry.crashed { "yes" } else { "no" }));
            push_versions(&mut out, &session_text, launcher_version);
            let lines: Vec<String> = tail(&path, 64 * 1024)
                .lines()
                .filter(|l| l.contains("[error]") || l.contains("[warning]"))
                .map(str::to_string)
                .collect();
            if !lines.is_empty() {
                out.push_str("\nThe last warnings and errors:\n```\n");
                for l in &lines[lines.len().saturating_sub(10)..] {
                    out.push_str(l);
                    out.push('\n');
                }
                out.push_str("```\n");
            }
        }
    }
    out.push_str("\n(The full report and log: \"Save report\" in the launcher makes a .zip to attach.)\n");
    Ok(tidy(&out, home().as_deref()))
}

fn push_versions(out: &mut String, session: &str, launcher_version: &str) {
    if let Some(v) = field(session, "openrac ") {
        out.push_str(&format!("- OpenRAC: {v}; launcher {launcher_version}\n"));
    } else {
        out.push_str(&format!("- Launcher: {launcher_version}\n"));
    }
    for (key, label) in [("game ", "Game"), ("system ", "System"), ("graphics ", "Graphics")] {
        if let Some(v) = field(session, key) {
            out.push_str(&format!("- {label}: {v}\n"));
        }
    }
}

/// The crash report's own frames (the handler and the report writer): not news.
fn is_handler_frame(line: &str) -> bool {
    [
        "openrac5crash",
        "_sigtramp",
        "on_signal",
        "on_fault",
        "on_exception",
        "on_terminate",
        "7runtime12_GLOBAL__N_16report",
        "KiUserExceptionDispatcher",
        "RtlDispatchException",
    ]
    .iter()
    .any(|k| line.contains(k))
}

/// A backtrace line with its runs of spaces made one.
fn squeeze(line: &str) -> String {
    line.split_whitespace().collect::<Vec<_>>().join(" ")
}

// ---- The .zip a player attaches -------------------------------------------------------

/// A session log bigger than this goes in as its start and its end.
const LOG_LIMIT: usize = 8 * 1024 * 1024;

fn shortened(text: String) -> String {
    if text.len() <= LOG_LIMIT {
        return text;
    }
    let cut = |at: usize| (at..text.len()).find(|&i| text.is_char_boundary(i)).unwrap_or(text.len());
    let start = cut(64 * 1024);
    let end = cut(text.len() - LOG_LIMIT);
    format!("{}\n[... {} bytes left out ...]\n{}", &text[..start], end - start, &text[end..])
}

/// Writes `out` (a .zip): the report or log `file`, its partner (a crash's session log, a
/// session's crash report), the launcher's log when there is one, and `summary.txt`
/// ([`copy_text`]); every text with the home folder as `~`. Returns the names inside.
pub fn bundle(
    data: &Path,
    file: &str,
    launcher_log: Option<&Path>,
    launcher_version: &str,
    out: &Path,
) -> Result<Vec<String>, String> {
    let entry = entry_for(data, file)?;
    let home = home();
    let mut files: Vec<(String, Vec<u8>)> = Vec::new();
    files.push(("summary.txt".into(), copy_text(data, file, launcher_version)?.into_bytes()));
    let mut add = |name: String, path: &Path| {
        if let Ok(bytes) = std::fs::read(path) {
            let text = shortened(String::from_utf8_lossy(&bytes).into_owned());
            files.push((name, tidy(&text, home.as_deref()).into_bytes()));
        }
    };
    let main = resolve(data, file)?;
    add(main.file_name().unwrap_or_default().to_string_lossy().into_owned(), &main);
    if let Some(p) = partner(data, &entry) {
        add(p.file_name().unwrap_or_default().to_string_lossy().into_owned(), &p);
    } else if entry.kind == ReportKind::Crash {
        // A report from before sessions had ids: the latest log of its program.
        if let Some(program) = &entry.program {
            let log = data.join(format!("{program}.log"));
            if log.is_file() {
                add(format!("{program}.log"), &log);
            }
        }
    }
    if let Some(l) = launcher_log.filter(|l| l.is_file()) {
        add("launcher.log".into(), l);
    }
    let names = files.iter().map(|(n, _)| n.clone()).collect();
    write_zip(out, &files)?;
    Ok(names)
}

/// Where Save report puts the .zip: the player's Downloads folder, or their home.
pub fn save_folder() -> Option<PathBuf> {
    let home = PathBuf::from(home()?);
    let downloads = home.join("Downloads");
    Some(if downloads.is_dir() { downloads } else { home })
}

/// "OpenRAC-crash-2026-10-17_20-08-29.zip" for `crashes/crash-2026-10-17_20-08-29.txt`.
pub fn bundle_name(file: &str) -> String {
    let base = file.rsplit('/').next().unwrap_or(file);
    let stem = base.trim_end_matches(".txt").trim_end_matches(".log");
    format!("OpenRAC-{stem}.zip")
}

// A .zip of stored (uncompressed) files: the format every system opens, with no dependency.

fn crc32(bytes: &[u8]) -> u32 {
    let mut crc = 0xFFFF_FFFFu32;
    for &b in bytes {
        crc ^= u32::from(b);
        for _ in 0..8 {
            crc = if crc & 1 != 0 { (crc >> 1) ^ 0xEDB8_8320 } else { crc >> 1 };
        }
    }
    !crc
}

/// The time now as MS-DOS writes it (local time is unknown here: UTC).
fn dos_time() -> (u16, u16) {
    let secs = std::time::SystemTime::now().duration_since(UNIX_EPOCH).map_or(0, |d| d.as_secs()) as i64;
    let days = secs.div_euclid(86400);
    let rest = secs.rem_euclid(86400);
    // civil from days (the inverse of days_from_civil)
    let z = days + 719468;
    let era = z.div_euclid(146097);
    let doe = z - era * 146097;
    let yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    let doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    let mp = (5 * doy + 2) / 153;
    let d = doy - (153 * mp + 2) / 5 + 1;
    let m = if mp < 10 { mp + 3 } else { mp - 9 };
    let y = yoe + era * 400 + i64::from(m <= 2);
    let time = (((rest / 3600) << 11) | ((rest / 60 % 60) << 5) | ((rest % 60) / 2)) as u16;
    let date = ((((y - 1980).max(0)) << 9) | (m << 5) | d) as u16;
    (time, date)
}

pub fn write_zip(out: &Path, files: &[(String, Vec<u8>)]) -> Result<(), String> {
    let (time, date) = dos_time();
    let mut zip: Vec<u8> = Vec::new();
    let mut central: Vec<u8> = Vec::new();
    let u16le = |v: &mut Vec<u8>, x: u16| v.extend_from_slice(&x.to_le_bytes());
    let u32le = |v: &mut Vec<u8>, x: u32| v.extend_from_slice(&x.to_le_bytes());
    for (name, data) in files {
        let crc = crc32(data);
        let size = u32::try_from(data.len()).map_err(|_| format!("{name} is too big for the .zip"))?;
        let offset = u32::try_from(zip.len()).map_err(|_| "the .zip is too big".to_string())?;
        let name_bytes = name.as_bytes();
        // Local file header.
        u32le(&mut zip, 0x0403_4b50);
        u16le(&mut zip, 20); // version needed
        u16le(&mut zip, 0x0800); // UTF-8 names
        u16le(&mut zip, 0); // stored
        u16le(&mut zip, time);
        u16le(&mut zip, date);
        u32le(&mut zip, crc);
        u32le(&mut zip, size);
        u32le(&mut zip, size);
        u16le(&mut zip, name_bytes.len() as u16);
        u16le(&mut zip, 0);
        zip.extend_from_slice(name_bytes);
        zip.extend_from_slice(data);
        // Its central directory entry.
        u32le(&mut central, 0x0201_4b50);
        u16le(&mut central, 20); // made by
        u16le(&mut central, 20);
        u16le(&mut central, 0x0800);
        u16le(&mut central, 0);
        u16le(&mut central, time);
        u16le(&mut central, date);
        u32le(&mut central, crc);
        u32le(&mut central, size);
        u32le(&mut central, size);
        u16le(&mut central, name_bytes.len() as u16);
        u16le(&mut central, 0); // extra
        u16le(&mut central, 0); // comment
        u16le(&mut central, 0); // disk
        u16le(&mut central, 0); // internal attributes
        u32le(&mut central, 0); // external attributes
        u32le(&mut central, offset);
        central.extend_from_slice(name_bytes);
    }
    let central_offset = zip.len() as u32;
    let central_size = central.len() as u32;
    zip.extend_from_slice(&central);
    u32le(&mut zip, 0x0605_4b50);
    u16le(&mut zip, 0);
    u16le(&mut zip, 0);
    u16le(&mut zip, files.len() as u16);
    u16le(&mut zip, files.len() as u16);
    u32le(&mut zip, central_size);
    u32le(&mut zip, central_offset);
    u16le(&mut zip, 0);
    let tmp = out.with_extension("zip.tmp");
    std::fs::File::create(&tmp)
        .and_then(|mut f| f.write_all(&zip))
        .map_err(|e| format!("cannot write {}: {e}", out.display()))?;
    std::fs::rename(&tmp, out).map_err(|e| format!("cannot write {}: {e}", out.display()))
}

#[cfg(test)]
mod tests {
    use super::*;

    const CRASH: &str = "==== OpenRAC crash report ====
Reason:   access (SIGBUS) at game address 0x0F000000, which is not mapped
Kind:     fault
When:     2026-10-17 20:08:29 -0600 (the session began 2026-10-17 20:08:09 -0600)
Program:  openrac-rac1-pal, OpenRAC 0.7.1, commit ceffee00
Game:     level 3, game mode 0, frame 3000, hero at 263.98 101.90 54.50

Backtrace (the newest call first):
0   Ratchet & Clank (PAL)               0x0000000102847018 _ZN7openrac5crash6reportEPKcNS0_4KindE + 100
1   libsystem_platform.dylib            0x0000000180aec144 _sigtramp + 56
2   Ratchet & Clank (PAL)               0x0000000104f1b3c0 openrac_game_vsync + 832
3   Ratchet & Clank (PAL)               0x0000000104f82c48 func_001E99D8 + 596
(the program is loaded at 0x104F18000; less that, an address is the one in its symbols)

The log's last lines (the oldest first):
  [info] now playing level 3

The session:
==== OpenRAC session ====
openrac   0.7.1, commit ceffee00
graphics  Apple M5 Max (Apple), OpenGL 4.1 Metal, hardware through SDL's cocoa driver
==== end of the report ====
";

    fn session_log(started: &str, home: &str, crashed: bool) -> String {
        format!(
            "==== OpenRAC session ====\nsession   {started}\nopenrac   0.7.1, commit ceffee00\ngame      rac1-pal: Ratchet & Clank (PAL), disc SCES_509.16\nsystem    macOS 27.0, 48.0 GB of memory\ncommand   openrac-rac1-pal --data {home}/Library/data\n=========================\n[info] now playing level 3\n[warning] something odd\n{}",
            if crashed { "[error] ==== the program stopped: fault ====\n" } else { "[info] 3000 frames\n" }
        )
    }

    fn data_with_files(home: &str) -> tempfile::TempDir {
        let dir = tempfile::tempdir().unwrap();
        std::fs::create_dir_all(dir.path().join("crashes")).unwrap();
        std::fs::write(dir.path().join("crashes/crash-2026-10-17_20-08-29.txt"), CRASH).unwrap();
        std::fs::write(dir.path().join("openrac-rac1-pal.log"), session_log("2026-10-17 20:08:09 -0600", home, true))
            .unwrap();
        std::fs::write(
            dir.path().join("openrac-rac1-pal.1.log"),
            session_log("2026-10-17 18:00:00 -0600", home, false),
        )
        .unwrap();
        std::fs::write(dir.path().join("other.txt"), "not listed").unwrap();
        dir
    }

    #[test]
    fn tidies_the_home_folder_only_where_it_is_a_folder() {
        assert_eq!(tidy("/Users/ann/x and /Users/ann", Some("/Users/ann")), "~/x and ~");
        assert_eq!(tidy("/Users/anna/x", Some("/Users/ann")), "/Users/anna/x");
        assert_eq!(tidy("C:\\Users\\ann\\a", Some("C:\\Users\\ann")), "~\\a");
        assert_eq!(tidy("nothing", None), "nothing");
    }

    #[test]
    fn parses_the_ports_times() {
        assert_eq!(parse_time("2026-10-17 12:03:22 +0000"), Some(1_792_238_602));
        assert_eq!(parse_time("2026-10-17 14:03:22 +0200"), Some(1_792_238_602));
        assert_eq!(parse_time("2026-10-17 06:03:22 -0600"), Some(1_792_238_602));
        assert_eq!(parse_time("soon"), None);
    }

    #[test]
    fn lists_sessions_and_crashes_newest_first() {
        let data = data_with_files("/Users/ann");
        let list = list(data.path());
        assert_eq!(list.len(), 3, "{list:?}");
        assert_eq!(list[0].kind, ReportKind::Crash);
        assert_eq!(list[0].file, "crashes/crash-2026-10-17_20-08-29.txt");
        assert_eq!(list[0].planet.as_deref(), Some("Kerwan"));
        assert_eq!(list[0].session.as_deref(), Some("2026-10-17 20:08:09 -0600"));
        assert_eq!(list[1].kind, ReportKind::Session);
        assert_eq!(list[1].file, "openrac-rac1-pal.log");
        assert!(list[1].crashed);
        assert_eq!(list[1].program.as_deref(), Some("openrac-rac1-pal"));
        assert_eq!(list[2].file, "openrac-rac1-pal.1.log");
        assert!(!list[2].crashed);
        assert_eq!(list[2].level, Some(3));
        let since = parse_time("2026-10-17 20:08:00 -0600").unwrap();
        assert_eq!(latest_crash(data.path(), since).map(|e| e.file), Some(list[0].file.clone()));
        assert!(latest_crash(data.path(), since + 3600).is_none());
    }

    #[test]
    fn refuses_files_it_does_not_list() {
        let data = data_with_files("/Users/ann");
        assert!(resolve(data.path(), "openrac-rac1-pal.log").is_ok());
        assert!(resolve(data.path(), "crashes/crash-2026-10-17_20-08-29.txt").is_ok());
        assert!(resolve(data.path(), "other.txt").is_err());
        assert!(resolve(data.path(), "../openrac-rac1-pal.log").is_err());
        assert!(resolve(data.path(), "crashes/../../x.txt").is_err());
        assert!(resolve(data.path(), "openrac-missing.log").is_err());
    }

    #[test]
    fn copy_text_has_what_happened_without_the_handler() {
        let data = data_with_files("/Users/ann");
        let text = copy_text(data.path(), "crashes/crash-2026-10-17_20-08-29.txt", "0.7.1").unwrap();
        assert!(text.contains("What happened: access (SIGBUS)"), "{text}");
        assert!(text.contains("Where: Kerwan (level 3)"), "{text}");
        assert!(text.contains("OpenRAC: 0.7.1, commit ceffee00; launcher 0.7.1"), "{text}");
        assert!(text.contains("Graphics: Apple M5 Max"), "{text}");
        assert!(text.contains("openrac_game_vsync"), "{text}");
        assert!(!text.contains("_sigtramp") && !text.contains("openrac5crash"), "{text}");
        let session = copy_text(data.path(), "openrac-rac1-pal.1.log", "0.7.1").unwrap();
        assert!(session.contains("Ended in a crash: no"), "{session}");
        assert!(session.contains("[warning] something odd"), "{session}");
    }

    /// Reads a stored .zip back: its names and contents.
    fn unzip(bytes: &[u8]) -> Vec<(String, String)> {
        let mut out = Vec::new();
        let mut at = 0;
        while bytes.len() >= at + 30 && bytes[at..at + 4] == 0x0403_4b50u32.to_le_bytes() {
            let le32 = |i: usize| u32::from_le_bytes(bytes[i..i + 4].try_into().unwrap());
            let le16 = |i: usize| u16::from_le_bytes(bytes[i..i + 2].try_into().unwrap()) as usize;
            let crc = le32(at + 14);
            let size = le32(at + 18) as usize;
            let name_len = le16(at + 26);
            let name = String::from_utf8(bytes[at + 30..at + 30 + name_len].to_vec()).unwrap();
            let data = &bytes[at + 30 + name_len..at + 30 + name_len + size];
            assert_eq!(crc32(data), crc, "{name}");
            out.push((name, String::from_utf8(data.to_vec()).unwrap()));
            at += 30 + name_len + size;
        }
        assert_eq!(bytes[bytes.len() - 22..bytes.len() - 18], 0x0605_4b50u32.to_le_bytes());
        out
    }

    #[test]
    fn bundles_the_report_its_session_and_the_launcher_log() {
        let home = home().unwrap_or_else(|| "/nonexistent-home".into());
        let data = data_with_files(&home);
        let launcher = data.path().join("launcher.log");
        std::fs::write(&launcher, format!("[2026-10-17 20:08:30] the game stopped (code 3); {home}/x\n")).unwrap();
        let out = data.path().join("out.zip");
        let names =
            bundle(data.path(), "crashes/crash-2026-10-17_20-08-29.txt", Some(&launcher), "0.7.1", &out).unwrap();
        assert_eq!(names, ["summary.txt", "crash-2026-10-17_20-08-29.txt", "openrac-rac1-pal.log", "launcher.log"]);
        let files = unzip(&std::fs::read(&out).unwrap());
        assert_eq!(files.len(), 4);
        let log = &files[2].1;
        assert!(log.contains("session   2026-10-17 20:08:09 -0600"), "{log}");
        assert!(log.contains("--data ~/Library/data"), "{log}");
        assert!(!files.iter().any(|(_, t)| t.contains(&format!("{home}/"))), "the home folder is left in");
        assert_eq!(bundle_name("crashes/crash-2026-10-17_20-08-29.txt"), "OpenRAC-crash-2026-10-17_20-08-29.zip");
    }

    #[test]
    fn deletes_one_crash_report_or_one_log() {
        let data = data_with_files("/home/x");
        delete(data.path(), "rac1", "crashes/crash-2026-10-17_20-08-29.txt").unwrap();
        assert!(!data.path().join("crashes/crash-2026-10-17_20-08-29.txt").exists());
        assert!(data.path().join("openrac-rac1-pal.log").is_file(), "its session's log is a row of its own");
        delete(data.path(), "rac1", "openrac-rac1-pal.1.log").unwrap();
        assert!(!data.path().join("openrac-rac1-pal.1.log").exists());
        // Already gone: not an error.
        delete(data.path(), "rac1", "openrac-rac1-pal.1.log").unwrap();
        assert_eq!(list(data.path()).len(), 1);
    }

    #[test]
    fn delete_refuses_anything_outside_the_game_s_reports() {
        let data = data_with_files("/home/x");
        let outside = data.path().parent().unwrap().join("outside-crash.txt");
        std::fs::write(&outside, "keep").unwrap();
        for bad in [
            "other.txt",
            "launcher.log",
            "crashes",
            "crashes/",
            "crashes/other.txt",
            "crashes/crash-x.log",
            "../outside-crash.txt",
            "crashes/../other.txt",
            "crashes/../../outside-crash.txt",
            "crashes/..",
            "crashes\\crash-2026.txt",
            "/etc/passwd",
            "openrac-rac1-pal.txt",
            "openrac-rac10-pal.log",
            "openrac-rac2-pal.log",
            "sub/openrac-rac1-pal.log",
            "",
        ] {
            assert!(delete(data.path(), "rac1", bad).is_err(), "{bad:?} was allowed");
        }
        assert!(data.path().join("other.txt").is_file());
        assert!(outside.is_file());
        assert_eq!(list(data.path()).len(), 3);
        std::fs::remove_file(outside).ok();
    }

    #[cfg(unix)]
    #[test]
    fn delete_never_follows_a_link() {
        let data = data_with_files("/home/x");
        let secret = data.path().parent().unwrap().join("secret-target.txt");
        std::fs::write(&secret, "keep").unwrap();
        std::os::unix::fs::symlink(&secret, data.path().join("crashes/crash-linked.txt")).unwrap();
        std::os::unix::fs::symlink(&secret, data.path().join("openrac-rac1-linked.log")).unwrap();
        assert!(delete(data.path(), "rac1", "crashes/crash-linked.txt").is_err());
        assert!(delete(data.path(), "rac1", "openrac-rac1-linked.log").is_err());
        assert_eq!(std::fs::read_to_string(&secret).unwrap(), "keep");
        // A crashes folder that is a link out of the data folder is not the game's.
        let elsewhere = tempfile::tempdir().unwrap();
        std::fs::write(elsewhere.path().join("crash-a.txt"), "keep").unwrap();
        let other = tempfile::tempdir().unwrap();
        std::os::unix::fs::symlink(elsewhere.path(), other.path().join("crashes")).unwrap();
        assert!(delete(other.path(), "rac1", "crashes/crash-a.txt").is_err());
        assert!(elsewhere.path().join("crash-a.txt").is_file());
        std::fs::remove_file(secret).ok();
    }

    #[test]
    fn delete_all_takes_the_reports_and_leaves_the_rest() {
        let data = data_with_files("/home/x");
        std::fs::write(data.path().join("openrac-rac2-pal.log"), "another game's").unwrap();
        std::fs::write(data.path().join("launcher.log"), "not a game log").unwrap();
        assert_eq!(delete_all(data.path(), "rac1").unwrap(), 3);
        assert!(!data.path().join("crashes/crash-2026-10-17_20-08-29.txt").exists());
        assert!(data.path().join("other.txt").is_file());
        assert!(data.path().join("launcher.log").is_file());
        assert!(data.path().join("openrac-rac2-pal.log").is_file());
        assert!(list(data.path()).iter().all(|e| e.file == "openrac-rac2-pal.log"));
        assert_eq!(delete_all(data.path(), "rac1").unwrap(), 0);
    }

    #[test]
    fn crc_is_the_zip_one() {
        assert_eq!(crc32(b"123456789"), 0xCBF4_3926);
    }

    #[test]
    fn shortens_a_huge_log() {
        let big = "x".repeat(LOG_LIMIT + 200_000);
        let short = shortened(big);
        assert!(short.len() < LOG_LIMIT + 70_000);
        assert!(short.contains("bytes left out"));
    }
}
