import { describe, expect, it } from "vitest";
import { jobDuration, jobTranscript } from "./jobText";

describe("jobDuration", () => {
  it("is empty for a job that has not started", () => {
    expect(jobDuration({ startedAt: null, endedAt: null }, 5000)).toBe("");
  });

  it("counts a running job up to now", () => {
    expect(jobDuration({ startedAt: 1000, endedAt: null }, 126_000)).toBe("2 min 05 s");
  });

  it("stops at the end of a finished one", () => {
    expect(jobDuration({ startedAt: 1000, endedAt: 5000 }, 999_999)).toBe("4 s");
  });
});

describe("jobTranscript", () => {
  it("puts the command first, then each line printed", () => {
    const lines = [
      { stream: "stdout" as const, line: "building" },
      { stream: "stderr" as const, line: "warning: x" },
    ];
    expect(jobTranscript({ command: "make all", lines })).toBe("$ make all\nbuilding\nwarning: x");
  });

  it("is the command alone when nothing was printed", () => {
    expect(jobTranscript({ command: "make", lines: [] })).toBe("$ make");
  });
});
