<script lang="ts">
  import Header from "$components/Header.svelte";
  import Toasts from "$components/Toasts.svelte";
  import UpdateModal from "$components/UpdateModal.svelte";
  import { app, refresh, start, updateDiscordPresence, type Page } from "$lib/app.svelte";
  import Game from "./pages/Game.svelte";
  import Library from "./pages/Library.svelte";
  import Mods from "./pages/Mods.svelte";
  import Settings from "./pages/Settings.svelte";
  import Tasks from "./pages/Tasks.svelte";

  $effect(() => {
    // The layout (Settings → Appearance): src/layouts/compact.css applies only to "compact".
    document.documentElement.dataset.layout = app.config?.layout ?? "compact";
  });

  // Another page starts at its top, as it arrives (app.css: its sections rise in turn).
  let view = $state<HTMLElement | null>(null);
  let shown: Page | null = null;
  $effect(() => {
    if (app.page === shown) return;
    shown = app.page;
    view?.scrollTo({ top: 0 });
  });

  $effect(() => {
    // Sync Discord Rich Presence when page or game changes
    updateDiscordPresence(app.page, app.version);
  });

  $effect(() => {
    // ?page=tasks (and ?version=rac1/pal) open a page directly: handy for screenshots.
    const params = new URLSearchParams(location.search);
    void start().then(() => {
      const version = params.get("version");
      if (version) {
        app.version = version;
        app.page = "game";
      }
      const page = params.get("page") as Page | null;
      if (page) app.page = page;
      if (params.get("modal") === "update") {
        app.update = {
          version: "0.5.3",
          tag: "v0.5.3",
          url: "https://github.com/OpenRAC/OpenRAC/releases",
          name: "OpenRAC Launcher v0.5.3",
          body: "OpenRAC Launcher release 0.5.3",
          publishedAt: "2026-10-16",
          canInstallDirectly: true,
        };
        app.showUpdateModal = true;
      }
    });
    // Pick up what changed outside the launcher (a build in a terminal, a new disc).
    const onFocus = () => void refresh();
    window.addEventListener("focus", onFocus);
    return () => {
      window.removeEventListener("focus", onFocus);
    };
  });
</script>

{#if !app.config}
  <div class="loading">
    <span class="spinner"></span>
    {#if app.preparing}<p>Preparing OpenRAC's tools. This happens once after installing or updating.</p>{/if}
  </div>
{:else}
  <div class="shell">
    {#if app.config.setupComplete}<Header />{/if}
    <main bind:this={view}>
      {#if !app.config.setupComplete}<Settings setup />
      {:else if app.page === "game"}<Game />
      {:else if app.page === "mods"}<Mods />
      {:else if app.page === "tasks"}<Tasks />
      {:else if app.page === "settings"}<Settings />
      {:else}<Library />
      {/if}
    </main>
  </div>
{/if}
<Toasts />
<UpdateModal />

<style>
  .loading {
    height: 100%;
    display: grid;
    place-items: center;
    place-content: center;
    gap: 16px;
    padding: 0 24px;
    text-align: center;
    color: var(--amber);
  }

  .loading p {
    margin: 0;
    color: var(--dim);
  }

  .shell {
    height: 100%;
    display: flex;
    flex-direction: column;
  }

  /* Android: the navigation rail beside the page, or its bar under it on a narrow window
     (Header.svelte). */
  :global(html[data-platform="android"]) .shell {
    flex-direction: row;
    /* Nothing beside the page scrolls the window: no strip of the background under the rail. */
    overflow: hidden;
  }

  @media (max-width: 599px) {
    :global(html[data-platform="android"]) .shell {
      flex-direction: column-reverse;
    }
  }

  /* A page scrolls down, never sideways: what is wider than the window is the page's to fit. */
  main {
    flex: 1;
    min-height: 0;
    min-width: 0;
    overflow: hidden auto;
  }
</style>
