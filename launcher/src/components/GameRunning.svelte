<script lang="ts">
  // Android: the game runs beside the launcher. While it does, the launcher in front is a screen
  // that says so, over the game's picture, with the three things to do: go back to it, end it, or
  // put this aside to use the launcher while the game waits (app.runningAside, until the next start).
  import { app, gameWindow } from "$lib/app.svelte";
  import { onAndroid } from "$lib/api";
  import { gameArt, hideMissing } from "$lib/media";
  import { enter, reduced } from "$lib/motion";
  import { themeStyle } from "$lib/themes";
  import { fade, fly } from "svelte/transition";
  import Icon from "./Icon.svelte";

  /** The running game's id ("rac1"), from its library ("openrac-rac1-pal"); null when unknown. */
  const game = $derived(app.game?.library?.replace(/^openrac-/, "").split("-")[0] ?? null);
  const title = $derived((game && app.library?.games.find((g) => g.id === game)?.title) ?? "Your game");
  const shown = $derived(onAndroid && !!app.game?.running && !app.runningAside && !app.launching);
  const art = $derived(game ? gameArt(game) : null);
  let busy = $state(false);

  async function act(stop: boolean) {
    if (busy) return;
    busy = true;
    try {
      await gameWindow(stop);
    } finally {
      busy = false;
    }
  }

  /** The way back is in reach of a controller's A at once. */
  function focus(node: HTMLElement) {
    node.focus({ preventScroll: true });
  }
</script>

{#if shown}
  <div
    class="running"
    role="dialog"
    aria-modal="true"
    aria-labelledby="running-title"
    style={game ? themeStyle(game) : ""}
    transition:fade={{ duration: reduced ? 0 : 260 }}
  >
    {#if art}<img class="art" src={art} alt="" decoding="async" onerror={hideMissing} />{/if}
    <div class="shade"></div>
    <div class="middle" in:fly|global={enter(80)}>
      <p class="state"><span class="pulse"></span>Running</p>
      <h1 id="running-title">{title}</h1>
      <p class="muted">It waits where you left it while the launcher is open.</p>
      <div class="actions">
        <button class="primary" onclick={() => void act(false)} disabled={busy} use:focus>
          <Icon name="play" size={18} />Return to game
        </button>
        <button onclick={() => void act(true)} disabled={busy}><Icon name="x" size={18} />Quit game</button>
      </div>
      <button class="aside" onclick={() => (app.runningAside = true)}>Use the launcher meanwhile</button>
    </div>
  </div>
{/if}

<style>
  .running {
    position: fixed;
    inset: 0;
    z-index: 55;
    display: grid;
    place-items: center;
    overflow: hidden;
    background: linear-gradient(160deg, var(--from, var(--panel)), var(--to, var(--ink-deep)));
  }

  .art {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    object-fit: cover;
    filter: blur(10px) saturate(1.05);
    /* A slow drift, so that the screen is alive without asking for attention. */
    animation: drift 24s ease-in-out infinite alternate;
  }

  .shade {
    position: absolute;
    inset: 0;
    background: radial-gradient(circle at 50% 45%, rgb(10 11 16 / 0.45), rgb(10 11 16 / 0.9) 75%);
  }

  .middle {
    position: relative;
    display: grid;
    justify-items: center;
    gap: 10px;
    padding: 0 24px;
    text-align: center;
  }

  h1 {
    margin: 0;
    font-family: var(--title-font, inherit);
    font-weight: var(--title-weight, 700);
    font-size: 1.75rem;
    color: var(--text);
    text-wrap: balance;
  }

  .state {
    display: flex;
    align-items: center;
    gap: 8px;
    margin: 0;
    color: var(--ok);
    font-weight: 600;
    letter-spacing: 0.04em;
    text-transform: uppercase;
    font-size: 0.8125rem;
  }

  .pulse {
    width: 9px;
    height: 9px;
    border-radius: 50%;
    background: currentColor;
    box-shadow: 0 0 0 0 currentColor;
    animation: pulse 1.8s ease-out infinite;
  }

  .muted {
    margin: 0 0 8px;
  }

  .actions {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    gap: 12px;
  }

  .actions button {
    min-width: 168px;
    justify-content: center;
  }

  .aside {
    margin-top: 6px;
    border: 0;
    background: none;
    color: var(--soft);
    font-weight: 500;
  }

  @keyframes pulse {
    0% {
      box-shadow: 0 0 0 0 color-mix(in srgb, currentColor 60%, transparent);
    }
    100% {
      box-shadow: 0 0 0 12px transparent;
    }
  }

  @keyframes drift {
    from {
      transform: scale(1.08) translate(-1%, -1%);
    }
    to {
      transform: scale(1.16) translate(1%, 1%);
    }
  }
</style>
