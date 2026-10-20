import { describe, expect, it } from "vitest";
import { distance, nearest, pollGamepads } from "./pad";

const box = (left: number, top: number, width = 100, height = 40) => ({
  left,
  top,
  right: left + width,
  bottom: top + height,
});

describe("distance", () => {
  const from = box(100, 100);

  it("is null for what is not that way", () => {
    expect(distance(from, box(100, 0), "down")).toBeNull();
    expect(distance(from, box(100, 200), "up")).toBeNull();
    expect(distance(from, box(300, 100), "left")).toBeNull();
    expect(distance(from, box(0, 100), "right")).toBeNull();
    // Beside it, not below it.
    expect(distance(from, box(250, 100), "down")).toBeNull();
  });

  it("counts the gap, and twice what is off to the side", () => {
    expect(distance(from, box(100, 160), "down")).toBe(20);
    expect(distance(from, box(260, 160), "down")).toBe(20 + 2 * 60);
    expect(distance(from, box(220, 100), "right")).toBe(20);
  });
});

describe("nearest", () => {
  it("takes the control straight ahead over a nearer one diagonally", () => {
    const from = box(100, 100);
    const items = [
      { item: "diagonal", box: box(230, 150) },
      { item: "below", box: box(100, 200) },
      { item: "above", box: box(100, 0) },
    ];
    expect(nearest(from, items, "down")).toBe("below");
    expect(nearest(from, items, "up")).toBe("above");
    expect(nearest(from, items, "right")).toBe("diagonal");
    // In line first: the button beside it, though one above-right is nearer.
    const row = [
      { item: "corner", box: box(205, 40) },
      { item: "beside", box: box(400, 100) },
    ];
    expect(nearest(from, row, "right")).toBe("beside");
    expect(nearest(from, items, "left")).toBeNull();
  });

  it("moves along a row of tiles", () => {
    const tiles = [0, 1, 2].map((i) => ({ item: i, box: box(i * 120, 0) }));
    expect(nearest(box(0, 0), tiles.slice(1), "right")).toBe(1);
    expect(nearest(box(240, 0), tiles.slice(0, 2), "left")).toBe(1);
  });
});

describe("pollGamepads", () => {
  // A standard-mapping controller with buttons `down` held (A is 0, the d-pad's right is 15).
  const pad = (...down: number[]) =>
    ({
      mapping: "standard",
      axes: [0, 0, 0, 0],
      buttons: Array.from({ length: 17 }, (_, i) => ({ pressed: down.includes(i), touched: false, value: 0 })),
    }) as unknown as Gamepad;

  it("notes what is held, acting on none of it, while the launcher is not in front", () => {
    // Acting would reach the document, which this test has none of: nothing throws, nothing acts.
    const held = pollGamepads(new Set(), [pad(0, 15), null], false);
    expect([...held].sort()).toEqual(["accept", "right"]);
    // Still held when the launcher comes back: no new press, so nothing to act on.
    expect([...pollGamepads(held, [pad(0, 15)], false)].sort()).toEqual(["accept", "right"]);
  });

  it("reads only standard-mapping controllers, and the left stick past half way", () => {
    const stick = { mapping: "standard", axes: [-0.8, 0.6], buttons: [] } as unknown as Gamepad;
    const other = { mapping: "", axes: [1, 1], buttons: [{ pressed: true }] } as unknown as Gamepad;
    expect([...pollGamepads(new Set(), [stick, other], false)].sort()).toEqual(["down", "left"]);
  });
});
