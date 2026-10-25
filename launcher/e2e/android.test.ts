// The Android launcher's pages, as the browser preview draws them (?android, src/lib/mock.ts), at
// the Ayn Odin 3's screen shape: each page opens without an error, and what Android has, or must
// not have, is there. A few seconds in all: one Vite server and one browser for every test.
// `yarn test:android`, with Playwright's headless Chromium (`yarn playwright-core install
// chromium-headless-shell`, once) or the browser OPENRAC_E2E_CHROMIUM names.
import { chromium, type Browser, type Page } from "playwright-core";
import { createServer, type ViteDevServer } from "vite";
import { afterAll, afterEach, beforeAll, beforeEach, describe, expect, it } from "vitest";

let server: ViteDevServer;
let browser: Browser;
let page: Page;
let errors: string[] = [];
let base = "";

beforeAll(async () => {
  server = await createServer({
    configFile: "vite.config.ts",
    logLevel: "error",
    server: { port: 0, strictPort: false },
  });
  await server.listen();
  base = server.resolvedUrls?.local[0] ?? "";
  // Vitest gives the environment's variables in import.meta.env.
  const executablePath = import.meta.env.OPENRAC_E2E_CHROMIUM as string | undefined;
  browser = await chromium.launch(executablePath ? { executablePath } : {});
}, 60_000);

afterAll(async () => {
  await browser.close();
  await server.close();
});

beforeEach(async () => {
  errors = [];
  page = await browser.newPage({ viewport: { width: 960, height: 540 } });
  page.on("pageerror", (e) => errors.push(e.message));
  page.on("console", (m) => {
    if (m.type() === "error") errors.push(m.text());
  });
});

afterEach(async () => {
  await page.close();
  // A page that threw is a regression whatever it shows.
  expect(errors).toEqual([]);
});

/** Opens the preview's Android layout with QUERY. No real look for a launcher update unless asked
 *  (?update): ?latest says there is none. */
async function open(query: string) {
  const q = new URLSearchParams(query);
  q.set("android", "");
  if (!q.has("update")) q.set("latest", "");
  await page.goto(`${base}?${q.toString()}`);
  await page.locator("nav").first().waitFor();
}

describe("Android launcher", () => {
  it("has the rail, and no desktop header", async () => {
    await open("");
    expect(await page.evaluate(() => document.documentElement.dataset.platform)).toBe("android");
    for (const name of ["Games", "Mods", "Settings"]) {
      await expect(page.getByRole("button", { name, exact: true }).count()).resolves.toBeGreaterThan(0);
    }
    expect(await page.locator(".discord").count()).toBe(0);
  });

  it("opens every page at one width", async () => {
    const lefts: number[] = [];
    for (const query of ["page=library", "version=rac1/pal", "page=mods", "page=tasks", "page=settings"]) {
      await open(`version=rac1/pal&${query}`);
      const box = await page.locator(".page").first().boundingBox();
      expect(box, query).not.toBeNull();
      lefts.push(Math.round(box?.x ?? -1));
    }
    expect(new Set(lefts).size).toBe(1);
  });

  it("shows Develop for the picked game, with its versions and only what Android runs", async () => {
    await open("developer&page=develop&version=rac1/pal");
    await expect(page.locator(".game .name").textContent()).resolves.toBe("Ratchet & Clank");
    const tabs = await page.getByRole("tab").allTextContents();
    expect(tabs.some((t) => t.startsWith("PAL"))).toBe(true);
    expect(tabs.some((t) => t.startsWith("NTSC"))).toBe(true);
    expect(tabs.some((t) => t.includes("Console"))).toBe(true);
    const text = (await page.locator("main").textContent()) ?? "";
    expect(text).not.toContain("This checkout");
    expect(text).not.toContain("more for other platforms");
    expect(text).not.toContain("Build the PS2 program");
    await page.getByRole("tab", { name: /NTSC/ }).click();
    await expect(page.getByRole("tab", { name: /NTSC/ }).getAttribute("aria-selected")).resolves.toBe("true");
  });

  it("shows Mods for the picked game, without the level editor", async () => {
    await open("page=mods&version=rac1/pal");
    await expect(page.locator(".game .name").textContent()).resolves.toBe("Ratchet & Clank");
    expect(await page.getByRole("tab", { name: /Level editor/ }).count()).toBe(0);
  });

  it("brings the game page in whole, its band not popping in after it", async () => {
    await open("version=rac1/pal");
    const hero = page.locator(".hero");
    await hero.waitFor();
    // At once, not after a fly-in of its own: the page's slide carries it.
    expect(await hero.evaluate((e) => getComputedStyle(e).opacity)).toBe("1");
    expect(await page.locator(".hero-bg").evaluate((e) => getComputedStyle(e).animationName)).toBe("none");
  });

  it("offers Rebuild for a game that is not built, and in the menu for one that is", async () => {
    await open("version=rac1/pal");
    await expect(page.locator(".go .big").textContent()).resolves.toContain("Rebuild");
    await open("built&version=rac1/pal");
    await expect(page.locator(".go .big").textContent()).resolves.toContain("Play");
    await page.locator(".go-row").getByRole("button", { name: "More actions" }).click();
    await expect(page.getByRole("menuitem", { name: "Rebuild the game" }).count()).resolves.toBe(1);
  });

  it("draws a running step's strip over the band's picture, not under its shade", async () => {
    await open("version=rac1/pal");
    await page.locator(".go .big").click();
    const strip = page.locator(".hero > .job");
    await strip.waitFor();
    expect(await strip.evaluate((e) => getComputedStyle(e).zIndex)).toBe("1");
  });

  it("shows no empty menu beside Set up, and keeps the main button in its place after it", async () => {
    const centre = async () => {
      const box = await page.locator(".go .big").boundingBox();
      return Math.round((box?.x ?? 0) + (box?.width ?? 0) / 2);
    };
    await open("fresh&version=rac1/pal");
    await page.locator(".go .big").waitFor();
    expect(await page.locator(".go-row").getByRole("button", { name: "More actions" }).count()).toBe(0);
    const setUp = await centre();
    await open("built&version=rac1/pal");
    await page.locator(".go .big").waitFor();
    expect(Math.abs((await centre()) - setUp)).toBeLessThanOrEqual(1);
  });

  it("spins the disc while a game starts", async () => {
    await open("launch&version=rac1/pal");
    await page.locator(".launch .disc").waitFor();
    await expect(page.locator(".launch h1").textContent()).resolves.toBe("Ratchet & Clank");
  });

  it("covers the launcher while the game runs, and steps aside when asked", async () => {
    await open("playing&built&version=rac1/pal");
    const running = page.getByRole("dialog", { name: "Ratchet & Clank" });
    await running.waitFor();
    await expect(running.getByRole("button", { name: /Return to game/ }).count()).resolves.toBe(1);
    await expect(running.getByRole("button", { name: /Quit game/ }).count()).resolves.toBe(1);
    await running.getByRole("button", { name: /Use the launcher meanwhile/ }).click();
    await running.waitFor({ state: "detached" });
    await expect(page.getByRole("button", { name: /Return to game/ }).count()).resolves.toBe(1);
  });

  it("offers a new launcher by itself, once until put off", async () => {
    await open("update");
    const offer = page.getByRole("dialog", { name: "A new launcher is ready" });
    await offer.waitFor();
    await offer.getByRole("button", { name: "Later" }).click();
    await offer.waitFor({ state: "detached" });
    await open("update");
    await page.waitForTimeout(500);
    expect(await offer.count()).toBe(0);
  });

  it("puts a theme on without the game's accent", async () => {
    await open("theme=oled&version=rac3/ntsc");
    // The accent eases from one game's to the next (app.css).
    await page.waitForTimeout(700);
    const [ink, amber] = await page.evaluate(() => {
      const s = getComputedStyle(document.documentElement);
      return [s.getPropertyValue("--ink").trim(), s.getPropertyValue("--amber").trim()];
    });
    expect(ink).toBe("#000000");
    expect(amber).toBe("rgb(255, 210, 63)");
  });
});
