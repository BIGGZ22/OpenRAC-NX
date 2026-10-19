import { describe, expect, it } from "vitest";
import { distance, nearest } from "./pad";

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
