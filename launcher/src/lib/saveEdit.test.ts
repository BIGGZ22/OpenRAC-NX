import { describe, expect, it } from "vitest";
import type { SaveView } from "./api";
import { changed, fieldErrors, goldBoltCount, setOwned, skillCount } from "./saveEdit";

function view(): SaveView {
  return {
    serial: "SCES_509.16",
    slot: 0,
    fingerprint: "x",
    planetName: "Veldin",
    savedAt: null,
    fields: {
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
    planets: [{ id: 1, name: "Novalis", location: "Tobruk Crater", canUnlock: true }],
    maxBolts: 9_999_999,
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
