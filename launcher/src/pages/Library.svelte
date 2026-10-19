<script lang="ts">
  // The Games tab's way in, as OpenGOAL's launcher opens: straight onto a game, the one the
  // player used last (config.lastVersion) or else the first, with the game list beside it to
  // switch. Without a library (no OpenRAC folder, a broken game.json) it says why, with the way
  // to Settings.
  import Icon from "$components/Icon.svelte";
  import { app, openVersion } from "$lib/app.svelte";
  import { playerVersion } from "$lib/player.svelte";

  const lib = $derived(app.library);
  /** The version to open: the last one used, when it is still in the library, else the first game's. */
  const target = $derived.by(() => {
    if (!lib) return null;
    const known = lib.games.flatMap((g) => g.versions).some((v) => v.key === app.version);
    if (app.version && known) return app.version;
    const first = lib.games[0];
    return first ? (playerVersion(first)?.key ?? null) : null;
  });

  $effect(() => {
    if (target) openVersion(target);
  });
</script>

<div class="page">
  {#if app.libraryError}
    <div class="panel notice">
      <Icon name="warn" />
      <p class="grow">{app.libraryError}</p>
      <button class="primary" onclick={() => (app.page = "settings")}>Settings</button>
    </div>
  {:else if lib && !lib.games.length}
    <div class="panel notice">
      <Icon name="warn" />
      <p class="grow">This OpenRAC folder lists no games (games/*/game.json).</p>
      <button class="primary" onclick={() => (app.page = "settings")}>Settings</button>
    </div>
  {:else}
    <p class="muted row"><span class="spinner"></span>Reading your games…</p>
  {/if}
</div>

<style>
  .page {
    max-width: 920px;
    margin: 0 auto;
    padding: var(--page-top, 28px) var(--page-x, 28px) var(--page-bottom, 60px);
    display: flex;
    flex-direction: column;
    gap: var(--page-gap, 22px);
  }

  .notice {
    display: flex;
    align-items: center;
    gap: 12px;
    border-color: color-mix(in srgb, var(--amber) 45%, transparent);
    color: var(--amber);
  }

  .notice p {
    margin: 0;
    color: var(--text);
  }
</style>
