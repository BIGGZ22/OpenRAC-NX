import { describe, expect, it } from "vitest";

// Two of docs/STYLE.md's rules, checked over app.css and every component's <style>: hover styles
// sit inside @media (hover: hover), and colours are written as hex only on :root, where each has a
// name (and a line in STYLE.md).
const sources = {
  ...import.meta.glob<string>("../**/*.svelte", { query: "?raw", import: "default", eager: true }),
  ...import.meta.glob<string>("../app.css", { query: "?raw", import: "default", eager: true }),
};

interface Rule {
  /** Where it is: the file and the line its block opens on. */
  where: string;
  /** Its selector or at-rule, and those of the blocks around it, outermost first. */
  path: string[];
  /** Its declarations, as written. */
  declarations: string[];
}

/** The CSS of a component (its <style>) or of a stylesheet, with comments blanked, and the line
 *  it starts on. */
function css(file: string, text: string): { code: string; line: number } {
  let code = text;
  let line = 1;
  if (file.endsWith(".svelte")) {
    const open = /<style[^>]*>/.exec(text);
    if (!open) return { code: "", line };
    const start = open.index + open[0].length;
    const end = text.indexOf("</style>", start);
    code = text.slice(start, end < 0 ? undefined : end);
    line = text.slice(0, start).split("\n").length;
  }
  return { code: code.replace(/\/\*[\s\S]*?\*\//g, (c) => c.replace(/[^\n]/g, " ")), line };
}

/** Every block with declarations in it, with the blocks it sits in. */
function rules(file: string, text: string): Rule[] {
  const { code, line: first } = css(file, text);
  const out: Rule[] = [];
  const stack: Rule[] = [];
  let buffer = "";
  let line = first;
  for (const ch of code) {
    if (ch === "{") {
      const path = [...(stack.at(-1)?.path ?? []), buffer.trim().replace(/\s+/g, " ")];
      stack.push({ where: `${file}:${line}`, path, declarations: [] });
      buffer = "";
    } else if (ch === ";" || ch === "}") {
      const top = stack.at(-1);
      if (top && buffer.trim()) top.declarations.push(buffer.trim());
      if (ch === "}") {
        const done = stack.pop();
        if (done?.declarations.length) out.push(done);
      }
      buffer = "";
    } else {
      buffer += ch;
    }
    if (ch === "\n") line++;
  }
  return out;
}

const all = Object.entries(sources).flatMap(([file, text]) => rules(file.replace(/^\.\.\//, "src/"), text));

describe("stylesheets", () => {
  it("finds the launcher's styles", () => {
    expect(Object.keys(sources).length).toBeGreaterThan(30);
    expect(all.length).toBeGreaterThan(500);
  });

  it("keep hover styles inside @media (hover: hover)", () => {
    const outside = all
      .filter((r) => r.path.at(-1)?.includes(":hover"))
      .filter((r) => !r.path.some((p) => /^@media\b.*\(hover:\s*hover\)/.test(p)))
      .map((r) => `${r.where} ${r.path.at(-1) ?? ""}`);
    expect(outside).toEqual([]);
  });

  it("write hex colours only on :root", () => {
    const loose = all
      .filter((r) => !r.path.includes(":root"))
      .flatMap((r) => r.declarations.filter((d) => /#[0-9a-f]{3,8}\b/i.test(d)).map((d) => `${r.where} ${d}`));
    expect(loose).toEqual([]);
  });
});
