import { describe, expect, it } from "vitest";
import type { DiscChoice } from "./api";
import { choiceFor, pickVersion, programOf } from "./discChoice";

const versions = [{ key: "rac1/pal" }, { key: "rac1/ntsc" }];
const none = () => false;

describe("disc choice", () => {
  it("plays the developer's chosen disc when there is a choice", () => {
    const pick = pickVersion(versions, { chosen: "rac1/ntsc", own: "rac1/pal", found: ["rac1/pal"], playable: none });
    expect(pick?.key).toBe("rac1/ntsc");
  });

  it("picks as before without a choice: the disc set up, then the first found", () => {
    expect(pickVersion(versions, { own: "rac1/ntsc", found: ["rac1/pal"], playable: none })?.key).toBe("rac1/ntsc");
    expect(pickVersion(versions, { found: ["rac1/ntsc"], playable: none })?.key).toBe("rac1/ntsc");
    expect(pickVersion(versions, { chosen: null, found: [], playable: none })?.key).toBe("rac1/pal");
    expect(pickVersion([], { found: [], playable: none })).toBeNull();
  });

  it("finds a game's choice, and none for a game with one disc", () => {
    const choices: DiscChoice[] = [{ game: "rac1", versions: ["rac1/pal", "rac1/ntsc"], chosen: "rac1/pal" }];
    expect(choiceFor(choices, "rac1")?.chosen).toBe("rac1/pal");
    expect(choiceFor(choices, "rac2")).toBeNull();
    expect(choiceFor([], "rac1")).toBeNull();
  });

  it("names the program whose logs a version's page lists", () => {
    expect(programOf("rac1/ntsc")).toBe("openrac-rac1-ntsc");
    expect(programOf("rac1/pal")).toBe("openrac-rac1-pal");
  });
});
