<script lang="ts">
  // The website's sticky orange bar (openrac-site src/components/Header.tsx):
  // the wrench, "OpenRAC" in Audiowide, round tabs, a white Discord button.
  // On Android the same places are a navigation rail on the left, or a bar at the bottom when
  // the window is narrow, as Android's own apps have them (Material's navigation rail and bar).
  import { api, inTauri, onAndroid } from "$lib/api";
  import { app, jobs, pendingJobs, runningJob, type Page } from "$lib/app.svelte";
  import { DISCORD_URL, SITE_URL } from "$lib/links";
  import { untrack } from "svelte";
  import Icon, { type IconName } from "./Icon.svelte";

  // A player has three places to be: the games, modding and the settings. What the launcher
  // ran (Activity) opens from the running job here and from a game's menu; with
  // developer tools on it is a tab of its own, Tasks.
  const tabs = $derived<{ page: Page; label: string; icon: IconName }[]>([
    { page: "library", label: "Games", icon: "library" },
    // The level editor works in a checkout's folders: not from the copy an installed launcher carries.
    ...(app.info?.standalone ? [] : [{ page: "mods" as Page, label: "Mods", icon: "edit" as IconName }]),
    ...(app.config?.developer ? [{ page: "tasks" as Page, label: "Tasks", icon: "terminal" as IconName }] : []),
    { page: "settings", label: "Settings", icon: "gear" },
  ]);

  const running = $derived(runningJob());
  const pending = $derived(pendingJobs());
  /** The tab that is open: a game's page belongs to the games. */
  const current = $derived<Page>(app.page === "game" ? "library" : app.page);

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
  <button
    class="brand"
    onclick={() => (app.page = "library")}
    aria-label="OpenRAC: your games"
    title={app.info?.version ? `OpenRAC Launcher ${app.info.version}` : undefined}
  >
    <img src="/wrench.webp" alt="" width="40" height="40" />
    <span>Open<b>RAC</b></span>
    <!-- The launcher's version beside its name, on every page (also in the tooltip, for a narrow
         window that hides the tag). -->
    <small
      >Launcher{#if app.info?.version}<span class="ver">{app.info.version}</span>{/if}</small
    >
  </button>

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
            {#if tab.page === "settings" && app.update}<span class="badge" aria-label="An update is out"></span>{/if}
          </span>
        {/if}
        <span class="label">{tab.label}</span>
        {#if tab.page === "tasks" && pending > 0}<span class="count">{pending}</span>{/if}
      </button>
    {/each}
  </nav>

  {#if running}
    <button
      class="job"
      title={`${running.title}: show its output`}
      onclick={() => {
        jobs.selected = running.key;
        app.page = "tasks";
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
      <button
        class="update"
        onclick={() => (app.showUpdateModal = true)}
        title={`Version ${update.tag} is available!\nClick to inspect or update.`}
      >
        ▲ Update {update.version}
      </button>
    {/if}
    {#if !inTauri}<span class="preview">Browser preview</span>{/if}
    {#if !onAndroid}
      <button class="site" onclick={() => void api.openUrl(SITE_URL)}>openrac.dev</button>
      <button class="discord" onclick={() => void api.openUrl(DISCORD_URL)}>Discord</button>
    {/if}
  </div>
</header>

<style>
  .site-header {
    position: relative;
    z-index: 20;
    display: flex;
    align-items: center;
    gap: 22px;
    flex: none;
    height: 60px;
    width: 100%;
    padding: 0 18px;
    background: linear-gradient(to bottom, var(--bar-top), var(--bar-bottom));
    border-bottom: 1px solid rgb(0 0 0 / 0.3);
    color: #000;
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

  .brand {
    display: inline-flex;
    align-items: center;
    flex-shrink: 0;
    gap: 8px;
    font-family: var(--font-brand);
    font-size: 26px;
    font-weight: 400;
    color: #000;
  }

  .brand b {
    font-weight: 400;
  }

  /* The wrench turns in when the launcher opens, and again under the pointer or a press (not on
     Android, below). */
  .brand img {
    width: 40px;
    height: 40px;
    transition: transform var(--slow) var(--ease);
    animation: wrench-in 0.9s var(--ease) both;
  }

  .brand:active img {
    transform: rotate(-25deg) scale(1.1);
  }

  @media (hover: hover) {
    .brand:hover img {
      transform: rotate(-25deg) scale(1.1);
    }
  }

  @keyframes wrench-in {
    from {
      opacity: 0;
      transform: rotate(-120deg) scale(0.6);
    }
  }

  .brand small {
    font-family: var(--font-head);
    font-size: 10px;
    font-weight: 700;
    letter-spacing: 0.22em;
    text-transform: uppercase;
    color: var(--bar-ink);
    border: 1px solid rgb(0 0 0 / 0.3);
    border-radius: 999px;
    padding: 2px 8px;
    margin-left: 4px;
  }

  .brand .ver {
    margin-left: 6px;
    padding-left: 6px;
    border-left: 1px solid rgb(0 0 0 / 0.3);
    letter-spacing: 0.06em;
    text-transform: none;
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
    padding: 7px 15px;
    font-size: 15px;
    font-weight: 500;
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
    font-size: 12px;
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
    font-size: 13px;
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
    font-size: 12px;
    font-weight: 700;
    color: var(--bar-ink);
    border: 1px dashed rgb(0 0 0 / 0.4);
    border-radius: 999px;
    padding: 3px 10px;
  }

  .site {
    font-size: 14px;
    font-weight: 600;
    color: var(--bar-ink);
    padding: 7px 10px;
    border-radius: 999px;
  }

  .discord {
    font-size: 14px;
    font-weight: 600;
    background: #fff;
    color: var(--bar-ink);
    border: 1px solid rgb(0 0 0 / 0.15);
    border-radius: 999px;
    padding: 7px 16px;
  }

  @media (hover: hover) {
    .site:hover {
      background: rgb(0 0 0 / 0.1);
    }

    .discord:hover {
      background: var(--bar-ink);
      color: #fff;
    }

    .update:hover {
      background: #00ea76;
      transform: scale(1.03);
    }
  }

  .update {
    font-size: 13px;
    font-weight: 700;
    background: #00d26a;
    color: #052611;
    border: 1px solid rgb(0 0 0 / 0.15);
    border-radius: 999px;
    padding: 6px 14px;
    cursor: pointer;
    animation: rise var(--medium) var(--ease) both;
  }

  /* A narrow window keeps the tabs: the site link and the tag give way first. */
  @media (max-width: 900px) {
    .site-header {
      gap: 12px;
    }

    .brand small,
    .site,
    .preview {
      display: none;
    }

    .job {
      max-width: 160px;
    }
  }

  /* A phone held upright: the wrench stands for the name, Discord and the running task give way,
     and the tabs scroll if they still do not fit. */
  @media (max-width: 560px) {
    .brand span,
    .discord,
    .job {
      display: none;
    }

    nav {
      flex-shrink: 1;
      min-width: 0;
      overflow-x: auto;
      scrollbar-width: none;
    }

    .tab {
      padding: 7px 11px;
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
    /* It scrolls only up and down, without a bar: a tab one pixel wider than the rail inside its
       border made a sideways scroll bar along its foot. A WebView older than Chromium 121 knows
       no scrollbar-width, only the ::-webkit-scrollbar below. */
    overflow-x: hidden;
    overflow-y: auto;
    scrollbar-width: none;
  }

  :global(html[data-platform="android"]) .site-header::-webkit-scrollbar {
    display: none;
  }

  /* The wrench on the bar's orange, as the app's icon has it. */
  :global(html[data-platform="android"]) .brand {
    flex: none;
    justify-content: center;
    width: 48px;
    height: 48px;
    border-radius: 14px;
    background: linear-gradient(to bottom, var(--bar-top), var(--bar-bottom));
    box-shadow: inset 0 1px 0 rgb(255 255 255 / 0.35);
  }

  /* It stays still there: the logo of an app, not a control that plays. */
  :global(html[data-platform="android"]) .brand img {
    width: 34px;
    height: 34px;
    animation: none;
    transform: none;
    transition: none;
  }

  :global(html[data-platform="android"]) .brand :is(span, small) {
    display: none;
  }

  :global(html[data-platform="android"]) nav {
    flex-direction: column;
    gap: 12px;
  }

  :global(html[data-platform="android"]) .slider {
    display: none;
  }

  /* A destination: its icon in a pill that widens out from the middle when it is the open one,
     and its name under it. */
  :global(html[data-platform="android"]) .tab {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 4px;
    width: var(--nav-left);
    padding: 0;
    border-radius: 0;
    font-size: 12px;
    font-weight: 600;
    color: var(--soft);
  }

  :global(html[data-platform="android"]) .tab:active {
    transform: none;
    filter: none;
  }

  .icon {
    position: relative;
    display: grid;
    place-items: center;
    width: 56px;
    height: 32px;
    border-radius: 16px;
  }

  .icon::before {
    content: "";
    position: absolute;
    inset: 0;
    border-radius: inherit;
    background: color-mix(in srgb, var(--amber) 22%, transparent);
    transform: scaleX(0.4);
    opacity: 0;
    transition:
      transform var(--medium) var(--ease),
      opacity var(--fast);
  }

  .icon :global(svg) {
    position: relative;
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
    top: 3px;
    right: 13px;
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: #00d26a;
    box-shadow: 0 0 0 2px var(--ink-deep);
  }

  :global(html[data-platform="android"]) .tab .count {
    position: absolute;
    top: -4px;
    left: 44px;
    min-width: 18px;
    height: 18px;
    font-size: 11px;
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
