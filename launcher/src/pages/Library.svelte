<script lang="ts">
  // Every game, a tile each: can it be played, and the next thing to do. With
  // developer tools on, the tiles also say how far each decompilation is, and
  // the checkout's own actions follow (identify discs, place inputs, tests).
  import ActionRow from "$components/ActionRow.svelte";
  import GameTile from "$components/GameTile.svelte";
  import Icon from "$components/Icon.svelte";
  import { api } from "$lib/api";
  import { app, guard, refresh } from "$lib/app.svelte";

  const repo = { kind: "repository" } as const;
  const lib = $derived(app.library);
  const actions = $derived((lib?.repository ?? []).filter((a) => a.thisPlatform));
  const discs = $derived(
    lib?.games.flatMap((g) => g.versions).filter((v) => v.status.disc.state === "found").length ?? 0,
  );
  const versions = $derived(lib?.games.flatMap((g) => g.versions).length ?? 0);
  const developer = $derived(app.config?.developer ?? false);
</script>

<div class="page">
  <h1 class="sr">Games</h1>

  {#if app.libraryError}
    <div class="panel notice">
      <Icon name="warn" />
      <p class="grow">{app.libraryError}</p>
      <button class="primary" onclick={() => (app.page = "settings")}>Settings</button>
    </div>
  {/if}
  {#if lib?.actionsError}
    <div class="panel notice">
      <Icon name="warn" />
      <p class="grow">The games show, but no actions: {lib.actionsError}</p>
    </div>
  {/if}

  {#if lib}
    <div class="tiles">
      {#each lib.games as game, i (game.id)}
        <GameTile {game} delay={i * 60} />
      {/each}
    </div>

    {#if developer}
      <section class="panel">
        <div class="row">
          <h2 class="grow">This checkout</h2>
          <button class="ghost small" onclick={() => void refresh()}><Icon name="refresh" size={14} />Refresh</button>
        </div>
        <p class="where">
          <button class="link mono" onclick={() => void guard(api.openPath("."))} title="Open the folder"
            >{lib.root}</button
          >
          · {discs} of {versions} disc images found in <span class="mono">baserom/</span>
        </p>
        <ul>
          {#each actions as action (action.id)}<ActionRow {action} scope={repo} />{/each}
        </ul>
      </section>
    {/if}
  {/if}
</div>

<style>
  .page {
    max-width: 1180px;
    margin: 0 auto;
    padding: 28px 28px 60px;
    display: flex;
    flex-direction: column;
    gap: 22px;
  }

  .link {
    border: 0;
    background: none;
    padding: 0;
    color: var(--amber);
    font-weight: 400;
    text-decoration: underline;
    text-decoration-color: color-mix(in srgb, var(--amber) 40%, transparent);
  }

  .where {
    margin: 2px 0 12px;
    font-size: 14px;
    color: var(--soft);
    overflow-wrap: anywhere;
  }

  ul {
    margin: 0;
    padding: 0;
    list-style: none;
  }

  .notice {
    display: flex;
    align-items: center;
    gap: 12px;
    border-color: color-mix(in srgb, var(--amber) 45%, transparent);
    color: var(--amber);
  }

  .notice p {
    color: var(--text);
  }

  /* A heading for screen readers: the header's tab already names the page. */
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

  @media (max-width: 820px) {
    .tiles {
      grid-template-columns: minmax(0, 1fr);
    }
  }
</style>
