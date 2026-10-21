// What the Mods tab says about a game's native mods (core/src/plugins.rs, docs/port/MODS.md): each
// one's state in a few words, and the control each of its settings gets (launcher/docs/STYLE.md: a
// switch for on/off, a segmented control for two or three choices, radios for more).
import type { PluginSetting, PluginView } from "./api";

export type PluginTone = "ok" | "off" | "warn" | "err";

/** A mod's state, as a dot and a few words. Purist mode loads none, whatever is switched on. */
export function pluginState(mod: PluginView, purist: boolean): { text: string; tone: PluginTone } {
  if (!mod.enabled) return { text: "Off", tone: "off" };
  if (purist) return { text: "Not in purist mode", tone: "off" };
  switch (mod.state) {
    case "ready":
      return { text: "On", tone: "ok" };
    case "failed":
      return { text: "Not loaded", tone: "err" };
    case "refused":
      return { text: "Cannot load", tone: "err" };
    default:
      return { text: "Off", tone: "off" };
  }
}

/** What a mod runs, as a dot and a few words: a native mod is a program on this computer, a script
 *  mod runs in the game's sandbox (docs/port/MODS.md, section 11), a pack runs nothing. */
export function modKind(kind: string): { text: string; tone: PluginTone } {
  switch (kind) {
    case "native":
      return { text: "Runs code on this computer", tone: "warn" };
    case "script":
      return { text: "Runs scripts (sandboxed)", tone: "ok" };
    default:
      return { text: "Textures and models", tone: "off" };
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
  const { min, max } = setting;
  if (min !== undefined && max !== undefined) return `${min} to ${max}`;
  if (min !== undefined) return `${min} or more`;
  if (max !== undefined) return `up to ${max}`;
  return "";
}

/** The other mods it changes the same part of the game with, in one sentence; empty when none. */
export function sharedWith(mod: PluginView): string {
  const names = [...new Set(mod.sharedHooks.flatMap((h) => h.with))];
  if (names.length === 0) return "";
  const list = names.length === 1 ? names[0] : `${names.slice(0, -1).join(", ")} and ${names.at(-1)}`;
  return `Changes the same part of the game as ${list}: the one lower in the list goes first.`;
}
