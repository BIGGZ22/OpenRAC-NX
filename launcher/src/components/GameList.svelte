<script lang="ts">
  // The games in the list beside the Games and Mods tabs (SideList): one entry each, all the same
  // size, as OpenGOAL's launcher lists its games, the open one marked; it is the only way to
  // switch games. By default it is the Games tab's: each game's state for playing, a click opens
  // the game's page. Another page passes its own SELECTED game, ONSELECT and STATE (Mods: what can
  // be modded).
  import type { GameView } from "$lib/api";
  import { app, openVersion } from "$lib/app.svelte";
  import { gameArt } from "$lib/media";
  import { playerState, playerVersion } from "$lib/player.svelte";
  import { themeStyle } from "$lib/themes";
  import type { EntryState, SideEntry } from "$lib/labels";
  import SideList from "./SideList.svelte";

  let {
    selected,
    onselect,
    state,
    label = "Games",
  }: {
    selected?: string | null;
    onselect?: (game: GameView) => void;
    state?: (game: GameView) => EntryState | null;
    label?: string;
  } = $props();

  const games = $derived(app.library?.games ?? []);
  const opened = $derived(
    selected !== undefined ? selected : app.page === "game" ? (app.version?.split("/")[0] ?? null) : null,
  );

  /** What the entry says under the name: the page's own words, else the game's state for playing. */
  function stateOf(game: GameView): EntryState | null {
    if (state) return state(game);
    const v = playerVersion(game);
    const next = v ? playerState(game, v) : null;
    return next ? { text: next.status, tone: next.tone, busy: !!next.job } : null;
  }

  const entries = $derived<SideEntry[]>(
    games.map((game) => ({
      key: game.id,
      name: game.title,
      state: stateOf(game),
      art: gameArt(game.id),
      style: themeStyle(game.id),
      mark: !state && (app.crashes[game.id]?.recent ?? 0) > 0,
    })),
  );

  function pick(key: string) {
    const game = games.find((g) => g.id === key);
    if (!game) return;
    if (onselect) {
      onselect(game);
      return;
    }
    const v = playerVersion(game);
    if (v) openVersion(v.key);
  }
</script>

<SideList {entries} selected={opened} onpick={pick} {label} />
