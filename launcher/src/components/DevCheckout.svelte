<script lang="ts">
  // Develop's "This checkout": the OpenRAC folder, how many of the games' disc images are in
  // baserom/, and the checkout's own actions (identify discs, place inputs, the tools' tests).
  import { api } from "$lib/api";
  import { app, guard, refresh } from "$lib/app.svelte";
  import ActionRow from "./ActionRow.svelte";
  import Icon from "./Icon.svelte";

  const repo = { kind: "repository" } as const;
  const lib = $derived(app.library);
  const actions = $derived((lib?.repository ?? []).filter((a) => a.thisPlatform));
  const elsewhere = $derived((lib?.repository ?? []).filter((a) => !a.thisPlatform));
  const versions = $derived(lib?.games.flatMap((g) => g.versions) ?? []);
  const discs = $derived(versions.filter((v) => v.status.disc.state === "found").length);
</script>

{#if lib}
  <section class="panel">
    <div class="row">
      <h2 class="grow">This checkout</h2>
      <button class="ghost small" onclick={() => void refresh()}><Icon name="refresh" size={14} />Refresh</button>
    </div>
    <p class="where">
      <button class="link mono" onclick={() => void guard(api.openPath("."))} title="Open the folder">{lib.root}</button
      >
      <span class="dim">·</span>
      {discs} of {versions.length} disc images found in <span class="mono">baserom/</span>
    </p>
    <ul>
      {#each actions as action (action.id)}<ActionRow {action} scope={repo} />{/each}
    </ul>
    {#if elsewhere.length}
      <details>
        <summary>{elsewhere.length} more for other platforms</summary>
        <ul>
          {#each elsewhere as action (action.id)}<ActionRow {action} scope={repo} />{/each}
        </ul>
      </details>
    {/if}
  </section>
{/if}

<style>
  .where {
    margin: 4px 0 12px;
    font-size: 14px;
    color: var(--soft);
    overflow-wrap: anywhere;
  }

  .link {
    border: 0;
    background: none;
    padding: 0;
    color: var(--amber);
    font-weight: 400;
    text-decoration: underline;
    text-decoration-color: color-mix(in srgb, var(--amber) 40%, transparent);
  }

  ul {
    margin: 0;
    padding: 0;
    list-style: none;
  }

  details summary {
    margin-top: 12px;
    cursor: pointer;
    color: var(--lav);
    font-size: 14px;
  }
</style>
