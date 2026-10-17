// The save editor's checks on the page: the same limits core/src/save_edit.rs enforces, so a
// field shows its problem before Save is pressed. The Rust side checks again before it writes.
import type { RawPatch, SaveFields, SaveView } from "./api";

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

// ---- the advanced editor: chunks as bytes ------------------------------------------

/** A chunk's key: `g:<id>` in the global section, `<level>:<id>` in a planet's. */
export function chunkKey(section: number | null, id: number): string {
  return `${section === null ? "g" : String(section)}:${String(id)}`;
}

export function bytesToHex(bytes: number[]): string {
  return bytes.map((b) => b.toString(16).padStart(2, "0")).join(" ");
}

/** Bytes from hex in any spacing, or an error in a player's words. */
export function hexToBytes(text: string): number[] | string {
  const digits = text.replace(/\s+/g, "");
  if (!/^[0-9a-fA-F]*$/.test(digits)) return "Only 0-9 and a-f";
  if (digits.length % 2 !== 0) return "An odd number of digits";
  const out: number[] = [];
  for (let i = 0; i < digits.length; i += 2) out.push(parseInt(digits.slice(i, i + 2), 16));
  return out;
}

/** Every chunk of a view as bytes, by key. */
export function chunkBytes(view: SaveView): Record<string, number[]> {
  const out: Record<string, number[]> = {};
  for (const c of view.chunks) {
    const b = hexToBytes(c.hex);
    out[chunkKey(c.section, c.id)] = typeof b === "string" ? [] : b;
  }
  return out;
}

/** The chunks whose bytes differ from the view's, as the raw change write_save takes. */
export function rawPatches(view: SaveView, bytes: Record<string, number[]>): RawPatch[] {
  return view.chunks
    .map((c) => ({ c, now: bytes[chunkKey(c.section, c.id)] ?? [] }))
    .filter(({ c, now }) => bytesToHex(now) !== c.hex)
    .map(({ c, now }) => ({ section: c.section, id: c.id, hex: bytesToHex(now) }));
}

export function readI32(b: number[], i: number): number {
  return (b[i * 4] ?? 0) | ((b[i * 4 + 1] ?? 0) << 8) | ((b[i * 4 + 2] ?? 0) << 16) | ((b[i * 4 + 3] ?? 0) << 24) | 0;
}

export function writeI32(b: number[], i: number, v: number): void {
  const n = v | 0;
  for (let k = 0; k < 4; k++) b[i * 4 + k] = (n >>> (8 * k)) & 0xff;
}

// ---- quick actions: the same rules as the single edits ----------------------------
// Giving goes through setOwned (a weapon given empty comes full); the Rust side then keeps the
// vendor stock, the acquired flags and the quick select in step, item by item, as for one edit.

/** Gives every item of a kind ("weapon" or "gadget") the save does not have. */
export function giveAll(view: SaveView, f: SaveFields, kind: "weapon" | "gadget"): void {
  for (const info of view.items.filter((i) => i.kind === kind)) setOwned(view, f, info.id, true);
}

/** Takes every item of a kind away. */
export function takeAll(view: SaveView, f: SaveFields, kind: "weapon" | "gadget"): void {
  for (const info of view.items.filter((i) => i.kind === kind)) setOwned(view, f, info.id, false);
}

/** Fills the ammo of every weapon the save owns. */
export function fillAllAmmo(view: SaveView, f: SaveFields): void {
  for (const item of f.items) {
    const max = view.items.find((i) => i.id === item.id)?.maxAmmo ?? null;
    if (item.owned && max !== null) item.ammo = max;
  }
}

/** Every gold bolt found, or none. */
export function setAllGoldBolts(f: SaveFields, found: boolean): void {
  for (const p of f.planets) p.goldBolts = p.goldBolts.map(() => found);
}

/** Every ship destination unlocked, or locked but the planet the save loads on. */
export function setAllPlanets(view: SaveView, f: SaveFields, unlocked: boolean): void {
  for (const p of f.planets) {
    if (!view.planets.find((i) => i.id === p.id)?.canUnlock) continue;
    p.unlocked = unlocked || p.id === f.planet;
  }
}

/** The slot's title: "Slot 2 · Kerwan · 4,890 bolts · 3h 12m", from the save as read. */
export function slotTitle(view: SaveView): string {
  const s = view.playedSeconds;
  const h = Math.floor(s / 3600);
  const m = Math.floor((s % 3600) / 60);
  const time = h > 0 ? `${String(h)}h ${String(m)}m` : `${String(m)}m`;
  return [
    `Slot ${String(view.slot + 1)}`,
    view.planetName ?? "Unknown planet",
    `${view.fields.bolts.toLocaleString("en-US")} bolts`,
    time,
  ].join(" · ");
}
