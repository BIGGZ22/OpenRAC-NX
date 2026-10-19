// The launcher with a controller, without a mouse or a touch screen: the d-pad or the left stick
// moves the focus to the nearest control in that direction, A presses it, B goes back (closes a
// menu or a dialog, then returns to the games), LB and RB change tabs. On Android the activity
// hands the controller's buttons to `window.__openracPad` (MainActivity.kt); on a desktop they
// are read with the Gamepad API. Nothing here knows a page: it reads the document as it is.

export type PadButton = "up" | "down" | "left" | "right" | "accept" | "back" | "prev" | "next";
type Direction = "up" | "down" | "left" | "right";

const DIRECTIONS: readonly string[] = ["up", "down", "left", "right"];
const BUTTONS: readonly string[] = [...DIRECTIONS, "accept", "back", "prev", "next"];

/** A held direction moves again after REPEAT_DELAY_MS, then every REPEAT_MS. */
const REPEAT_DELAY_MS = 350;
const REPEAT_MS = 90;

const FOCUSABLE = 'button, a[href], input, select, textarea, summary, [tabindex]:not([tabindex="-1"])';

/** What B and Android's Back do once nothing is open; true when they did something. */
let backHome: () => boolean = () => false;

/** The buttons held, with the timer that repeats a direction. */
const held = new Map<PadButton, ReturnType<typeof setTimeout> | undefined>();
/** The control focused before a menu opened, focused again when it closes. */
let opener: HTMLElement | null = null;

function visible(el: HTMLElement): boolean {
  if (el.getClientRects().length === 0) return false;
  const style = getComputedStyle(el);
  return style.visibility !== "hidden" && style.display !== "none";
}

function usable(el: HTMLElement): boolean {
  return (
    !(el as HTMLButtonElement).disabled &&
    el.tabIndex !== -1 &&
    !el.closest("[inert], [aria-hidden=true]") &&
    visible(el)
  );
}

/** Where the focus may go: the open menu, else the topmost dialog, else the page. */
export function scope(doc: Document = document): HTMLElement {
  const menu = doc.querySelector<HTMLElement>('[role="menu"]');
  if (menu && visible(menu)) return menu;
  const dialogs = [...doc.querySelectorAll<HTMLElement>('[role="dialog"][aria-modal="true"]')].filter(visible);
  return dialogs.at(-1) ?? doc.body;
}

export function focusables(root: HTMLElement): HTMLElement[] {
  return [...root.querySelectorAll<HTMLElement>(FOCUSABLE)].filter(usable);
}

interface Box {
  left: number;
  right: number;
  top: number;
  bottom: number;
}

/**
 * How far `to` is from `from` going `dir`, or null when it is not that way: the gap along the
 * direction, plus twice how far it is off to the side (nothing when the two overlap there), so
 * that a control straight ahead wins over a nearer one diagonally.
 */
export function distance(from: Box, to: Box, dir: Direction): number | null {
  const centre = (a: number, b: number) => (a + b) / 2;
  const vertical = dir === "up" || dir === "down";
  const gap =
    dir === "down"
      ? to.top - from.bottom
      : dir === "up"
        ? from.top - to.bottom
        : dir === "right"
          ? to.left - from.right
          : from.left - to.right;
  // Ahead: past the current control's edge, or at least with its centre beyond it.
  const ahead =
    dir === "down"
      ? centre(to.top, to.bottom) > centre(from.top, from.bottom) && to.top >= from.top + 1
      : dir === "up"
        ? centre(to.top, to.bottom) < centre(from.top, from.bottom) && to.bottom <= from.bottom - 1
        : dir === "right"
          ? centre(to.left, to.right) > centre(from.left, from.right) && to.left >= from.left + 1
          : centre(to.left, to.right) < centre(from.left, from.right) && to.right <= from.right - 1;
  if (!ahead) return null;
  const side = vertical
    ? Math.max(0, to.left - from.right, from.left - to.right)
    : Math.max(0, to.top - from.bottom, from.top - to.bottom);
  return Math.max(0, gap) + 2 * side;
}

/** Whether `to` is in line with `from` across `dir`: beside it for left and right, above or below
 *  it for up and down. */
function inLine(from: Box, to: Box, dir: Direction): boolean {
  return dir === "up" || dir === "down"
    ? to.left < from.right && to.right > from.left
    : to.top < from.bottom && to.bottom > from.top;
}

/** The control nearest `from` going `dir` among `items`, or null; one in line with it first. */
export function nearest<T>(from: Box, items: { item: T; box: Box }[], dir: Direction): T | null {
  let best: T | null = null;
  let score = Infinity;
  let lined = false;
  for (const { item, box } of items) {
    const d = distance(from, box, dir);
    if (d === null) continue;
    const line = inLine(from, box, dir);
    if ((line && !lined) || (line === lined && d < score)) {
      score = d;
      best = item;
      lined = line;
    }
  }
  return best;
}

/** The control last focused on the page, focused again when a dialog or a menu closes. */
let lastOnPage: HTMLElement | null = null;

function show(el: HTMLElement) {
  if (!el.closest('[role="dialog"], [role="menu"]')) lastOnPage = el;
  el.focus({ preventScroll: true });
  el.scrollIntoView({ block: "nearest", inline: "nearest" });
}

function current(items: HTMLElement[]): HTMLElement | null {
  const active = document.activeElement;
  return active instanceof HTMLElement && items.includes(active) ? active : null;
}

/** The first control to focus in a scope: in a page, the one focused last, else the first in its
 *  main content; in a dialog, its main button. */
function start(root: HTMLElement, items: HTMLElement[]): HTMLElement | undefined {
  if (root === document.body) {
    if (lastOnPage && items.includes(lastOnPage)) return lastOnPage;
    const main = document.querySelector("main");
    const inMain = items.find((el) => main?.contains(el));
    if (inMain) return inMain;
  }
  return items.find((el) => el.matches("[autofocus], .primary, .btn-primary")) ?? items[0];
}

function scrollable(el: HTMLElement | null): HTMLElement | null {
  for (let at = el; at; at = at.parentElement) {
    const style = getComputedStyle(at);
    if (/(auto|scroll)/.test(style.overflowY) && at.scrollHeight > at.clientHeight) return at;
  }
  return document.scrollingElement as HTMLElement | null;
}

function move(dir: Direction) {
  const root = scope();
  const items = focusables(root);
  const from = current(items);
  if (!from) {
    const first = start(root, items);
    if (first) show(first);
    return;
  }
  // A range moves its value sideways.
  if (from instanceof HTMLInputElement && from.type === "range" && (dir === "left" || dir === "right")) {
    if (dir === "left") from.stepDown();
    else from.stepUp();
    from.dispatchEvent(new Event("input", { bubbles: true }));
    from.dispatchEvent(new Event("change", { bubbles: true }));
    return;
  }
  const box = from.getBoundingClientRect();
  const next = nearest(
    box,
    items.filter((el) => el !== from).map((el) => ({ item: el, box: el.getBoundingClientRect() })),
    dir,
  );
  if (next) {
    show(next);
    return;
  }
  // Nothing further that way: the text there scrolls into view instead.
  if (dir === "up" || dir === "down") {
    const area = scrollable(from);
    area?.scrollBy({ top: (dir === "down" ? 1 : -1) * area.clientHeight * 0.4, behavior: "smooth" });
  }
}

function escape(target: Element) {
  target.dispatchEvent(new KeyboardEvent("keydown", { key: "Escape", code: "Escape", bubbles: true }));
}

function accept() {
  const items = focusables(scope());
  const el = current(items);
  if (!el) {
    const first = start(scope(), items);
    if (first) show(first);
    return;
  }
  if (el instanceof HTMLSelectElement) {
    try {
      el.showPicker();
    } catch {
      el.focus();
    }
    return;
  }
  if (el.getAttribute("aria-haspopup") === "menu") {
    opener = el;
    el.click();
    // The menu is drawn on the next frame: its first item takes the focus.
    requestAnimationFrame(() => {
      const menu = scope();
      if (menu.getAttribute("role") === "menu") focusables(menu)[0]?.focus();
    });
    return;
  }
  el.click();
  // A choice in a menu closes it: back to the button that opened it.
  if (el.getAttribute("role") === "menuitem") restoreOpener();
}

function restoreOpener() {
  const el = opener;
  opener = null;
  requestAnimationFrame(() => {
    if (el?.isConnected && usable(el)) show(el);
  });
}

/** B, or Android's Back: true when it did something here (else Android leaves the launcher). */
export function back(): boolean {
  const active = document.activeElement;
  // Typing: B puts the keyboard away first.
  const typing =
    active instanceof HTMLTextAreaElement ||
    (active instanceof HTMLInputElement && !["range", "checkbox", "radio", "button", "submit"].includes(active.type));
  if (typing) {
    active.blur();
    return true;
  }
  const root = scope();
  if (root !== document.body) {
    escape(active instanceof HTMLElement && root.contains(active) ? active : root);
    if (root.getAttribute("role") === "menu") restoreOpener();
    return true;
  }
  return backHome();
}

/** LB and RB: the tab before or after the one open. */
function tab(step: number) {
  const tabs = [...document.querySelectorAll<HTMLElement>("header nav .tab")];
  if (!tabs.length || scope() !== document.body) return;
  const at = tabs.findIndex((t) => t.classList.contains("on"));
  const next = tabs[(at + step + tabs.length) % tabs.length];
  next?.click();
  // The new page starts at its content.
  lastOnPage = null;
  (document.activeElement as HTMLElement | null)?.blur();
}

function act(button: PadButton) {
  switch (button) {
    case "up":
    case "down":
    case "left":
    case "right":
      move(button);
      break;
    case "accept":
      accept();
      break;
    case "back":
      back();
      break;
    case "prev":
      tab(-1);
      break;
    case "next":
      tab(1);
      break;
  }
}

/** The controller is in use: focus is drawn (app.css); a touch or the mouse hides it again. */
function usingPad(on: boolean) {
  const mode = on ? "pad" : "pointer";
  if (document.documentElement.dataset.input !== mode) document.documentElement.dataset.input = mode;
}

/** A button pressed (`down:accept`) or let go (`up:accept`). A press while held is ignored, so
 *  that a hat seen both as keys and as an axis acts once. */
export function padEvent(event: string) {
  const [kind, name] = event.split(":");
  if (!name || !BUTTONS.includes(name)) return;
  const button = name as PadButton;
  if (kind === "down") {
    if (held.has(button)) return;
    usingPad(true);
    act(button);
    const repeat = () => {
      act(button);
      held.set(button, setTimeout(repeat, REPEAT_MS));
    };
    held.set(button, DIRECTIONS.includes(button) ? setTimeout(repeat, REPEAT_DELAY_MS) : undefined);
  } else if (kind === "up") {
    clearTimeout(held.get(button));
    held.delete(button);
  }
}

/** Every held button let go: the launcher lost the window (the game, a picker, a system dialog)
 *  and their ups go elsewhere. Without it the next press of a button held then was ignored, and a
 *  direction held then went on moving the focus. */
export function releasePad() {
  for (const timer of held.values()) clearTimeout(timer);
  held.clear();
}

// ---- a desktop's controllers, through the Gamepad API ----

/** The standard mapping's buttons (w3c.github.io/gamepad), and the left stick past half way. */
const STANDARD: [number, PadButton][] = [
  [0, "accept"],
  [1, "back"],
  [4, "prev"],
  [5, "next"],
  [9, "accept"],
  [12, "up"],
  [13, "down"],
  [14, "left"],
  [15, "right"],
];

function pollGamepads(previous: Set<PadButton>) {
  const now = new Set<PadButton>();
  for (const pad of navigator.getGamepads()) {
    if (pad?.mapping !== "standard") continue;
    for (const [index, button] of STANDARD) if (pad.buttons[index]?.pressed) now.add(button);
    const [x = 0, y = 0] = pad.axes;
    if (x <= -0.5) now.add("left");
    if (x >= 0.5) now.add("right");
    if (y <= -0.5) now.add("up");
    if (y >= 0.5) now.add("down");
  }
  for (const button of now) if (!previous.has(button)) padEvent(`down:${button}`);
  for (const button of previous) if (!now.has(button)) padEvent(`up:${button}`);
  return now;
}

/**
 * Starts reading the controller. `home` is what B does once nothing is open (back to the games);
 * true when it did something. `gamepadApi`: read controllers with the Gamepad API (a desktop's;
 * Android's come from its activity).
 */
export function startPad(home: () => boolean, gamepadApi: boolean) {
  backHome = home;
  const w = window as unknown as {
    __openracPad?: typeof padEvent;
    __openracPadRelease?: typeof releasePad;
    __openracBack?: typeof back;
  };
  w.__openracPad = padEvent;
  w.__openracPadRelease = releasePad;
  w.__openracBack = back;
  window.addEventListener("blur", releasePad);
  document.addEventListener("visibilitychange", () => {
    if (document.hidden) releasePad();
  });
  window.addEventListener("pointerdown", () => {
    usingPad(false);
  });
  if (!gamepadApi || typeof navigator.getGamepads !== "function") return;
  let pressed = new Set<PadButton>();
  let polling = false;
  const poll = () => {
    pressed = pollGamepads(pressed);
    if (navigator.getGamepads().some((p) => p)) requestAnimationFrame(poll);
    else polling = false;
  };
  window.addEventListener("gamepadconnected", () => {
    if (polling) return;
    polling = true;
    requestAnimationFrame(poll);
  });
}
