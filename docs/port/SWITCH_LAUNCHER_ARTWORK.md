# Switch launcher artwork

The Play page now displays the existing Ratchet & Clank game icon by default.
The Artwork page supports custom PNG/JPEG banners and optional SteamGridDB
downloads. Artwork is launcher-only; its worker and decoded images are released
before the game starts. The Vulkan experiment remains a separate build.

The [Appearance page](SWITCH_LAUNCHER_APPEARANCE.md) controls full-banner backgrounds,
the small artwork window, custom text, menu colors and Dark/Light presets.

## SteamGridDB

1. Obtain your personal key at https://www.steamgriddb.com/profile/preferences/api.
2. Open **Artwork > SteamGridDB API key** and enter it with the Switch keyboard.
   Input is masked. Entering an empty key removes it.
3. Select **Search game**, then choose the correct game with Left / Right.
4. Select **Find banners**, choose a **Banner choice**, and **Download and apply**.
   The applied artwork appears in the preview and on Play. Repeat with another
   banner to compare choices.

The launcher uses SteamGridDB API v2's `search/autocomplete` and `heroes/game`
endpoints. It requests static PNG/JPEG art, caches the chosen image for offline
use and records its SteamGridDB ID, author and image URL in `banner-credit.txt`.
It does not sign into Steam or automatically download anything at startup.
The API key is sent only to SteamGridDB, never to image-download hosts. HTTPS
verification stays enabled. Redirects, oversized downloads and invalid images
are rejected. B cancels a pending request; network timeouts are bounded.

Live authenticated requests need console testing with a user's own key. No API
key is bundled, checked into source or printed in application logs.

## Local art and files

Put `custom.png` or `custom.jpg` in `/switch/openrac/artwork/`, then select
**Load local artwork**. Wide images work best on the Play banner; images are
center-cropped without stretching. The maximum file size is 8 MiB, maximum side
4096 pixels and maximum decoded area 12 million pixels. Decoded images are
reduced to fit 1280×720 in memory.

Files in this directory:

- `steamgriddb.key`: personal key, stored locally as plain text; do not share it.
- `banner.img`: validated downloaded PNG/JPEG, kept across NRO updates.
- `banner-credit.txt`: downloaded artwork attribution.
- `selection.txt`: included/local/downloaded artwork preference.
- `custom.png` / `custom.jpg`: optional user-supplied artwork, never deleted by Reset.

**Included game art** restores the built-in presentation and saves that preference.
The art directory is separate from game settings and saves. Download failures
leave the current displayed artwork in place.

## Validation

`port/tests/switch_artwork_test.cpp` checks API parsing, rejected image hosts,
malformed images, bundled image decoding, key validation/removal and persistence
of the included/local selection. Desktop previews exercise both launcher pages.
The OpenGL test NRO requires console checks for the software keyboard, HTTPS,
download cancellation, offline restart and launcher-to-game handoff.
