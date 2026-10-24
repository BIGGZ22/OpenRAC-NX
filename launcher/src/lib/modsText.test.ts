import { describe, expect, it } from "vitest";
import {
  ADD,
  ASSETS,
  COMPATIBLE_LABEL,
  EDITOR,
  PACKS,
  PAGE,
  PANEL,
  PURIST,
  MOD_PAGE,
  SAVE,
  SETTING,
  KIND_LABEL,
  STATE_LABEL,
  added,
  crashModsLine,
  installedFrom,
  rangeText,
  sharesWith,
  updatesFound,
  updatedAll,
  type Label,
} from "./modsText";

describe("the label maps", () => {
  it("give every state, kind and compatibility a few words and a tone of the palette", () => {
    const tones = new Set(["ok", "off", "warn", "err"]);
    const labels: Label[] = [
      ...Object.values(STATE_LABEL),
      ...Object.values(KIND_LABEL),
      ...Object.values(COMPATIBLE_LABEL),
    ];
    for (const label of labels) {
      expect(label.text.length).toBeGreaterThan(0);
      expect(label.text.length).toBeLessThanOrEqual(30);
      expect(tones.has(label.tone)).toBe(true);
    }
  });

  it("keep the words players already see", () => {
    expect(STATE_LABEL.on).toEqual({ text: "On", tone: "ok" });
    expect(STATE_LABEL.purist.text).toBe("Not in purist mode");
    expect(STATE_LABEL.refused).toEqual({ text: "Cannot load", tone: "err" });
    expect(KIND_LABEL.native.tone).toBe("warn");
    expect(KIND_LABEL.script.text).toBe("Sandboxed script");
    expect(COMPATIBLE_LABEL.no.text).toBe("Cannot be installed");
  });
});

describe("the page's words", () => {
  it("stay short: a line each, the longer why in a tooltip", () => {
    for (const group of [ADD, ASSETS, EDITOR, PACKS, PAGE, PANEL, PURIST, MOD_PAGE, SAVE, SETTING]) {
      for (const [key, text] of Object.entries(group)) {
        if (typeof text !== "string" || key.endsWith("Url") || key.endsWith("Title")) continue;
        expect(text.length, `${key}: ${text}`).toBeLessThanOrEqual(120);
      }
    }
  });
});

describe("the sentences", () => {
  it("count updates and name what was not checked", () => {
    expect(updatesFound(0, [])).toBe("Every mod from a link is up to date.");
    expect(updatedAll(2, 0)).toBe("Updated 2 mods; their settings are kept.");
    expect(updatedAll(1, 1)).toBe("Updated 1 mod; their settings are kept. 1 could not be updated: open each for why.");
    expect(PANEL.updateAll(3)).toBe("Update all (3)");
    expect(updatesFound(1, [])).toBe("1 update found.");
    expect(updatesFound(2, ["a: down"])).toBe("2 updates found. Not checked: a: down");
  });

  it("say what an install came to", () => {
    expect(installedFrom.on("Lock-strafe", "OpenRAC/rac1-mods")).toBe(
      "Installed Lock-strafe from OpenRAC/rac1-mods. It is on from the game's next start.",
    );
    expect(added.replaced("Lock-strafe", "1.1.0")).toBe("Updated Lock-strafe to 1.1.0; its settings are kept.");
    expect(added.packFailed("pack.zip", "")).toBe("pack.zip: it could not be added");
  });

  it("list shared mods and ranges", () => {
    expect(sharesWith([])).toBe("");
    expect(sharesWith(["A", "B", "C"])).toContain("A, B and C");
    expect(rangeText(50, 110)).toBe("50 to 110");
    expect(rangeText(undefined, 3)).toBe("up to 3");
    expect(crashModsLine(["lock-strafe"])).toMatch(/^Gameplay mods were on \(lock-strafe\)\./);
  });
});
