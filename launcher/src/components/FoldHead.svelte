<script lang="ts">
  // A panel's title that folds the panel (Android's game page: what a player needs less often stays
  // closed until they open it): the title, a line saying what is inside, what `extra` adds (a
  // badge), and an arrow that turns as it opens.
  import type { Snippet } from "svelte";
  import Icon from "./Icon.svelte";

  let {
    title,
    id,
    controls,
    line = "",
    open = $bindable(false),
    extra,
  }: {
    title: string;
    /** The heading's id, which the panel is labelled by. */
    id: string;
    /** The id of what it folds. */
    controls: string;
    line?: string;
    open?: boolean;
    extra?: Snippet;
  } = $props();
</script>

<button class="fold-head" aria-expanded={open} aria-controls={controls} onclick={() => (open = !open)}>
  <span class="text">
    <h2 {id}>{title}</h2>
    {#if line}<small>{line}</small>{/if}
  </span>
  {@render extra?.()}
  <span class="arrow" class:open><Icon name="down" size={18} /></span>
</button>

<style>
  .fold-head {
    display: flex;
    align-items: center;
    gap: 12px;
    width: 100%;
    padding: 0;
    border: 0;
    background: none;
    color: inherit;
    text-align: left;
  }

  .text {
    flex: 1;
    min-width: 0;
    display: flex;
    flex-direction: column;
    gap: 2px;
  }

  small {
    font-size: 0.8125rem;
    color: var(--soft);
  }

  .arrow {
    flex: none;
    display: grid;
    place-items: center;
    width: 36px;
    height: 36px;
    border-radius: 50%;
    background: var(--panel-hi);
    color: var(--lav);
    transition: transform var(--medium) var(--ease);
  }

  .arrow.open {
    transform: rotate(180deg);
  }

  .fold-head:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 4px;
    border-radius: 6px;
  }
</style>
