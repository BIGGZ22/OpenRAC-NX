// What the Mods tab says about a game's native mods (core/src/plugins.rs, docs/port/MODS.md): each
// one's state in a few words, the one badge its row shows, its settings' sections, and the control
// each setting gets (launcher/docs/STYLE.md: a switch for on/off, a segmented control for a few short
// choices, a select for more, a slider for a number with both ends). The words are
// lib/modsText.ts's; here only which ones.
import type { PluginSetting, PluginView } from "./api";
import { hasRange } from "./settingValues";
import {
  BADGE,
  GROUP_TITLE,
  KIND_HINT,
  KIND_LABEL,
  STATE_LABEL,
  rangeText,
  sharesWith,
  type Label,
  type ModTone,
} from "./modsText";

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

/** What a mod's kind means, in a sentence: its label's tooltip. */
export function modHint(kind: string): string {
  return kind === "native" ? KIND_HINT.native : kind === "script" ? KIND_HINT.script : KIND_HINT.pack;
}

export type SettingControl = "switch" | "segmented" | "select" | "slider" | "number" | "text" | "keybind";

/** A choice fits a segmented control when it has two to four short choices; else a select. */
const segmentable = (s: PluginSetting): boolean => {
  const choices = s.choices ?? [];
  return choices.length <= 4 && choices.reduce((n, c) => n + c.name.length, 0) <= 44;
};

/** The control a setting is shown with. */
export function settingControl(setting: PluginSetting): SettingControl {
  switch (setting.type) {
    case "bool":
      return "switch";
    case "choice":
      return segmentable(setting) ? "segmented" : "select";
    case "int":
    case "float":
      return hasRange(setting) ? "slider" : "number";
    case "keybind":
      return "keybind";
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

/** A mod's settings in the sections its page shows them in: by their manifest "group", in the
 *  order each first appears; those without one first, as General (Settings when they are all). */
export interface SettingGroup {
  id: string;
  title: string;
  settings: PluginSetting[];
}

export function settingGroups(settings: PluginSetting[]): SettingGroup[] {
  const groups = new Map<string, PluginSetting[]>();
  for (const s of settings) {
    const id = s.group ?? "";
    groups.set(id, [...(groups.get(id) ?? []), s]);
  }
  const ordered = [...groups.entries()].sort(([a], [b]) => (a === "" ? -1 : b === "" ? 1 : 0));
  return ordered.map(([id, list]) => ({
    id: id || "general",
    title: groupTitle(id, list, ordered.length),
    settings: list,
  }));
}

/** "play" → "Play controls" for a group of keybinds, "Camera" for others; the ungrouped ones General. */
function groupTitle(id: string, list: PluginSetting[], count: number): string {
  if (id === "") return count === 1 ? GROUP_TITLE.only : GROUP_TITLE.general;
  const name = id.replace(/[_-]+/g, " ").replace(/^\w/, (c) => c.toUpperCase());
  return list.every((s) => s.type === "keybind") ? GROUP_TITLE.controls(name) : name;
}

/** Whether a mod's page has a choice to make: its settings, or a preset. */
export const hasSettings = (mod: PluginView): boolean => mod.settings.length > 0;

/** The one badge a mod's row shows, most pressing first: it cannot load, an update, what it runs. */
export function modBadge(mod: PluginView, update: string | null, purist: boolean): Label & { hint: string } {
  if (mod.enabled && !purist && (mod.state === "refused" || mod.state === "failed")) {
    return { ...STATE_LABEL[mod.state], hint: mod.reason };
  }
  if (update) return { text: BADGE.update(update), tone: "warn", hint: BADGE.updateHint };
  return { ...modKind(mod.kind), hint: modHint(mod.kind) };
}
