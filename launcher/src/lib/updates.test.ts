import { describe, expect, it } from "vitest";
import { isNewerVersion } from "./updates";

describe("isNewerVersion", () => {
  it("detects major updates", () => {
    expect(isNewerVersion("0.4.4", "1.0.0")).toBe(true);
    expect(isNewerVersion("1.0.0", "0.4.4")).toBe(false);
  });

  it("detects minor updates", () => {
    expect(isNewerVersion("0.4.4", "0.5.0")).toBe(true);
    expect(isNewerVersion("0.5.0", "0.4.4")).toBe(false);
  });

  it("detects patch updates", () => {
    expect(isNewerVersion("0.4.4", "0.4.5")).toBe(true);
    expect(isNewerVersion("0.4.4", "0.4.4")).toBe(false);
  });

  it("handles prefixes such as v or launcher-v", () => {
    expect(isNewerVersion("0.4.4", "v0.4.5")).toBe(true);
    expect(isNewerVersion("0.4.4", "launcher-v0.4.5")).toBe(true);
    expect(isNewerVersion("launcher-v0.4.4", "launcher-v0.4.4")).toBe(false);
  });

  it("orders the builds of a branch after one another and before their release", () => {
    expect(isNewerVersion("0.8.0-dev.9", "0.8.0-dev.10")).toBe(true);
    expect(isNewerVersion("0.8.0-dev.10", "0.8.0-dev.10")).toBe(false);
    expect(isNewerVersion("0.8.0-dev.10", "0.8.0-dev.9")).toBe(false);
    expect(isNewerVersion("0.8.0-dev.10", "0.8.0")).toBe(true);
    expect(isNewerVersion("0.8.0", "0.8.0-dev.10")).toBe(false);
  });
});
