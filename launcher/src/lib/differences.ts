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

/** The folded list in a line (Android's game page): "3 game fixes on · 7 more changes", with how
 *  many of the fixes are off when some are. */
export function differencesSummary(entries: DifferenceView[]): string {
  const fixes = entries.filter((e) => e.kind === "fix");
  const off = fixes.filter((e) => e.on === false).length;
  const others = entries.length - fixes.length;
  const parts: string[] = [];
  if (fixes.length) {
    const noun = fixes.length === 1 ? "game fix" : "game fixes";
    parts.push(off ? `${fixes.length - off} of ${fixes.length} ${noun} on` : `${fixes.length} ${noun} on`);
  }
  if (others) parts.push(`${others} ${fixes.length ? "more " : ""}${others === 1 ? "change" : "changes"}`);
  return parts.join(" · ");
}

/** A version key's two halves: `rac1/pal` -> [`rac1`, `pal`]. */
export function splitVersion(key: string): [string, string] {
  const at = key.indexOf("/");
  return at < 0 ? [key, ""] : [key.slice(0, at), key.slice(at + 1)];
}
