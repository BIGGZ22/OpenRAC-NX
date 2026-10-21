// Every word the Mods tab shows (Mods.svelte, AddMod.svelte, PluginsPanel.svelte, PacksPanel.svelte,
// AssetsPanel.svelte), with CrashNotice.svelte's mods line, Settings' Purist mode, lib/plugins.ts and
// lib/addMods.ts, in one place (launcher/docs/MODS_TAB.md lists where each shows). Written for
// players: short, plain, one idea per line; the longer "why" goes in a tooltip.
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
  native: { text: "Runs code", tone: "warn" },
  script: { text: "Sandboxed script", tone: "ok" },
  pack: { text: "Textures and models", tone: "off" },
} as const satisfies Record<string, Label>;

/** The kinds' tooltips: what each means, for whoever wonders. */
export const KIND_HINT = {
  native: "A program: it runs on this computer with the game's rights. Install it only from people you trust.",
  script: "Runs in the game's sandbox: it reaches only what the game lets scripts change, never your files.",
  pack: "Data only: it replaces textures, models or levels, and runs nothing.",
} as const satisfies Record<KindKey, string>;
export type KindKey = keyof typeof KIND_LABEL;

/** Whether a mod from a link can be installed here, in its preview row. */
export const COMPATIBLE_LABEL = {
  yes: { text: "Works here", tone: "ok" },
  no: { text: "Cannot be installed", tone: "err" },
} as const satisfies Record<string, Label>;

/** The Mods tab's game list (Mods.svelte, modState). */
export const GAME_LIST = {
  gameplayMods: "Gameplay mods",
  notModdable: "Not moddable yet",
  early: "Early access",
  working: "Working…",
  setUp: "Set it up first",
  ready: "Ready to mod",
} as const;

/** The page around the panels (Mods.svelte). */
export const PAGE = {
  lead: "Add mods, switch them on, and play.",
  tabsLabel: "Mods",
  tabMine: "My mods",
  tabEditor: "Level editor",
  tabAssets: "Models and textures",
  play: "Play with mods",
  /** Under Play with mods when it cannot start. */
  playEarly: "Not playable yet",
  playMissing: "Not on this computer yet",
  playBuild: "Build the game first, on its page",
  earlyTitle: "Early access",
  earlyLine:
    "Its levels open in Godot and you can make texture and model packs. Packing level edits and playing come later.",
  noneTitle: "No mods for this game yet",
  noneLine: "Mods arrive once the level editor can read its levels.",
  noneLink: "Follow the progress on openrac.dev",
  makeOwn: "Want to make your own?",
  makeOwnButton: "Edit a level in Godot",
  developerTitle: "For developers",
} as const;

/** Adding a mod: a link, a file or a folder (AddMod.svelte). */
export const ADD = {
  title: "Add a mod",
  linkLabel: "A mod's link",
  linkPlaceholder: "Paste a mod's link, like github.com/OpenRAC/rac1-mods",
  go: "Find",
  file: "File…",
  folder: "Folder…",
  pickZip: "Choose a mod (.zip)",
  pickFolder: "Choose a mod's folder",
  drop: "or drop a .zip or folder on this page",
  dropHere: "Drop to add it",
  browse: "Browse mods",
  modsUrl: "https://github.com/OpenRAC/rac1-mods",
  close: "Close",
} as const;

/** The gameplay mods' list (PluginsPanel.svelte). */
export const PANEL = {
  title: "Gameplay mods",
  checkUpdates: "Check for updates",
  sub: "Change how the game plays",
  empty: "None yet. Paste a mod's link above, or choose its file.",
  orderHint: "Lower in the list wins",
  moveUpTitle: "Move up: it loads earlier",
  moveDownTitle: "Move down: it loads later, and wins where it changes what a mod above it does",
  settings: "Settings",
  keepVersion: "Keep this version",
  letUpdate: "Let it update",
  homepage: "Its page",
  remove: "Remove",
  hooks: "Hooks:",
  license: "License:",
  source: "Source:",
  interface: "interface",
} as const;

/** The packs' list (PacksPanel.svelte). */
export const PACKS = {
  title: "Textures, models and levels",
  sub: "Change how the game looks",
  empty: "None yet. Add one above, or make your own in the Level editor or Models and textures.",
  orderHint: "Lower in the list wins",
  moveUpTitle: "Move up: the packs under it win where they change the same thing",
  moveDownTitle: "Move down: it wins over the packs above it",
  remove: "Remove",
} as const;

/** The preview of what a link holds. */
export const PREVIEW = {
  nativeWarning: "Mods that run code are programs. Install them only from people you trust.",
  install: "Install",
  reinstall: "Reinstall",
} as const;

/** Purist mode, on the panel and in Settings. */
export const PURIST = {
  title: "Purist mode",
  panelLine: "Play exactly as on the PS2: no gameplay mod loads.",
  settingsLine: "Play as the PS2 plays: the game loads none of your gameplay mods (Mods).",
  onNotice: "Purist mode is on: gameplay mods are skipped.",
  turnOff: "Turn off",
} as const;

/** The level editor's tab (Mods.svelte). */
export const EDITOR = {
  lead: "Change a level in Godot, then pack your changes and play them.",
  leadEarly: "Look around a level in Godot.",
  openTitle: "Open a level in Godot",
  packTitle: "Pack your changes",
  levelLabel: "Level to edit",
  noDisc: "Set the game up from your disc first.",
  setUp: "Set up the disc",
  noGodot: "Levels open in Godot 4, a free editor: get it, then choose where it is.",
  getGodot: "Get Godot",
  chooseGodot: "Choose Godot…",
  pickGodot: "Godot 4",
  ready: "Move, turn or scale objects, then save in Godot.",
  readyEarly: "Look around a level's objects and textures.",
  firstTime: "The first time reads the levels from your disc: about 20 seconds.",
  firstTimeEarly: "The first time reads the levels from your disc: a few minutes.",
  open: "Open in Godot",
  prepare: "Prepare and open",
  opening: (level: string) => `Godot is opening ${level}.`,
  notGodot: (why: string) => `That is not a Godot 4 the editor can use: ${why}`,
  packLine: "Turns what you saved into level data the game reads.",
  packEarly: "Not yet for this game. Texture and model packs work.",
  packReading: "Reading your levels…",
  pack: "Pack",
  openFolder: "Open folder",
  packStopped: "Packing stopped. The activity log says why.",
  packed: "Packed.",
  packedSome: (edits: number, levels: number) =>
    `Packed ${edits} changed object${edits === 1 ? "" : "s"} in ${levels} level${levels === 1 ? "" : "s"}. Play with mods to try them.`,
  packedNone: "Nothing to pack: your levels are as on the disc.",
  docs: "Editor documentation",
  projects: "Level projects",
  packedFolder: "Packed levels",
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

/** Models and textures (AssetsPanel.svelte). */
export const ASSETS = {
  lead: "Export a level's models and textures, edit them in Blender or an image editor, then import them back as a pack.",
  howItWorks: "How it works",
  exportTitle: "Export",
  exportLine: "Opens a folder of glTF models and PNG textures.",
  exporting: "Writing the files…",
  readFirst: "Read the levels from your disc first: a few minutes, once.",
  readyFirst: "Get the levels ready first, on the game's page.",
  levelLabel: "Level to export",
  whatLabel: "What to export",
  read: "Read the levels",
  export: "Export",
  openFolder: "Open folder",
  importTitle: "Import your edits",
  importLine: "Choose the folder you edited: your changes become a pack in My mods.",
  importing: "Checking what you changed…",
  import: "Import…",
  pickImport: "Choose the export folder you edited",
  imported: "Imported: your edits are a pack in My mods, switched on.",
  importedRefused: (n: number) =>
    `Imported, but ${n} change${n === 1 ? " was" : "s were"} refused: the activity log says why and what to do.`,
  importStopped: "The import stopped. The activity log has what it said.",
  exportStopped: "The export stopped. The activity log has what it said.",
  readStopped: "Reading the levels stopped. The activity log has what it said.",
  exported: (level: string) => `Exported ${level}: the folder is opening.`,
} as const;
