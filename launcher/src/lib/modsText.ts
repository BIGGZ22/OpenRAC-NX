// Every word the gameplay mods' part of the launcher shows (PluginsPanel.svelte, the plugin parts of
// Mods.svelte, CrashNotice.svelte's mods line, Settings' Purist mode, lib/plugins.ts, lib/addMods.ts),
// in one place for the Mods tab's redesign (launcher/docs/MODS_TAB.md lists where each shows).
// Plain constants, maps from a state to its words, and small functions where a name or a number goes
// into a sentence: no decision is made here, only wording.

export type ModTone = "ok" | "off" | "warn" | "err";

/** A dot and a few words. */
export interface Label {
  text: string;
  tone: ModTone;
}

/** An installed mod's state on its row (lib/plugins.ts, pluginState, decides which). */
export const STATE_LABEL = {
  on: { text: "On", tone: "ok" },
  off: { text: "Off", tone: "off" },
  purist: { text: "Not in purist mode", tone: "off" },
  failed: { text: "Not loaded", tone: "err" },
  refused: { text: "Cannot load", tone: "err" },
} as const satisfies Record<string, Label>;
export type StateKey = keyof typeof STATE_LABEL;

/** What a mod runs (lib/plugins.ts, modKind): code, scripts in the sandbox, or data only. */
export const KIND_LABEL = {
  native: { text: "Runs code on this computer", tone: "warn" },
  script: { text: "Runs scripts (sandboxed)", tone: "ok" },
  pack: { text: "Textures and models", tone: "off" },
} as const satisfies Record<string, Label>;
export type KindKey = keyof typeof KIND_LABEL;

/** Whether a mod from a link can be installed here, in its preview row. */
export const COMPATIBLE_LABEL = {
  yes: { text: "Works here", tone: "ok" },
  no: { text: "Cannot be installed", tone: "err" },
} as const satisfies Record<string, Label>;

/** The Mods tab's game list, for a game the level editor cannot open (Mods.svelte, modState). */
export const GAME_LIST = {
  gameplayMods: "Gameplay mods",
  notModdable: "Not moddable yet",
} as const;

/** The panel (PluginsPanel.svelte). */
export const PANEL = {
  title: "Gameplay mods",
  addZip: "Add a .zip…",
  addFolder: "Add a folder…",
  pickZip: "Choose a mod (.zip)",
  pickFolder: "Choose a mod's folder",
  checkUpdates: "Check for updates",
  linkLabel: "A mod's repository address",
  linkPlaceholder: "Add from a link: https://github.com/owner/repository",
  look: "Look",
  close: "Close",
  modsUrl: "https://github.com/OpenRAC/rac1-mods",
  modsUrlLabel: "OpenRAC/rac1-mods",
  /** The empty list, around the link to modsUrl. */
  emptyBefore: "No gameplay mods yet. They change how the game plays, as a lock-strafe does. Ratchet & Clank's are at",
  emptyAfter: ": paste its address above, or add the .zip for your computer here.",
  note: "Mods load from the top down; where two change the same thing, the lower one goes first. Changes count from the game's next start. A mod is a program: add only those you trust.",
  moveUpTitle: "Move up: it loads earlier",
  moveDownTitle: "Move down: it loads later, and goes first where it changes what a mod above it does",
  keepVersion: "Keep this version",
  letUpdate: "Let it update",
  homepage: "Its page",
  remove: "Remove",
  hooks: "Hooks:",
  license: "License:",
  source: "Source:",
  interface: "interface",
} as const;

/** The preview of what a link holds. */
export const PREVIEW = {
  nativeWarning:
    "A gameplay mod with code is a program: it runs on this computer with the game's rights. Install only mods from people you trust.",
  scriptNote:
    "A script mod runs in the game's sandbox: it can change only what the game lets scripts reach, never your files.",
  install: "Install",
  reinstall: "Reinstall",
} as const;

/** Purist mode, on the panel and in Settings. */
export const PURIST = {
  title: "Purist mode",
  panelLine: "Play the game as the PS2 plays it: no gameplay mod is loaded, whatever is on below.",
  settingsLine: "Play as the PS2 plays: the game loads none of your gameplay mods (Mods).",
} as const;

// ---- Sentences with a name or a number in them ----

export const modsAria = (label: string) => `Mods in ${label}`;
export const useAria = (name: string) => `Use ${name}`;
export const moveUpAria = (name: string) => `Move ${name} up`;
export const moveDownAria = (name: string) => `Move ${name} down`;
export const moreAria = (name: string) => `More for ${name}`;
export const fromPublisher = (label: string, owner: string) =>
  owner ? { before: "In ", label, after: `, published by ${owner}` } : { before: "In ", label, after: "" };
export const installedAlready = (version: string) => `${version} installed`;
export const updateTo = (version: string) => `Update to ${version}`;
export const fromSource = (label: string) => `From ${label}`;
export const keptAt = (version: string) => `kept at ${version}`;
export const cannotSwitchOn = (why: string) => `Cannot be switched on: ${why}`;
export const removeConfirm = (name: string) => `Remove ${name}? The launcher's copy and its settings are deleted.`;
export const settingTakes = (name: string, range: string, int: boolean) =>
  `${name} takes ${range || (int ? "a whole number" : "a number")}.`;
export const withName = (name: string, why: string) => `${name}: ${why}`;

/** The update check's answer. */
export function updatesFound(count: number, errors: string[]): string {
  const found =
    count === 0 ? "Every mod from a link is up to date." : `${count} update${count === 1 ? "" : "s"} found.`;
  return found + (errors.length ? ` Not checked: ${errors.join("; ")}` : "");
}

export const updated = (name: string, version: string) => `Updated ${name} to ${version}; its settings are kept.`;

/** A mod installed from a link: replaced, on, or off (its row says why). */
export const installedFrom = {
  replaced: (name: string, version: string, from: string) =>
    `Installed ${name} ${version} from ${from}; its settings are kept.`,
  on: (name: string, from: string) => `Installed ${name} from ${from}. It is on from the game's next start.`,
  off: (name: string, from: string) => `Installed ${name} from ${from}. It is off: its row says why.`,
};

/** A mod added from a .zip or a folder (lib/addMods.ts). */
export const added = {
  replaced: (name: string, version: string) => updated(name, version),
  on: (name: string) => `Added ${name}. It is on from the game's next start.`,
  off: (name: string) => `Added ${name}. It is off: its row says why.`,
  pack: (name: string) => `Added ${name}.`,
  packHereNot: (name: string) => `${name} is a texture and model pack, which cannot be added here yet.`,
  packFailed: (name: string, why: string) => `${name}: ${why.length > 0 ? why : "it could not be added"}`,
};

/** Two mods on one hook point (lib/plugins.ts, sharedWith). */
export function sharesWith(names: string[]): string {
  if (names.length === 0) return "";
  const list = names.length === 1 ? names[0] : `${names.slice(0, -1).join(", ")} and ${names.at(-1)}`;
  return `Changes the same part of the game as ${list}: the one lower in the list goes first.`;
}

/** A number setting's range on its line: "50 to 110". */
export function rangeText(min?: number, max?: number): string {
  if (min !== undefined && max !== undefined) return `${min} to ${max}`;
  if (min !== undefined) return `${min} or more`;
  if (max !== undefined) return `up to ${max}`;
  return "";
}

/** The crash notice, when gameplay mods were on at the start that crashed (CrashNotice.svelte). */
export const crashModsLine = (ids: string[]) =>
  `Gameplay mods were on (${ids.join(", ")}). If it crashes again, try Purist mode on the Mods tab, and tell us which mods you had.`;
