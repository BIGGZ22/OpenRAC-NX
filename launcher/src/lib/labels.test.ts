import { describe, expect, it } from "vitest";
import { regionLabel, shortTitle } from "./labels";

describe("labels", () => {
  it("shortens a game's name for a narrow list", () => {
    expect(shortTitle("Ratchet & Clank")).toBe("Ratchet & Clank");
    expect(shortTitle("Ratchet & Clank: Going Commando")).toBe("Going Commando");
    expect(shortTitle("Ratchet: Deadlocked")).toBe("Deadlocked");
    expect(shortTitle("Ratchet & Clank: Size Matters")).toBe("Size Matters");
  });

  it("names regions", () => {
    expect(regionLabel("ntsc")).toBe("NTSC-U");
    expect(regionLabel("pal")).toBe("PAL");
  });
});
