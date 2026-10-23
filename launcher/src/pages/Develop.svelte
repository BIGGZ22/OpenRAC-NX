<script lang="ts">
  // Develop: everything a contributor runs, in one place (developer tools on). The list on the
  // left is the checkout, every version of every game with how far its decompilation is, and the
  // console; beside it, the one chosen (app.develop). The player's pages keep only a row of chips
  // that lead here. Android has no list beside it: the page is for the game picked in Games (its
  // chip at the top right goes back there), with its versions, one per region, and the console in
  // a row of tabs. No checkout there, and only what Android runs (core/src/actions.rs, runs_on).
  import DevCheckout from "$components/DevCheckout.svelte";
  import DevConsole from "$components/DevConsole.svelte";
  import DeveloperPanel from "$components/DeveloperPanel.svelte";
  import GameChip from "$components/GameChip.svelte";
  import Icon from "$components/Icon.svelte";
  import SideFrame from "$components/SideFrame.svelte";
  import SideList from "$components/SideList.svelte";
  import { onAndroid } from "$lib/api";
  import { app, jobs, pendingJobs } from "$lib/app.svelte";
  import { pct } from "$lib/format";
  import { regionLabel, shortTitle, type SideEntry } from "$lib/labels";
  import { gameThumb } from "$lib/media";
  import { enter } from "$lib/motion";
  import { playerVersion } from "$lib/player.svelte";
  import { tablist } from "$lib/tabs";
  import { themeStyle } from "$lib/themes";
  import { fly } from "svelte/transition";

  const games = $derived(app.library?.games ?? []);
  const pending = $derived(pendingJobs());
  const failed = $derived(jobs.list.filter((j) => j.state === "failed").length);

  const entries = $derived<SideEntry[]>([
    {
      key: "checkout",
      name: "This checkout",
      icon: "library",
      group: "Checkout",
      state: {
        text: `${games.flatMap((g) => g.versions).filter((v) => v.status.disc.state === "found").length} of ${games.flatMap((g) => g.versions).length} discs found`,
        tone: "info",
      },
    },
    ...games.flatMap((game, gi) =>
      game.versions.map((v, vi) => ({
        key: v.key,
        name: shortTitle(game.title),
        art: gameThumb(game.id),
        style: themeStyle(game.id),
        group: gi === 0 && vi === 0 ? "Versions" : undefined,
        state: v.progress
          ? { text: `${regionLabel(v.name)} · ${pct(v.progress.percent)}`, tone: "ok" as const }
          : { text: `${regionLabel(v.name)} · no report yet`, tone: "info" as const },
      })),
    ),
    {
      key: "console",
      name: "Console",
      icon: "terminal",
      group: "Jobs",
      state: pending
        ? { text: `${pending} running or waiting`, tone: "warn", busy: true }
        : failed
          ? { text: `${failed} failed`, tone: "warn" }
          : { text: jobs.list.length ? "Nothing running" : "Nothing has run yet", tone: "info" },
    },
  ]);

  /** Android: the game picked in Games, and the tab open, one of its versions or the console. */
  const picked = $derived(games.find((g) => g.id === app.version?.split("/")[0]) ?? games[0] ?? null);
  const tab = $derived.by(() => {
    if (!picked || app.develop === "console") return app.develop;
    if (picked.versions.some((v) => v.key === app.develop)) return app.develop;
    return (playerVersion(picked) ?? picked.versions[0])?.key ?? "console";
  });
  const selected = $derived(onAndroid ? tab : app.develop);

  const version = $derived(games.flatMap((g) => g.versions).find((v) => v.key === selected) ?? null);
  const game = $derived(version ? (games.find((g) => g.id === version.game) ?? null) : null);
</script>

{#snippet view()}
  {#key selected}
    <div class="view" in:fly={enter()}>
      {#if selected === "console"}
        <DevConsole />
      {:else if version && game}
        <DeveloperPanel {game} {version} />
      {:else}
        <DevCheckout />
      {/if}
    </div>
  {/key}
{/snippet}

{#if onAndroid}
  <div class="page" style={picked ? themeStyle(picked.id) : ""}>
    <div class="intro">
      <h1 class="grow">Develop</h1>
      {#if picked}<GameChip title={picked.title} what="development" />{/if}
    </div>
    {#if picked}
      <div class="tabs" role="tablist" aria-label="Versions and the console" use:tablist>
        {#each picked.versions as v (v.key)}
          <button
            role="tab"
            aria-selected={tab === v.key}
            tabindex={tab === v.key ? 0 : -1}
            class:on={tab === v.key}
            onclick={() => (app.develop = v.key)}
            >{regionLabel(v.name)}<span class="dim">{v.progress ? pct(v.progress.percent) : "–"}</span></button
          >
        {/each}
        <button
          role="tab"
          aria-selected={tab === "console"}
          tabindex={tab === "console" ? 0 : -1}
          class:on={tab === "console"}
          onclick={() => (app.develop = "console")}
          ><Icon name="terminal" size={16} />Console{#if pending}<span class="busy">{pending}</span>{/if}</button
        >
      </div>
    {/if}
    {@render view()}
  </div>
{:else}
  <SideFrame>
    {#snippet list()}
      <SideList {entries} selected={app.develop} onpick={(key) => (app.develop = key)} label="Develop" />
    {/snippet}
    <div class="page">
      <h1 class="sr">Develop</h1>
      {@render view()}
    </div>
  </SideFrame>
{/if}

<style>
  .page {
    max-width: var(--page-max, 1080px);
    margin: 0 auto;
    padding: var(--page-top, 24px) var(--page-x, 24px) var(--page-bottom, 48px);
    display: flex;
    flex-direction: column;
    gap: var(--page-gap, 18px);
  }

  .view {
    display: flex;
    flex-direction: column;
    gap: 18px;
  }

  .intro {
    display: flex;
    align-items: center;
    flex-wrap: wrap;
    gap: 12px 24px;
  }

  /* The page's name, as Mods' on Android: the game's own typeface is its chip's. */
  .intro h1 {
    font-weight: 600;
    color: var(--text);
  }

  /* Android: the game's versions and the console, as Mods' tabs. */
  .tabs {
    display: flex;
    align-self: flex-start;
    flex-wrap: wrap;
    gap: 4px;
    padding: 4px;
    border: 1px solid var(--line-strong);
    border-radius: 999px;
    background: var(--panel);
  }

  .tabs button {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    border: 0;
    background: none;
    padding: 8px 18px;
    font-size: 0.875rem;
    font-weight: 600;
    color: var(--soft);
    transition:
      color var(--fast) var(--ease),
      background var(--fast) var(--ease);
  }

  .tabs button.on {
    background: var(--panel-hi);
    color: var(--amber);
    box-shadow: inset 0 0 0 1px var(--line-strong);
  }

  .tabs button:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .tabs .dim {
    font-weight: 500;
    font-variant-numeric: tabular-nums;
  }

  .busy {
    min-width: 20px;
    padding: 0 6px;
    border-radius: 999px;
    background: color-mix(in srgb, var(--warn) 22%, transparent);
    color: var(--warn);
    font-size: 0.75rem;
    text-align: center;
  }

  /* A heading for screen readers: the header's tab and the list already name the place. */
  .sr {
    position: absolute;
    width: 1px;
    height: 1px;
    overflow: hidden;
    clip-path: inset(50%);
  }
</style>
