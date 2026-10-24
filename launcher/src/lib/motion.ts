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

/** The slide under way, if any. */
let sliding: ViewTransition | null = null;

// While a slide runs, the browser shows pictures of the pages and sends every press to the root
// element, not to what is under the pointer: a tab pressed then was lost. Such a press ends the
// slide where it was going and is passed on to what it was meant for.
if (slides) {
  document.addEventListener(
    "click",
    (e) => {
      const transition = sliding;
      if (!transition || e.target !== document.documentElement) return;
      e.stopPropagation();
      const { clientX: x, clientY: y } = e;
      transition.skipTransition();
      void transition.finished.finally(() => {
        const target = document.elementFromPoint(x, y)?.closest<HTMLElement>("button, a, [role='button']");
        target?.click();
      });
    },
    true,
  );
}

/** Makes CHANGE (a page or a game opened) as one slide, going WAY; at once where slides are off.
 *  A change while a slide is still under way goes into that slide's page as it is: a second slide
 *  would cut the first short and start again from a picture of the page it was leaving, which
 *  showed as the page jumping back and forth. */
export function slide(change: () => void, way: Way) {
  if (!slides || sliding) {
    change();
    return;
  }
  document.documentElement.dataset.nav = way;
  const transition = document.startViewTransition(async () => {
    change();
    await tick();
    await pictures();
  });
  sliding = transition;
  void transition.finished.finally(() => {
    if (sliding === transition) sliding = null;
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
