// How the game page talks about save files: the empty slots in one line, a slot's save time and a
// backup's name in a player's words. No Svelte or Tauri here, so it can be tested.
import type { SaveBackupInfo } from "./api";

/** The slots' numbers as people count them (1 to 5), in a sentence: "Slots 3 to 5 are empty". */
export function emptySlots(numbers: number[]): string {
  const n = [...numbers].sort((a, b) => a - b);
  if (n.length === 0) return "";
  if (n.length === 1) return `Slot ${n[0]} is empty`;
  const run = n.every((x, i) => i === 0 || x === (n[i - 1] ?? x) + 1);
  if (run) return `Slots ${n[0]} to ${n.at(-1)} are empty`;
  return `Slots ${n.slice(0, -1).join(", ")} and ${n.at(-1)} are empty`;
}

/** A slot's save time as the core writes it ("2026-10-09 17:01:21", the console's own clock), in
 *  seconds since 1970 for reports.when(); null when it cannot be read. */
export function stampSeconds(stamp: string | null): number | null {
  const m = stamp?.match(/^(\d{4})-(\d{2})-(\d{2})[ T](\d{2}):(\d{2})(?::(\d{2}))?$/);
  if (!m) return null;
  const [, y, mo, d, h, mi] = m.map(Number);
  const date = new Date(y ?? 0, (mo ?? 1) - 1, d ?? 1, h ?? 0, mi ?? 0, m[6] ? Number(m[6]) : 0);
  return Number.isNaN(date.getTime()) ? null : Math.floor(date.getTime() / 1000);
}

/** What a backup is called: its note, starting with a capital ("Before the last boss"), else "Backup". */
export function backupLabel(b: Pick<SaveBackupInfo, "note">): string {
  const note = b.note?.trim();
  return note ? note.charAt(0).toUpperCase() + note.slice(1) : "Backup";
}
