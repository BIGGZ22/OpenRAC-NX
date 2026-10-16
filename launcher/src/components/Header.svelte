<script lang="ts">
  // The website's sticky orange bar (openrac-site src/components/Header.tsx):
  // the wrench, "OpenRAC" in Audiowide, round tabs, a white Discord button.
  import { api, inTauri } from "$lib/api";
  import { app, jobs, pendingJobs, runningJob, type Page } from "$lib/app.svelte";
  import { DISCORD_URL, SITE_URL } from "$lib/links";

  // A player has three places to be: the games, modding and the settings. What the launcher
  // ran (Activity) opens from the running job here and from a game's menu; with
  // developer tools on it is a tab of its own, Tasks.
  const tabs = $derived<{ page: Page; label: string }[]>([
    { page: "library", label: "Games" },
    // The level editor works in a checkout's folders: not from the copy an installed launcher carries.
    ...(app.info?.standalone ? [] : [{ page: "mods" as Page, label: "Mods" }]),
    ...(app.config?.developer ? [{ page: "tasks" as Page, label: "Tasks" }] : []),
    { page: "settings", label: "Settings" },
  ]);

  const running = $derived(runningJob());
  const pending = $derived(pendingJobs());
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

  <nav>
    {#each tabs as tab (tab.page)}
      {@const on = app.page === tab.page || (tab.page === "library" && app.page === "game")}
      <button class="tab" class:on aria-current={on ? "page" : undefined} onclick={() => (app.page = tab.page)}>
        {tab.label}
        {#if tab.page === "tasks" && pending > 0}<span class="count">{pending}</span>{/if}
      </button>
    {/each}
  </nav>

  {#if running}
    <button
      class="job"
      title="Show its output"
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
    {#if app.update}
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
    <button class="site" onclick={() => void api.openUrl(SITE_URL)}>openrac.dev</button>
    <button class="discord" onclick={() => void api.openUrl(DISCORD_URL)}>Discord</button>
  </div>
</header>

<style>
  header,
  .site-header {
    position: relative;
    z-index: 20;
    display: flex !important;
    flex-direction: row;
    align-items: center;
    gap: 22px;
    height: 60px;
    min-height: 60px;
    width: 100%;
    padding: 0 18px;
    background: linear-gradient(to bottom, var(--bar-top), var(--bar-bottom)) !important;
    border-bottom: 1px solid rgb(0 0 0 / 0.3);
    color: #000;
    box-shadow:
      inset 0 1px 0 rgb(255 255 255 / 0.35),
      0 4px 24px rgb(0 0 0 / 0.45);
    flex: none;
    box-sizing: border-box;
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

  .brand img {
    width: 40px;
    height: 40px;
    transition: transform 0.5s;
  }

  .brand:hover img {
    transform: rotate(-25deg) scale(1.1);
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
    display: flex;
    align-items: center;
    flex-shrink: 0;
    gap: 4px;
  }

  .tab {
    position: relative;
    border-radius: 999px;
    padding: 7px 15px;
    font-size: 15px;
    font-weight: 500;
    color: var(--bar-ink);
  }

  .tab:hover:not(.on) {
    background: rgb(0 0 0 / 0.1);
  }

  .tab.on {
    background: var(--bar-ink-deep);
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

  .site:hover {
    background: rgb(0 0 0 / 0.1);
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

  .discord:hover {
    background: var(--bar-ink);
    color: #fff;
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
    transition:
      transform 0.15s ease,
      background 0.15s ease;
  }

  .update:hover {
    background: #00ea76;
    transform: scale(1.03);
  }

  /* A narrow window keeps the tabs: the site link and the tag give way first. */
  @media (max-width: 900px) {
    header,
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
</style>
