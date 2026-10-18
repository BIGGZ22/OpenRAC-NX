<script lang="ts">
  // The website's sticky orange bar (openrac-site src/components/Header.tsx):
  // the wrench, "OpenRAC" in Audiowide, round tabs, a white Discord button.
  import { api, inTauri } from "$lib/api";
  import { app, jobs, openDevelop, pendingJobs, runningJob, type Page } from "$lib/app.svelte";
  import { DISCORD_URL, SITE_URL } from "$lib/links";
  import Icon from "./Icon.svelte";

  // A player has three places to be: the games, modding and the settings. What the launcher
  // ran (Activity) opens from the running job here and from a game's menu. With
  // developer tools on, Develop is a tab of its own: the checkout, each version and the jobs.
  const tabs = $derived<{ page: Page; label: string }[]>([
    { page: "library", label: "Games" },
    // The level editor works in a checkout's folders: not from the copy an installed launcher carries.
    ...(app.info?.standalone ? [] : [{ page: "mods" as Page, label: "Mods" }]),
    ...(app.config?.developer ? [{ page: "develop" as Page, label: "Develop" }] : []),
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
        {#if tab.page === "develop" && pending > 0}<span class="count">{pending}</span>{/if}
      </button>
    {/each}
  </nav>

  {#if running}
    <button
      class="job"
      title="Show its output"
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
    {#if app.update}
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
    <button class="site" onclick={() => void api.openUrl(SITE_URL)} aria-label="openrac.dev" title="openrac.dev"
      ><Icon name="external" size={14} /><span class="label">openrac.dev</span></button
    >
    <!-- The community: the site's white button, with Discord's mark, which tilts like the wrench. -->
    <button class="discord" onclick={() => void api.openUrl(DISCORD_URL)} aria-label="Join the OpenRAC Discord">
      <svg class="mark" viewBox="0 0 24 24" width="18" height="18" aria-hidden="true">
        <path
          d="M20.317 4.37a19.791 19.791 0 0 0-4.885-1.515.074.074 0 0 0-.079.037c-.21.375-.444.864-.608 1.25a18.27 18.27 0 0 0-5.487 0 12.64 12.64 0 0 0-.617-1.25.077.077 0 0 0-.079-.037A19.736 19.736 0 0 0 3.677 4.37a.07.07 0 0 0-.032.027C.533 9.046-.32 13.58.099 18.057a.082.082 0 0 0 .031.057 19.9 19.9 0 0 0 5.993 3.03.078.078 0 0 0 .084-.028 14.09 14.09 0 0 0 1.226-1.994.076.076 0 0 0-.041-.106 13.107 13.107 0 0 1-1.872-.892.077.077 0 0 1-.008-.128 10.2 10.2 0 0 0 .372-.292.074.074 0 0 1 .077-.01c3.928 1.793 8.18 1.793 12.062 0a.074.074 0 0 1 .078.01c.12.098.246.198.373.292a.077.077 0 0 1-.006.127 12.299 12.299 0 0 1-1.873.892.077.077 0 0 0-.041.107c.36.698.772 1.362 1.225 1.993a.076.076 0 0 0 .084.028 19.839 19.839 0 0 0 6.002-3.03.077.077 0 0 0 .032-.054c.5-5.177-.838-9.674-3.549-13.66a.061.061 0 0 0-.031-.03zM8.02 15.33c-1.183 0-2.157-1.085-2.157-2.419 0-1.333.956-2.419 2.157-2.419 1.21 0 2.176 1.096 2.157 2.42 0 1.333-.956 2.418-2.157 2.418zm7.975 0c-1.183 0-2.157-1.085-2.157-2.419 0-1.333.955-2.419 2.157-2.419 1.21 0 2.176 1.096 2.157 2.42 0 1.333-.946 2.418-2.157 2.418z"
          fill="currentColor"
        />
      </svg>
      <span class="label">Discord</span>
    </button>
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
    display: inline-flex;
    align-items: center;
    gap: 6px;
    font-size: 14px;
    font-weight: 600;
    color: var(--bar-ink);
    padding: 7px 10px;
    border-radius: 999px;
  }

  .site:hover {
    background: rgb(0 0 0 / 0.1);
  }

  /* Discord: the site's white button, raised a little off the bar, its mark in the bar's ink. */
  .discord {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    font-size: 14px;
    font-weight: 700;
    background: #fff;
    color: var(--bar-ink-deep);
    border: 1px solid rgb(0 0 0 / 0.12);
    border-radius: 999px;
    padding: 6px 16px 6px 12px;
    box-shadow:
      0 2px 0 rgb(0 0 0 / 0.18),
      inset 0 -2px 0 rgb(0 0 0 / 0.06);
    transition:
      transform 0.15s ease,
      box-shadow 0.15s ease,
      background 0.15s ease;
  }

  .discord .mark {
    flex: none;
    transition: transform 0.25s cubic-bezier(0.2, 0.8, 0.2, 1);
  }

  .discord:hover {
    transform: translateY(-1px);
    background: var(--bar-ink-deep);
    color: #fff;
    box-shadow: 0 6px 16px rgb(0 0 0 / 0.28);
  }

  .discord:hover .mark {
    transform: rotate(-12deg) scale(1.08);
    color: var(--gold);
  }

  .discord:active {
    transform: translateY(1px);
    box-shadow: 0 1px 0 rgb(0 0 0 / 0.2);
  }

  /* An update: the selected tab's dark brown and gold, with a dot that pulses. */
  .update {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    font-size: 13px;
    font-weight: 700;
    background: var(--bar-ink-deep);
    color: var(--gold);
    border: 1px solid rgb(0 0 0 / 0.3);
    border-radius: 999px;
    padding: 6px 14px 6px 11px;
    transition: transform 0.15s ease;
  }

  .update:hover {
    transform: translateY(-1px);
    color: var(--amber);
  }

  .pulse {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: var(--amber);
    box-shadow: 0 0 0 0 color-mix(in srgb, var(--amber) 60%, transparent);
    animation: pulse 1.8s ease-out infinite;
  }

  @keyframes pulse {
    to {
      box-shadow: 0 0 0 8px transparent;
    }
  }

  .update .label {
    margin-left: -4px;
  }

  /* The version tag beside the name gives way first (it is in the tooltip and in Settings). */
  @media (max-width: 1240px) {
    header,
    .site-header {
      gap: 18px;
    }

    .brand small,
    .preview {
      display: none;
    }
  }

  /* Down to the window's smallest (960 px), with developer tools, a job and an update all showing:
     the site link and Discord keep their marks, the update its word, the job less of its name. */
  @media (max-width: 1180px) {
    header,
    .site-header {
      gap: 14px;
    }

    .site .label,
    .discord .label,
    .update .label {
      display: none;
    }

    .site,
    .discord {
      padding: 7px 9px;
    }

    .job {
      max-width: 150px;
    }
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
