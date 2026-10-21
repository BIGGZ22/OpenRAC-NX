// What the Mods tab says about a game's native mods (core/src/plugins.rs, docs/port/MODS.md): each
// one's state in a few words, and the control each of its settings gets (launcher/docs/STYLE.md: a
// switch for on/off, a segmented control for two or three choices, radios for more). The words are
// lib/modsText.ts's; here only which ones.
import type { PluginSetting, PluginView } from "./api";
import { KIND_LABEL, STATE_LABEL, rangeText, sharesWith, type Label, type ModTone } from "./modsText";

export type PluginTone = ModTone;

/** A mod's state, as a dot and a few words. Purist mode loads none, whatever is switched on. */
export function pluginState(mod: PluginView, purist: boolean): Label {
  if (!mod.enabled) return STATE_LABEL.off;
  if (purist) return STATE_LABEL.purist;
  switch (mod.state) {
    case "ready":
      return STATE_LABEL.on;
    case "failed":
      return STATE_LABEL.failed;
    case "refused":
      return STATE_LABEL.refused;
    default:
      return STATE_LABEL.off;
  }
}

/** What a mod runs, as a dot and a few words: a native mod is a program on this computer, a script
 *  mod runs in the game's sandbox (docs/port/MODS.md, section 11), a pack runs nothing. */
export function modKind(kind: string): Label {
  switch (kind) {
    case "native":
      return KIND_LABEL.native;
    case "script":
      return KIND_LABEL.script;
    default:
      return KIND_LABEL.pack;
  }
}

export type SettingControl = "switch" | "segmented" | "radios" | "number" | "text";

/** The control a setting is shown with. */
export function settingControl(setting: PluginSetting): SettingControl {
  switch (setting.type) {
    case "bool":
      return "switch";
    case "choice":
      return (setting.choices?.length ?? 0) <= 3 ? "segmented" : "radios";
    case "int":
    case "float":
      return "number";
    default:
      return "text";
  }
}

/** A number typed in a setting's field as the setting takes it, or null when it does not. */
export function settingNumber(setting: PluginSetting, typed: string): number | null {
  if (typed.trim() === "") return null;
  const n = Number(typed);
  if (!Number.isFinite(n)) return null;
  if (setting.type === "int" && !Number.isInteger(n)) return null;
  if (setting.min !== undefined && n < setting.min) return null;
  if (setting.max !== undefined && n > setting.max) return null;
  return n;
}

/** The range a number setting takes, for its line: "50 to 110". */
export function settingRange(setting: PluginSetting): string {
  return rangeText(setting.min, setting.max);
}

/** The other mods it changes the same part of the game with, in one sentence; empty when none. */
export function sharedWith(mod: PluginView): string {
  return sharesWith([...new Set(mod.sharedHooks.flatMap((h) => h.with))]);
}
