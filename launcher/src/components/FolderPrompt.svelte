<script lang="ts">
  // The first-run question, at the top of the game page: a player who has set nothing up and named
  // no games folder is asked for one, once; a folder that holds no game OpenRAC knows says so.
  import { onAndroid } from "$lib/api";
  import { app } from "$lib/app.svelte";
  import { chooseGamesFolder } from "$lib/player.svelte";
  import Icon from "./Icon.svelte";

  const lib = $derived(app.library);
  const nothingYet = $derived(!!lib && !lib.games.some((g) => g.install?.extracted));
  const askFolder = $derived(nothingYet && !app.config?.discsDir);
  const emptyFolder = $derived(nothingYet && !!app.config?.discsDir && app.discs.length === 0);
  let looking = $state(false);
  async function choose() {
    looking = true;
    await chooseGamesFolder();
    looking = false;
  }
</script>

{#if askFolder || emptyFolder}
  <section class="panel folder">
    <Icon name="folder" size={22} />
    <div class="grow">
      {#if onAndroid}
        <h2>Add your Ratchet &amp; Clank disc</h2>
        <p class="muted">
          Choose the image of your own disc (.iso or .chd) from this device's storage. It is copied into the app, a few
          GB, and the game is set up from it. OpenRAC never downloads a game.
        </p>
      {:else if askFolder}
        <h2>Where are your PlayStation 2 games?</h2>
        <p class="muted">
          Choose the folder with your disc images (.iso). OpenRAC finds the Ratchet &amp; Clank games in it and sets
          them up. Nothing is copied off your computer, and OpenRAC never downloads a game.
        </p>
      {:else}
        <h2>No Ratchet &amp; Clank game found there</h2>
        <p class="muted">
          <span class="mono">{app.config?.discsDir}</span> holds no disc image (.iso) of a game OpenRAC knows. Choose another
          folder, or set this game up and pick its image yourself.
        </p>
      {/if}
    </div>
    <button class="primary" disabled={looking} onclick={() => void choose()}>
      {#if looking}<span class="spinner"></span>{/if}{onAndroid
        ? "Choose disc image"
        : askFolder
          ? "Choose folder"
          : "Choose another"}
    </button>
  </section>
{/if}

<style>
  .folder {
    position: relative;
    z-index: 1;
    display: flex;
    align-items: center;
    gap: 16px;
    border-color: color-mix(in srgb, var(--amber) 45%, transparent);
  }

  .folder h2 {
    margin-bottom: 2px;
  }

  .folder p {
    margin: 0;
    font-size: 14px;
    overflow-wrap: anywhere;
  }

  .folder button {
    flex: none;
  }

  @media (max-width: 700px) {
    .folder {
      flex-direction: column;
      align-items: stretch;
    }
  }
</style>
