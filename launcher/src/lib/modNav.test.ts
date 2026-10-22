import { describe, expect, it } from "vitest";
import { modFromAddress } from "./modNav.svelte";

describe("modFromAddress", () => {
  it("reads ?mod= and #mod=", () => {
    expect(modFromAddress({ search: "?page=mods&mod=difficulty", hash: "" })).toBe("difficulty");
    expect(modFromAddress({ search: "", hash: "#mod=keyboard-mouse" })).toBe("keyboard-mouse");
    expect(modFromAddress({ search: "?page=mods", hash: "" })).toBeNull();
  });
});
