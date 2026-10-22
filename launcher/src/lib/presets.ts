// A mod's presets (its manifest's "presets", mod interface 1.2): buttons over its settings that set
// several at once, of any type: the sliders, a choice, a switch, text, keybinds. A preset sets the
// settings it names; the others it covers go back to their defaults. It covers every keybind of
// the mod and each setting one of its presets names, so a keybind preset leaves a mod's switches
// alone and a preset of sliders the keys (core/src/plugins.rs checks the values).
import type { PluginPreset, PluginSetting } from "./api";
import { formatBinds, parseBinds } from "./keybinds";

/** The keys of the settings the mod's presets cover. */
export function presetScope(settings: PluginSetting[], presets: PluginPreset[]): Set<string> {
  const named = new Set(presets.flatMap((p) => Object.keys(p.values)));
  return new Set(settings.filter((s) => s.type === "keybind" || named.has(s.key)).map((s) => s.key));
}

/** A value as the setting holds it, to compare: a keybind's binds in order, without repeats. */
const held = (s: PluginSetting, value: unknown): unknown =>
  s.type === "keybind" ? formatBinds(parseBinds(value)) : value;

/** The values a preset (null: the defaults) gives each setting it covers: its own, else the
 *  setting's default. Only those that differ from the value now. */
export function presetChanges(
  settings: PluginSetting[],
  presets: PluginPreset[],
  preset: PluginPreset | null,
): Record<string, unknown> {
  const scope = presetScope(settings, presets);
  const out: Record<string, unknown> = {};
  for (const s of settings) {
    if (!scope.has(s.key)) continue;
    const wanted = held(s, preset && s.key in preset.values ? preset.values[s.key] : s.default);
    if (wanted !== held(s, s.value)) out[s.key] = wanted;
  }
  return out;
}

/** Whether the settings are as a preset (null: the defaults) would set them. */
export const isPreset = (settings: PluginSetting[], presets: PluginPreset[], preset: PluginPreset | null): boolean =>
  Object.keys(presetChanges(settings, presets, preset)).length === 0;

/** Whether the presets set keybinds only (the row is then the mod's controls). */
export const presetsBindOnly = (settings: PluginSetting[], presets: PluginPreset[]): boolean =>
  presets.every((p) => Object.keys(p.values).every((k) => settings.find((s) => s.key === k)?.type === "keybind"));
