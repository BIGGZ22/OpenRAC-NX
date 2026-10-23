import { describe, expect, it } from "vitest";
import type { PluginSetting, PluginView } from "./api";
import {
  modBadge,
  modHint,
  modKind,
  pluginState,
  settingControl,
  settingGroups,
  settingNumber,
  settingRange,
  sharedWith,
} from "./plugins";

const mod = (over: Partial<PluginView> = {}): PluginView => ({
  id: "lock-strafe",
  name: "Lock-strafe",
  version: "1.0.0",
  author: "",
  description: "",
  homepage: "",
  license: "",
  enabled: true,
  state: "ready",
  reason: "",
  blocked: "",
  settings: [],
  hooks: [],
  sharedHooks: [],
  native: true,
  kind: "native",
  path: "",
  ...over,
});

const setting = (over: Partial<PluginSetting>): PluginSetting => ({
  key: "k",
  name: "K",
  type: "bool",
  default: false,
  value: false,
  ...over,
});

describe("pluginState", () => {
  it("says on, off, refused and not loaded, and purist mode over all of them", () => {
    expect(pluginState(mod(), false)).toEqual({ text: "On", tone: "ok" });
    expect(pluginState(mod({ enabled: false, state: "off" }), false).tone).toBe("off");
    expect(pluginState(mod({ state: "refused" }), false)).toEqual({ text: "Cannot load", tone: "err" });
    expect(pluginState(mod({ state: "failed" }), false).text).toBe("Not loaded");
    expect(pluginState(mod(), true).tone).toBe("off");
    expect(pluginState(mod({ enabled: false }), true).text).toBe("Off");
  });
});

describe("settings", () => {
  it("get the control STYLE.md asks for", () => {
    const choices = (n: number) => Array.from({ length: n }, (_, i) => ({ value: `${i}`, name: `${i}` }));
    expect(settingControl(setting({ type: "bool" }))).toBe("switch");
    expect(settingControl(setting({ type: "choice", choices: choices(2) }))).toBe("segmented");
    expect(settingControl(setting({ type: "choice", choices: choices(3) }))).toBe("segmented");
    expect(settingControl(setting({ type: "choice", choices: choices(4) }))).toBe("segmented");
    expect(settingControl(setting({ type: "choice", choices: choices(5) }))).toBe("select");
    const long = [
      { value: "a", name: "Lock with weapons, always" },
      { value: "b", name: "Hold L2 or R2 to strafe" },
    ];
    expect(settingControl(setting({ type: "choice", choices: long }))).toBe("select");
    expect(settingControl(setting({ type: "keybind", default: "Space" }))).toBe("keybind");
    expect(settingControl(setting({ type: "int" }))).toBe("number");
    expect(settingControl(setting({ type: "float", min: 0 }))).toBe("number");
    expect(settingControl(setting({ type: "float", min: 0, max: 2 }))).toBe("slider");
    expect(settingControl(setting({ type: "int", min: 1, max: 30 }))).toBe("slider");
    expect(settingControl(setting({ type: "string" }))).toBe("text");
  });

  it("take only numbers in their range", () => {
    const int = setting({ type: "int", min: 50, max: 110 });
    expect(settingNumber(int, "80")).toBe(80);
    expect(settingNumber(int, "80.5")).toBeNull();
    expect(settingNumber(int, "40")).toBeNull();
    expect(settingNumber(int, "")).toBeNull();
    expect(settingNumber(int, "abc")).toBeNull();
    expect(settingNumber(setting({ type: "float", min: 0.5 }), "0.75")).toBe(0.75);
    expect(settingRange(int)).toBe("50 to 110");
    expect(settingRange(setting({ type: "float", min: 0.5 }))).toBe("0.5 or more");
    expect(settingRange(setting({ type: "int" }))).toBe("");
  });
});

describe("sharedWith", () => {
  it("names the other mods once each", () => {
    expect(sharedWith(mod())).toBe("");
    const shared = mod({
      sharedHooks: [
        { hook: "A", with: ["Camera tweaks"] },
        { hook: "B", with: ["Camera tweaks", "Fast walk"] },
      ],
    });
    expect(sharedWith(shared)).toBe(
      "Changes the same part of the game as Camera tweaks and Fast walk: the one lower in the list goes first.",
    );
  });
});

describe("modKind", () => {
  it("marks native code and says scripts run sandboxed", () => {
    expect(modKind("native")).toEqual({ text: "Runs code", tone: "warn" });
    expect(modKind("built-in")).toEqual({ text: "Experimental", tone: "warn" });
    expect(modKind("script")).toEqual({ text: "Sandboxed script", tone: "ok" });
    expect(modKind("pack").tone).toBe("off");
    expect(modKind("data").text).toBe("Textures and models");
  });

  it("explains each kind in its tooltip, trust for native code", () => {
    expect(modHint("native")).toContain("people you trust");
    expect(modHint("script")).toContain("sandbox");
    expect(modHint("data")).toContain("runs nothing");
  });
});

describe("settingGroups", () => {
  it("keeps the ungrouped settings first, then each group in the order it appears", () => {
    const groups = settingGroups([
      setting({ key: "sens", type: "float" }),
      setting({ key: "jump", type: "keybind", group: "play" }),
      setting({ key: "back", type: "keybind", group: "menu" }),
      setting({ key: "fire", type: "keybind", group: "play" }),
      setting({ key: "tilt", type: "choice" }),
    ]);
    expect(groups.map((g) => g.title)).toEqual(["General", "Play controls", "Menu controls"]);
    expect(groups[0]?.settings.map((s) => s.key)).toEqual(["sens", "tilt"]);
    expect(groups[1]?.settings.map((s) => s.key)).toEqual(["jump", "fire"]);
  });

  it("calls a mod's only section Settings, and a mixed group by its name", () => {
    expect(settingGroups([setting({ key: "a" })]).map((g) => g.title)).toEqual(["Settings"]);
    expect(settingGroups([setting({ key: "a", group: "camera_feel" })])[0]?.title).toBe("Camera feel");
  });
});

describe("modBadge", () => {
  it("says what is most pressing: cannot load, then an update, then what it runs", () => {
    expect(modBadge(mod({ state: "refused", reason: "too old" }), "1.1.0", false)).toMatchObject({
      text: "Cannot load",
      tone: "err",
      hint: "too old",
    });
    expect(modBadge(mod(), "1.1.0", false)).toMatchObject({ text: "Update 1.1.0", tone: "warn" });
    expect(modBadge(mod({ kind: "script" }), null, false).text).toBe("Sandboxed script");
    expect(modBadge(mod({ state: "refused" }), null, true).text).toBe("Runs code");
  });
});
