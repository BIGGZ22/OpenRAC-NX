<script lang="ts">
  import GameRunning from "$components/GameRunning.svelte";
  import Header from "$components/Header.svelte";
  import LaunchScreen from "$components/LaunchScreen.svelte";
  import Toasts from "$components/Toasts.svelte";
  import UpdateModal from "$components/UpdateModal.svelte";
  import { errorText, onAndroid } from "$lib/api";
  import { app, openDevelop, refresh, showLaunch, start, updateDiscordPresence, type Page } from "$lib/app.svelte";
  import { reduced } from "$lib/motion";
  import { tick } from "svelte";
  import { ACCENT_PROPERTIES, accentProperties } from "$lib/themes";
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
    // Android: the launcher's accent is the picked game's own (themes.ts), the colour easing from
    // one to the next (app.css).
    if (!onAndroid) return;
    const style = document.documentElement.style;
    for (const name of ACCENT_PROPERTIES) style.removeProperty(name);
    for (const [name, value] of accentProperties(app.version?.split("/")[0] ?? null)) style.setProperty(name, value);
  });

  $effect(() => {
    // The layout (Settings → Appearance): src/layouts/compact.css applies only to "compact".
    document.documentElement.dataset.layout = app.config?.layout ?? "compact";
  });

  // The page shown. On Android one page gives way to the next in a view transition (app.css: the
  // one leaving fades out as the other fades in); elsewhere, or without the browser's support, it
  // simply changes.
  let page = $state<Page>(app.page);
  $effect(() => {
    const next = app.page;
    if (next === page) return;
    if (!onAndroid || reduced || !("startViewTransition" in document)) {
      page = next;
      return;
    }
    document.startViewTransition(async () => {
      page = next;
      await tick();
    });
  });

  // Another page starts at its top, as it arrives (app.css: its sections rise in turn).
  let view = $state<HTMLElement | null>(null);
  let shown: Page | null = null;
  $effect(() => {
    if (page === shown) return;
    shown = page;
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
    void start()
      .then(() => {
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
        // ?launch: the version's start (Android's spinning disc), as Play shows it.
        if (params.has("launch") && version) {
          const game = app.library?.games.find((g) => g.id === version.split("/")[0]);
          if (game) showLaunch(game.id, game.title);
        }
      })
      .catch((e: unknown) => {
        // start() says what went wrong itself; this is the last word for anything else.
        app.startError = errorText(e);
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
    {#if app.startError}
      <p>The launcher could not start: {app.startError}</p>
      <button class="primary" onclick={() => void start()}>Try again</button>
    {:else}
      <span class="spinner"></span>
      {#if app.preparing}<p>Preparing OpenRAC's tools. This happens once after installing or updating.</p>{/if}
    {/if}
  </div>
{:else}
  <div class="shell">
    <!-- The first run has the bar with only the name (Android's rail is only its tabs). -->
    {#if app.config.setupComplete}<Header />{:else if !onAndroid}<Header setup />{/if}
    <main bind:this={view}>
      {#if !app.config.setupComplete}<Settings setup />
      {:else if page === "game"}<Game />
      {:else if page === "mods"}{#await modsPage() then loaded}<loaded.default />{/await}
      {:else if page === "tasks"}{#await tasksPage() then loaded}<loaded.default />{/await}
      {:else if page === "develop"}{#await developPage() then loaded}<loaded.default />{/await}
      {:else if page === "settings"}<Settings />
      {:else}<Library />
      {/if}
    </main>
  </div>
{/if}
<LaunchScreen />
<GameRunning />
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
