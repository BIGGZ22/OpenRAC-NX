# Launcher appearance

Open **Appearance** in the OpenGL launcher's sidebar. Changes apply immediately
and save to `/switch/openrac/artwork/appearance.ini`. They do not change gameplay,
video settings or saves. Press **Y** on this page to preview the Play banner.

## Themes and colors

- **Theme preset:** Dark, Light or Custom.
- Separate colors for the menu background, sidebar, panels, main text, secondary
  text, accent/buttons and borders.
- Press **A** on a color to enter `#RRGGBB`, such as `#B890FF`. Left/Right cycles
  suggested colors. Editing a theme color switches to Custom; switching presets
  keeps the previous custom menu palette available.
- Button labels automatically use dark or white text against the chosen accent.

## Play banner

The entire top card is customizable:

- **Banner background:** Solid color, Selected artwork from the Artwork page,
  or a separate Local background image.
- **Banner color:** Card color and image tint color.
- **Banner image tint:** 0–90%; higher values show more tint and less image.
- **Banner text color:** Independent of the menu text color.
- **Small artwork window:** Included game logo, Selected artwork, or Off.
- Editable heading, game title and subtitle. Empty text hides that line.

The launch button stays available. Wide images are centered and cropped to fill
the card without stretching; the small window preserves its image's proportions.

For an independent background, place `background.png` or `background.jpg` in
`/switch/openrac/artwork/` and select **Reload local background**. This allows a
background and small-window image to be chosen separately. Image limits match the
Artwork page: PNG/JPEG, 8 MiB, 4096 pixels per side and 12 million pixels total.
Local backgrounds load offline at startup. A missing background falls back to the
banner color; a failed reload keeps the current valid image.

**Restore appearance defaults** returns to Dark, default banner text and the
included logo window. It keeps downloaded artwork, personal images and API keys.

Native tests cover color parsing, theme selection, custom palette retention,
settings round trips, bounds on corrupt values and recovery from an interrupted
file replacement. Dark/Light/custom Play pages and the scrolling Appearance page
are rendered for visual review. Switch keyboard and touch interactions still need
console testing.
