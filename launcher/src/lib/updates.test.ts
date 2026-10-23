import { afterEach, describe, expect, it, vi } from "vitest";
import { checkGitHubRelease, isNewerVersion, offerUpdate, PUT_OFF_MS } from "./updates";

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

  it("passes over a latest release that is not the launcher's", async () => {
    const stable = { repository: "someone/OpenRAC", channel: null, manifest: "", page: "" };
    const sdk = JSON.stringify({ tag_name: "sdk-v1.0.1", name: "Mod SDK 1.0.1" });
    const list = JSON.stringify([
      { tag_name: "sdk-v1.0.1", name: "Mod SDK 1.0.1" },
      { tag_name: "launcher-v0.9.0", name: "draft", draft: true },
      { tag_name: "launcher-v0.8.4", name: "OpenRAC Launcher v0.8.4" },
    ]);
    vi.stubGlobal(
      "fetch",
      vi.fn((url: string) => Promise.resolve(new Response(url.endsWith("/latest") ? sdk : list, { status: 200 }))),
    );
    expect((await checkGitHubRelease("0.8.3", stable))?.version).toBe("0.8.4");
    expect(await checkGitHubRelease("0.8.4", stable)).toBeNull();
  });
});

describe("offerUpdate", () => {
  const now = 1_800_000_000_000;

  it("opens the window for a version not put off", () => {
    expect(offerUpdate("0.9.0", null, now)).toBe(true);
    expect(offerUpdate("0.9.1", { version: "0.9.0", at: now }, now)).toBe(true);
  });

  it("waits after Later, then asks again", () => {
    expect(offerUpdate("0.9.0", { version: "0.9.0", at: now - 1000 }, now)).toBe(false);
    expect(offerUpdate("0.9.0", { version: "0.9.0", at: now - PUT_OFF_MS }, now)).toBe(true);
  });
});
