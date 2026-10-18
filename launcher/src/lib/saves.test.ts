import { describe, expect, it } from "vitest";
import { backupLabel, emptySlots, stampSeconds } from "./saves";

describe("emptySlots", () => {
  it("says one slot, a run, or a list", () => {
    expect(emptySlots([])).toBe("");
    expect(emptySlots([4])).toBe("Slot 4 is empty");
    expect(emptySlots([5, 3, 4])).toBe("Slots 3 to 5 are empty");
    expect(emptySlots([1, 3, 5])).toBe("Slots 1, 3 and 5 are empty");
  });
});

describe("stampSeconds", () => {
  it("reads the core's save time as local time", () => {
    expect(stampSeconds("2026-10-09 17:01:21")).toBe(new Date(2026, 9, 9, 17, 1, 21).getTime() / 1000);
  });

  it("gives null for what it cannot read", () => {
    expect(stampSeconds(null)).toBeNull();
    expect(stampSeconds("yesterday")).toBeNull();
  });
});

describe("backupLabel", () => {
  it("names a backup by its note", () => {
    expect(backupLabel({ note: "before the last boss" })).toBe("Before the last boss");
    expect(backupLabel({ note: "Before editing slot 2" })).toBe("Before editing slot 2");
    expect(backupLabel({ note: null })).toBe("Backup");
    expect(backupLabel({ note: "  " })).toBe("Backup");
  });
});
