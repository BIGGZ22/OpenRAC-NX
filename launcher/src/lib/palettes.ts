// The launcher's themes (Settings → General): its surfaces, text, lines and states. The accent is
// not a theme's: on Android it is the picked game's (themes.ts, accentProperties), elsewhere the
// launcher's orange, under every theme. "openrac" is app.css's own colours, the website's.

export type PaletteId = "openrac" | "gruvbox" | "oled" | "nord" | "mocha";

/** The app.css tokens a theme sets. */
type Token =
  | "--ink"
  | "--ink-deep"
  | "--panel"
  | "--panel-hi"
  | "--text"
  | "--body"
  | "--lav"
  | "--soft"
  | "--dim"
  | "--line"
  | "--line-strong"
  | "--line-field"
  | "--ok"
  | "--warn"
  | "--err";

export interface Palette {
  id: PaletteId;
  name: string;
  /** Its colours; none for "openrac", whose are app.css's. */
  colors: Partial<Record<Token, string>>;
  /** What Settings shows of it: the page, a panel and the text (the first two app.css's for "openrac"). */
  swatch: [string, string, string];
}

/** The lines of a theme from one colour at the three strengths app.css uses. */
const lines = (rgb: string): Partial<Record<Token, string>> => ({
  "--line": `rgb(${rgb} / 0.22)`,
  "--line-strong": `rgb(${rgb} / 0.36)`,
  "--line-field": `rgb(${rgb} / 0.6)`,
});

const OPENRAC: Palette = { id: "openrac", name: "OpenRAC", colors: {}, swatch: ["#181820", "#22222f", "#e8f0ff"] };

export const PALETTES: Palette[] = [
  OPENRAC,
  {
    // Gruvbox dark (MIT, morhetz/gruvbox): its warm browns, cream text and faded colours.
    id: "gruvbox",
    name: "Gruvbox",
    colors: {
      "--ink": "#282828",
      "--ink-deep": "#1d2021",
      "--panel": "#32302f",
      "--panel-hi": "#3c3836",
      "--text": "#fbf1c7",
      "--body": "#ebdbb2",
      "--lav": "#83a598",
      "--soft": "#d5c4a1",
      "--dim": "#a89984",
      ...lines("168 153 132"),
      "--ok": "#b8bb26",
      "--warn": "#fabd2f",
      "--err": "#fb4934",
    },
    swatch: ["#282828", "#3c3836", "#ebdbb2"],
  },
  {
    // Black where nothing is drawn, which an OLED screen leaves off: no glow around the pages.
    id: "oled",
    name: "OLED black",
    colors: {
      "--ink": "#000000",
      "--ink-deep": "#000000",
      "--panel": "#0b0b0f",
      "--panel-hi": "#15151c",
      "--text": "#f4f6ff",
      "--body": "#c9cfe4",
      "--soft": "#a9b2cf",
      "--dim": "#8a93b4",
      ...lines("160 165 200"),
    },
    swatch: ["#000000", "#15151c", "#f4f6ff"],
  },
  {
    // Nord (MIT, nordtheme): its polar night and snow storm, the frost for links.
    id: "nord",
    name: "Nord",
    colors: {
      "--ink": "#2e3440",
      "--ink-deep": "#242933",
      "--panel": "#3b4252",
      "--panel-hi": "#434c5e",
      "--text": "#eceff4",
      "--body": "#e5e9f0",
      "--lav": "#88c0d0",
      "--soft": "#d0d6e2",
      "--dim": "#a7b1c6",
      ...lines("216 222 233"),
      "--ok": "#a3be8c",
      "--warn": "#ebcb8b",
      "--err": "#e8858e",
    },
    swatch: ["#2e3440", "#434c5e", "#eceff4"],
  },
  {
    // Catppuccin Mocha (MIT, catppuccin): its mantle, base and lavender.
    id: "mocha",
    name: "Catppuccin Mocha",
    colors: {
      "--ink": "#181825",
      "--ink-deep": "#11111b",
      "--panel": "#1e1e2e",
      "--panel-hi": "#29293d",
      "--text": "#cdd6f4",
      "--body": "#bac2de",
      "--lav": "#b4befe",
      "--soft": "#a6adc8",
      "--dim": "#9399b2",
      ...lines("147 153 178"),
      "--ok": "#a6e3a1",
      "--warn": "#f9e2af",
      "--err": "#f38ba8",
    },
    swatch: ["#181825", "#29293d", "#cdd6f4"],
  },
];

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
