// The save editor's checks on the page: the same limits core/src/save_edit.rs enforces, so a
// field shows its problem before Save is pressed. The Rust side checks again before it writes.
import type { SaveFields, SaveView } from "./api";

/** A problem with what was typed, by field ("bolts", "ammo-15"), in a player's words. */
export function fieldErrors(view: SaveView, f: SaveFields): Record<string, string> {
  const errors: Record<string, string> = {};
  if (!Number.isInteger(f.bolts) || f.bolts < 0 || f.bolts > view.maxBolts) {
    errors.bolts = `From 0 to ${view.maxBolts.toLocaleString()}`;
  }
  for (const item of f.items) {
    const info = view.items.find((i) => i.id === item.id);
    if (!info?.maxAmmo) continue;
    const ammo = item.ammo;
    if (ammo === null || !Number.isInteger(ammo) || ammo < 0 || ammo > info.maxAmmo) {
      errors[`ammo-${String(item.id)}`] = `From 0 to ${String(info.maxAmmo)}`;
    }
  }
  return errors;
}

/** Whether `f` differs from what the save holds. */
export function changed(view: SaveView, f: SaveFields): boolean {
  return JSON.stringify(view.fields) !== JSON.stringify(f);
}

/** Turns an item on or off; a weapon given with no ammo comes full, so it is ready to use. */
export function setOwned(view: SaveView, f: SaveFields, id: number, owned: boolean): void {
  const item = f.items.find((i) => i.id === id);
  if (!item) return;
  item.owned = owned;
  const max = view.items.find((i) => i.id === id)?.maxAmmo ?? null;
  if (owned && max !== null && item.ammo === 0) item.ammo = max;
}

/** Skill points earned, of how many. */
export function skillCount(f: SaveFields): [number, number] {
  return [f.skillPoints.filter(Boolean).length, f.skillPoints.length];
}

/** Gold bolts found, of how many, over every planet. */
export function goldBoltCount(f: SaveFields): [number, number] {
  const all = f.planets.flatMap((p) => p.goldBolts);
  return [all.filter(Boolean).length, all.length];
}
