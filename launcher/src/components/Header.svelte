<script lang="ts">
  // The website's sticky orange bar (openrac-site src/components/Header.tsx):
  // the OpenRAC logo in the bar's ink, and round pills in a shade of it: the tabs, the site, Discord.
  // On Android the same places are a navigation rail on the left, or a bar at the bottom when
  // the window is narrow, as Android's own apps have them (Material's navigation rail and bar).
  import { api, inTauri, onAndroid } from "$lib/api";
  import { app, jobs, openDevelop, pendingJobs, runningJob, type Page } from "$lib/app.svelte";
  import { DISCORD_URL, SITE_URL } from "$lib/links";
  import { untrack } from "svelte";
  import Icon, { type IconName } from "./Icon.svelte";
  import Logo from "./Logo.svelte";

  /** The first run: the bar with the name only, nothing to go to yet. */
  /** `shown`: the page on screen, which follows app.page inside a page's slide (App.svelte), so that
   *  the tabs change with the page rather than a moment before it. */
  let { setup = false, shown }: { setup?: boolean; shown?: Page } = $props();

  // A player has three places to be: the games, modding and the settings. What the launcher
  // ran (Activity) opens from the running job here and from a game's menu. With
  // developer tools on, Develop is a tab of its own: the checkout, each version and the jobs.
  const tabs = $derived<{ page: Page; label: string; icon: IconName }[]>([
    { page: "library", label: "Games", icon: "library" },
    // Every launcher, installed or on a checkout (an installed one mods in its data folder,
    // tools/bundle_standalone.py). Android's has the mods, Enhanced graphics and the packs, without
    // the level editor (no Godot there: Mods.svelte).
    { page: "mods", label: "Mods", icon: "edit" },
    ...(app.config?.developer ? [{ page: "develop" as Page, label: "Develop", icon: "terminal" as IconName }] : []),
    { page: "settings", label: "Settings", icon: "gear" },
  ]);

  const running = $derived(runningJob());
  const pending = $derived(pendingJobs());
  /** The tab that is open: a game's page belongs to the games. */
  const current = $derived.by<Page>(() => {
    const page = shown ?? app.page;
    return page === "game" ? "library" : page;
  });

  // The bar's tabs share one highlight that slides to the tab opened. Placed without a
  // transition the first time, so that it does not fly in from the left edge.
  let nav = $state<HTMLElement | null>(null);
  let slider = $state({ left: 0, width: 0, shown: false, moving: false });
  /** Whether the highlight has a place yet: not state, as place() is run by an effect. */
  let placed = false;
  function place(page: Page) {
    const on = nav?.querySelector<HTMLElement>(`.tab[data-page="${page}"]`);
    slider = on
      ? { left: on.offsetLeft, width: on.offsetWidth, shown: true, moving: placed }
      : { left: 0, width: 0, shown: false, moving: false };
    placed = !!on;
  }
  $effect(() => {
    place(current);
  });
  $effect(() => {
    if (!nav) return;
    // The fonts arrive after the first layout and change the tabs' widths.
    const resized = new ResizeObserver(() => {
      place(untrack(() => current));
    });
    resized.observe(nav);
    return () => {
      resized.disconnect();
    };
  });
</script>

<header class="site-header">
  <!-- The first run: the name, not yet a way back to the games. -->
  {#if setup}
    <div class="brand" title={app.info?.version ? `OpenRAC Launcher ${app.info.version}` : undefined}>
      <Logo height={34} />
      <span class="brand-mark"><Logo mark height={37} label="" /></span>
      {#if app.info?.version}<small class="ver">{app.info.version}</small>{/if}
    </div>
  {:else}
    <button
      class="brand"
      onclick={() => (app.page = "library")}
      aria-label="OpenRAC: your games"
      title={app.info?.version ? `OpenRAC Launcher ${app.info.version}` : undefined}
    >
      <Logo height={34} />
      <span class="brand-mark"><Logo mark height={37} label="" /></span>
      <!-- The launcher's version beside the logo, on every page (also in the tooltip, for a phone's
           width, which shows the mark alone). -->
      {#if app.info?.version}<small class="ver">{app.info.version}</small>{/if}
    </button>
  {/if}

  {#if !setup}
    <nav bind:this={nav}>
      <span
        class="slider"
        class:shown={slider.shown}
        class:moving={slider.moving}
        style={`transform: translateX(${slider.left}px); width: ${slider.width}px`}
        aria-hidden="true"
      ></span>
      {#each tabs as tab (tab.page)}
        {@const on = current === tab.page}
        <button
          class="tab"
          class:on
          data-page={tab.page}
          aria-current={on ? "page" : undefined}
          onclick={() => (app.page = tab.page)}
        >
          {#if onAndroid}
            <span class="icon">
              <Icon name={tab.icon} size={22} />
              {#if tab.page === "settings" && app.update}<span class="badge" aria-hidden="true"></span>{/if}
            </span>
          {/if}
          <span class="label">{tab.label}</span>
          {#if onAndroid && tab.page === "settings" && app.update}<span class="sr-only">, an update is out</span>{/if}
          {#if tab.page === "develop" && pending > 0}<span class="count">{pending}</span>{/if}
        </button>
      {/each}
    </nav>
  {/if}

  {#if running && !setup}
    <button
      class="job"
      title={`${running.title}: show its output`}
      onclick={() => {
        jobs.selected = running.key;
        if (app.config?.developer) openDevelop("console");
        else app.page = "tasks";
      }}
    >
      <span class="spinner"></span>
      <span class="clip">{running.title}</span>
      {#if pending > 1}<span class="count">+{pending - 1}</span>{/if}
    </button>
  {/if}

  <div class="end">
    <!-- Android says it with a dot on Settings, where the update is (a pill at the rail's foot
         looked like a stray bar). -->
    {#if app.update && !onAndroid}
      {@const update = app.update}
      <!-- A newer launcher: in the bar's dark brown, with a dot that says something is new. -->
      <button
        class="update"
        onclick={() => (app.showUpdateModal = true)}
        title={`Launcher ${update.version} is out: see what's new and update`}
      >
        <span class="pulse" aria-hidden="true"></span>Update<span class="label">{update.version}</span>
      </button>
    {/if}
    {#if !inTauri}<span class="preview">Browser preview</span>{/if}
    {#if !onAndroid}
      <!-- Where OpenRAC lives: the site and the community, two marks in one capsule of the selected
           tab's dark brown. -->
      <div class="links">
        <button class="site" onclick={() => void api.openUrl(SITE_URL)} aria-label="openrac.dev" title="openrac.dev"
          ><Icon name="globe" size={18} /></button
        >
        <button
          class="discord"
          onclick={() => void api.openUrl(DISCORD_URL)}
          aria-label="Join the OpenRAC Discord"
          title="Discord"
        >
          <svg class="mark" viewBox="0 0 24 24" width="18" height="18" aria-hidden="true">
            <path
              d="M20.317 4.37a19.791 19.791 0 0 0-4.885-1.515.074.074 0 0 0-.079.037c-.21.375-.444.864-.608 1.25a18.27 18.27 0 0 0-5.487 0 12.64 12.64 0 0 0-.617-1.25.077.077 0 0 0-.079-.037A19.736 19.736 0 0 0 3.677 4.37a.07.07 0 0 0-.032.027C.533 9.046-.32 13.58.099 18.057a.082.082 0 0 0 .031.057 19.9 19.9 0 0 0 5.993 3.03.078.078 0 0 0 .084-.028 14.09 14.09 0 0 0 1.226-1.994.076.076 0 0 0-.041-.106 13.107 13.107 0 0 1-1.872-.892.077.077 0 0 1-.008-.128 10.2 10.2 0 0 0 .372-.292.074.074 0 0 1 .077-.01c3.928 1.793 8.18 1.793 12.062 0a.074.074 0 0 1 .078.01c.12.098.246.198.373.292a.077.077 0 0 1-.006.127 12.299 12.299 0 0 1-1.873.892.077.077 0 0 0-.041.107c.36.698.772 1.362 1.225 1.993a.076.076 0 0 0 .084.028 19.839 19.839 0 0 0 6.002-3.03.077.077 0 0 0 .032-.054c.5-5.177-.838-9.674-3.549-13.66a.061.061 0 0 0-.031-.03zM8.02 15.33c-1.183 0-2.157-1.085-2.157-2.419 0-1.333.956-2.419 2.157-2.419 1.21 0 2.176 1.096 2.157 2.42 0 1.333-.956 2.418-2.157 2.418zm7.975 0c-1.183 0-2.157-1.085-2.157-2.419 0-1.333.955-2.419 2.157-2.419 1.21 0 2.176 1.096 2.157 2.42 0 1.333-.946 2.418-2.157 2.418z"
              fill="currentColor"
            />
          </svg>
        </button>
      </div>
    {/if}
  </div>
</header>

<style>
  /* On a desktop the bar writes in the theme's page colour, so the logo and the dark chips match
     the window under them; Android keeps the picked game's own ink. */
  :global(html:not([data-platform="android"])) .site-header {
    --bar-ink-deep: var(--ink-deep);
    --bar-name: var(--ink-deep);
  }

  .site-header {
    position: relative;
    z-index: 20;
    display: flex;
    align-items: center;
    gap: 22px;
    flex: none;
    height: var(--nav-top);
    width: 100%;
    padding: 0 18px;
    /* Wide stripes of two close golds: the bar's colour, quieter than one bright yellow. */
    background: repeating-linear-gradient(35deg, var(--bar-stripe) 0 20px, var(--bar-stripe-alt) 20px 40px);
    border-bottom: 1px solid rgb(0 0 0 / 0.3);
    color: var(--bar-name);
    box-shadow:
      inset 0 1px 0 rgb(255 255 255 / 0.35),
      0 4px 24px rgb(0 0 0 / 0.45);
  }

  button {
    border: 0;
    background: none;
    color: inherit;
    padding: 0;
  }

  /* Amber on the orange bar would not show: the ring is the bar's ink there (not on Android's
     dark rail, which keeps the amber). */
  :global(html:not([data-platform="android"])) .site-header button:focus-visible,
  :global(html[data-input="pad"]:not([data-platform="android"])) .site-header button:focus {
    outline: 2px solid var(--bar-ink-deep);
    outline-offset: 2px;
  }

  /* The logo in the bar's ink: the accent is the bar itself. */
  .brand {
    display: inline-flex;
    align-items: center;
    flex-shrink: 0;
    gap: 8px;
    color: var(--bar-name);
  }

  /* Its mark alone, for a window too narrow for the name (below). */
  .brand .brand-mark {
    display: none;
  }

  /* The logo comes in when the launcher opens (not on Android, below), and then holds still: its
     mark is the "O" of the name, so it cannot move on its own. */
  .brand :global(.logo) {
    animation: logo-in 0.7s var(--ease) both;
  }

  @keyframes logo-in {
    from {
      opacity: 0;
      transform: translateX(-8px);
    }
  }

  /* The version beside the logo, as tall as the capitals of "RAC" and level with them (the logo's
     box is centred on its letters: Logo.svelte), in the selected tab's dark brown and gold. */
  .brand .ver {
    display: inline-flex;
    align-items: center;
    height: 15px;
    padding: 0 6px;
    margin-left: 2px;
    border-radius: 999px;
    background: var(--bar-ink-deep);
    color: var(--gold);
    font-size: 0.625rem;
    font-weight: 700;
    line-height: 1;
    font-variant-numeric: tabular-nums;
  }

  nav {
    position: relative;
    display: flex;
    align-items: center;
    flex-shrink: 0;
    gap: 4px;
  }

  /* The open tab's dark pill, sliding from tab to tab. */
  .slider {
    position: absolute;
    top: 0;
    bottom: 0;
    left: 0;
    border-radius: 999px;
    background: var(--bar-ink-deep);
    opacity: 0;
  }

  .slider.shown {
    opacity: 1;
  }

  .slider.moving {
    transition:
      transform var(--medium) var(--ease),
      width var(--medium) var(--ease);
  }

  .tab {
    position: relative;
    border-radius: 999px;
    padding: 5px 14px;
    font-size: 0.9375rem;
    /* Heavier than the page's text, to stand beside the logo's strokes. */
    font-weight: 600;
    color: var(--bar-ink);
    transition: color var(--medium) var(--ease);
  }

  @media (hover: hover) {
    .tab:hover:not(.on) {
      background: rgb(0 0 0 / 0.1);
    }
  }

  .tab.on {
    color: var(--amber);
  }

  .count {
    min-width: 20px;
    height: 20px;
    border-radius: 10px;
    background: var(--bar-ink-deep);
    color: var(--amber);
    font-size: 0.75rem;
    display: inline-grid;
    place-items: center;
    padding: 0 6px;
  }

  .tab.on .count {
    background: var(--amber);
    color: var(--bar-ink-deep);
  }

  .job {
    gap: 8px;
    max-width: 280px;
    font-size: 0.8125rem;
    font-weight: 600;
    color: var(--bar-ink-deep);
    background: rgb(0 0 0 / 0.1);
    border: 1px solid rgb(0 0 0 / 0.25);
    border-radius: 999px;
    padding: 5px 12px;
    animation: rise var(--medium) var(--ease) both;
  }

  .end {
    margin-left: auto;
    display: flex;
    align-items: center;
    flex-shrink: 0;
    gap: 10px;
  }

  .preview {
    display: inline-flex;
    align-items: center;
    height: 24px;
    padding: 0 10px;
    border-radius: 999px;
    background: rgb(0 0 0 / 0.12);
    font-size: 0.75rem;
    font-weight: 700;
    color: var(--bar-ink-deep);
  }

  /* A capsule as tall as the tabs, in the selected tab's dark brown, its marks in gold that turn
     amber on a lighter round under the pointer. */
  .links {
    display: inline-flex;
    align-items: center;
    gap: 2px;
    height: 30px;
    padding: 0 3px;
    border-radius: 999px;
    background: var(--bar-ink-deep);
  }

  .site,
  .discord {
    display: grid;
    place-items: center;
    width: 26px;
    height: 26px;
    border-radius: 50%;
    color: var(--gold);
    transition:
      background var(--fast) var(--ease),
      color var(--fast) var(--ease);
  }

  .discord .mark {
    transition: transform var(--medium) var(--ease);
  }

  @media (hover: hover) {
    .site:hover,
    .discord:hover {
      background: rgb(255 255 255 / 0.1);
      color: var(--amber);
    }

    .discord:hover .mark {
      transform: rotate(-12deg) scale(1.08);
    }
  }

  .site:active,
  .discord:active {
    background: rgb(255 255 255 / 0.1);
    color: var(--amber);
  }

  /* An update: the selected tab's dark brown and gold, with a dot that pulses. */
  .update {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    font-size: 0.8125rem;
    font-weight: 700;
    background: var(--bar-ink-deep);
    color: var(--gold);
    border: 1px solid rgb(0 0 0 / 0.3);
    border-radius: 999px;
    padding: 4px 14px 4px 11px;
    transition: color var(--fast);
  }

  /* Under the pointer only its gold brightens: no blue edge (app.css's buttons) and no lift, which
     moved it by a pixel and left its text redrawn a little differently. */
  @media (hover: hover) {
    .update:hover {
      border-color: rgb(0 0 0 / 0.3);
      color: var(--amber);
    }
  }

  /* The dot rings a few times when the update shows, then rests: a ring that grows and fades
     (transform and opacity, which the compositor runs without restyling the page each frame). */
  .pulse {
    position: relative;
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: var(--amber);
  }

  .pulse::after {
    content: "";
    position: absolute;
    inset: 0;
    border-radius: 50%;
    background: var(--amber);
    opacity: 0;
    animation: pulse 1.8s ease-out 5;
  }

  @keyframes pulse {
    from {
      opacity: 0.6;
      transform: scale(1);
    }
    to {
      opacity: 0;
      transform: scale(3);
    }
  }

  .update .label {
    margin-left: -4px;
  }

  @media (max-width: 1240px) {
    header,
    .site-header {
      gap: 18px;
    }

    .preview {
      display: none;
    }
  }

  /* Down to the window's smallest (960 px), with developer tools, a job and an update all showing:
     the update keeps its word, the job less of its name. */
  @media (max-width: 1180px) {
    header,
    .site-header {
      gap: 14px;
    }

    .update .label {
      display: none;
    }

    .job {
      max-width: 150px;
    }
  }

  /* A narrow window keeps the tabs. */
  @media (max-width: 900px) {
    .site-header {
      gap: 12px;
    }

    .job {
      max-width: 160px;
    }
  }

  /* A phone held upright: the logo's mark stands for the name, the version (in the tooltip), the
     links and the running task give way, and the tabs scroll if they still do not fit. */
  @media (max-width: 560px) {
    .brand > :global(.logo),
    .brand .ver,
    .links,
    .job {
      display: none;
    }

    .brand .brand-mark {
      display: contents;
    }

    nav {
      flex-shrink: 1;
      min-width: 0;
      overflow-x: auto;
      scrollbar-width: none;
    }

    .tab {
      padding: 5px 11px;
    }
  }

  /* ---- Android: a navigation rail on the left ---- */

  :global(html[data-platform="android"]) .site-header {
    flex-direction: column;
    gap: 16px;
    flex: none;
    /* The window's whole height beside the page, which scrolls on its own (App.svelte): the
       shell is the window's height, and the rail stretches to it, never past it. */
    align-self: stretch;
    width: var(--nav-left);
    height: auto;
    min-height: 0;
    padding: 16px 0 12px;
    background: var(--ink-deep);
    border-bottom: 0;
    border-right: 1px solid var(--line);
    box-shadow: none;
    color: var(--text);
    /* It scrolls only up and down (no bar on Android: app.css): a tab one pixel wider than the
       rail inside its border made a sideways scroll bar along its foot. */
    overflow-x: hidden;
    overflow-y: auto;
  }

  /* The logo's mark in the picked game's accent, as wide as the rail's destinations: the rail has
     no room for the name. */
  :global(html[data-platform="android"]) .brand {
    flex: none;
    justify-content: center;
    width: 56px;
    padding-top: 6px;
  }

  :global(html[data-platform="android"]) .brand > :global(.logo),
  :global(html[data-platform="android"]) .brand .ver {
    display: none;
  }

  /* It stays still there: the logo of an app, not a control that plays. */
  :global(html[data-platform="android"]) .brand .brand-mark {
    display: contents;
    color: var(--amber);
  }

  :global(html[data-platform="android"]) .brand .brand-mark :global(.logo) {
    width: 53px;
    height: auto;
    animation: none;
  }

  :global(html[data-platform="android"]) nav {
    flex-direction: column;
    gap: 12px;
  }

  :global(html[data-platform="android"]) .slider {
    display: none;
  }

  /* A destination: its icon in a circle that grows out from the middle when it is the open one,
     and its name under it. */
  :global(html[data-platform="android"]) .tab {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 4px;
    width: var(--nav-left);
    padding: 0;
    border-radius: 0;
    font-size: 0.75rem;
    font-weight: 700;
    color: var(--soft);
  }

  :global(html[data-platform="android"]) .tab:active {
    transform: none;
    filter: none;
  }

  /* A circle, as round as the logo's "O" above them. */
  .icon {
    position: relative;
    display: grid;
    place-items: center;
    width: 40px;
    height: 40px;
    border-radius: 50%;
  }

  .icon::before {
    content: "";
    position: absolute;
    inset: 0;
    border-radius: inherit;
    background: color-mix(in srgb, var(--amber) 22%, transparent);
    transform: scale(0.5);
    opacity: 0;
    transition:
      transform var(--medium) var(--ease),
      opacity var(--fast);
  }

  /* Their strokes heavier than the page's icons, to stand under the logo's. */
  .icon :global(svg) {
    position: relative;
    stroke-width: 2.3;
    transition: transform var(--medium) var(--ease);
  }

  :global(html[data-platform="android"]) .tab.on {
    color: var(--amber);
  }

  .tab.on .icon::before {
    transform: none;
    opacity: 1;
  }

  .tab.on .icon :global(svg) {
    transform: scale(1.08);
  }

  /* A new launcher is out: a dot on Settings, where it is installed from. */
  .badge {
    position: absolute;
    top: 4px;
    right: 4px;
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: var(--ok);
    box-shadow: 0 0 0 2px var(--ink-deep);
  }

  :global(html[data-platform="android"]) .tab .count {
    position: absolute;
    top: -2px;
    left: 26px;
    min-width: 18px;
    height: 18px;
    font-size: 0.75rem;
    background: var(--amber);
    color: var(--bar-ink-deep);
  }

  /* The running task: its spinner, its name in the tooltip. */
  :global(html[data-platform="android"]) .job {
    display: inline-flex;
    flex-direction: column;
    gap: 4px;
    max-width: none;
    padding: 8px;
    color: var(--amber);
    background: none;
    border: 0;
  }

  :global(html[data-platform="android"]) .job .clip {
    display: none;
  }

  :global(html[data-platform="android"]) .job .count {
    background: var(--panel-hi);
  }

  :global(html[data-platform="android"]) .end {
    margin: auto 0 0;
    flex-direction: column;
  }

  /* ---- Android on a narrow window: a navigation bar at the bottom ---- */

  @media (max-width: 599px) {
    :global(html[data-platform="android"]) .site-header {
      flex-direction: row;
      justify-content: space-around;
      width: 100%;
      height: var(--nav-bottom);
      padding: 12px 0 16px;
      border-right: 0;
      border-top: 1px solid var(--line);
      overflow: visible;
    }

    :global(html[data-platform="android"]) .brand,
    :global(html[data-platform="android"]) .job,
    :global(html[data-platform="android"]) .end {
      display: none;
    }

    :global(html[data-platform="android"]) nav {
      flex: 1;
      flex-direction: row;
      justify-content: space-around;
      overflow: visible;
    }

    :global(html[data-platform="android"]) .tab {
      flex: 1;
      width: auto;
    }
  }
</style>
