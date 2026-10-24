// The launcher's themes (Settings → General): its surfaces, text, lines and states. The accent is
// not a theme's: on Android it is the picked game's (themes.ts, accentProperties), elsewhere the
// launcher's orange, under every theme. "openrac" is app.css's own colours, the website's. The
// game's options drawer wears the same (port/game/common/ui_colors.h).

import data from "./palettes.json";

export type PaletteId = "openrac" | "gruvbox" | "oled" | "nord" | "mocha";

export interface Palette {
  id: PaletteId;
  name: string;
  /** The app.css tokens it sets ("--ink": "#282828"); none for "openrac", whose are app.css's. */
  colors: Record<string, string>;
  /** What Settings shows of it: the page, a panel and the text (the first two app.css's for "openrac"). */
  swatch: [string, string, string];
}

/** The themes, from palettes.json, which the launcher's core reads too: it hands the game the one
 *  picked, for the game's own options (core/src/theme.rs). */
export const PALETTES = data.palettes as [Palette, ...Palette[]];

/** app.css's own colours (:root), as palettes.json repeats them for the core. */
export const BASE: Record<string, string> = data.base;

/** The first, app.css's own. */
const [OPENRAC] = PALETTES;

export function palette(id: string | null | undefined): Palette {
  return PALETTES.find((p) => p.id === id) ?? OPENRAC;
}

/** Every token any theme sets, to take a theme off again. */
export const PALETTE_TOKENS = [...new Set(PALETTES.flatMap((p) => Object.keys(p.colors)))];

/** Where the theme is kept for the first paint, before the settings have loaded. */
const STORED = "openrac-theme";

/** Puts theme `id` on the page and keeps it for the next start. */
export function applyPalette(id: string | null | undefined, root: HTMLElement = document.documentElement) {
  const p = palette(id);
  for (const name of PALETTE_TOKENS) root.style.removeProperty(name);
  for (const [name, value] of Object.entries(p.colors)) root.style.setProperty(name, value);
  root.dataset.theme = p.id;
  try {
    localStorage.setItem(STORED, p.id);
  } catch {
    // Storage refused (a private window): the theme then comes with the settings.
  }
}

/** The theme kept from the last start, if any. */
export function storedPalette(): string | null {
  try {
    return localStorage.getItem(STORED);
  } catch {
    return null;
  }
}
