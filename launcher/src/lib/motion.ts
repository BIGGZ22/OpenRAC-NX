// The launcher's motion: short, eased, and only on what changed (a game opened, a view switched,
// something unfolded), moving with transform and opacity alone so it stays smooth. Svelte's
// transitions run outside CSS, so app.css's reduced-motion rule cannot reach them: with the
// system's "reduce motion" on, every duration here is 0.
import { cubicOut } from "svelte/easing";
import type { FlyParams, SlideParams } from "svelte/transition";

/** The player asked their system for less motion. */
export const reduced = typeof matchMedia === "function" && matchMedia("(prefers-reduced-motion: reduce)").matches;

/** What comes into view: up from 10 px below, fading in; DELAY staggers what follows. */
export function enter(delay = 0): FlyParams {
  return { y: 10, opacity: 0, duration: reduced ? 0 : 260, delay: reduced ? 0 : delay, easing: cubicOut };
}

/** What unfolds and folds away. */
export const fold: SlideParams = { duration: reduced ? 0 : 180, easing: cubicOut };
