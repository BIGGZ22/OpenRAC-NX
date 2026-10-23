// Each game's colours and typeface, from the OpenRAC website's game cards
// (openrac-site src/lib/projects.ts, `theme` and `fontClass`). The site
// names the games by their own ids; OpenRAC by release order:
//   rac1 = "rac1", rac2 = "gc", rac3 = "uya", rac4 = "deadlocked", rac5 = "sm".
// Keep the values in step with the site (docs/STYLE.md).

export interface Theme {
  /** The boxes' fill, their border, the main and the second text colour. */
  box: string;
  border: string;
  text: string;
  text2: string;
  /** The card's gradient, top to bottom. */
  from: string;
  to: string;
  /** The title's typeface: the site's Audiowide for RAC1, Chakra Petch for Size Matters,
   *  Orbitron for the others. */
  font: "brand" | "head" | "chakra";
  weight: number;
}

export const THEMES: Record<string, Theme> = {
  rac1: {
    box: "#181820",
    border: "#747669",
    text: "#c8d2ff",
    text2: "#9ca1d4",
    from: "#3c7a3a",
    to: "#12331f",
    font: "brand",
    weight: 400,
  },
  rac2: {
    box: "#101a2a",
    border: "#5a7fa6",
    text: "#9fd3ff",
    text2: "#6fa8d6",
    from: "#7a4d1a",
    to: "#2a1a0c",
    font: "head",
    weight: 600,
  },
  rac3: {
    box: "rgba(98,66,27,.8)",
    border: "#a0742e",
    text: "#ebbe67",
    // Lighter than the site's #c99a45: the year under the title reads at 4.5:1 on the box.
    text2: "#d4a752",
    from: "#25514f",
    to: "#0d1f24",
    font: "head",
    weight: 600,
  },
  rac4: {
    box: "#12151b",
    border: "#b91c1c",
    text: "#f3f4f6",
    text2: "#ef4444",
    from: "#7f1d1d",
    to: "#0f1115",
    font: "head",
    weight: 700,
  },
  rac5: {
    box: "#1b2936",
    border: "#eab308",
    text: "#f8fafc",
    text2: "#facc15",
    from: "#223342",
    to: "#131d26",
    font: "chakra",
    weight: 600,
  },
};

/** For a game the site has no card for yet: the launcher's own colours. */
export const FALLBACK: Theme = {
  box: "#1d1d28",
  border: "#d58a00",
  text: "#ffd08a",
  text2: "#ffb443",
  from: "#22222f",
  to: "#12121a",
  font: "head",
  weight: 600,
};

export function theme(game: string): Theme {
  return THEMES[game] ?? FALLBACK;
}

/** The theme as CSS custom properties, for a style attribute (the site's names: --box, --bd, --tx, --tx2). */
export function themeStyle(game: string): string {
  const t = theme(game);
  const font = { brand: "var(--font-brand)", head: "var(--font-head)", chakra: "var(--font-chakra)" }[t.font];
  return [
    `--box: ${t.box}`,
    `--bd: ${t.border}`,
    `--tx: ${t.text}`,
    `--tx2: ${t.text2}`,
    `--from: ${t.from}`,
    `--to: ${t.to}`,
    `--title-font: ${font}`,
    `--title-weight: ${t.weight}`,
  ].join("; ");
}

/** The launcher's accent while a game is the one picked (Android): its bar gradient and the ink on
 *  it, its amber (headings, the focus ring, what is on) and gold, in that game's colour, as the
 *  games' own menus have it: Going Commando's pale blue, Up Your Arsenal's yellow, Deadlocked's
 *  red, Size Matters' blue. Ratchet & Clank keeps the launcher's own orange (app.css). */
export interface Accent {
  barTop: string;
  barBottom: string;
  /** The text on the bar's colour (a main button's), and its deeper shade. */
  barInk: string;
  barInkDeep: string;
  amber: string;
  gold: string;
}

export const ACCENTS: Record<string, Accent> = {
  rac2: {
    barTop: "#7fcdf5",
    barBottom: "#43a3de",
    barInk: "#06233a",
    barInkDeep: "#03141f",
    amber: "#7fcff5",
    gold: "#c4e8fb",
  },
  rac3: {
    barTop: "#ffd84a",
    barBottom: "#e9b30c",
    barInk: "#2a2000",
    barInkDeep: "#1c1500",
    amber: "#ffd23f",
    gold: "#ffe9a3",
  },
  rac4: {
    barTop: "#f25a5a",
    barBottom: "#e23c3c",
    barInk: "#2a0606",
    barInkDeep: "#1f0303",
    amber: "#ff6b61",
    gold: "#ffb8b2",
  },
  rac5: {
    barTop: "#6aa0ff",
    barBottom: "#3f78f0",
    barInk: "#06173a",
    barInkDeep: "#020c24",
    amber: "#7aa8ff",
    gold: "#c2d6ff",
  },
};

/** The custom properties an accent sets, and its colour each takes. */
const ACCENT_SET: [string, keyof Accent][] = [
  ["--bar-top", "barTop"],
  ["--bar-bottom", "barBottom"],
  ["--bar-ink", "barInk"],
  ["--bar-ink-deep", "barInkDeep"],
  ["--amber", "amber"],
  ["--gold", "gold"],
  ["--brand", "barBottom"],
];

/** The custom properties accentProperties may set, to take them all off again. */
export const ACCENT_PROPERTIES = ACCENT_SET.map(([name]) => name);

/** The custom properties a game's accent sets on the page; none for the launcher's own (rac1, or
 *  no game). The warning colour is its own (app.css), so a warning still reads as one. */
export function accentProperties(game: string | null): [string, string][] {
  const a = game ? ACCENTS[game] : undefined;
  return a ? ACCENT_SET.map(([name, key]) => [name, a[key]]) : [];
}
