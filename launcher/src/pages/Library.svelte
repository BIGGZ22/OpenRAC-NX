<script lang="ts">
  // Every game. A player sees a tile per game: can it be played, and the next
  // thing to do. With developer tools on, each version's progress and inputs,
  // and the actions for the checkout as a whole (identify discs, place inputs).
  import ActionCard from "$components/ActionCard.svelte";
  import GameCard from "$components/GameCard.svelte";
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

<div class="page" class:sections={developer}>
  {#if developer}
    <div class="intro row">
      <div class="grow">
        <span class="sec-num"></span>
        <h1>Your games</h1>
        {#if lib}
          <p class="muted">
            {lib.games.length} games, {versions} versions · {discs} of {versions} discs found ·
            <button class="link mono" onclick={() => void guard(api.openPath("."))} title="Open the folder"
              >{lib.root}</button
            >
          </p>
        {/if}
      </div>
      <button class="ghost" onclick={() => void refresh()}><Icon name="refresh" size={16} />Refresh</button>
    </div>
  {:else}
    <h1 class="sr">Games</h1>
  {/if}

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
    {#if developer}
      <div class="games">
        {#each lib.games as game, i (game.id)}
          <GameCard {game} delay={i * 80} />
        {/each}
      </div>
    {:else}
      <div class="tiles">
        {#each lib.games as game, i (game.id)}
          <GameTile {game} delay={i * 60} />
        {/each}
      </div>
    {/if}

    {#if developer}
      <section class="panel start">
        <div class="row">
          <div class="grow">
            <span class="sec-num"></span>
            <h2>Getting started</h2>
          </div>
        </div>
        <ol class="steps muted">
          <li>
            Copy the images of your own discs into <code>baserom/</code> in the OpenRAC folder. OpenRAC never downloads a
            game.
          </li>
          <li><strong>Identify discs</strong> checks each image against the checksums OpenRAC knows.</li>
          <li><strong>Place inputs</strong> puts what each game's build reads where it expects it.</li>
          <li>Open a game to set up its toolchain, build it and play it.</li>
        </ol>
        <div class="actions">
          {#each actions as action (action.id)}
            <ActionCard {action} scope={repo} icon={action.kind === "check" ? "check" : "disc"} />
          {/each}
        </div>
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

  .intro {
    align-items: flex-end;
  }

  .intro h1 {
    margin: 4px 0 4px;
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

  .games {
    display: flex;
    flex-direction: column;
    gap: 24px;
    width: 100%;
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

  .start {
    display: flex;
    flex-direction: column;
    gap: 14px;
  }

  .start h2 {
    margin-top: 4px;
  }

  .steps {
    margin: 0;
    padding-left: 20px;
    display: flex;
    flex-direction: column;
    gap: 4px;
    font-size: 14px;
  }

  .steps strong {
    color: var(--text);
  }

  .actions {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(min(300px, 100%), 1fr));
    gap: 14px;
  }
</style>
