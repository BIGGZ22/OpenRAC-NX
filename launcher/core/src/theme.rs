//! The launcher's colours for the game: the theme picked in Settings (and on Android the picked
//! game's accent, as the launcher wears it) handed to the game's options drawer, so that it looks
//! like the launcher it was started from (port/game/common/ui_colors.h). The colours
//! are src/lib/palettes.json's, the file the launcher's pages read.

use crate::actions::Platform;

const PALETTES: &str = include_str!("../../src/lib/palettes.json");

/// The colours the drawer draws with, by their app.css names.
const DRAWER: &[&str] = &[
    "--ink-deep",
    "--panel",
    "--panel-hi",
    "--brand",
    "--bar-top",
    "--bar-bottom",
    "--bar-ink-deep",
    "--amber",
    "--gold",
    "--text",
    "--soft",
    "--dim",
    "--line",
    "--line-strong",
    "--err",
];

/// The variable the game reads them from: `name=#rrggbb[aa]` pairs, comma-separated, each name
/// app.css's without its leading dashes ("panel-hi=#22222f,line=#9ca1d438,...").
pub const ENV: &str = "OPENRAC_UI_COLORS";

/// The drawer's colours for theme `theme` (an unknown one is the launcher's own) and, on Android,
/// the accent of `game`.
pub fn ui_colors(theme: &str, game: Option<&str>, platform: Platform) -> String {
    let data: serde_json::Value = serde_json::from_str(PALETTES).unwrap_or_default();
    let palette =
        data["palettes"].as_array().and_then(|all| all.iter().find(|p| p["id"] == theme)).map(|p| &p["colors"]);
    let accent = match (platform, game) {
        (Platform::Android, Some(game)) => Some(&data["accents"][game]),
        _ => None,
    };
    DRAWER
        .iter()
        .filter_map(|token| {
            let value = [accent, palette, Some(&data["base"])]
                .into_iter()
                .flatten()
                .find_map(|colors| colors[*token].as_str())?;
            Some(format!("{}={}", token.trim_start_matches('-'), hex(value)?))
        })
        .collect::<Vec<_>>()
        .join(",")
}

/// A CSS colour as palettes.json writes them, `#rrggbb` or `rgb(r g b / a)`, as `#rrggbb[aa]`.
fn hex(css: &str) -> Option<String> {
    if css.len() == 7 && css.starts_with('#') && css[1..].chars().all(|c| c.is_ascii_hexdigit()) {
        return Some(css.to_ascii_lowercase());
    }
    let inner = css.strip_prefix("rgb(")?.strip_suffix(')')?;
    let (rgb, alpha) = inner.split_once('/').map_or((inner, None), |(c, a)| (c, Some(a)));
    let channels = rgb.split_whitespace().map(|c| c.parse::<u8>().ok()).collect::<Option<Vec<_>>>()?;
    let [r, g, b] = channels[..] else { return None };
    let mut out = format!("#{r:02x}{g:02x}{b:02x}");
    if let Some(alpha) = alpha {
        let a = alpha.trim().parse::<f32>().ok()?.clamp(0.0, 1.0);
        out += &format!("{:02x}", (a * 255.0).round() as u8);
    }
    Some(out)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn color<'a>(colors: &'a str, name: &str) -> Option<&'a str> {
        colors.split(',').find_map(|pair| pair.strip_prefix(name)?.strip_prefix('='))
    }

    #[test]
    fn hands_the_game_the_theme_and_the_accent() {
        let own = ui_colors("openrac", None, Platform::Linux);
        assert_eq!(own.split(',').count(), DRAWER.len(), "{own}");
        assert_eq!(color(&own, "panel"), Some("#1d1d28"));
        assert_eq!(color(&own, "line"), Some("#9ca1d438"));
        assert_eq!(color(&own, "amber"), Some("#ffb443"));
        // A theme sets its own; what it leaves is the launcher's.
        let nord = ui_colors("nord", Some("rac3"), Platform::Linux);
        assert_eq!(color(&nord, "panel"), Some("#3b4252"));
        assert_eq!(color(&nord, "amber"), Some("#ffb443"), "a desktop keeps the orange");
        assert_eq!(ui_colors("no-such", None, Platform::Linux), own);
        // Android wears the game's accent; Ratchet & Clank's is the launcher's own.
        let rac3 = ui_colors("nord", Some("rac3"), Platform::Android);
        assert_eq!(color(&rac3, "amber"), Some("#ffd23f"));
        assert_eq!(color(&rac3, "panel"), Some("#3b4252"));
        assert_eq!(ui_colors("openrac", Some("rac1"), Platform::Android), own);
    }

    #[test]
    fn reads_the_colours_palettes_json_writes() {
        assert_eq!(hex("#E8F0FF").as_deref(), Some("#e8f0ff"));
        assert_eq!(hex("rgb(168 153 132 / 0.36)").as_deref(), Some("#a899845c"));
        assert_eq!(hex("rgb(1 2 3)").as_deref(), Some("#010203"));
        assert_eq!(hex("var(--amber)"), None);
        assert_eq!(hex("rgb(1 2)"), None);
    }
}
