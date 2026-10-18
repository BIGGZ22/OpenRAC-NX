import { describe, expect, it } from "vitest";
import type { SaveView } from "./api";
import {
  bytesToHex,
  changed,
  editList,
  fillAllAmmo,
  giveAll,
  setAllGoldBolts,
  setAllPlanets,
  slotTitle,
  takeAll,
  chunkBytes,
  chunkKey,
  fieldErrors,
  goldBoltCount,
  hexToBytes,
  ownedCount,
  rawPatches,
  readI32,
  setOwned,
  skillCount,
  writeI32,
} from "./saveEdit";

function view(): SaveView {
  return {
    serial: "SCES_509.16",
    slot: 0,
    fingerprint: "x",
    planetName: "Veldin",
    savedAt: null,
    fields: {
      planet: 1,
      bolts: 10,
      nanotech: "standard",
      items: [
        { id: 2, owned: false, ammo: null, gold: null },
        { id: 15, owned: false, ammo: 0, gold: false },
      ],
      planets: [{ id: 1, unlocked: true, goldBolts: [true, false, false] }],
      skillPoints: [true, false, false],
    },
    items: [
      { id: 2, name: "Heli-Pack", kind: "gadget", maxAmmo: null, hasGold: false },
      { id: 15, name: "Blaster", kind: "weapon", maxAmmo: 200, hasGold: true },
    ],
    planets: [{ id: 1, name: "Novalis", location: "Tobruk Crater", canUnlock: true, canLand: true }],
    levels: [],
    maxBolts: 9_999_999,
    chunks: [],
    playedSeconds: 3 * 3600 + 12 * 60 + 5,
    skillNames: [],
  };
}

describe("the save editor's checks", () => {
  it("accepts what the save holds", () => {
    const v = view();
    expect(fieldErrors(v, v.fields)).toEqual({});
    expect(changed(v, structuredClone(v.fields))).toBe(false);
  });

  it("names the field that is out of range", () => {
    const v = view();
    const f = structuredClone(v.fields);
    f.bolts = 10_000_000;
    const blaster = f.items.find((i) => i.id === 15);
    if (blaster) blaster.ammo = 201;
    expect(Object.keys(fieldErrors(v, f)).sort()).toEqual(["ammo-15", "bolts"]);
    f.bolts = 1.5;
    expect(fieldErrors(v, f).bolts).toBeDefined();
    expect(changed(v, f)).toBe(true);
    if (blaster) blaster.ammo = null;
    expect(fieldErrors(v, f)["ammo-15"]).toBeDefined();
  });

  it("fills an empty weapon when it is given", () => {
    const v = view();
    const f = structuredClone(v.fields);
    setOwned(v, f, 15, true);
    expect(f.items[1]).toMatchObject({ owned: true, ammo: 200 });
    setOwned(v, f, 2, true);
    expect(f.items[0]).toMatchObject({ owned: true, ammo: null });
  });

  it("counts skill points and gold bolts", () => {
    const f = view().fields;
    expect(skillCount(f)).toEqual([1, 3]);
    expect(goldBoltCount(f)).toEqual([1, 3]);
  });
});

describe("the advanced editor's bytes", () => {
  it("reads and writes hex in any spacing", () => {
    expect(hexToBytes("0a 0B\n0c0d")).toEqual([10, 11, 12, 13]);
    expect(hexToBytes("0g")).toBe("Only 0-9 and a-f");
    expect(hexToBytes("abc")).toBe("An odd number of digits");
    expect(bytesToHex([0, 255, 16])).toBe("00 ff 10");
  });

  it("reads and writes little-endian words", () => {
    const b = [0, 0, 0, 0, 0, 0, 0, 0];
    writeI32(b, 1, 123456);
    expect(b).toEqual([0, 0, 0, 0, 0x40, 0xe2, 1, 0]);
    expect(readI32(b, 1)).toBe(123456);
    writeI32(b, 0, -1);
    expect(readI32(b, 0)).toBe(-1);
  });

  it("sends only the chunks that changed", () => {
    const v = view();
    v.chunks = [
      { section: null, id: 1, size: 4, name: "Bolts", known: true, hex: "0a 00 00 00" },
      { section: 3, id: 3003, size: 4, name: "Gold bolts found", known: true, hex: "00 00 00 00" },
    ];
    const bytes = chunkBytes(v);
    expect(rawPatches(v, bytes)).toEqual([]);
    bytes[chunkKey(3, 3003)] = [1, 0, 0, 0];
    expect(rawPatches(v, bytes)).toEqual([{ section: 3, id: 3003, hex: "01 00 00 00" }]);
  });
});

describe("the quick actions", () => {
  it("give, fill and take the way single edits do", () => {
    const v = view();
    const f = structuredClone(v.fields);
    giveAll(v, f, "weapon");
    expect(f.items.map((i) => i.owned)).toEqual([false, true]);
    expect(f.items[1]?.ammo).toBe(200);
    giveAll(v, f, "gadget");
    expect(f.items[0]?.owned).toBe(true);
    const blaster = f.items[1];
    if (blaster) blaster.ammo = 3;
    fillAllAmmo(v, f);
    expect(f.items[1]?.ammo).toBe(200);
    takeAll(v, f, "weapon");
    expect(f.items[1]?.owned).toBe(false);
    expect(f.items[0]?.owned).toBe(true);
  });

  it("set every gold bolt and planet, keeping the current planet unlocked", () => {
    const v = view();
    const f = structuredClone(v.fields);
    setAllGoldBolts(f, true);
    expect(f.planets[0]?.goldBolts).toEqual([true, true, true]);
    setAllPlanets(v, f, false);
    expect(f.planets[0]?.unlocked).toBe(true); // the save is on Novalis
    f.planet = 3;
    setAllPlanets(v, f, false);
    expect(f.planets[0]?.unlocked).toBe(false);
  });

  it("titles the slot with what is in it", () => {
    expect(slotTitle(view())).toBe("Slot 1 · Veldin · 10 bolts · 3h 12m");
  });
});

describe("the list of changes", () => {
  it("is empty while nothing changed", () => {
    const v = view();
    expect(editList(v, structuredClone(v.fields))).toEqual([]);
  });

  it("says what changed in a player's words, by section", () => {
    const v = view();
    v.levels = [
      { id: 1, name: "Novalis", location: "Tobruk Crater", canUnlock: true, canLand: true },
      { id: 3, name: "Kerwan", location: "Metropolis", canUnlock: true, canLand: true },
    ];
    const f = structuredClone(v.fields);
    f.bolts = 50_000;
    f.nanotech = "ultra";
    setOwned(v, f, 15, true);
    setOwned(v, f, 2, true);
    f.planet = 3;
    setAllGoldBolts(f, true);
    f.skillPoints = [false, true, false];
    expect(editList(v, f)).toEqual([
      { section: "ratchet", text: "Bolts: 10 → 50,000" },
      { section: "ratchet", text: "Health: No upgrade → Ultra Nanotech" },
      { section: "weapons", text: "Given: Blaster" },
      { section: "gadgets", text: "Given: Heli-Pack" },
      { section: "planets", text: "Loads on: Novalis → Kerwan" },
      { section: "planets", text: "Gold bolts: 1 → 3 of 3" },
      { section: "skills", text: "Skill points: 1 earned, 1 taken away" },
    ]);
  });

  it("names ammo, gold and planets it changed", () => {
    const v = view();
    const blaster = v.fields.items[1];
    if (blaster) Object.assign(blaster, { owned: true, ammo: 100 });
    const f = structuredClone(v.fields);
    const now = f.items[1];
    if (now) Object.assign(now, { ammo: 200, gold: true });
    const novalis = f.planets[0];
    if (novalis) {
      novalis.unlocked = false;
      novalis.goldBolts = [false, true, false];
    }
    f.skillPoints = [true, true, true];
    expect(editList(v, f).map((e) => e.text)).toEqual([
      "Ammo: Blaster 100 → 200",
      "Made gold: Blaster",
      "Locked: Novalis",
      "Gold bolts changed on: Novalis",
      "Skill points: 1 → 3 of 3",
    ]);
  });

  it("counts what the save owns", () => {
    const v = view();
    const f = structuredClone(v.fields);
    expect(ownedCount(v, f, "weapon")).toEqual([0, 1]);
    giveAll(v, f, "weapon");
    expect(ownedCount(v, f, "weapon")).toEqual([1, 1]);
    expect(ownedCount(v, f, "gadget")).toEqual([0, 1]);
  });
});
