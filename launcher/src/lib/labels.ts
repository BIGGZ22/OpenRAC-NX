// Words for the codes the Rust side sends.
import type { IconName } from "$components/Icon.svelte";
import type { ActionState, DiscState } from "./api";

/** What a game list entry (GameList.svelte) says under the game's name, and in which colour;
 *  BUSY shows a spinner. */
export interface EntryState {
  text: string;
  tone: "ok" | "warn" | "info";
  busy?: boolean;
}

/** An entry in the list on the left of the Games, Mods and Develop tabs (SideList.svelte). */
export interface SideEntry {
  key: string;
  name: string;
  state?: EntryState | null;
  /** A picture, shown cropped. */
  art?: string | null;
  /** Without a picture: an icon. */
  icon?: IconName;
  /** The game's colours (themeStyle) for the picture's frame. */
  style?: string;
  /** A small red dot on the picture: it crashed recently. */
  mark?: boolean;
  /** A heading before this entry, starting a group. */
  group?: string;
}

/** A game's name without the series in front, for a narrow list: "Ratchet & Clank: Going Commando"
 *  -> "Going Commando", "Ratchet: Deadlocked" -> "Deadlocked"; the first game keeps its name. */
export function shortTitle(title: string): string {
  return title.replace(/^Ratchet( & Clank)?: /, "");
}

/** `pal` -> "PAL", `ntsc` -> "NTSC-U", `ntsc-j` -> "NTSC-J". */
export function regionLabel(name: string): string {
  if (name === "ntsc") return "NTSC-U";
  return name.toUpperCase();
}

export const DISC: Record<DiscState, { text: string; tone: "ok" | "warn" | "err" | "info" }> = {
  found: { text: "Disc found", tone: "ok" },
  mismatch: { text: "Wrong disc size", tone: "err" },
  missing: { text: "No disc", tone: "warn" },
  unknown: { text: "No disc needed", tone: "info" },
};

/** What each kind of input in game.json is, for the game page. */
export const INPUT: Record<string, string> = {
  disc: "Image link",
  boot: "Executable",
  toolchains: "Toolchains",
  link: "Link",
};

export const STATE: Record<ActionState, { text: string; tone: "ok" | "warn" | "info"; hint: string }> = {
  connected: { text: "Ready", tone: "ok", hint: "Run from the launcher and known to work." },
  unverified: {
    text: "Unverified",
    tone: "warn",
    hint: "Written from the game's documentation and not yet run from the launcher: watch its output.",
  },
  planned: { text: "Not connected", tone: "info", hint: "Planned: the launcher cannot run this yet." },
};

/** Ratchet & Clank's levels, by the number the game and the level editor give them. */
export const RAC1_LEVELS = [
  "Veldin",
  "Novalis",
  "Aridia",
  "Kerwan",
  "Eudora",
  "Rilgar",
  "Blarg Station",
  "Umbris",
  "Batalia",
  "Gaspar",
  "Orxon",
  "Pokitaru",
  "Hoven",
  "Gemlik Base",
  "Oltanis",
  "Quartu",
  "Kalebo III",
  "Drek's Fleet",
  "Veldin (the return)",
];
