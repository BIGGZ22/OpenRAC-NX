import { describe, expect, it } from "vitest";
import { backgroundStatus, isGame, phaseLabel, readProgress, STARTING_MS, tailLines } from "./progress";

const out = (...lines: string[]) => lines.map((line) => ({ stream: "stdout" as const, line }));
const job = (actionId: string, state: "queued" | "running" = "running", startedAt: number | null = 0) => ({
  actionId,
  state,
  startedAt,
});

describe("phaseLabel", () => {
  it("names each step in plain words", () => {
    expect(phaseLabel(job("install:rac1"))).toBe("Setting up from your disc");
    expect(phaseLabel(job("port-build"))).toBe("Building the game for this computer");
    expect(phaseLabel(job("port-levels"))).toBe("Reading the levels from your disc");
    expect(phaseLabel(job("port-levels-check"))).toBe("Checking the levels");
    expect(phaseLabel(job("probe-bake"))).toBe("Preparing the enhanced lighting");
  });

  it("says the game is starting, then running", () => {
    expect(phaseLabel(job("play"), 1000)).toBe("Starting the game");
    expect(phaseLabel(job("play"), STARTING_MS + 1)).toBe("The game is running");
    expect(isGame(job("play-edited"))).toBe(true);
    expect(isGame(job("port-build"))).toBe(false);
  });

  it("says a queued job is waiting, whatever it is", () => {
    expect(phaseLabel(job("port-build", "queued", null))).toBe("Waiting for another task to finish");
  });
});

describe("readProgress", () => {
  it("reads the lighting bake's levels done (bake_all.py)", () => {
    const p = readProgress("probe-bake", out("[3/19] level_04: baked", "region-002.bake", "sparse.verify"));
    expect(p.percent).toBeCloseTo(15.79, 1);
  });

  it("reads ninja's [n/m] counter", () => {
    const p = readProgress("port-build", out("[1/4] a", "[1351/2273] Building C object CMakeFiles/x.o"));
    expect(p.percent).toBeCloseTo(59.43, 1);
  });

  it("reads cmake's and other percentages", () => {
    expect(readProgress("port-build", out("[ 45%] Building C object")).percent).toBe(45);
    expect(readProgress("install:rac1", out("copying 62 % done")).percent).toBe(62);
    expect(readProgress("x", out("[100/100] done")).percent).toBe(100);
  });

  it("takes the newest counter and never goes past 100", () => {
    expect(readProgress("x", out("10%", "noise", "20%")).percent).toBe(20);
    expect(readProgress("x", out("[5/4] odd")).percent).toBe(100);
  });

  it("ignores lines that only look like counters", () => {
    expect(readProgress("x", out("[error] 0/0", "see page [2/3] of the manual")).percent).toBeNull();
  });

  it("counts the levels the reader has finished, without a total", () => {
    const lines = out("level 03: 9 meshes", "level 00: 1 meshes");
    expect(readProgress("port-levels", lines)).toEqual({ percent: null, note: "2 levels read" });
    expect(readProgress("port-levels", out("level 03: x")).note).toBe("1 level read");
    expect(readProgress("port-build", lines).note).toBeNull();
  });

  it("has nothing to say about silent jobs", () => {
    expect(readProgress("play", out("hello"))).toEqual({ percent: null, note: null });
    expect(readProgress("play", [])).toEqual({ percent: null, note: null });
  });
});

describe("tailLines", () => {
  it("keeps the last lines, oldest first", () => {
    const lines = out(...Array.from({ length: 20 }, (_, i) => `l${i}`));
    expect(tailLines(lines)).toEqual(["l12", "l13", "l14", "l15", "l16", "l17", "l18", "l19"]);
    expect(tailLines(out("a"))).toEqual(["a"]);
  });
});

describe("backgroundStatus", () => {
  const job = (actionId: string, state: "running" | "queued", lines: string[] = []) => ({
    actionId,
    state,
    startedAt: 0,
    title: "Get the levels ready",
    lines: lines.map((line) => ({ line })),
  });

  it("counts a disc image's copy in percent", () => {
    expect(backgroundStatus(undefined, 0, { name: "game.chd", copied: 1, total: 4 })).toEqual({
      title: "Preparing your disc",
      text: "game.chd",
      percent: 25,
    });
  });

  it("follows the running job, but not the game", () => {
    expect(backgroundStatus(job("port-levels", "running", ["[ 45%] level 3"]), 0, null)).toEqual({
      title: "Reading the levels from your disc",
      text: "Get the levels ready",
      percent: 45,
    });
    expect(backgroundStatus(job("play", "running"), 0, null)).toBeNull();
  });

  it("stays between the steps of a chain and ends after the last", () => {
    expect(backgroundStatus(undefined, 1, null)?.percent).toBeNull();
    expect(backgroundStatus(undefined, 0, { name: "x.iso", copied: 4, total: 4 })).toBeNull();
  });
});
