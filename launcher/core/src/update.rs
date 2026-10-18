//! The launcher's own updates where Tauri's updater does not run: Android, whose launcher is an APK;
//! and, for every system, where updates come from and the version they are compared with.
//! The release's manifest is the desktop's (`latest.json`, tools/updater_manifest.py), with the APK
//! under `android-aarch64`; the APK is signed with the same minisign key as the desktop's archives
//! and checked against the public key in tauri.conf.json before it is handed to Android's
//! installer, which also checks that it is signed as the installed launcher is.
use std::fs::File;
use std::io::Read;
use std::path::Path;

use base64::Engine;
use minisign_verify::{PublicKey, Signature};
use serde::Serialize;
use serde_json::Value;

/// The platform key of the Android launcher in the manifest.
pub const ANDROID: &str = "android-aarch64";

/// A newer launcher in the manifest, for one platform.
#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Release {
    pub version: String,
    pub notes: String,
    pub date: String,
    pub url: String,
    /// The minisign signature of the file at `url`, base64 as the manifest carries it.
    pub signature: String,
}

/// Whether `latest` is a later version than `current` (`major.minor.patch`, any prefix such as
/// `launcher-v` ignored; a pre-release is older than its release, and of two pre-releases of one
/// version the one whose last number is larger is newer: `0.8.0-dev.10` after `0.8.0-dev.9`, the
/// builds of a branch, as semver orders them).
pub fn is_newer(current: &str, latest: &str) -> bool {
    fn parse(v: &str) -> ([u64; 3], Option<u64>) {
        let v = v.trim_start_matches(|c: char| !c.is_ascii_digit());
        let (core, pre) = match v.split_once(['-', '+']) {
            Some((c, rest)) if !rest.is_empty() => {
                (c, Some(rest.rsplit('.').next().and_then(|n| n.parse().ok()).unwrap_or(0)))
            }
            Some((c, _)) => (c, None),
            None => (v, None),
        };
        let mut parts = [0u64; 3];
        for (part, text) in parts.iter_mut().zip(core.split('.')) {
            *part = text.parse().unwrap_or(0);
        }
        (parts, pre)
    }
    let (c, c_pre) = parse(current);
    let (l, l_pre) = parse(latest);
    if l != c {
        return l > c;
    }
    match (c_pre, l_pre) {
        (Some(_), None) => true,
        (Some(c), Some(l)) => l > c,
        _ => false,
    }
}

/// Where a launcher looks for its updates, read from its manifest's address (tauri.conf.json's
/// `plugins.updater.endpoints`, set by the build to the repository that built it: a fork's
/// launcher reads the fork's releases). Shown on the Settings page, which cannot change it.
#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct UpdateSource {
    /// `owner/name` of the GitHub repository; the address itself when it is not one.
    pub repository: String,
    /// The release it reads: None for the repository's latest, else that release's tag (a
    /// branch's rolling `launcher-dev-<branch>`).
    pub channel: Option<String>,
    /// The manifest's address.
    pub manifest: String,
    /// The page of the release it reads, or of all of them.
    pub page: String,
}

pub fn source(manifest: &str) -> UpdateSource {
    let unknown = || UpdateSource {
        repository: manifest.to_string(),
        channel: None,
        manifest: manifest.to_string(),
        page: manifest.to_string(),
    };
    let Some(path) = manifest.strip_prefix("https://github.com/") else { return unknown() };
    let parts: Vec<&str> = path.split('/').collect();
    let (owner, name) = match parts.as_slice() {
        [owner, name, "releases", ..] if !owner.is_empty() && !name.is_empty() => (*owner, *name),
        _ => return unknown(),
    };
    let repository = format!("{owner}/{name}");
    let channel = match parts.as_slice() {
        [_, _, "releases", "download", tag, _] => Some((*tag).to_string()),
        _ => None,
    };
    let page = match &channel {
        Some(tag) => format!("https://github.com/{repository}/releases/tag/{tag}"),
        None => format!("https://github.com/{repository}/releases"),
    };
    UpdateSource { repository, channel, manifest: manifest.to_string(), page }
}

/// The version updates are compared with: the package's, or for a build of a branch, whose
/// package carries the release's plain version (Windows' MSI and RPM take no `-dev`), that version
/// with the build's own `dev.<run>` (`OPENRAC_LAUNCHER_BUILD` when it was compiled).
pub fn effective_version(version: &str, build: Option<&str>) -> String {
    match build.map(str::trim) {
        Some(build) if !build.is_empty() && !version.contains('-') => format!("{version}-{build}"),
        _ => version.to_string(),
    }
}

/// The release the manifest offers `platform`, when it is newer than `current`; an error when the
/// manifest cannot be read.
pub fn release_for(manifest: &str, platform: &str, current: &str) -> Result<Option<Release>, String> {
    let manifest: Value = serde_json::from_str(manifest).map_err(|e| format!("the update manifest: {e}"))?;
    let text = |v: &Value, key: &str| v.get(key).and_then(Value::as_str).unwrap_or_default().to_string();
    let version = text(&manifest, "version");
    if version.is_empty() {
        return Err("the update manifest names no version".into());
    }
    let Some(entry) = manifest.get("platforms").and_then(|p| p.get(platform)) else { return Ok(None) };
    if !is_newer(current, &version) {
        return Ok(None);
    }
    let (url, signature) = (text(entry, "url"), text(entry, "signature"));
    if !url.starts_with("https://") || signature.is_empty() {
        return Err(format!("the update manifest's {platform} entry has no https address or no signature"));
    }
    Ok(Some(Release { version, notes: text(&manifest, "notes"), date: text(&manifest, "pub_date"), url, signature }))
}

fn decoded(base64: &str, what: &str) -> Result<String, String> {
    let bytes = base64::engine::general_purpose::STANDARD
        .decode(base64.trim())
        .map_err(|e| format!("{what} is not base64: {e}"))?;
    String::from_utf8(bytes).map_err(|_| format!("{what} is not text"))
}

/// Checks `file` against `signature` (a .sig file's content, base64) made with the key whose public
/// half is `public_key` (tauri.conf.json's `plugins.updater.pubkey`, base64). Read in pieces: an APK
/// is hundreds of megabytes.
pub fn verify(file: &Path, signature: &str, public_key: &str) -> Result<(), String> {
    let key = PublicKey::decode(&decoded(public_key, "the update key")?)
        .map_err(|e| format!("the update key cannot be read: {e}"))?;
    let signature = Signature::decode(&decoded(signature, "the update's signature")?)
        .map_err(|e| format!("the update's signature cannot be read: {e}"))?;
    let mut verifier =
        key.verify_stream(&signature).map_err(|e| format!("the update's signature cannot be checked: {e}"))?;
    let mut input = File::open(file).map_err(|e| format!("{}: {e}", file.display()))?;
    let mut buffer = vec![0u8; 1 << 20];
    loop {
        let n = input.read(&mut buffer).map_err(|e| format!("{}: {e}", file.display()))?;
        if n == 0 {
            break;
        }
        verifier.update(&buffer[..n]);
    }
    verifier.finalize().map_err(|_| "the update is not signed with OpenRAC's key: it was not installed".to_string())
}

#[cfg(test)]
mod tests {
    use super::*;

    // A key made for these tests only (`tauri signer generate`), its private half thrown away, and
    // its signature of DATA (`tauri signer sign`).
    const PUBLIC_KEY: &str = "dW50cnVzdGVkIGNvbW1lbnQ6IG1pbmlzaWduIHB1YmxpYyBrZXk6IEEzMzRGOTAwNkNDNzQ4REUKUldUZVNNZHNBUGswbzVDVk5MRCt6ZExLaHNIOWFaN1pFWGF4M3FYOGE4TjFERE83alY5TFplYmwK";
    const SIGNATURE: &str = "dW50cnVzdGVkIGNvbW1lbnQ6IHNpZ25hdHVyZSBmcm9tIHRhdXJpIHNlY3JldCBrZXkKUlVUZVNNZHNBUGswb3lNeGtLY0YvTm1aTm9DQlVkZlQrVWtWeS9tcW9pNVYvbE1Ndkh5bVd4V0E0L0luMDZ0WDRRam9PcHdTT2NoWFdydmN0SzdqTG5TanZKbllvMFpXYkFVPQp0cnVzdGVkIGNvbW1lbnQ6IHRpbWVzdGFtcDoxNzkyMzIzNTkwCWZpbGU6ZGF0YS5iaW4KUUJjTDRxT0FNVHBhK0Z1VVhoNlIxeUYxRGN5eFJsNkJSVXhJb2g4VW1Fd2JNZ3NUY2V6UEhEcEdvM2NWQkl3U1htMkhuNWJBWk1rNGJNejJReFFpQnc9PQo=";
    const DATA: &[u8] = b"OpenRAC update test data\n";

    #[test]
    fn compares_versions() {
        assert!(is_newer("0.7.4", "0.8.0"));
        assert!(is_newer("0.7.4", "launcher-v0.7.10"));
        assert!(is_newer("0.8.0-rc.1", "0.8.0"));
        assert!(!is_newer("0.8.0", "0.8.0"));
        assert!(!is_newer("0.8.0", "0.7.9"));
        assert!(!is_newer("0.8.0", "0.8.0-rc.1"));
        assert!(!is_newer("1.0.0", "0.99.99"));
        assert!(is_newer("0.8.0-dev.9", "0.8.0-dev.10"));
        assert!(!is_newer("0.8.0-dev.10", "0.8.0-dev.10"));
        assert!(!is_newer("0.8.0-dev.10", "0.8.0-dev.9"));
        assert!(is_newer("0.8.0-dev.10", "0.8.0"));
    }

    #[test]
    fn names_the_source() {
        let latest = source("https://github.com/someone/OpenRAC/releases/latest/download/latest.json");
        assert_eq!(latest.repository, "someone/OpenRAC");
        assert_eq!(latest.channel, None);
        assert_eq!(latest.page, "https://github.com/someone/OpenRAC/releases");
        let branch = source("https://github.com/someone/OpenRAC/releases/download/launcher-dev-feature-x/latest.json");
        assert_eq!(branch.repository, "someone/OpenRAC");
        assert_eq!(branch.channel.as_deref(), Some("launcher-dev-feature-x"));
        assert_eq!(branch.page, "https://github.com/someone/OpenRAC/releases/tag/launcher-dev-feature-x");
        assert_eq!(source("https://example.com/latest.json").repository, "https://example.com/latest.json");
    }

    #[test]
    fn versions_a_branch_build() {
        assert_eq!(effective_version("0.8.0", Some("dev.12")), "0.8.0-dev.12");
        assert_eq!(effective_version("0.8.0", Some("")), "0.8.0");
        assert_eq!(effective_version("0.8.0", None), "0.8.0");
        assert_eq!(effective_version("0.8.0-dev.12", Some("dev.12")), "0.8.0-dev.12");
        assert!(is_newer(&effective_version("0.8.0", Some("dev.12")), "0.8.0-dev.13"));
    }

    #[test]
    fn reads_the_manifest() {
        let manifest = r#"{"version":"0.8.0","notes":"n","pub_date":"2026-10-18T00:00:00Z","platforms":{
            "linux-x86_64":{"signature":"s","url":"https://x/a.AppImage"},
            "android-aarch64":{"signature":"sig","url":"https://x/a.apk"}}}"#;
        let release = release_for(manifest, ANDROID, "0.7.4").unwrap().unwrap();
        assert_eq!(release.url, "https://x/a.apk");
        assert_eq!(release.signature, "sig");
        assert_eq!(release.date, "2026-10-18T00:00:00Z");
        assert_eq!(release_for(manifest, ANDROID, "0.8.0").unwrap(), None);
        assert_eq!(release_for(manifest, "windows-x86_64", "0.1.0").unwrap(), None);
        assert!(release_for(r#"{"platforms":{}}"#, ANDROID, "0.1.0").is_err());
        let plain = r#"{"version":"0.8.0","platforms":{"android-aarch64":{"signature":"s","url":"http://x/a.apk"}}}"#;
        assert!(release_for(plain, ANDROID, "0.1.0").is_err());
    }

    #[test]
    fn checks_the_signature() {
        let dir = tempfile::tempdir().unwrap();
        let file = dir.path().join("update.apk");
        std::fs::write(&file, DATA).unwrap();
        verify(&file, SIGNATURE, PUBLIC_KEY).unwrap();
        let mut changed = DATA.to_vec();
        changed[0] ^= 1;
        std::fs::write(&file, changed).unwrap();
        assert!(verify(&file, SIGNATURE, PUBLIC_KEY).is_err());
        assert!(verify(&file, "not base64!", PUBLIC_KEY).is_err());
    }
}
