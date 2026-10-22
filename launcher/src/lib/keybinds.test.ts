import { describe, expect, it } from "vitest";
import keysH from "../../../sdk/include/openrac/keys.h?raw";
import type { PluginSetting } from "./api";
import {
  BIND_NAMES,
  bindLabel,
  bindOfKey,
  bindOfMouse,
  bindOfWheel,
  formatBinds,
  keybindConflicts,
  parseBinds,
  sharedBinds,
  withBind,
  withoutBind,
} from "./keybinds";

const bind = (key: string, value: string, over: Partial<PluginSetting> = {}): PluginSetting => ({
  key,
  name: key.charAt(0).toUpperCase() + key.slice(1),
  type: "keybind",
  default: value,
  value,
  group: "play",
  ...over,
});

describe("bind names", () => {
  it("are keys.h's, in its order", () => {
    const names = [...keysH.matchAll(/\{"([A-Za-z0-9]+)", /g)].map((m) => m[1]);
    expect(BIND_NAMES).toEqual(names);
  });

  it("parse and format, leaving out what is no name and what is there twice", () => {
    expect(parseBinds("KeyW, ArrowUp")).toEqual(["KeyW", "ArrowUp"]);
    expect(parseBinds("")).toEqual([]);
    expect(parseBinds("W,KeyW,KeyW")).toEqual(["KeyW"]);
    expect(parseBinds(3)).toEqual([]);
    expect(formatBinds(["KeyF", "MouseMiddle"])).toBe("KeyF,MouseMiddle");
  });

  it("read as key caps", () => {
    expect(bindLabel("KeyW")).toBe("W");
    expect(bindLabel("Digit1")).toBe("1");
    expect(bindLabel("Space")).toBe("Space");
    expect(bindLabel("ShiftLeft")).toBe("Left Shift");
    expect(bindLabel("MouseLeft")).toBe("Left click");
    expect(bindLabel("MouseRight")).toBe("Right click");
    expect(bindLabel("WheelUp")).toBe("Wheel up");
    expect(bindLabel("Numpad0")).toBe("Num 0");
    expect(bindLabel("ArrowUp")).toBe("Up");
    expect(bindLabel("F11")).toBe("F11");
    for (const n of BIND_NAMES) expect(bindLabel(n).length).toBeGreaterThan(0);
  });
});

describe("capture", () => {
  it("names keys by where they are, mouse buttons and the wheel", () => {
    expect(bindOfKey({ code: "KeyW" })).toBe("KeyW");
    expect(bindOfKey({ code: "Escape" })).toBe("Escape");
    expect(bindOfKey({ code: "" })).toBeNull();
    expect(bindOfKey({ code: "Unidentified" })).toBeNull();
    expect(bindOfKey({ code: "Fn" })).toBeNull();
    expect([0, 1, 2, 3, 4, 5].map((button) => bindOfMouse({ button }))).toEqual([
      "MouseLeft",
      "MouseMiddle",
      "MouseRight",
      "MouseBack",
      "MouseForward",
      null,
    ]);
    expect(bindOfWheel({ deltaX: 0, deltaY: -100 })).toBe("WheelUp");
    expect(bindOfWheel({ deltaX: 0, deltaY: 3 })).toBe("WheelDown");
    expect(bindOfWheel({ deltaX: -5, deltaY: 1 })).toBe("WheelLeft");
    expect(bindOfWheel({ deltaX: 5, deltaY: 0 })).toBe("WheelRight");
    expect(bindOfWheel({ deltaX: 0, deltaY: 0 })).toBeNull();
  });

  it("puts a bind in a slot, swapping rather than binding one input twice", () => {
    expect(withBind([], 0, "KeyW")).toEqual(["KeyW"]);
    expect(withBind(["KeyW"], 1, "ArrowUp")).toEqual(["KeyW", "ArrowUp"]);
    expect(withBind(["KeyW", "ArrowUp"], 0, "ArrowUp")).toEqual(["ArrowUp", "KeyW"]);
    expect(withBind(["KeyW"], 1, "KeyW")).toEqual(["KeyW"]);
    expect(withBind(["KeyW", "ArrowUp"], 0, "KeyQ")).toEqual(["KeyQ", "ArrowUp"]);
    expect(withoutBind(["KeyW", "ArrowUp"], 0)).toEqual(["ArrowUp"]);
  });
});

describe("conflicts", () => {
  const settings = [
    bind("jump", "Space,KeyQ"),
    bind("fire", "KeyQ"),
    bind("back", "Space", { group: "menu" }),
    { key: "speed", name: "Speed", type: "float", default: 1, value: 1 } as PluginSetting,
  ];

  it("flag keybinds of one group that share an input, both ways", () => {
    expect(keybindConflicts(settings)).toEqual({ jump: ["Fire"], fire: ["Jump"] });
    expect(keybindConflicts([bind("jump", "Space"), bind("fire", "MouseLeft")])).toEqual({});
    expect(sharedBinds(settings, "jump")).toEqual(["KeyQ"]);
    expect(sharedBinds(settings, "back")).toEqual([]);
  });
});
