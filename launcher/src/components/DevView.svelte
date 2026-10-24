<script lang="ts">
  // What Develop shows beside its list (or, on Android, under its tabs): the console, a version
  // (DeveloperPanel), or the checkout, by `selected`, sliding in when it changes.
  import { app } from "$lib/app.svelte";
  import { enter } from "$lib/motion";
  import { fly } from "svelte/transition";
  import DevCheckout from "./DevCheckout.svelte";
  import DevConsole from "./DevConsole.svelte";
  import DeveloperPanel from "./DeveloperPanel.svelte";

  let { selected }: { selected: string } = $props();

  const games = $derived(app.library?.games ?? []);
  const version = $derived(games.flatMap((g) => g.versions).find((v) => v.key === selected) ?? null);
  const game = $derived(version ? (games.find((g) => g.id === version.game) ?? null) : null);
</script>

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

<style>
  .view {
    display: flex;
    flex-direction: column;
    gap: 18px;
  }
</style>
