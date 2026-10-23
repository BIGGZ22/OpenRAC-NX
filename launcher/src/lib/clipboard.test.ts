import { afterEach, describe, expect, it, vi } from "vitest";
import { app } from "./app.svelte";
import { copyText } from "./clipboard";

afterEach(() => {
  vi.unstubAllGlobals();
  app.toasts.length = 0;
});

describe("copyText", () => {
  it("writes the text and says nothing", async () => {
    const writeText = vi.fn(() => Promise.resolve());
    vi.stubGlobal("navigator", { clipboard: { writeText } });
    expect(await copyText("$ make", "refused")).toBe(true);
    expect(writeText).toHaveBeenCalledWith("$ make");
    expect(app.toasts).toHaveLength(0);
  });

  it("shows the caller's words when the browser refuses", async () => {
    vi.stubGlobal("navigator", { clipboard: { writeText: () => Promise.reject(new Error("denied")) } });
    expect(await copyText("x", "Select it and copy it yourself.")).toBe(false);
    expect(app.toasts.map((t) => [t.text, t.kind])).toEqual([["Select it and copy it yourself.", "error"]]);
  });

  it("treats a browser without a clipboard as a refusal", async () => {
    vi.stubGlobal("navigator", {});
    expect(await copyText("x", "No clipboard.")).toBe(false);
    expect(app.toasts.map((t) => t.text)).toEqual(["No clipboard."]);
  });
});
