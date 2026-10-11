//! Discord Rich Presence integration via local IPC socket.
//!
//! Connects to Discord's local IPC endpoint (`discord-ipc-0` .. `9`): a Unix domain socket on
//! Linux and macOS, a named pipe (`\\.\pipe\discord-ipc-N`) on Windows, and updates the
//! activity: in the launcher, working on a game, or playing one.

use std::io::{Read, Write};
use std::path::PathBuf;
use std::time::{SystemTime, UNIX_EPOCH};

use serde::{Deserialize, Serialize};

/// The connection to Discord: its socket, or on Windows its named pipe opened as a file.
#[cfg(unix)]
type Connection = std::os::unix::net::UnixStream;
#[cfg(windows)]
type Connection = std::fs::File;

pub const DEFAULT_CLIENT_ID: &str = "1558096324602363955";

/// OpenRAC's wrench on its orange, an art asset of the Discord application (docs/discord/openrac.png):
/// the launcher's image, and the small one while a game plays.
pub const OPENRAC_IMAGE: &str = "openrac";

#[derive(Debug, Clone, Serialize, Deserialize, Default)]
#[serde(rename_all = "camelCase")]
pub struct Activity {
    /// The name after "Playing": the Discord application's own name when left out. While a game
    /// plays it is the game's title, so Discord shows "Playing Ratchet & Clank".
    #[serde(skip_serializing_if = "Option::is_none")]
    pub name: Option<String>,
    /// 0: Playing.
    #[serde(rename = "type", skip_serializing_if = "Option::is_none")]
    pub kind: Option<u8>,
    /// What the member list shows after "Playing": 0 the name, 1 the state, 2 the details.
    #[serde(rename = "status_display_type", skip_serializing_if = "Option::is_none")]
    pub status_display_type: Option<u8>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub details: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub state: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub timestamps: Option<ActivityTimestamps>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub assets: Option<ActivityAssets>,
    /// Up to two links under the activity; Discord shows them to others, not to yourself.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub buttons: Option<Vec<ActivityButton>>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ActivityButton {
    pub label: String,
    pub url: String,
}

/// The links every activity carries.
fn buttons() -> Option<Vec<ActivityButton>> {
    Some(vec![
        ActivityButton { label: "OpenRAC".into(), url: "https://openrac.dev".into() },
        ActivityButton { label: "GitHub".into(), url: "https://github.com/OpenRAC/OpenRAC".into() },
    ])
}

#[derive(Debug, Clone, Serialize, Deserialize, Default)]
#[serde(rename_all = "camelCase")]
pub struct ActivityTimestamps {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub start: Option<u64>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub end: Option<u64>,
}

/// Discord's field names: snake_case (`large_image`); other spellings are ignored.
#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct ActivityAssets {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub large_image: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub large_text: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub small_image: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub small_text: Option<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(tag = "kind", rename_all = "camelCase")]
pub enum DiscordStatus {
    Idle,
    ViewingGame {
        title: String,
        region: String,
        progress_pct: Option<f64>,
        game_id: String,
    },
    PlayingGame {
        title: String,
        region: String,
        game_id: String,
        start_time: Option<u64>,
    },
    /// A long job for a game: setting it up from the disc, building it, preparing the editor.
    Working {
        task: String,
        title: String,
        game_id: String,
        start_time: Option<u64>,
    },
}

pub struct DiscordIpc {
    client_id: String,
    #[cfg(any(unix, windows))]
    stream: Option<Connection>,
    seq: u64,
    /// The planet the running game reported (`planet_from_log`), shown while it plays, and since
    /// when (the elapsed time counts from there).
    planet: Option<Place>,
    planet_since: u64,
    /// The player's bolts, as the running game last logged them.
    bolts: Option<u32>,
    last: Option<(DiscordStatus, u64)>,
    /// No new connection attempt before this (seconds): a socket Discord no longer answers costs
    /// a whole handshake wait, so it is not retried on every status change.
    retry_at: u64,
}

impl DiscordIpc {
    pub fn new(client_id: impl Into<String>) -> Self {
        Self {
            client_id: client_id.into(),
            #[cfg(any(unix, windows))]
            stream: None,
            seq: 0,
            planet: None,
            planet_since: 0,
            bolts: None,
            last: None,
            retry_at: 0,
        }
    }

    pub fn set_client_id(&mut self, id: impl Into<String>) {
        let new_id = id.into();
        if self.client_id != new_id {
            self.client_id = new_id;
            self.disconnect();
            self.retry_at = 0;
        }
    }

    pub fn disconnect(&mut self) {
        #[cfg(any(unix, windows))]
        {
            self.stream = None;
        }
    }

    #[cfg(any(unix, windows))]
    fn connect(&mut self) -> Result<&mut Connection, String> {
        if self.stream.is_some() {
            return self.stream.as_mut().ok_or_else(|| "Discord IPC socket closed".to_string());
        }
        if now_sec() < self.retry_at {
            return Err("Discord IPC socket not available".to_string());
        }

        for path in find_socket_paths() {
            let Some(mut stream) = open_connection(&path) else {
                continue;
            };
            // Handshake (Opcode 0): {"v": 1, "client_id": "..."}
            let payload = serde_json::json!({
                "v": 1,
                "client_id": self.client_id
            })
            .to_string();

            if send_packet(&mut stream, 0, &payload).is_ok() {
                // Try reading handshake response packet
                match read_packet(&mut stream) {
                    Ok((opcode, data)) => {
                        if opcode == 1 {
                            set_reply_timeout(&stream);
                            return Ok(self.stream.insert(stream));
                        } else if opcode == 2 {
                            if let Ok(err) = std::str::from_utf8(&data) {
                                eprintln!("[discord-rpc] Handshake error: {err}");
                            }
                        }
                    }
                    Err(_) => {
                        // Read timeout or socket busy, proceed silently to next candidate
                    }
                }
            }
        }

        self.retry_at = now_sec() + RETRY_SECS;
        Err("Discord IPC socket not available".to_string())
    }

    pub fn update_activity(&mut self, activity: Option<&Activity>) -> Result<(), String> {
        #[cfg(any(unix, windows))]
        {
            self.seq += 1;
            let nonce = self.seq.to_string();
            let pid = std::process::id();

            let payload = serde_json::json!({
                "cmd": "SET_ACTIVITY",
                "args": {
                    "pid": pid,
                    "activity": activity
                },
                "nonce": nonce
            })
            .to_string();

            // Try sending to current or newly connected stream
            let stream = self.connect()?;
            let failed = send_packet(stream, 1, &payload).is_err();
            if !failed {
                // Drain the response without waiting for it.
                let _ = read_packet(stream);
            }

            if failed {
                self.disconnect();
                // Attempt one reconnect
                if let Ok(stream) = self.connect() {
                    let _ = send_packet(stream, 1, &payload);
                    let _ = read_packet(stream);
                }
            }
            Ok(())
        }
        #[cfg(not(any(unix, windows)))]
        {
            let _ = activity;
            Ok(())
        }
    }

    /// The planet being played; shown at once if a game is playing.
    pub fn set_planet(&mut self, planet: Option<Place>) -> Result<(), String> {
        if self.planet == planet {
            return Ok(());
        }
        if planet.is_none() {
            self.bolts = None;
        }
        self.planet = planet;
        self.planet_since = now_sec();
        self.resend_playing()
    }

    /// The player's bolts; shown at once if a game is playing.
    pub fn set_bolts(&mut self, bolts: u32) -> Result<(), String> {
        if self.bolts == Some(bolts) {
            return Ok(());
        }
        self.bolts = Some(bolts);
        self.resend_playing()
    }

    /// A line of the running game's log: its planet or its bolts.
    pub fn on_game_log(&mut self, game: &str, line: &str) -> Result<(), String> {
        if let Some(place) = planet_from_log(game, line) {
            return self.set_planet(Some(place));
        }
        match bolts_from_log(line) {
            Some(bolts) => self.set_bolts(bolts),
            None => Ok(()),
        }
    }

    fn resend_playing(&mut self) -> Result<(), String> {
        match self.last.clone() {
            Some((status @ DiscordStatus::PlayingGame { .. }, app_start)) => self.set_status(&status, app_start),
            _ => Ok(()),
        }
    }

    pub fn set_status(&mut self, status: &DiscordStatus, app_start: u64) -> Result<(), String> {
        self.last = Some((status.clone(), app_start));
        let activity = match status {
            DiscordStatus::Idle => Activity {
                details: Some("In Launcher".into()),
                state: Some("Browsing Library".into()),
                timestamps: Some(ActivityTimestamps { start: Some(app_start), end: None }),
                assets: Some(ActivityAssets {
                    large_image: Some(OPENRAC_IMAGE.into()),
                    large_text: Some("OpenRAC Launcher".into()),
                    small_image: None,
                    small_text: None,
                }),
                ..Default::default()
            },
            DiscordStatus::ViewingGame { title, region, progress_pct, game_id: _ } => {
                let state_str = match progress_pct {
                    Some(p) => format!("{region} · {p:.1}% matched"),
                    None => region.clone(),
                };
                Activity {
                    details: Some(format!("Viewing {title}")),
                    state: Some(state_str),
                    timestamps: Some(ActivityTimestamps { start: Some(app_start), end: None }),
                    assets: Some(ActivityAssets {
                        large_image: Some(OPENRAC_IMAGE.into()),
                        large_text: Some("OpenRAC Launcher".into()),
                        small_image: None,
                        small_text: None,
                    }),
                    ..Default::default()
                }
            }
            DiscordStatus::PlayingGame { title, region, game_id, start_time } => {
                let current_sec = now_sec();
                Activity {
                    name: Some(title.clone()),
                    kind: Some(0),
                    status_display_type: Some(0),
                    details: Some(match &self.planet {
                        Some(Place { planet, location: Some(location) }) => format!("Exploring {planet} · {location}"),
                        Some(Place { planet, location: None }) => format!("Exploring {planet}"),
                        None => "In the menus".into(),
                    }),
                    state: self.planet.as_ref().and(self.bolts).map(|b| format!("{} bolts", thousands(b))),
                    timestamps: Some(ActivityTimestamps {
                        start: Some(match self.planet {
                            Some(_) => self.planet_since,
                            None => start_time.unwrap_or(current_sec),
                        }),
                        end: None,
                    }),
                    assets: Some(ActivityAssets {
                        large_image: Some(game_image_url(game_id)),
                        large_text: Some(format!("{title} ({region})")),
                        small_image: Some(OPENRAC_IMAGE.into()),
                        small_text: Some("OpenRAC".into()),
                    }),
                    ..Default::default()
                }
            }
            DiscordStatus::Working { task, title, game_id, start_time } => Activity {
                details: Some(task.clone()),
                state: Some(title.clone()),
                timestamps: Some(ActivityTimestamps { start: Some(start_time.unwrap_or_else(now_sec)), end: None }),
                assets: Some(ActivityAssets {
                    large_image: Some(game_image_url(game_id)),
                    large_text: Some(title.clone()),
                    small_image: Some(OPENRAC_IMAGE.into()),
                    small_text: Some("OpenRAC".into()),
                }),
                ..Default::default()
            },
        };
        let activity = Activity { buttons: buttons(), ..activity };

        self.update_activity(Some(&activity))
    }

    pub fn clear(&mut self) -> Result<(), String> {
        self.update_activity(None)
    }
}

/// Where the player is: the planet, and the place on it when the game's level is known.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Place {
    pub planet: String,
    pub location: Option<String>,
}

/// The level a running port announces in its log ("now playing level N", port/game/common/main.cpp)
/// as a place, for the games whose levels are known.
pub fn planet_from_log(game: &str, line: &str) -> Option<Place> {
    let level: usize = line.split("now playing level ").nth(1)?.trim().parse().ok()?;
    match game {
        "rac1" => Some(Place {
            planet: crate::saves::RAC1_PLANETS.get(level)?.to_string(),
            location: crate::saves::RAC1_LOCATIONS.get(level).map(|l| l.to_string()),
        }),
        _ => Some(Place { planet: format!("Level {level}"), location: None }),
    }
}

/// The bolts a running port logs ("bolts N", port/game/common/main.cpp).
pub fn bolts_from_log(line: &str) -> Option<u32> {
    line.split("] bolts ").nth(1)?.trim().parse().ok()
}

/// 1250 as "1,250", the way the game writes bolts.
fn thousands(n: u32) -> String {
    let digits = n.to_string();
    let mut out = String::new();
    for (i, c) in digits.chars().enumerate() {
        if i > 0 && (digits.len() - i).is_multiple_of(3) {
            out.push(',');
        }
        out.push(c);
    }
    out
}

/// The large image while a game plays: the game's logo, an art asset of the Discord application
/// named after the game (Developer Portal, Rich Presence, Art Assets: `rac1` to `rac4`), so no
/// logo is kept in OpenRAC. Another game shows OpenRAC's wrench.
pub fn game_image_url(game_id: &str) -> String {
    match game_id {
        "rac1" | "rac2" | "rac3" | "rac4" => game_id.to_string(),
        _ => OPENRAC_IMAGE.to_string(),
    }
}

pub fn now_sec() -> u64 {
    SystemTime::now().duration_since(UNIX_EPOCH).unwrap_or_default().as_secs()
}

/// Discord answers the handshake in anything from a few hundred milliseconds to several seconds;
/// a shorter wait never connects. Callers run off the UI thread, so the wait stalls nothing.
#[cfg(unix)]
const HANDSHAKE_TIMEOUT: std::time::Duration = std::time::Duration::from_secs(10);
/// An activity's reply, read so that replies do not pile up in the socket.
#[cfg(unix)]
const REPLY_TIMEOUT: std::time::Duration = std::time::Duration::from_secs(2);
#[cfg(any(unix, windows))]
const RETRY_SECS: u64 = 15;

#[cfg(unix)]
fn open_connection(path: &std::path::Path) -> Option<Connection> {
    let stream = std::os::unix::net::UnixStream::connect(path).ok()?;
    let _ = stream.set_read_timeout(Some(HANDSHAKE_TIMEOUT));
    let _ = stream.set_write_timeout(Some(REPLY_TIMEOUT));
    Some(stream)
}

#[cfg(unix)]
fn set_reply_timeout(stream: &Connection) {
    let _ = stream.set_read_timeout(Some(REPLY_TIMEOUT));
}

#[cfg(windows)]
fn set_reply_timeout(_: &Connection) {}

/// A pipe has no timeouts: Discord answers every packet, so a read returns.
#[cfg(windows)]
fn open_connection(path: &std::path::Path) -> Option<Connection> {
    std::fs::OpenOptions::new().read(true).write(true).open(path).ok()
}

#[cfg(windows)]
fn find_socket_paths() -> Vec<PathBuf> {
    (0..10).map(|i| PathBuf::from(format!(r"\\.\pipe\discord-ipc-{i}"))).collect()
}

#[cfg(unix)]
fn find_socket_paths() -> Vec<PathBuf> {
    let mut candidates = Vec::new();
    let mut seen_canonical = Vec::new();
    let mut add_candidate = |p: PathBuf| {
        if p.exists() {
            let canonical = p.canonicalize().unwrap_or_else(|_| p.clone());
            if !seen_canonical.contains(&canonical) {
                seen_canonical.push(canonical);
                candidates.push(p);
            }
        }
    };

    let flatpak_subdirs = [
        ".flatpak/com.discordapp.Discord/xdg-run",
        "app/dev.vencord.Vesktop",
        "app/io.github.spacingbat3.webcord",
        "app/com.discordapp.DiscordCanary",
        "app/com.discordapp.Discord",
    ];

    // Check XDG_RUNTIME_DIR
    if let Ok(runtime_dir) = std::env::var("XDG_RUNTIME_DIR") {
        let base = PathBuf::from(&runtime_dir);
        for i in 0..10 {
            add_candidate(base.join(format!("discord-ipc-{i}")));
        }
        for sub in &flatpak_subdirs {
            for i in 0..10 {
                add_candidate(base.join(sub).join(format!("discord-ipc-{i}")));
            }
        }
    }

    if let Ok(uid) = std::env::var("UID") {
        let base = PathBuf::from(format!("/run/user/{uid}"));
        for sub in &flatpak_subdirs {
            for i in 0..10 {
                add_candidate(base.join(sub).join(format!("discord-ipc-{i}")));
            }
        }
        for i in 0..10 {
            add_candidate(base.join(format!("discord-ipc-{i}")));
        }
    }

    // Fallback: /tmp
    for i in 0..10 {
        add_candidate(PathBuf::from(format!("/tmp/discord-ipc-{i}")));
    }

    candidates
}

#[cfg(any(unix, windows))]
fn send_packet<W: Write>(writer: &mut W, opcode: u32, payload: &str) -> std::io::Result<()> {
    let bytes = payload.as_bytes();
    let len = bytes.len() as u32;

    let mut buf = Vec::with_capacity(8 + bytes.len());
    buf.extend_from_slice(&opcode.to_le_bytes());
    buf.extend_from_slice(&len.to_le_bytes());
    buf.extend_from_slice(bytes);

    writer.write_all(&buf)?;
    writer.flush()?;
    Ok(())
}

#[cfg(any(unix, windows))]
fn read_packet<R: Read>(reader: &mut R) -> std::io::Result<(u32, Vec<u8>)> {
    let mut header = [0u8; 8];
    reader.read_exact(&mut header)?;
    let opcode = u32::from_le_bytes(header[0..4].try_into().unwrap());
    let len = u32::from_le_bytes(header[4..8].try_into().unwrap()) as usize;

    let mut buf = vec![0u8; len];
    reader.read_exact(&mut buf)?;
    Ok((opcode, buf))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn serializes_activity_cleanly() {
        let act = Activity {
            name: Some("Ratchet & Clank".into()),
            kind: Some(0),
            status_display_type: Some(0),
            buttons: buttons(),
            details: Some("Exploring Kerwan".into()),
            state: None,
            timestamps: Some(ActivityTimestamps { start: Some(1700000000), end: None }),
            assets: Some(ActivityAssets {
                large_image: Some(game_image_url("rac1")),
                large_text: Some("Ratchet & Clank".into()),
                small_image: Some(OPENRAC_IMAGE.into()),
                small_text: Some("OpenRAC".into()),
            }),
        };

        let json = serde_json::to_string(&act).unwrap();
        assert!(json.contains("\"name\":\"Ratchet & Clank\",\"type\":0,\"status_display_type\":0"));
        assert!(json.contains("\"details\":\"Exploring Kerwan\""));
        assert!(!json.contains("\"state\""));
        assert!(json.contains("\"large_image\":\"rac1\""));
        assert!(json.contains("\"buttons\":[{\"label\":\"OpenRAC\",\"url\":\"https://openrac.dev\"}"));
    }

    #[test]
    fn reads_the_planet_from_the_port_log() {
        assert_eq!(
            planet_from_log("rac1", "[info] now playing level 3"),
            Some(Place { planet: "Kerwan".into(), location: Some("Metropolis".into()) })
        );
        assert_eq!(
            planet_from_log("rac1", "[info] now playing level 6"),
            Some(Place { planet: "Nebula G34".into(), location: Some("Blarg Station".into()) })
        );
        assert_eq!(bolts_from_log("[info] bolts 1250"), Some(1250));
        assert_eq!(bolts_from_log("[info] 2500 frames"), None);
        assert_eq!(thousands(0), "0");
        assert_eq!(thousands(1250), "1,250");
        assert_eq!(thousands(1234567), "1,234,567");
        assert_eq!(
            planet_from_log("rac2", "[info] now playing level 4"),
            Some(Place { planet: "Level 4".into(), location: None })
        );
        assert_eq!(planet_from_log("rac1", "[info] 4000 frames"), None);
    }

    #[test]
    fn handles_unconnected_ipc_gracefully() {
        let mut ipc = DiscordIpc::new(DEFAULT_CLIENT_ID);
        // Does not panic or crash when Discord is offline
        let _ = ipc.set_status(&DiscordStatus::Idle, now_sec());
        let _ = ipc.clear();
    }
}
