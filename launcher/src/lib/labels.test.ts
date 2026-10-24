import { describe, expect, it } from "vitest";
import { levelName, levelsOf, regionLabel, shortTitle } from "./labels";

describe("labels", () => {
  it("shortens a game's name for a narrow list", () => {
    expect(shortTitle("Ratchet & Clank")).toBe("Ratchet & Clank");
    expect(shortTitle("Ratchet & Clank: Going Commando")).toBe("Going Commando");
    expect(shortTitle("Ratchet: Deadlocked")).toBe("Deadlocked");
    expect(shortTitle("Ratchet & Clank: Size Matters")).toBe("Size Matters");
    expect(shortTitle("Ratchet & Clank: Secret Agent Clank")).toBe("Secret Agent Clank");
  });

  it("lists each game's levels by their numbers", () => {
    expect(levelsOf("rac1")).toHaveLength(19);
    expect(levelsOf("rac1")[18]).toEqual({ number: 18, name: "Veldin (the return)" });
    // The later games number their levels as their level headers do: gaps, and Going Commando's museum is 30.
    expect(levelsOf("rac2").map((l) => l.number)).toContain(30);
    expect(levelsOf("rac3")[0]).toEqual({ number: 1, name: "Veldin" });
    expect(levelsOf("rac4")).toHaveLength(47);
    expect(levelName("rac2", 30)).toBe("Insomniac Museum");
    expect(levelName("rac4", 99)).toBe("Level 99");
  });

  it("names regions", () => {
    expect(regionLabel("ntsc")).toBe("NTSC-U");
    expect(regionLabel("pal")).toBe("PAL");
  });
});
