import { describe, expect, it } from "vitest";
import { hasPath, hidePaths } from "./paths";

describe("hidePaths", () => {
  it("cuts a folder or file to its last part, the user's name with it", () => {
    expect(hidePaths("cannot read /home/alice/.local/share/dev.openrac.launcher/plugins/difficulty").text).toBe(
      "cannot read difficulty",
    );
    expect(hidePaths("/Users/bob/OpenRAC/active/rac1/data/plugins.json: not valid JSON.").text).toBe(
      "plugins.json: not valid JSON.",
    );
    expect(hidePaths("C:\\Users\\carol\\AppData\\Roaming\\OpenRAC\\mods\\x.zip is not a mod").text).toBe(
      "x.zip is not a mod",
    );
    expect(hidePaths("in ~/Downloads/lock-strafe, no manifest").text).toBe("in lock-strafe, no manifest");
  });

  it("says whether it hid anything", () => {
    expect(hidePaths("/home/you/a").hidden).toBe(true);
    expect(hasPath("Needs OpenRAC with mod interface 2.0; this one has 1.0.")).toBe(false);
  });

  it("leaves links, repositories, fractions and buttons alone", () => {
    for (const text of [
      "Installed Lock-strafe from OpenRAC/rac1-mods.",
      "This repository is private: https://github.com/OpenRAC/rac1-mods/tree/master/x",
      "Hold L2 or R2, 1/2 of the time",
      "not for this disc (rac1/pal)",
    ]) {
      expect(hidePaths(text)).toEqual({ text, hidden: false });
    }
  });
});
