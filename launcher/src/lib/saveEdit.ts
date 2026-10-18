// The save editor's checks on the page: the same limits core/src/save_edit.rs enforces, so a
// field shows its problem before Save is pressed. The Rust side checks again before it writes.
import type { Nanotech, RawPatch, SaveFields, SaveView } from "./api";

/** The player view's sections, in page order. */
export type SaveSection = "ratchet" | "weapons" | "gadgets" | "planets" | "skills";

/** Ratchet's health upgrades, as the vendor sells them, with the health each gives. */
export const NANOTECH: { value: Nanotech; label: string; health: number }[] = [
  { value: "standard", label: "No upgrade", health: 4 },
  { value: "premium", label: "Premium Nanotech", health: 5 },
  { value: "ultra", label: "Ultra Nanotech", health: 8 },
];

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

/** Items of a kind owned, of how many. */
export function ownedCount(view: SaveView, f: SaveFields, kind: "weapon" | "gadget"): [number, number] {
  const ids = new Set(view.items.filter((i) => i.kind === kind).map((i) => i.id));
  const items = f.items.filter((i) => ids.has(i.id));
  return [items.filter((i) => i.owned).length, items.length];
}

/** One line of what the edits change, in a player's words, and the section it belongs to. */
export interface SaveEdit {
  section: SaveSection;
  text: string;
}

/**
 * What `f` changes in the save, for the list the player reviews before saving: one line per kind
 * of change ("Given: Devastator, Visibomb Gun"), so a quick action reads as the one change it was.
 */
export function editList(view: SaveView, f: SaveFields): SaveEdit[] {
  const was = view.fields;
  const out: SaveEdit[] = [];
  const add = (section: SaveSection, label: string, names: string[]) => {
    if (names.length) out.push({ section, text: `${label}: ${names.join(", ")}` });
  };
  const num = (n: number) => n.toLocaleString("en-US");
  const health = (n: Nanotech) => NANOTECH.find((c) => c.value === n)?.label ?? n;
  const level = (id: number) => view.levels.find((l) => l.id === id)?.name ?? `Level ${String(id)}`;

  if (f.bolts !== was.bolts) out.push({ section: "ratchet", text: `Bolts: ${num(was.bolts)} → ${num(f.bolts)}` });
  if (f.nanotech !== was.nanotech) {
    out.push({ section: "ratchet", text: `Health: ${health(was.nanotech)} → ${health(f.nanotech)}` });
  }

  for (const [kind, section] of [
    ["weapon", "weapons"],
    ["gadget", "gadgets"],
  ] as const) {
    const given: string[] = [];
    const taken: string[] = [];
    const ammo: string[] = [];
    const gold: string[] = [];
    const plain: string[] = [];
    for (const info of view.items.filter((i) => i.kind === kind)) {
      const a = was.items.find((i) => i.id === info.id);
      const b = f.items.find((i) => i.id === info.id);
      if (!a || !b) continue;
      if (a.owned !== b.owned) (b.owned ? given : taken).push(info.name);
      else if (a.ammo !== b.ammo && b.ammo !== null)
        ammo.push(`${info.name} ${String(a.ammo ?? 0)} → ${String(b.ammo)}`);
      if (a.gold !== b.gold && b.gold !== null) (b.gold ? gold : plain).push(info.name);
    }
    add(section, "Given", given);
    add(section, "Taken away", taken);
    add(section, "Ammo", ammo);
    add(section, "Made gold", gold);
    add(section, "No longer gold", plain);
  }

  if (f.planet !== was.planet) {
    out.push({ section: "planets", text: `Loads on: ${level(was.planet)} → ${level(f.planet)}` });
  }
  const unlocked: string[] = [];
  const locked: string[] = [];
  const bolts: string[] = [];
  for (const info of view.planets) {
    const a = was.planets.find((p) => p.id === info.id);
    const b = f.planets.find((p) => p.id === info.id);
    if (!a || !b) continue;
    if (a.unlocked !== b.unlocked) (b.unlocked ? unlocked : locked).push(info.name);
    if (a.goldBolts.join() !== b.goldBolts.join()) bolts.push(info.name);
  }
  add("planets", "Unlocked", unlocked);
  add("planets", "Locked", locked);
  const [boltsWas] = goldBoltCount(was);
  const [boltsNow, boltsAll] = goldBoltCount(f);
  if (boltsWas !== boltsNow) {
    out.push({
      section: "planets",
      text: `Gold bolts: ${String(boltsWas)} → ${String(boltsNow)} of ${String(boltsAll)}`,
    });
  } else {
    add("planets", "Gold bolts changed on", bolts);
  }

  const [skillsWas] = skillCount(was);
  const [skillsNow, skillsAll] = skillCount(f);
  if (skillsWas !== skillsNow) {
    out.push({
      section: "skills",
      text: `Skill points: ${String(skillsWas)} → ${String(skillsNow)} of ${String(skillsAll)}`,
    });
  } else {
    const earned = f.skillPoints.filter((on, k) => on && !was.skillPoints[k]).length;
    if (earned) {
      out.push({ section: "skills", text: `Skill points: ${String(earned)} earned, ${String(earned)} taken away` });
    }
  }
  return out;
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
