import { beforeEach, describe, expect, it } from "vitest";
import { applyPalette, BASE, palette, PALETTE_TOKENS, PALETTES, storedPalette } from "./palettes";
import appCss from "../app.css?raw";
import { ACCENT_PROPERTIES } from "./themes";

/** The page's root as applyPalette uses it: its style's properties and its data attributes. */
function fakeRoot() {
  const props = new Map<string, string>();
  return {
    style: {
      setProperty: (k: string, v: string) => props.set(k, v),
      removeProperty: (k: string) => props.delete(k),
      getPropertyValue: (k: string) => props.get(k) ?? "",
    },
    dataset: {} as Record<string, string>,
  };
}

describe("palettes", () => {
  let root = fakeRoot();
  beforeEach(() => {
    root = fakeRoot();
    const kept = new Map<string, string>();
    globalThis.localStorage = {
      getItem: (k: string) => kept.get(k) ?? null,
      setItem: (k: string, v: string) => void kept.set(k, v),
    } as Storage;
  });

  it("leave the accent to the game", () => {
    for (const p of PALETTES) {
      expect(Object.keys(p.colors).filter((t) => ACCENT_PROPERTIES.includes(t))).toEqual([]);
    }
  });

  it("are app.css's own for an unknown id", () => {
    expect(palette("no-such").id).toBe("openrac");
    expect(palette(null).colors).toEqual({});
  });

  it("put a theme on and take it off again", () => {
    applyPalette("gruvbox", root as unknown as HTMLElement);
    expect(root.style.getPropertyValue("--ink")).toBe("#282828");
    expect(root.dataset.theme).toBe("gruvbox");
    expect(storedPalette()).toBe("gruvbox");
    applyPalette("openrac", root as unknown as HTMLElement);
    expect(PALETTE_TOKENS.filter((t) => root.style.getPropertyValue(t))).toEqual([]);
    expect(storedPalette()).toBe("openrac");
  });

  it("set the page, the panels and the text", () => {
    for (const p of PALETTES.filter((p) => p.id !== "openrac")) {
      for (const t of ["--ink", "--panel", "--panel-hi", "--text", "--soft", "--dim", "--line"]) {
        expect(p.colors, `${p.id} ${t}`).toHaveProperty(t);
      }
    }
  });

  it("repeat app.css's own colours for the core and the game, as app.css has them", () => {
    const root = appCss.slice(appCss.indexOf(":root {"), appCss.indexOf("}", appCss.indexOf(":root {")));
    for (const [token, value] of Object.entries(BASE)) {
      expect(root, token).toContain(`${token}: ${value};`);
    }
  });
});
