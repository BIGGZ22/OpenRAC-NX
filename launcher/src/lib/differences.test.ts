import { describe, expect, it } from "vitest";
import type { DifferenceView } from "./api";
import { differencesSummary, groupDifferences, splitVersion } from "./differences";
import rac1 from "../../../port/game/rac1/differences.json";

const entry = (id: string, kind: DifferenceView["kind"], on: boolean | null = null): DifferenceView => ({
  id,
  kind,
  name: id,
  original: "o",
  port: "p",
  refs: [],
  setting: kind === "fix" ? { key: `fix_${id}`, default: true } : null,
  on,
});

describe("groupDifferences", () => {
  it("puts the fixes first, then the guards, then the PC's own, and leaves empty groups out", () => {
    const groups = groupDifferences([entry("p", "port"), entry("f", "fix", true), entry("q", "port")]);
    expect(groups.map((g) => g.kind)).toEqual(["fix", "port"]);
    expect(groups[1]?.entries.map((e) => e.id)).toEqual(["p", "q"]);
  });
});

describe("differencesSummary", () => {
  it("counts the fixes that are on, and the rest", () => {
    const fixes = [entry("a", "fix", true), entry("b", "fix", true), entry("c", "fix", false)];
    expect(differencesSummary([...fixes, entry("g", "guard"), entry("p", "port")])).toBe(
      "2 of 3 game fixes on · 2 more changes",
    );
    expect(differencesSummary([entry("a", "fix", true)])).toBe("1 game fix on");
    expect(differencesSummary([entry("a", "fix", null), entry("p", "port")])).toBe("1 game fix on · 1 more change");
    expect(differencesSummary([entry("p", "port"), entry("q", "port")])).toBe("2 changes");
    expect(differencesSummary([])).toBe("");
  });
});

describe("rac1's list", () => {
  it("has each kind, and a setting for every fix and only for them", () => {
    const kinds = new Set(rac1.differences.map((d) => d.kind));
    expect([...kinds].sort()).toEqual(["fix", "guard", "port"]);
    for (const d of rac1.differences) {
      expect("setting" in d, d.id).toBe(d.kind === "fix");
    }
  });
});

describe("splitVersion", () => {
  it("splits a version key", () => {
    expect(splitVersion("rac1/pal")).toEqual(["rac1", "pal"]);
    expect(splitVersion("rac1")).toEqual(["rac1", ""]);
  });
});
