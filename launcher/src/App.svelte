<script lang="ts">
  import Header from "$components/Header.svelte";
  import Toasts from "$components/Toasts.svelte";
  import UpdateModal from "$components/UpdateModal.svelte";
  import { app, openDevelop, refresh, start, updateDiscordPresence, type Page } from "$lib/app.svelte";
  import Game from "./pages/Game.svelte";
  import Library from "./pages/Library.svelte";
  import Settings from "./pages/Settings.svelte";

  // Mods, Activity and Develop load the first time they are opened, so starting the launcher reads
  // only what the game page needs; they are fetched while the launcher idles after it starts, so
  // opening them later is instant.
  const once = <T,>(load: () => Promise<T>) => {
    let loaded: Promise<T> | null = null;
    return () => (loaded ??= load());
  };
  const modsPage = once(() => import("./pages/Mods.svelte"));
  const tasksPage = once(() => import("./pages/Tasks.svelte"));
  const developPage = once(() => import("./pages/Develop.svelte"));
  $effect(() => {
    const later = setTimeout(() => {
      void modsPage();
      void tasksPage();
      if (app.config?.developer) void developPage();
    }, 2000);
    return () => {
      clearTimeout(later);
    };
  });

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
    // With developer tools on, what the launcher ran is Develop's console (a game's menu still
    // asks for Activity).
    if (app.page === "tasks" && app.config?.developer) openDevelop("console");
  });

  $effect(() => {
    // Sync Discord Rich Presence when page or game changes
    updateDiscordPresence(app.page, app.version);
  });

  $effect(() => {
    // ?page=tasks (and ?version=rac1/pal) open a page directly: handy for screenshots.
    // ?page=develop opens Develop on ?version, or on ?develop=checkout|console.
    const params = new URLSearchParams(location.search);
    void start().then(() => {
      const version = params.get("version");
      if (version) {
        app.version = version;
        app.page = "game";
      }
      const page = params.get("page") as Page | null;
      if (page === "develop") openDevelop(params.get("develop") ?? version ?? undefined);
      else if (page) app.page = page;
      // ?update&modal=update: the update window open on the preview's update (mock.ts).
      if (params.get("modal") === "update") app.showUpdateModal = true;
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
      {:else if app.page === "mods"}{#await modsPage() then page}<page.default />{/await}
      {:else if app.page === "tasks"}{#await tasksPage() then page}<page.default />{/await}
      {:else if app.page === "develop"}{#await developPage() then page}<page.default />{/await}
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
