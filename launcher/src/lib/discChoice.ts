// For developers with more than one disc of a game in the games folder (core/src/disc_choice.rs):
// which version the game's page plays. Without a choice (a player, or one disc), the launcher
// picks as it always has: the disc set up, else the first found, else the first playable.
import type { DiscChoice } from "./api";

/** The choice the games folder offers for GAME, if any (only with developer tools on). */
export function choiceFor(choices: readonly DiscChoice[], game: string): DiscChoice | null {
  return choices.find((c) => c.game === game) ?? null;
}

/**
 * The version a player means by "this game": a developer's chosen disc, else the one set up
 * (OWN), else the first whose disc image was FOUND, else the first that can be played, else
 * the first.
 */
export function pickVersion<V extends { key: string }>(
  versions: readonly V[],
  pick: { chosen?: string | null; own?: string | null; found: readonly string[]; playable: (v: V) => boolean },
): V | null {
  return (
    versions.find((v) => v.key === pick.chosen) ??
    versions.find((v) => v.key === pick.own) ??
    versions.find((v) => pick.found.includes(v.key)) ??
    versions.find(pick.playable) ??
    versions.at(0) ??
    null
  );
}

/** The program a version's port is, as its logs and crash reports name it: `openrac-rac1-pal`. */
export function programOf(key: string): string {
  return `openrac-${key.replace("/", "-")}`;
}
