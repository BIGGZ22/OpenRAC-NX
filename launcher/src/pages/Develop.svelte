<script lang="ts">
  // Develop: everything a contributor runs, in one place (developer tools on). The list on the
  // left is the checkout, every version of every game with how far its decompilation is, and the
  // console; beside it, the one chosen (app.develop). The player's pages keep only a row of chips
  // that lead here.
  import DevCheckout from "$components/DevCheckout.svelte";
  import DevConsole from "$components/DevConsole.svelte";
  import DeveloperPanel from "$components/DeveloperPanel.svelte";
  import SideFrame from "$components/SideFrame.svelte";
  import SideList from "$components/SideList.svelte";
  import { app, jobs, pendingJobs } from "$lib/app.svelte";
  import { pct } from "$lib/format";
  import { regionLabel, shortTitle, type SideEntry } from "$lib/labels";
  import { gameThumb } from "$lib/media";
  import { enter } from "$lib/motion";
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

  const version = $derived(games.flatMap((g) => g.versions).find((v) => v.key === app.develop) ?? null);
  const game = $derived(version ? (games.find((g) => g.id === version.game) ?? null) : null);
</script>

<SideFrame>
  {#snippet list()}
    <SideList {entries} selected={app.develop} onpick={(key) => (app.develop = key)} label="Develop" />
  {/snippet}
  <div class="page">
    <h1 class="sr">Develop</h1>
    {#key app.develop}
      <div class="view" in:fly={enter()}>
        {#if app.develop === "console"}
          <DevConsole />
        {:else if version && game}
          <DeveloperPanel {game} {version} />
        {:else}
          <DevCheckout />
        {/if}
      </div>
    {/key}
  </div>
</SideFrame>

<style>
  .page {
    max-width: 1080px;
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

  /* A heading for screen readers: the header's tab and the list already name the place. */
  .sr {
    position: absolute;
    width: 1px;
    height: 1px;
    overflow: hidden;
    clip-path: inset(50%);
  }
</style>
