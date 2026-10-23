import { describe, expect, it } from "vitest";
import { ACCENT_PROPERTIES, ACCENTS, FALLBACK, THEMES, accentProperties, theme, themeStyle } from "./themes";

// Every game in the checkout has a theme: a new games/<id>/ needs one too.
const manifests = import.meta.glob<{ id: string }>("../../../games/*/game.json", { eager: true, import: "default" });

describe("themes", () => {
  it("covers every game in the checkout", () => {
    const ids = Object.values(manifests).map((g) => g.id);
    expect(ids.length).toBeGreaterThanOrEqual(4);
    for (const id of ids) expect(THEMES[id], `no theme for ${id} in src/lib/themes.ts`).toBeDefined();
  });

  it("falls back for an unknown game", () => {
    expect(theme("rac9")).toBe(FALLBACK);
  });

  it("writes the site's custom properties", () => {
    const style = themeStyle("rac1");
    expect(style).toContain("--bd: #747669");
    expect(style).toContain("--title-font: var(--font-brand)");
  });

  it("gives each game but Ratchet & Clank an accent, and keeps the launcher's own for it", () => {
    expect(accentProperties("rac1")).toEqual([]);
    expect(accentProperties(null)).toEqual([]);
    for (const id of Object.keys(THEMES).filter((id) => id !== "rac1")) {
      expect(ACCENTS[id], `no accent for ${id}`).toBeDefined();
      const names = accentProperties(id).map(([name]) => name);
      expect(names.sort()).toEqual([...ACCENT_PROPERTIES].sort());
    }
  });

  it("keeps a main button's text readable on its accent (4.5:1)", () => {
    const luminance = (hex: string) => {
      const [r, g, b] = [1, 3, 5].map((i) => parseInt(hex.slice(i, i + 2), 16) / 255);
      const lin = (c: number) => (c <= 0.03928 ? c / 12.92 : ((c + 0.055) / 1.055) ** 2.4);
      return 0.2126 * lin(r ?? 0) + 0.7152 * lin(g ?? 0) + 0.0722 * lin(b ?? 0);
    };
    const contrast = (a: string, b: string) => {
      const [hi, lo] = [luminance(a), luminance(b)].sort((x, y) => y - x);
      return ((hi ?? 0) + 0.05) / ((lo ?? 0) + 0.05);
    };
    for (const [id, a] of Object.entries(ACCENTS)) {
      for (const bar of [a.barTop, a.barBottom]) {
        expect(contrast(a.barInkDeep, bar), `${id}: ${a.barInkDeep} on ${bar}`).toBeGreaterThanOrEqual(4.5);
      }
    }
  });
});
