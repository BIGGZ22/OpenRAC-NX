import { describe, expect, it } from "vitest";
import type { ReportEntry } from "./api";
import { duration, headline, place, stopText, unseenRecent, when } from "./reports";

const crash: ReportEntry = {
  kind: "crash",
  file: "crashes/crash-2026-10-17_20-08-29.txt",
  program: "openrac-rac1-pal",
  time: 0,
  duration: null,
  reason: "access (SIGSEGV)",
  level: 6,
  planet: "Batalia",
  crashed: true,
  session: null,
  size: 1,
};

describe("reports", () => {
  it("says what happened in a player's words", () => {
    expect(headline(crash)).toBe("Crashed on Batalia");
    expect(headline({ ...crash, planet: null })).toBe("Crashed in level 6");
    expect(headline({ ...crash, planet: null, level: null })).toBe("Crashed in the menus");
    const session = { ...crash, kind: "session" as const, crashed: false, duration: 23 * 60 };
    expect(headline(session)).toBe("Played 23 minutes on Batalia");
    expect(headline({ ...session, crashed: true })).toBe("Played 23 minutes, then crashed on Batalia");
    expect(headline({ ...session, level: null, planet: null, duration: 30 })).toBe("Played less than a minute");
    expect(place({ planet: null, level: null })).toBe("in the menus");
  });

  it("writes durations", () => {
    expect(duration(59)).toBe("less than a minute");
    expect(duration(60)).toBe("1 minute");
    expect(duration(3600)).toBe("1 hour");
    expect(duration(74 * 60)).toBe("1 hour 14 minutes");
    expect(duration(2 * 3600 + 60)).toBe("2 hours 1 minute");
  });

  it("dates rows relative to today", () => {
    const now = new Date(2026, 9, 17, 21, 0).getTime();
    expect(when(new Date(2026, 9, 17, 20, 8).getTime() / 1000, now, "en-GB")).toBe("Today at 20:08");
    expect(when(new Date(2026, 9, 16, 18, 0).getTime() / 1000, now, "en-GB")).toBe("Yesterday at 18:00");
    expect(when(new Date(2026, 9, 14, 10, 0).getTime() / 1000, now, "en-GB")).toBe("14 October at 10:00");
    expect(when(new Date(2025, 9, 14, 10, 0).getTime() / 1000, now, "en-GB")).toBe("14 October 2025 at 10:00");
    // No leading zero on the hour.
    expect(when(new Date(2026, 9, 17, 17, 1).getTime() / 1000, now, "en-CA")).toBe("Today at 5:01 p.m.");
  });

  it("copies the last lines when there is no report", () => {
    const text = stopText(
      { game: "rac1", versionKey: "rac1/pal", code: 1, at: 0, crash: null, lines: ["[error] no window: x"] },
      "Ratchet & Clank",
      "0.7.1",
    );
    expect(text).toContain("exit code 1");
    expect(text).toContain("[error] no window: x");
  });
});

describe("unseenRecent", () => {
  const at = (time: number) => ({ ...crash, time });
  it("hides the crashes a player dismissed, and shows a newer one", () => {
    const list = [at(100), at(200), at(300)];
    expect(unseenRecent(list, 0, 0)).toHaveLength(3);
    expect(unseenRecent(list, 200, 0).map((e) => e.time)).toEqual([300]);
    expect(unseenRecent(list, 300, 0)).toEqual([]);
  });

  it("leaves out the old ones", () => {
    expect(unseenRecent([at(100), at(200)], 0, 150).map((e) => e.time)).toEqual([200]);
  });
});
