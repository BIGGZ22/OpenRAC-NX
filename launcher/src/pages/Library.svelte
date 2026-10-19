<script lang="ts">
  // The Games tab's way in, as OpenGOAL's launcher opens: straight onto a game, the one the
  // player used last (config.lastVersion) or else the first, with the game list beside it to
  // switch. Without a library (no OpenRAC folder, a broken game.json) it says why, with the way
  // to Settings.
  // On Android it is the games as tiles instead, two to a row (GameTile), as the launcher had them
  // before the list: the navigation rail is already on the left there, and a list beside it left
  // the page too narrow. A tile opens its game's page, and the Games tab comes back here.
  import FolderPrompt from "$components/FolderPrompt.svelte";
  import GameTile from "$components/GameTile.svelte";
  import Icon from "$components/Icon.svelte";
  import { onAndroid } from "$lib/api";
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
    if (target && !onAndroid) openVersion(target);
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
  {:else if onAndroid && lib}
    <h1 class="sr">Games</h1>
    <FolderPrompt />
    {#if lib.actionsError}
      <div class="panel notice">
        <Icon name="warn" />
        <p class="grow">The games show, but no actions: {lib.actionsError}</p>
      </div>
    {/if}
    <div class="tiles">
      {#each lib.games as game, i (game.id)}
        <GameTile {game} delay={i * 60} />
      {/each}
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

  /* A heading for screen readers: the rail's tab already names the page. */
  .sr {
    position: absolute;
    width: 1px;
    height: 1px;
    overflow: hidden;
    clip-path: inset(50%);
  }

  .tiles {
    display: grid;
    grid-template-columns: repeat(2, minmax(0, 1fr));
    gap: 20px;
  }

  @media (max-width: 640px) {
    .tiles {
      grid-template-columns: minmax(0, 1fr);
    }
  }
</style>
