// Words for the codes the Rust side sends.
import type { ActionState, DiscState } from "./api";

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
