// Key-binding settings (a mod's "keybind" settings, mod interface 1.2): a bind is one key, mouse
// button or wheel direction, named as sdk/include/openrac/keys.h names it (keys by position, as the
// web's KeyboardEvent.code does: "KeyW", "Space", "ShiftLeft"; then "MouseLeft" .. "MouseForward"
// and "WheelUp" .. "WheelRight"). A setting holds up to two, comma-separated. The core checks the
// same names (core/src/plugins.rs, BIND_NAMES); here the control's words and its sums.
import type { PluginSetting } from "./api";

/** keys.h's names, in its order. */
export const BIND_NAMES: readonly string[] = [
  ..."ABCDEFGHIJKLMNOPQRSTUVWXYZ".split("").map((c) => `Key${c}`),
  ..."1234567890".split("").map((d) => `Digit${d}`),
  "Enter", "Escape", "Backspace", "Tab", "Space", "Minus", "Equal", "BracketLeft", "BracketRight",
  "Backslash", "IntlHash", "Semicolon", "Quote", "Backquote", "Comma", "Period", "Slash", "CapsLock",
  ...Array.from({ length: 12 }, (_, i) => `F${i + 1}`),
  "PrintScreen", "ScrollLock", "Pause", "Insert", "Home", "PageUp", "Delete", "End", "PageDown",
  "ArrowRight", "ArrowLeft", "ArrowDown", "ArrowUp",
  "NumLock", "NumpadDivide", "NumpadMultiply", "NumpadSubtract", "NumpadAdd", "NumpadEnter",
  ..."1234567890".split("").map((d) => `Numpad${d}`),
  "NumpadDecimal", "IntlBackslash", "ContextMenu", "NumpadEqual",
  "ControlLeft", "ShiftLeft", "AltLeft", "MetaLeft", "ControlRight", "ShiftRight", "AltRight", "MetaRight",
  "MouseLeft", "MouseRight", "MouseMiddle", "MouseBack", "MouseForward",
  "WheelUp", "WheelDown", "WheelLeft", "WheelRight",
]; // prettier-ignore

const KNOWN = new Set(BIND_NAMES);

/** A keybind's most binds. */
export const mostBinds = (s: PluginSetting): number => (s.binds === 1 ? 1 : 2);

/** A keybind's value as its names; unknown names are left out. */
export function parseBinds(value: unknown): string[] {
  if (typeof value !== "string") return [];
  return value
    .split(/[,\s]+/)
    .filter((n) => KNOWN.has(n))
    .filter((n, i, all) => all.indexOf(n) === i);
}

/** Names as a keybind's value: "KeyW,ArrowUp". */
export const formatBinds = (names: string[]): string => names.filter(Boolean).join(",");

const WORDS: Record<string, string> = {
  Enter: "Enter",
  Escape: "Esc",
  Backspace: "Backspace",
  Tab: "Tab",
  Space: "Space",
  Minus: "-",
  Equal: "=",
  BracketLeft: "[",
  BracketRight: "]",
  Backslash: "\\",
  IntlHash: "#",
  Semicolon: ";",
  Quote: "'",
  Backquote: "`",
  Comma: ",",
  Period: ".",
  Slash: "/",
  CapsLock: "Caps Lock",
  PrintScreen: "Print Screen",
  ScrollLock: "Scroll Lock",
  Pause: "Pause",
  Insert: "Insert",
  Home: "Home",
  PageUp: "Page Up",
  Delete: "Delete",
  End: "End",
  PageDown: "Page Down",
  ArrowRight: "Right",
  ArrowLeft: "Left",
  ArrowDown: "Down",
  ArrowUp: "Up",
  NumLock: "Num Lock",
  NumpadDivide: "Num /",
  NumpadMultiply: "Num *",
  NumpadSubtract: "Num -",
  NumpadAdd: "Num +",
  NumpadEnter: "Num Enter",
  NumpadDecimal: "Num .",
  NumpadEqual: "Num =",
  IntlBackslash: "\\",
  ContextMenu: "Menu",
  ControlLeft: "Left Ctrl",
  ShiftLeft: "Left Shift",
  AltLeft: "Left Alt",
  MetaLeft: "Left Meta",
  ControlRight: "Right Ctrl",
  ShiftRight: "Right Shift",
  AltRight: "Right Alt",
  MetaRight: "Right Meta",
  MouseLeft: "Left click",
  MouseRight: "Right click",
  MouseMiddle: "Middle click",
  MouseBack: "Mouse back",
  MouseForward: "Mouse forward",
  WheelUp: "Wheel up",
  WheelDown: "Wheel down",
  WheelLeft: "Wheel left",
  WheelRight: "Wheel right",
};

/** A bind as a key cap shows it: "W", "1", "Space", "Left Shift", "Right click", "Num 0", "Up". */
export function bindLabel(name: string): string {
  if (/^Key[A-Z]$/.test(name)) return name.slice(3);
  if (/^Digit\d$/.test(name)) return name.slice(5);
  if (/^Numpad\d$/.test(name)) return `Num ${name.slice(6)}`;
  if (/^F\d+$/.test(name)) return name;
  return WORDS[name] ?? name;
}

/** A bind's words for a screen reader, where the cap is a symbol: "Up arrow". */
export function bindSpoken(name: string): string {
  const arrows: Record<string, string> = {
    ArrowUp: "Up arrow",
    ArrowDown: "Down arrow",
    ArrowLeft: "Left arrow",
    ArrowRight: "Right arrow",
  };
  return arrows[name] ?? bindLabel(name);
}

/** The bind a key press is, by where the key is (event.code); null for none of keys.h's. */
export function bindOfKey(e: { code: string }): string | null {
  return e.code && e.code !== "Unidentified" && KNOWN.has(e.code) ? e.code : null;
}

/** The bind a mouse button is (MouseEvent.button); null for another. */
export function bindOfMouse(e: { button: number }): string | null {
  return ["MouseLeft", "MouseMiddle", "MouseRight", "MouseBack", "MouseForward"][e.button] ?? null;
}

/** The bind a turn of the wheel is (WheelEvent's deltas: y > 0 is toward the player); null for none. */
export function bindOfWheel(e: { deltaX: number; deltaY: number }): string | null {
  if (Math.abs(e.deltaY) >= Math.abs(e.deltaX)) {
    return e.deltaY < 0 ? "WheelUp" : e.deltaY > 0 ? "WheelDown" : null;
  }
  return e.deltaX < 0 ? "WheelLeft" : "WheelRight";
}

/** A keybind's binds with `name` put in slot `slot`: when the setting has it in its other slot
 *  already, the two swap (never one input twice in one setting). */
export function withBind(binds: string[], slot: number, name: string): string[] {
  const out = [...binds];
  const there = out.indexOf(name);
  if (there >= 0 && there !== slot) out[there] = out[slot] ?? "";
  out[slot] = name;
  return out.filter((n) => !!n);
}

/** A keybind's binds without slot `slot`. */
export const withoutBind = (binds: string[], slot: number): string[] => binds.filter((_, i) => i !== slot);

/** For each keybind of a mod, the names of its other keybinds of the same group that share one of
 *  its inputs (key: the setting's key; only the ones with any). */
export function keybindConflicts(settings: PluginSetting[]): Record<string, string[]> {
  const keybinds = settings.filter((s) => s.type === "keybind");
  const binds = new Map(keybinds.map((s) => [s.key, parseBinds(s.value)]));
  const out: Record<string, string[]> = {};
  for (const s of keybinds) {
    const mine = binds.get(s.key) ?? [];
    const with_ = keybinds
      .filter((o) => o.key !== s.key && (o.group ?? "") === (s.group ?? ""))
      .filter((o) => (binds.get(o.key) ?? []).some((b) => mine.includes(b)))
      .map((o) => o.name);
    if (with_.length) out[s.key] = with_;
  }
  return out;
}

/** The binds of keybind `key` that another keybind of its group has too. */
export function sharedBinds(settings: PluginSetting[], key: string): string[] {
  const me = settings.find((s) => s.key === key && s.type === "keybind");
  if (!me) return [];
  const others = settings
    .filter((o) => o.type === "keybind" && o.key !== key && (o.group ?? "") === (me.group ?? ""))
    .flatMap((o) => parseBinds(o.value));
  return parseBinds(me.value).filter((b) => others.includes(b));
}
