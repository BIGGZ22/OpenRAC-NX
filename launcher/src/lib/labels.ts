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
const RAC1_LEVELS = [
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

/** A level the editor can open: the number its folder has (level_NN) and its name. */
export interface LevelName {
  number: number;
  name: string;
}

/** The later games' levels the editor reads (early access), by the number in their level header,
 *  as editor/games.py has them (names from Wrench's data/underlay). */
const LATER_LEVELS: Record<string, Record<number, string>> = {
  rac2: {
    0: "Aranos Tutorial",
    1: "Oozla",
    2: "Maktar Nebula",
    3: "Endako",
    4: "Barlow",
    5: "Feltzin System",
    6: "Notak",
    7: "Siberius",
    8: "Tabora",
    9: "Dobbo",
    10: "Hrugis Cloud",
    11: "Joba",
    12: "Todano",
    13: "Boldan",
    14: "Aranos Prison",
    15: "Gorn",
    16: "Snivelak",
    17: "Smolg",
    18: "Damosel",
    19: "Grelbin",
    20: "Yeedil",
    22: "Dobbo Orbit",
    23: "Damosel Orbit",
    24: "Ship Shack",
    25: "Wupash Nebula",
    26: "Jamming Array",
    30: "Insomniac Museum",
  },
  rac3: {
    1: "Veldin",
    2: "Florana",
    3: "Starship Phoenix",
    4: "Marcadia",
    5: "Daxx",
    6: "Starship Phoenix (Under Attack)",
    7: "Annihilation Nation",
    8: "Aquatos",
    9: "Tyhrranosis",
    10: "Zeldrin Starport",
    11: "Obani Moons",
    12: "Rilgar",
    13: "Holostar Studios Ratchet",
    14: "Koros",
    16: "Kerwan",
    17: "Crash Site",
    18: "Aridia",
    19: "Thran Asteroid Belt",
    20: "Final Boss",
    21: "Obani Draco",
    22: "Mylon",
    23: "Holostar Studios Clank",
    24: "Insomniac Museum",
    26: "Kerwan Ranger Missions",
    27: "Aquatos Base",
    28: "Aquatos Sewers",
    29: "Tyhrranosis Ranger Missions",
    30: "Vid-Comic Unnamed",
    31: "Vid-Comic 1",
    32: "Vid-Comic 4",
    33: "Vid-Comic 2",
    34: "Vid-Comic 3",
    35: "Vid-Comic 5",
    36: "Vid-Comic 1 Special Edition",
    39: "Multiplayer Menu",
    40: "Bakisi Isles",
    41: "Hoven Gorge",
    42: "Outpost X12",
    43: "Korgon Outpost",
    44: "Metropolis",
    45: "Blackwater City",
    46: "Command Center",
    47: "Blackwater Docks",
    48: "Aquatos Sewers (multiplayer)",
    49: "Marcadia Palace",
    50: "Bakisi Isles (split screen)",
    51: "Hoven Gorge (split screen)",
    52: "Outpost X12 (split screen)",
    53: "Korgon Outpost (split screen)",
    54: "Metropolis (split screen)",
    55: "Blackwater City (split screen)",
  },
  rac4: {
    0: "Multiplayer Menu",
    1: "DreadZone Station",
    2: "Catacrom Graveyard",
    4: "Sarathos Swamp",
    5: "Dark Cathedral",
    6: "Temple of Shaar",
    7: "Valix Lighthouse",
    8: "Mining Facility",
    10: "Torval Ruins",
    11: "Tempus Station",
    13: "Maraxus Prison",
    14: "Ghost Station",
    15: "Control Level",
    21: "DreadZone Station (split screen)",
    22: "Catacrom Graveyard (split screen)",
    24: "Sarathos Swamp (split screen)",
    25: "Dark Cathedral (split screen)",
    26: "Temple of Shaar (split screen)",
    27: "Valix Lighthouse (split screen)",
    28: "Mining Facility (split screen)",
    30: "Torval Ruins (split screen)",
    31: "Tempus Station (split screen)",
    33: "Maraxus Prison (split screen)",
    34: "Ghost Station (split screen)",
    35: "Control Level (split screen)",
    41: "Battledome Tower",
    42: "Catacrom Graveyard (multiplayer)",
    44: "Sarathos Swamp (multiplayer)",
    45: "Dark Cathedral (multiplayer)",
    46: "Temple of Shaar (multiplayer)",
    47: "Valix Lighthouse (multiplayer)",
    48: "Mining Facility (multiplayer)",
    50: "Torval Ruins (multiplayer)",
    51: "Tempus Station (multiplayer)",
    53: "Maraxus Prison (multiplayer)",
    54: "Ghost Station (multiplayer)",
    61: "Battledome Tower (split screen)",
    62: "Catacrom Graveyard (multiplayer, split screen)",
    64: "Sarathos Swamp (multiplayer, split screen)",
    65: "Dark Cathedral (multiplayer, split screen)",
    66: "Temple of Shaar (multiplayer, split screen)",
    67: "Valix Lighthouse (multiplayer, split screen)",
    68: "Mining Facility (multiplayer, split screen)",
    70: "Torval Ruins (multiplayer, split screen)",
    71: "Tempus Station (multiplayer, split screen)",
    73: "Maraxus Prison (multiplayer, split screen)",
    74: "Ghost Station (multiplayer, split screen)",
  },
};

/** A game's levels in the order of their numbers: Ratchet & Clank's 19, the later games' own. */
export function levelsOf(game: string): LevelName[] {
  const later = LATER_LEVELS[game];
  if (!later) return RAC1_LEVELS.map((name, number) => ({ number, name }));
  return Object.entries(later)
    .map(([number, name]) => ({ number: Number(number), name }))
    .sort((a, b) => a.number - b.number);
}

/** A level's name, or "Level N" for one the list does not have. */
export function levelName(game: string, number: number): string {
  return levelsOf(game).find((l) => l.number === number)?.name ?? `Level ${number}`;
}
