// A mod's number settings as a slider and a field beside it (ModSetting.svelte): the step each
// takes, a value kept inside its range and on its steps, and how it is written. The manifest gives
// min, max and, for a float, step (sdk/docs/settings.md); an int steps by 1. The core checks the
// value again when it is saved (core/src/plugins.rs), so these only keep the controls honest.
import type { PluginSetting } from "./api";

/** Whether a number setting is drawn as a slider: it has both ends. */
export const hasRange = (s: PluginSetting): boolean =>
  (s.type === "int" || s.type === "float") && s.min !== undefined && s.max !== undefined && s.max > s.min;

/** The decimals a number needs to be written without noise ("0.05" has 2). */
export function decimalsOf(n: number): number {
  if (!Number.isFinite(n) || Number.isInteger(n)) return 0;
  const text = String(n);
  const e = /e-(\d+)$/.exec(text);
  if (e) return Number(e[1]);
  return text.split(".")[1]?.length ?? 0;
}

/** The step a setting's slider and field take: its own, 1 for an int, else a hundredth of its
 *  range rounded to a plain number (0.01, 0.05, 0.1, ...). */
export function stepOf(s: PluginSetting): number {
  if (s.type === "int") return Math.max(1, Math.round(s.step ?? 1));
  if (s.step !== undefined && s.step > 0) return s.step;
  if (s.min === undefined || s.max === undefined) return 0.01;
  const raw = (s.max - s.min) / 100;
  const power = 10 ** Math.floor(Math.log10(raw));
  const nice = [1, 2, 5, 10].find((m) => m * power >= raw) ?? 10;
  return nice * power;
}

/** The decimals a setting's values are written with: its step's, its ends' and its default's. */
export function settingDecimals(s: PluginSetting): number {
  if (s.type === "int") return 0;
  const of = [stepOf(s), s.min, s.max, typeof s.default === "number" ? s.default : undefined];
  return Math.min(6, Math.max(...of.map((n) => (n === undefined ? 0 : decimalsOf(n)))));
}

/** A value on the setting's steps and inside its range: steps count from min (from 0 without one),
 *  the float's error rounded away, so 0.1 + 0.2 is 0.3 and a dragged slider never saves 0.30000000000000004. */
export function snapValue(s: PluginSetting, n: number): number {
  if (!Number.isFinite(n)) return typeof s.default === "number" ? s.default : (s.min ?? 0);
  const step = stepOf(s);
  const base = s.min ?? 0;
  let v = base + Math.round((n - base) / step) * step;
  if (s.max !== undefined && v > s.max) v = s.max;
  if (s.min !== undefined && v < s.min) v = s.min;
  const places = Math.max(settingDecimals(s), decimalsOf(step));
  v = Number(v.toFixed(places));
  return s.type === "int" ? Math.round(v) : v;
}

/** What a player typed in the field, as the value it saves (snapped, clamped), or null when it is
 *  not a number at all (the field then goes back to the value). */
export function parseTyped(s: PluginSetting, typed: string): number | null {
  const text = typed.trim().replace(",", ".");
  if (text === "" || text === "-" || text === ".") return null;
  const n = Number(text);
  return Number.isFinite(n) ? snapValue(s, n) : null;
}

/** A value as the field and the slider's label write it: the setting's decimals. */
export function formatValue(s: PluginSetting, n: unknown): string {
  if (typeof n !== "number" || !Number.isFinite(n)) return "";
  return n.toFixed(settingDecimals(s));
}

/** Where a value sits on the slider's track, 0 to 100, for its filled part and the default's mark. */
export function percentOf(s: PluginSetting, n: unknown): number {
  if (typeof n !== "number" || s.min === undefined || s.max === undefined || s.max <= s.min) return 0;
  return Math.min(100, Math.max(0, ((n - s.min) / (s.max - s.min)) * 100));
}

/** Whether a setting holds its default (a keybind compares its binds, as written). */
export const isDefault = (s: PluginSetting, value: unknown = s.value): boolean => value === s.default;
