import { describe, expect, it } from "vitest";
import { androidUserAgent } from "./api";

describe("androidUserAgent", () => {
  it("knows Android's WebView", () => {
    expect(
      androidUserAgent(
        "Mozilla/5.0 (Linux; Android 14; AYN Odin3 Build/UKQ1.231108.001; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/130.0.6723.107 Mobile Safari/537.36",
      ),
    ).toBe(true);
  });

  it("takes WebKitGTK on a Linux tablet or handheld for the desktop it is", () => {
    expect(
      androidUserAgent(
        "Mozilla/5.0 (Linux; like Android 4.4) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile Safari/605.1.15",
      ),
    ).toBe(false);
    expect(
      androidUserAgent(
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/18.0 Safari/605.1.15",
      ),
    ).toBe(false);
  });
});
