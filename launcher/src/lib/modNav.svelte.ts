// Which mod's own page the Mods tab shows (ModPage.svelte), or none for the list. Opening one
// adds a history entry (#mod=<id>), so the window's Back, a mouse's back button and Escape all
// return to the list; ?mod=<id> opens one straight away (handy for screenshots, as ?page= is).

const KEY = "openracMod";

export const modNav = $state<{ mod: string | null }>({ mod: null });

/** The mod named in the address at start (?mod=id or #mod=id), if any. */
export function modFromAddress(location: { search: string; hash: string }): string | null {
  const query = /[?&]mod=([^&#]+)/.exec(location.search);
  if (query?.[1]) return decodeURIComponent(query[1]);
  const hash = /^#mod=(.+)$/.exec(location.hash);
  return hash?.[1] ? decodeURIComponent(hash[1]) : null;
}

export function openMod(id: string): void {
  if (modNav.mod === id) return;
  modNav.mod = id;
  try {
    history.pushState({ [KEY]: id }, "", `#mod=${encodeURIComponent(id)}`);
  } catch {
    // A webview that refuses: the page still changes, only Back does not take it back.
  }
}

/** Back to the list: through the history entry openMod added, so Back does not reopen it. */
export function closeMod(): void {
  if (modNav.mod === null) return;
  const state: unknown = history.state;
  modNav.mod = null;
  if (state && typeof state === "object" && KEY in state) history.back();
}

/** Leaves the page without a trace (another game, another tab): the list next time. */
export function resetMod(): void {
  modNav.mod = null;
}

/** Follows Back and Forward; returns the listener's removal. */
export function followHistory(): () => void {
  const pop = (e: PopStateEvent) => {
    const state: unknown = e.state;
    modNav.mod =
      state && typeof state === "object" && KEY in state ? String((state as Record<string, unknown>)[KEY]) : null;
  };
  window.addEventListener("popstate", pop);
  return () => {
    window.removeEventListener("popstate", pop);
  };
}
