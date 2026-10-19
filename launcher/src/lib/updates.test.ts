import { afterEach, describe, expect, it, vi } from "vitest";
import { checkGitHubRelease, isNewerVersion } from "./updates";

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

describe("checkGitHubRelease", () => {
  const source = { repository: "someone/OpenRAC", channel: "launcher-dev-main", manifest: "", page: "" };
  afterEach(() => vi.unstubAllGlobals());

  it("says so when nothing is published where the launcher looks", async () => {
    vi.stubGlobal("fetch", vi.fn().mockResolvedValue(new Response("", { status: 404 })));
    await expect(checkGitHubRelease("0.8.0-dev.3", source)).rejects.toThrow(/no release/);
  });

  it("reads a branch's version from the release's title", async () => {
    const body = JSON.stringify({ tag_name: "launcher-dev-main", name: "OpenRAC Launcher 0.8.0-dev.4 (main)" });
    vi.stubGlobal(
      "fetch",
      vi.fn(() => Promise.resolve(new Response(body, { status: 200 }))),
    );
    expect((await checkGitHubRelease("0.8.0-dev.3", source))?.version).toBe("0.8.0-dev.4");
    expect(await checkGitHubRelease("0.8.0-dev.4", source)).toBeNull();
  });
});
