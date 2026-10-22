import { afterEach, beforeEach, describe, expect, it, vi } from "vitest";
import { Saver } from "./autosave";

/** A save the test answers by hand: resolve or reject the oldest call. */
function slowSave() {
  const calls: { key: string; value: unknown; done: () => void; fail: (e: unknown) => void }[] = [];
  const save = vi.fn(
    (key: string, value: unknown) =>
      new Promise<void>((done, fail) => {
        calls.push({ key, value, done, fail });
      }),
  );
  return { save, calls };
}

/** Lets the saver's promise chain move on. */
const settle = async () => {
  for (let i = 0; i < 5; i++) await Promise.resolve();
};

describe("Saver", () => {
  beforeEach(() => {
    vi.useFakeTimers();
  });
  afterEach(() => {
    vi.useRealTimers();
  });

  it("saves a dragged value once, its last, after it settles", async () => {
    const save = vi.fn(() => Promise.resolve());
    const saver = new Saver({ save });
    saver.set("speed", 1.1, 300);
    vi.advanceTimersByTime(200);
    saver.set("speed", 1.2, 300);
    vi.advanceTimersByTime(200);
    saver.set("speed", 1.3, 300);
    expect(save).not.toHaveBeenCalled();
    expect(saver.state).toBe("waiting");
    expect(saver.value("speed", 1)).toBe(1.3);
    vi.advanceTimersByTime(300);
    await settle();
    expect(save).toHaveBeenCalledTimes(1);
    expect(save).toHaveBeenCalledWith("speed", 1.3);
    expect(saver.state).toBe("saved");
    expect(saver.holds("speed")).toBe(false);
  });

  it("saves a switch at once, without waiting", async () => {
    const save = vi.fn(() => Promise.resolve());
    const saver = new Saver({ save });
    expect(saver.state).toBe("idle");
    saver.set("chaos", true);
    await settle();
    expect(save).toHaveBeenCalledWith("chaos", true);
  });

  it("keeps showing a value until it is saved, and one moved on since after", async () => {
    const { save, calls } = slowSave();
    const saver = new Saver({ save });
    saver.set("a", 1);
    await settle();
    expect(saver.state).toBe("saving");
    saver.set("a", 2, 100);
    calls[0]?.done();
    await settle();
    // The first save is done, but the player has moved it on: 2 still shows.
    expect(saver.value("a", 0)).toBe(2);
    vi.advanceTimersByTime(100);
    await settle();
    calls[1]?.done();
    await settle();
    expect(saver.holds("a")).toBe(false);
    expect(save.mock.calls.map((c) => c[1])).toEqual([1, 2]);
  });

  it("sends one save at a time, in the order the values settled", async () => {
    const { save, calls } = slowSave();
    const saver = new Saver({ save });
    saver.set("a", 1);
    saver.set("b", 2);
    saver.set("c", 3);
    await settle();
    expect(calls.map((c) => c.key)).toEqual(["a"]);
    calls[0]?.done();
    await settle();
    expect(calls.map((c) => c.key)).toEqual(["a", "b"]);
    calls[1]?.done();
    await settle();
    calls[2]?.done();
    await settle();
    expect(calls.map((c) => c.key)).toEqual(["a", "b", "c"]);
    expect(saver.state).toBe("saved");
  });

  it("says a refusal, drops the refused value and goes on", async () => {
    const errors: string[] = [];
    let fail = true;
    const saver = new Saver({
      save: () => (fail ? Promise.reject(new Error("out of range")) : Promise.resolve()),
      onerror: (key, e) => errors.push(`${key}: ${(e as Error).message}`),
    });
    saver.set("a", 99);
    await settle();
    expect(errors).toEqual(["a: out of range"]);
    expect(saver.state).toBe("failed");
    expect(saver.holds("a")).toBe(false);
    fail = false;
    saver.set("a", 2);
    await settle();
    expect(saver.state).toBe("saved");
  });

  it("sends what waits at once on flush, as the page is left", async () => {
    const save = vi.fn(() => Promise.resolve());
    const saved: string[] = [];
    const saver = new Saver({ save, onsaved: (key) => saved.push(key) });
    saver.set("seed", "abc", 600);
    await saver.flush();
    expect(save).toHaveBeenCalledWith("seed", "abc");
    expect(saved).toEqual(["seed"]);
    vi.advanceTimersByTime(1000);
    await settle();
    expect(save).toHaveBeenCalledTimes(1);
  });

  it("tells the page of every change", () => {
    const onchange = vi.fn();
    const saver = new Saver({ save: () => Promise.resolve(), onchange });
    saver.set("a", 1, 100);
    expect(onchange).toHaveBeenCalled();
    expect(saver.held()).toEqual({ a: 1 });
    saver.drop();
    expect(saver.held()).toEqual({});
  });
});
