import { describe, expect, it } from "vitest";
import {
  COMPATIBLE_LABEL,
  KIND_LABEL,
  STATE_LABEL,
  added,
  crashModsLine,
  installedFrom,
  rangeText,
  settingTakes,
  sharesWith,
  updatesFound,
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
    expect(KIND_LABEL.script.text).toBe("Runs scripts (sandboxed)");
    expect(COMPATIBLE_LABEL.no.text).toBe("Cannot be installed");
  });
});

describe("the sentences", () => {
  it("count updates and name what was not checked", () => {
    expect(updatesFound(0, [])).toBe("Every mod from a link is up to date.");
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

  it("list shared mods, ranges and setting refusals", () => {
    expect(sharesWith([])).toBe("");
    expect(sharesWith(["A", "B", "C"])).toContain("A, B and C");
    expect(rangeText(50, 110)).toBe("50 to 110");
    expect(rangeText(undefined, 3)).toBe("up to 3");
    expect(settingTakes("Speed", "", true)).toBe("Speed takes a whole number.");
    expect(crashModsLine(["lock-strafe"])).toMatch(/^Gameplay mods were on \(lock-strafe\)\./);
  });
});
