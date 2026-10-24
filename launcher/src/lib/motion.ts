// The launcher's motion: short, eased, and only on what changed (a game opened, a view switched,
// something unfolded), moving with transform and opacity alone so it stays smooth. Svelte's
// transitions run outside CSS, so app.css's reduced-motion rule cannot reach them: with the
// system's "reduce motion" on, every duration here is 0.
import { tick } from "svelte";
import { cubicOut } from "svelte/easing";
import type { FlyParams, SlideParams } from "svelte/transition";

/** The player asked their system for less motion. */
export const reduced = typeof matchMedia === "function" && matchMedia("(prefers-reduced-motion: reduce)").matches;

/** Whether a page, or the game on it, gives way to the next in one slide (the browser's view
 *  transitions, app.css), rather than simply changing. */
export const slides = !reduced && typeof document !== "undefined" && "startViewTransition" in document;

/** Which way a slide goes: to a tab further along the bar (or down the rail on Android) or back,
 *  or to a game further down the game list or up it. */
export type Way = "forward" | "back" | "down" | "up";

/** Makes CHANGE (a page or a game opened) as one slide, going WAY; at once where slides are off. */
export function slide(change: () => void, way: Way) {
  if (!slides) {
    change();
    return;
  }
  document.documentElement.dataset.nav = way;
  document.startViewTransition(async () => {
    change();
    await tick();
    await pictures();
  });
}

/** Waits, briefly, for the pictures of the page coming in: the slide shows the page as it is when
 *  this settles, and a picture still loading would pop in after the page had slid in without it. */
function pictures(): Promise<void> {
  const pending = [...document.querySelectorAll("main img")].filter(
    (img): img is HTMLImageElement => img instanceof HTMLImageElement && !img.complete,
  );
  if (!pending.length) return Promise.resolve();
  const loaded = Promise.all(pending.map((img) => img.decode().catch(() => undefined)));
  return Promise.race([loaded, new Promise((done) => setTimeout(done, 120))]).then(() => undefined);
}

/** What comes into view: up from 10 px below, fading in; DELAY staggers what follows. */
export function enter(delay = 0): FlyParams {
  return { y: 10, opacity: 0, duration: reduced ? 0 : 260, delay: reduced ? 0 : delay, easing: cubicOut };
}

/** What comes in with its page, staggered as enter(). Where the page itself slides in, its parts
 *  come with it rather than popping in one by one after it. */
export function arrive(delay = 0): FlyParams {
  return slides ? { duration: 0 } : enter(delay);
}

/** What unfolds and folds away. */
export const fold: SlideParams = { duration: reduced ? 0 : 180, easing: cubicOut };
