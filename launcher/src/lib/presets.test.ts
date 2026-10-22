import { describe, expect, it } from "vitest";
import type { PluginPreset, PluginSetting } from "./api";
import { isPreset, presetChanges, presetScope, presetsBindOnly } from "./presets";

const bind = (key: string, value: string, over: Partial<PluginSetting> = {}): PluginSetting => ({
  key,
  name: key,
  type: "keybind",
  default: value,
  value,
  ...over,
});
const float = (key: string, value: number): PluginSetting => ({ key, name: key, type: "float", default: 1, value });

describe("presets", () => {
  it("set the keybinds a preset names, the others back to their defaults", () => {
    const moved = [bind("jump", "KeyJ", { default: "Space" }), bind("fire", "MouseLeft")];
    const arrows: PluginPreset = { id: "arrows", name: "Arrows", values: { fire: "ControlRight" } };
    expect(presetChanges(moved, [arrows], arrows)).toEqual({ jump: "Space", fire: "ControlRight" });
    expect(presetChanges(moved, [arrows], null)).toEqual({ jump: "Space" });
    expect(isPreset(moved, [arrows], null)).toBe(false);
    expect(isPreset([bind("jump", "Space")], [arrows], null)).toBe(true);
    expect(presetsBindOnly(moved, [arrows])).toBe(true);
  });

  it("fill sliders, choices and switches, and leave alone what no preset names", () => {
    const settings: PluginSetting[] = [
      float("damage", 1),
      float("speed", 1.5),
      { key: "chaos", name: "Chaos", type: "bool", default: false, value: false },
      { key: "mode", name: "Mode", type: "choice", default: "a", value: "a" },
      { key: "trace", name: "Trace", type: "bool", default: false, value: true },
    ];
    const hard: PluginPreset = { id: "hard", name: "Hard", values: { damage: 2, speed: 1.2, mode: "b" } };
    const chaos: PluginPreset = { id: "chaos", name: "Chaos", values: { chaos: true } };
    const presets = [hard, chaos];
    expect([...presetScope(settings, presets)]).toEqual(["damage", "speed", "chaos", "mode"]);
    expect(presetChanges(settings, presets, hard)).toEqual({ damage: 2, speed: 1.2, mode: "b" });
    expect(presetChanges(settings, presets, chaos)).toEqual({ speed: 1, chaos: true });
    expect(presetChanges(settings, presets, null)).toEqual({ speed: 1 });
    const asHard = settings.map((s) => (s.key in hard.values ? { ...s, value: hard.values[s.key] } : s));
    expect(isPreset(asHard, presets, hard)).toBe(true);
    expect(isPreset(asHard, presets, chaos)).toBe(false);
    expect(presetsBindOnly(settings, presets)).toBe(false);
  });
});
