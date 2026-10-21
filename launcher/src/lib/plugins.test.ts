import { describe, expect, it } from "vitest";
import type { PluginSetting, PluginView } from "./api";
import { modKind, pluginState, settingControl, settingNumber, settingRange, sharedWith } from "./plugins";

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
    expect(settingControl(setting({ type: "choice", choices: choices(4) }))).toBe("radios");
    expect(settingControl(setting({ type: "int" }))).toBe("number");
    expect(settingControl(setting({ type: "float" }))).toBe("number");
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
  it("warns about native code and says scripts run sandboxed", () => {
    expect(modKind("native")).toEqual({ text: "Runs code on this computer", tone: "warn" });
    expect(modKind("script")).toEqual({ text: "Runs scripts (sandboxed)", tone: "ok" });
    expect(modKind("pack").tone).toBe("off");
    expect(modKind("data").text).toBe("Textures and models");
  });
});
