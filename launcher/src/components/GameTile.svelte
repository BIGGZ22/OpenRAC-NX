<script lang="ts">
  // One game on Android's Games tab (Library.svelte): its art, its name, whether it can be
  // played, and the one thing to do next. The whole tile opens the game's page.
  import type { GameView } from "$lib/api";
  import { app, dismissCrashes, openVersion } from "$lib/app.svelte";
  import { pct } from "$lib/format";
  import { regionLabel } from "$lib/labels";
  import { gameArt, hideMissing } from "$lib/media";
  import { playerAct, playerState, playerVersion } from "$lib/player.svelte";
  import { themeStyle } from "$lib/themes";
  import Icon from "./Icon.svelte";

  /** `selected`: the game picked last, whose colour the launcher wears (themes.ts accent). */
  let { game, delay = 0, selected = false }: { game: GameView; delay?: number; selected?: boolean } = $props();

  // The game's still picture (media.ts), zoomed in a little while the tile has the pointer or the
  // focus; without one, the tile shows the game's colours.
  const art = $derived(gameArt(game.id));
  const v = $derived(playerVersion(game));
  const next = $derived(v ? playerState(game, v) : null);
  /** The tile offers the next step itself only when there is one to take. */
  const offers = $derived(!!next && !next.disabled && next.kind !== "installing" && (next.primary || !!next.job));
  /** With developer tools on: how far each version's decompilation is. */
  const matched = $derived(
    app.config?.developer
      ? game.versions.map((o) => `${regionLabel(o.name)} ${o.progress ? pct(o.progress.percent) : "–"}`).join(" · ")
      : "",
  );
</script>

{#if v && next}
  <article class="tile rise" class:selected style={`${themeStyle(game.id)}; animation-delay: ${delay}ms`}>
    {#if art}
      <img class="art" src={art} alt="" decoding="sync" onerror={hideMissing} />
    {/if}
    <div class="shade"></div>

    <button
      class="open"
      onclick={() => {
        openVersion(v.key);
      }}
      aria-label={`${game.title}: ${next.status}${(app.crashes[game.id]?.recent ?? 0) > 0 ? ". It crashed recently: a report is on its page" : ""}`}
    ></button>

    <div class="foot">
      <div class="name">
        <h2>{game.title}</h2>
        <p class={`status ${next.tone}`}>
          {#if next.job}<span class="spinner"></span>{:else}<span class="dot"></span>{/if}{next.status}
        </p>
        {#if matched}<p class="matched">{matched} matched</p>{/if}
        {#if (app.crashes[game.id]?.recent ?? 0) > 0}
          <p class="crashed">
            <Icon name="warn" size={13} />Crashed recently<span class="where">: a report is on its page</span>
            <button
              class="x"
              onclick={() => void dismissCrashes(game.id)}
              aria-label="Dismiss: hide this until the game crashes again"
              title="Dismiss: hide this until the game crashes again"
            >
              <Icon name="x" size={12} />
            </button>
          </p>
        {/if}
      </div>
      {#if offers}
        <button class="act" class:primary={next.primary} onclick={() => void playerAct(v, next)}>
          <Icon name={next.icon} size={18} />{next.kind === "set-up" ? "Set up" : next.label}
        </button>
      {/if}
    </div>
  </article>
{/if}

<style>
  .tile {
    position: relative;
    display: flex;
    align-items: flex-end;
    /* Two rows of tiles fill the window at any height it can have. */
    min-height: clamp(240px, calc((100vh - var(--nav-top) - var(--nav-bottom) - 108px) / 2), 400px);
    border-radius: 22px;
    overflow: hidden;
    background: linear-gradient(160deg, var(--from), var(--to));
    box-shadow: var(--shadow-sm);
    /* Its foot follows the tile's own width (below). */
    container-type: inline-size;
    transition:
      transform var(--medium) var(--ease),
      box-shadow var(--medium);
  }

  /* Raised under the pointer, or with the focus of a keyboard or a controller; a touch only
     presses it in while the finger is down, and leaves nothing behind. */
  @media (hover: hover) {
    .tile:hover {
      transform: translateY(-3px);
      box-shadow: var(--shadow);
    }

    .tile:hover .art {
      transform: scale(1.06);
    }

    .crashed .x:hover {
      opacity: 1;
      background: rgb(255 255 255 / 0.12);
    }
  }

  .tile:has(:focus-visible),
  :global(html[data-input="pad"]) .tile:focus-within {
    transform: translateY(-3px);
    box-shadow: var(--shadow);
  }

  .tile:has(.open:active) {
    transform: scale(0.98);
    transition-duration: var(--fast);
  }

  /* Android: compact cards, three rows of them in the screen held sideways, so that every game
     shows at once. */
  :global(html[data-platform="android"]) .tile {
    min-height: clamp(128px, calc((100vh - 2 * var(--page-top) - var(--nav-bottom) - 28px) / 3), 200px);
    border-radius: 18px;
  }

  :global(html[data-platform="android"]) .foot {
    padding: 14px 16px;
    gap: 12px;
  }

  :global(html[data-platform="android"]) h2 {
    font-size: 1.125rem;
  }

  /* The compact card says only that it crashed; the button's label and the page say the rest. */
  :global(html[data-platform="android"]) .crashed .where {
    display: none;
  }

  :global(html[data-platform="android"]) .act {
    padding: 8px 16px;
    font-size: 0.875rem;
  }

  /* The game picked last: ringed in the launcher's accent, which is its colour. */
  .tile::after {
    content: "";
    position: absolute;
    inset: 0;
    border-radius: inherit;
    box-shadow: inset 0 0 0 2px var(--amber);
    opacity: 0;
    transition: opacity var(--medium) var(--ease);
    pointer-events: none;
  }

  .tile.selected::after {
    opacity: 1;
  }

  .art {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    object-fit: cover;
    transition: transform 0.45s cubic-bezier(0.2, 0.8, 0.2, 1);
  }

  .tile:has(:focus-visible) .art,
  :global(html[data-input="pad"]) .tile:focus-within .art {
    transform: scale(1.06);
  }

  @media (prefers-reduced-motion: reduce) {
    .art {
      transition: none;
    }

    .tile .art {
      transform: none !important;
    }
  }

  .crashed {
    display: flex;
    align-items: center;
    gap: 6px;
    margin: 6px 0 0;
    font-size: 0.7812rem;
    color: var(--err);
  }

  .shade {
    position: absolute;
    inset: 0;
    background: linear-gradient(to top, rgb(10 11 16 / 0.94) 0%, rgb(10 11 16 / 0.55) 38%, rgb(10 11 16 / 0.05) 72%);
  }

  /* The tile's own click target sits under the foot's button. */
  .open {
    position: absolute;
    inset: 0;
    border: 0;
    border-radius: inherit;
    background: none;
    padding: 0;
  }

  .open:focus-visible {
    outline-offset: -4px;
  }

  .foot {
    position: relative;
    width: 100%;
    display: flex;
    align-items: flex-end;
    justify-content: space-between;
    gap: 16px;
    padding: 20px 22px;
    pointer-events: none;
  }

  .foot .act,
  .foot .x {
    pointer-events: auto;
  }

  .crashed .x {
    display: inline-flex;
    padding: 3px;
    color: inherit;
    background: none;
    border: 0;
    border-radius: 50%;
    opacity: 0.75;
  }

  h2 {
    font-family: var(--title-font);
    font-weight: var(--title-weight);
    font-size: 1.375rem;
    line-height: 1.2;
    color: var(--on-game);
    text-wrap: balance;
  }

  .status {
    display: flex;
    align-items: center;
    gap: 8px;
    margin-top: 6px;
    font-size: 0.875rem;
    font-weight: 500;
    color: var(--soft);
  }

  .matched {
    margin-top: 2px;
    font-size: 0.8125rem;
    color: var(--soft);
    font-variant-numeric: tabular-nums;
  }

  .status.ok {
    color: var(--ok);
  }

  .status.warn {
    color: var(--amber);
  }

  .dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: currentColor;
  }

  .act {
    flex: none;
    font-size: 0.9375rem;
    padding: 10px 22px;
    background: rgb(0 0 0 / 0.5);
    border-color: rgb(255 255 255 / 0.25);
  }

  /* A narrow tile: its button under its name, the tile's width. */
  @container (max-width: 340px) {
    .foot {
      flex-direction: column;
      align-items: stretch;
      gap: 14px;
    }

    .act {
      justify-content: center;
    }
  }

  .act.primary {
    background: linear-gradient(to bottom, var(--bar-top), var(--bar-bottom));
    border-color: rgb(0 0 0 / 0.3);
  }
</style>
