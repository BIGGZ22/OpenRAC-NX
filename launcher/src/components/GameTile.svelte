<script lang="ts">
  // One game in a player's library: its art, its name, whether it can be played,
  // and the one thing to do next. The whole tile opens the game's page.
  import type { GameView } from "$lib/api";
  import { openVersion } from "$lib/app.svelte";
  import { playerAct, playerState, playerVersion } from "$lib/player.svelte";
  import { themeStyle } from "$lib/themes";
  import Icon from "./Icon.svelte";

  let { game, delay = 0 }: { game: GameView; delay?: number } = $props();

  const MEDIA: Record<string, { bg: string; gif: string }> = {
    rac1: { bg: "/img/rac1-bg.webp", gif: "/img/rac1-gameplay.gif" },
    rac2: { bg: "/img/gc-bg.webp", gif: "/img/rac2-gameplay.gif" },
    rac3: { bg: "/img/uya-bg.webp", gif: "/img/rac3-gameplay.gif" },
    rac4: { bg: "/img/deadlocked-bg.webp", gif: "/img/rac4-gameplay.gif" },
  };

  const media = $derived(MEDIA[game.id] ?? null);
  const v = $derived(playerVersion(game));
  const next = $derived(v ? playerState(game, v) : null);
  /** The tile offers the next step itself only when there is one to take. */
  const offers = $derived(!!next && !next.disabled && next.kind !== "installing" && (next.primary || !!next.job));
  let hovered = $state(false);
</script>

{#if v && next}
  <article
    class="tile rise"
    style={`${themeStyle(game.id)}; animation-delay: ${delay}ms`}
    onmouseenter={() => (hovered = true)}
    onmouseleave={() => (hovered = false)}
  >
    {#if media}
      <img class="art" src={media.bg} alt="" />
      {#if hovered}<img class="art clip-art" src={media.gif} alt="" />{/if}
    {/if}
    <div class="shade"></div>

    <button
      class="open"
      onclick={() => {
        openVersion(v.key);
      }}
      aria-label={`${game.title}: ${next.status}`}
    ></button>

    <div class="foot">
      <div class="name">
        <h2>{game.title}</h2>
        <p class={`status ${next.tone}`}>
          {#if next.job}<span class="spinner"></span>{:else}<span class="dot"></span>{/if}{next.status}
        </p>
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
    min-height: clamp(240px, calc((100vh - 60px - 108px) / 2), 400px);
    border-radius: 22px;
    overflow: hidden;
    background: linear-gradient(160deg, var(--from), var(--to));
    box-shadow: var(--shadow-sm);
    transition:
      transform 0.2s cubic-bezier(0.2, 0.8, 0.2, 1),
      box-shadow 0.2s;
  }

  .tile:hover,
  .tile:focus-within {
    transform: translateY(-3px);
    box-shadow: var(--shadow);
  }

  .art {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    object-fit: cover;
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

  .foot .act {
    pointer-events: auto;
  }

  h2 {
    font-family: var(--title-font);
    font-weight: var(--title-weight);
    font-size: 22px;
    line-height: 1.2;
    color: #fff;
    text-wrap: balance;
  }

  .status {
    display: flex;
    align-items: center;
    gap: 8px;
    margin-top: 6px;
    font-size: 14px;
    font-weight: 500;
    color: var(--soft);
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
    font-size: 15px;
    padding: 10px 22px;
    background: rgb(0 0 0 / 0.5);
    border-color: rgb(255 255 255 / 0.25);
  }

  .act.primary {
    background: linear-gradient(to bottom, var(--bar-top), var(--bar-bottom));
    border-color: rgb(0 0 0 / 0.3);
  }
</style>
