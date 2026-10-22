import { describe, expect, it } from "vitest";
import type { PluginSetting } from "./api";
import {
  formatValue,
  hasRange,
  isDefault,
  parseTyped,
  percentOf,
  settingDecimals,
  snapValue,
  stepOf,
} from "./settingValues";

const setting = (over: Partial<PluginSetting>): PluginSetting => ({
  key: "k",
  name: "K",
  type: "float",
  default: 1,
  value: 1,
  ...over,
});

// The difficulty mod's Enemy count, Enemy speed and Weapon prices; keyboard-mouse's Stick ramp.
const enemies = setting({ min: 0.25, max: 4, step: 0.25 });
const speed = setting({ min: 0.5, max: 2, step: 0.05 });
const prices = setting({ min: 0.1, max: 5, step: 0.05 });
const ramp = setting({ type: "int", min: 0, max: 300, default: 60, value: 60 });

describe("hasRange", () => {
  it("draws a slider only for a number with both ends", () => {
    expect(hasRange(enemies)).toBe(true);
    expect(hasRange(ramp)).toBe(true);
    expect(hasRange(setting({ min: 0 }))).toBe(false);
    expect(hasRange(setting({ type: "string", min: 0, max: 1 }))).toBe(false);
    expect(hasRange(setting({ min: 1, max: 1 }))).toBe(false);
  });
});

describe("stepOf", () => {
  it("takes the manifest's step, 1 for an int, else a plain hundredth of the range", () => {
    expect(stepOf(enemies)).toBe(0.25);
    expect(stepOf(ramp)).toBe(1);
    expect(stepOf(setting({ type: "int", step: 5 }))).toBe(5);
    expect(stepOf(setting({ min: 0, max: 2 }))).toBe(0.02);
    expect(stepOf(setting({ min: 0, max: 10 }))).toBeCloseTo(0.1);
    expect(stepOf(setting({ min: 0, max: 300 }))).toBe(5);
  });
});

describe("snapValue", () => {
  it("clamps to the ends", () => {
    expect(snapValue(enemies, 9)).toBe(4);
    expect(snapValue(enemies, -1)).toBe(0.25);
    expect(snapValue(ramp, 400)).toBe(300);
  });

  it("snaps to the steps counted from min", () => {
    expect(snapValue(enemies, 1.1)).toBe(1);
    expect(snapValue(enemies, 1.13)).toBe(1.25);
    expect(snapValue(prices, 0.12)).toBe(0.1);
    expect(snapValue(prices, 0.13)).toBe(0.15);
    expect(snapValue(setting({ min: 1, max: 10, step: 2 }), 4)).toBe(5);
  });

  it("never saves a float's error", () => {
    expect(snapValue(speed, 0.1 + 0.2 + 0.85)).toBe(1.15);
    expect(snapValue(speed, 1.15)).toBe(1.15);
    expect(String(snapValue(prices, 0.7000000001))).toBe("0.7");
  });

  it("keeps an int whole", () => {
    expect(snapValue(ramp, 61.6)).toBe(62);
    expect(Number.isInteger(snapValue(ramp, 12.4))).toBe(true);
  });

  it("puts something that is not a number back to the default", () => {
    expect(snapValue(enemies, Number.NaN)).toBe(1);
  });
});

describe("parseTyped", () => {
  it("takes what is typed, snapped and clamped, a comma for the point too", () => {
    expect(parseTyped(enemies, "2")).toBe(2);
    expect(parseTyped(enemies, " 1,5 ")).toBe(1.5);
    expect(parseTyped(enemies, "1.6")).toBe(1.5);
    expect(parseTyped(enemies, "12")).toBe(4);
    expect(parseTyped(ramp, "75.4")).toBe(75);
  });

  it("refuses what is not a number yet", () => {
    for (const typed of ["", " ", "-", ".", "abc", "1..2"]) expect(parseTyped(enemies, typed)).toBeNull();
  });
});

describe("formatValue and percentOf", () => {
  it("write a value with the setting's decimals", () => {
    expect(settingDecimals(enemies)).toBe(2);
    expect(formatValue(enemies, 1)).toBe("1.00");
    expect(formatValue(setting({ min: 0.5, max: 2, step: 0.1 }), 1.2)).toBe("1.2");
    expect(formatValue(ramp, 60)).toBe("60");
    expect(formatValue(ramp, "x")).toBe("");
  });

  it("place a value on the track", () => {
    expect(percentOf(ramp, 150)).toBe(50);
    expect(percentOf(ramp, -5)).toBe(0);
    expect(percentOf(ramp, 999)).toBe(100);
    expect(percentOf(setting({}), 1)).toBe(0);
  });

  it("tell a default", () => {
    expect(isDefault(enemies)).toBe(true);
    expect(isDefault(enemies, 2)).toBe(false);
  });
});
