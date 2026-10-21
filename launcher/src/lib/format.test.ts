import { describe, expect, it } from "vitest";
import { elapsed, grouped, matchedLine, pct, size } from "./format";

describe("format", () => {
  it("writes percentages like the website", () => {
    expect(pct(59.3217)).toBe("59,32 %");
    expect(pct(0)).toBe("0,00 %");
  });

  it("groups bytes with narrow spaces", () => {
    expect(grouped(2202320)).toBe("2 202 320");
    expect(matchedLine(5, 1000)).toBe("5 of 1 000 code bytes matched");
  });

  it("writes sizes for people", () => {
    expect(size(512)).toBe("512 bytes");
    expect(size(4214784000)).toBe("4,2 GB");
    expect(size(1388100)).toBe("1,4 MB");
    expect(size(630_000_000)).toBe("630 MB");
    // Rounded before the unit: 999 999 bytes is a megabyte, not "1000 KB".
    expect(size(999_999)).toBe("1,0 MB");
    expect(size(999_999_999)).toBe("1,0 GB");
    expect(size(1000)).toBe("1,0 KB");
  });

  it("writes durations", () => {
    expect(elapsed(4200)).toBe("4 s");
    expect(elapsed(125_000)).toBe("2 min 05 s");
    expect(elapsed(3_780_000)).toBe("1 h 03 min");
  });
});
