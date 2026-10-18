// Each game's picture (public/img), under OpenRAC's game ids, with the site's file names: the art
// of its entry in the game list and its page. Where these come from is in docs/STYLE.md.
// A picture that is not there (size-matters-bg.webp until it is copied from the site) is left
// out where it is shown, and the game's colours carry it.
export const GAME_ART: Record<string, string> = {
  rac1: "/img/rac1-bg.webp",
  rac2: "/img/gc-bg.webp",
  rac3: "/img/uya-bg.webp",
  rac4: "/img/deadlocked-bg.webp",
  rac5: "/img/size-matters-bg.webp",
};

/** A game's picture, or null for a game without one (its entry then shows its colours). */
export function gameArt(game: string): string | null {
  return GAME_ART[game] ?? null;
}

/** For an <img onerror>: a picture that could not be loaded is hidden, so the colours show. */
export function hideMissing(e: Event): void {
  (e.currentTarget as HTMLElement).style.display = "none";
}
