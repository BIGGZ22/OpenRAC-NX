// A game's differences from the original (core/src/differences.rs), grouped as its page shows
// them: the fixes a player can switch back first, then the guards, then the PC's own differences.
import type { DifferenceKind, DifferenceView } from "./api";

export interface DifferenceGroup {
  kind: DifferenceKind;
  title: string;
  /** One line under the group's title. */
  about: string;
  entries: DifferenceView[];
}

const GROUPS: { kind: DifferenceKind; title: string; about: string }[] = [
  {
    kind: "fix",
    title: "Game fixes",
    about:
      "On, the port's fix; off, the game plays as on a PlayStation 2. The game's Options menu has the same switches.",
  },
  {
    kind: "guard",
    title: "Kept from crashing",
    about:
      "Where a PlayStation 2 carried on unnoticed but a PC would crash. Always on: off would only bring the crash back.",
  },
  {
    kind: "port",
    title: "Playing on a PC",
    about: "What comes with the game being a PC program.",
  },
];

/** The entries in their groups, in that order; a group with none is left out. */
export function groupDifferences(entries: DifferenceView[]): DifferenceGroup[] {
  return GROUPS.map((g) => ({ ...g, entries: entries.filter((e) => e.kind === g.kind) })).filter(
    (g) => g.entries.length > 0,
  );
}

/** A version key's two halves: `rac1/pal` -> [`rac1`, `pal`]. */
export function splitVersion(key: string): [string, string] {
  const at = key.indexOf("/");
  return at < 0 ? [key, ""] : [key.slice(0, at), key.slice(at + 1)];
}
