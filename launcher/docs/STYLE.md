# The launcher's look

The launcher looks like [openrac.dev](https://openrac.dev): the same dark ink
background, orange header bar, wrench, type and per-game cards. Everything
here comes from [OpenRAC/openrac-site](https://github.com/OpenRAC/openrac-site)
at commit `2964d72`; when the site changes, change the files in the last
section to match.

## Colours

`src/app.css`, on `:root`, with the site's names (its `@theme` in
`src/app/globals.css`):

| Token                         | Value                             | Use                                                |
| ----------------------------- | --------------------------------- | -------------------------------------------------- |
| `--ink`                       | `#181820`                         | the page                                           |
| `--ink-deep`                  | `#12121a`                         | inputs, job output                                 |
| `--panel`                     | `#1d1d28`                         | panels                                             |
| `--panel-hi`                  | `#22222f`                         | cards inside panels, buttons                       |
| `--brand`                     | `#d58a00`                         | section numbers, the selected job                  |
| `--amber`                     | `#ffb443`                         | links, focus rings, blockers ("needs …"), warnings |
| `--gold`                      | `#ffd08a`                         | headings                                           |
| `--lav`, `--soft`, `--dim`    | `#c8d2ff`, `#aab6dc`, `#8f9ac4`   | secondary text, from most to least prominent       |
| `--line`, `--line-strong`     | `rgb(156 161 212 / .22)`, `/ .35` | borders                                            |
| `--text`, `--body`            | `#e8f0ff`, `#c3cdea`              | text (the site's body and prose colours)           |
| `--bar-top`, `--bar-bottom`   | `#e5a013`, `#cd8300`              | the header bar's gradient, primary buttons         |
| `--bar-ink`, `--bar-ink-deep` | `#2a1800`, `#1b1206`              | text on the bar; the selected tab                  |

The site has no status colours; the launcher adds `--ok` (`#8fd694`), `--warn`
(amber) and `--err` (`#ff8a7a`), used only in pills and messages. The theme is
dark only, as the site is.

## Type

| Token          | Font                   | Use                                                |
| -------------- | ---------------------- | -------------------------------------------------- |
| `--font-brand` | Audiowide              | the "OpenRAC" wordmark; RAC1's titles              |
| `--font-head`  | Orbitron               | headings, section numbers; the other games' titles |
| `--font-sans`  | Inter                  | everything else                                    |
| `--font-mono`  | the system's monospace | paths, commands, job output                        |

The fonts are bundled from npm (`@fontsource/audiowide`,
`@fontsource-variable/orbitron`, `@fontsource-variable/inter`), imported in
`src/main.ts`: no font or style loads from the network.

## Pieces taken from the site

| Site                                                                                                                                                                                                                                         | Launcher                                                                                                                                           |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------- |
| `Header.tsx`: the sticky orange bar, the wrench that tilts on hover, "Open**RAC**" in Audiowide, round tabs (selected: dark brown with amber text), the white Discord button                                                                 | `src/components/Header.svelte`                                                                                                                     |
| `Progress.tsx`: a game's card, a gradient from its `from` to its `to` colour, boxes with a thick border in its `border` colour, the "Progress / 59,32 %" pair, the bar with a light sweeping across it, the "… of … code bytes matched" line | `src/components/GameTile.svelte` (smaller), the band in `src/pages/GamePlayer.svelte`, `src/components/ProgressBar.svelte` and `.bar` in `app.css` |
| `globals.css`: section numbers that count up (`01`, `02`, …), the `rise` and `sweep` animations, reduced motion                                                                                                                              | `.sec-num`, `.sections`, `.rise`, `.bar` in `app.css`                                                                                              |
| Numbers: two decimals with a decimal comma (`59,32 %`), bytes grouped with narrow spaces                                                                                                                                                     | `src/lib/format.ts`                                                                                                                                |

The launcher's own additions keep to the same palette: pills (a dot and a
word, coloured by state), rounded buttons (primary ones in the bar's orange),
panels with a thin `--line` border and 16 px corners, dashed borders for what
is planned but not there yet.

## A game's colours

`src/lib/themes.ts` holds each game's `theme` from the site's
`src/lib/projects.ts`, under OpenRAC's ids:

| OpenRAC | Site         | Title font       |
| ------- | ------------ | ---------------- |
| `rac1`  | `rac1`       | Audiowide        |
| `rac2`  | `gc`         | Orbitron 600     |
| `rac3`  | `uya`        | Orbitron 600     |
| `rac4`  | `deadlocked` | Orbitron 700     |
| `rac5`  | `sm`         | Chakra Petch 600 |

`themeStyle(id)` turns a theme into the custom properties the cards use
(`--box`, `--bd`, `--tx`, `--tx2`, `--from`, `--to`, `--title-font`,
`--title-weight`): set it on an element and everything inside it takes that
game's colours. A game without a theme gets `FALLBACK` (the launcher's own
orange); the tests fail until every game in `games/` has one.

## A game's picture

Each game has a picture (`public/img/<name>-bg.webp`, the site's backdrop
under the site's name: `rac1`, `gc`, `uya`, `deadlocked`, `size-matters`),
listed in `src/lib/media.ts`. It shows in the game list, under the game's
band and faintly behind its page. A file that is not there is left out: the
game's colours show, as on the site.

The site serves its backdrops from its server, not its repository, so a new
one is copied from there (`size-matters-bg.webp` is not in the launcher yet).
The game list shows each picture 64 × 44 from a thumbnail,
`<name>-thumb.webp` (192 × 132, a few KB), so that it does not decode every
game's full picture: `python3 scripts/media.py thumbs` makes them again after
a picture is added or replaced. `python3 scripts/media.py still RECORDING GAME
--at S` writes a picture and its thumbnail from a frame of a recording of
your own play.

The band shows no moving clip: one was planned (an animated WebP under the
picture), and taken out on 2026-10-20 with the old `*-gameplay.gif` files,
which nothing showed. A still picture does the band's work without the
download, and a page with a Play button in it needs nothing moving behind it.

Where each picture comes from (who made it, from which disc, under what
terms) is not recorded yet for the backdrops here since 2026-10-09. It
belongs in [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md) before a
release, and a new file adds its line there when it is added.

## Structure

- A page has one main action, in the bar's orange, in a fixed place. Stops,
  dead ends and secondary actions are never orange.
- Secondary actions go in a menu beside the main one (`Menu.svelte`), with
  anything that removes set apart at the end in `--err`.
- A panel holds rows, not more boxes: lists are rows with a `--line` rule
  between them (`SaveManagerBlock.svelte`).
- A state is a dot and a few words, or a tick; a pill only where a list of
  states has to be scanned (the developer pages, Tasks).
- What only a contributor reads (serials, percentages, file names, paths) is
  shown with developer tools on.
- A task done in a few steps in order is a numbered list (`ol.steps` in
  `app.css`: the Mods tab's level editor, models and textures): each step a
  row, its name and one line on the left, its controls on the right.
- A page with several jobs gives each a tab of its own (the Mods tab: My
  mods, Level editor, Models and textures), so every one can be seen from the
  top; the main action stays in the page's head, above the tabs.
- A setting is a row: its name and at most one short line on the left, its
  control on the right (`.setting` in `app.css`). On/off is a switch
  (`input.switch`), two or three choices a segmented control (`.segmented`),
  more a list of radios. Settings are grouped in panels by what they are
  about; what most players never change goes under Advanced, folded
  (`details.more`), which opens by itself when something in it is on. A panel
  with nothing to choose on this system is not shown, and a choice that only
  matters with another one (Android's GPU driver, with Vulkan) shows with it.
- A list of things that each have settings of their own (the gameplay mods)
  is an overview of rows (a switch, a name and a line, one badge, "Settings ›"),
  and each opens a page of its own with a back button and where you are
  (`ModPage.svelte`), not settings unfolded in the list.
- A setting applies as it changes, without Enter or a Save button, and says
  that it saved. A number with both ends is a slider (the brand's orange up to
  the value, a mark at the default) with its value in a field beside it; a
  setting off its default has a Reset beside its control.
- No path in plain view on a page players share screenshots of (Mods): a
  folder is opened with a button, and its path waits in a folded About or
  Details (`lib/paths.ts`, `Reason.svelte`).

## Rules

- Use the tokens, not new colours. A new colour goes on `:root` with a name
  and a line in this file.
- Keep text readable on its background: the pills' and blockers' colours are
  only used on `--panel`, `--panel-hi` or a game's dark boxes.
- Respect `prefers-reduced-motion` (already global in `app.css`).
- Motion and spacing have tokens too: `--ease` and `--fast`, `--medium`,
  `--slow` for transitions; `--page-x`, `--page-top`, `--page-bottom` and
  `--page-gap` for every page's margins. A page's sections rise in turn as
  it opens (`.page > section` in `app.css`).
- Hover styles go inside `@media (hover: hover)`: on a touch screen a tapped
  control would otherwise keep them until the next tap elsewhere. A press
  shows with `:active`, a keyboard's or controller's focus with
  `:focus-visible` or `html[data-input="pad"]`.
- Android follows Android's own patterns, not the desktop's: a navigation rail
  on the left (a bar at the bottom below 600 px wide) in place of the orange
  bar, Material's 24/16 px margins, a ripple under the finger
  (`src/lib/ripple.ts`), snackbar-style toasts and no text selection on
  controls. The `--nav-*` tokens say what the navigation takes of the window.
- Window sizes: the window opens at 1200 × 780, can be resized, and cannot be
  made smaller than 960 × 560 (`tauri.conf.json`). 560 is as tall as a
  1366 × 768 laptop at 125 % scaling leaves room for above the taskbar, so the
  floor does not go higher. On a screen with less room than that (a 1080p
  screen at 150 % or 200 %) the window opens at what the screen has room for,
  and that is its floor (`core/src/window.rs`). A page scrolls down, never
  sideways. A tiling
  window manager may ignore the floor, so lay a page out to still fit from
  760 px wide.

## Keeping in step with the site

| When the site changes                                      | Change                                                                                                                            |
| ---------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| `src/app/globals.css` `@theme` colours or fonts            | `src/app.css` `:root`, the tables above                                                                                           |
| `src/lib/projects.ts` themes                               | `src/lib/themes.ts`                                                                                                               |
| `src/components/Header.tsx`, `Progress.tsx`                | `Header.svelte`, `SideList.svelte`, `GamePlayer.svelte`                                                                           |
| `public/wrench.webp`, `src/app/icon.png`, `apple-icon.png` | `public/`, and `npx tauri icon <1024 px png> -o src-tauri/icons` (then delete the `android/`, `ios/` and `Square*` files it adds) |
| `DISCORD_URL`                                              | `src/lib/links.ts`                                                                                                                |
