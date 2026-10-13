<script lang="ts" module>
  export interface MenuItem {
    label: string;
    icon: IconName;
    run: () => void;
    /** Removes or deletes something: set apart, in the error colour. */
    danger?: boolean;
  }
</script>

<script lang="ts">
  // A button that opens a short list of secondary actions, below it and aligned
  // to its right edge. Closes on a choice, a click elsewhere or Escape.
  import Icon, { type IconName } from "./Icon.svelte";

  let { items, label = "More actions" }: { items: MenuItem[]; label?: string } = $props();

  let open = $state(false);
  let root = $state<HTMLElement | null>(null);
  const plain = $derived(items.filter((i) => !i.danger));
  const danger = $derived(items.filter((i) => i.danger));

  $effect(() => {
    if (!open) return;
    const outside = (e: MouseEvent) => {
      if (root && !root.contains(e.target as Node)) open = false;
    };
    const key = (e: KeyboardEvent) => {
      if (e.key === "Escape") open = false;
    };
    window.addEventListener("mousedown", outside);
    window.addEventListener("keydown", key);
    return () => {
      window.removeEventListener("mousedown", outside);
      window.removeEventListener("keydown", key);
    };
  });

  function choose(item: MenuItem) {
    open = false;
    item.run();
  }
</script>

<div class="menu" bind:this={root}>
  <button
    class="trigger"
    aria-label={label}
    title={label}
    aria-haspopup="menu"
    aria-expanded={open}
    onclick={() => (open = !open)}><Icon name="more" size={20} /></button
  >
  {#if open}
    <div class="list" role="menu">
      {#each plain as item (item.label)}
        <button
          role="menuitem"
          onclick={() => {
            choose(item);
          }}><Icon name={item.icon} size={16} />{item.label}</button
        >
      {/each}
      {#if danger.length && plain.length}<hr />{/if}
      {#each danger as item (item.label)}
        <button
          role="menuitem"
          class="danger"
          onclick={() => {
            choose(item);
          }}><Icon name={item.icon} size={16} />{item.label}</button
        >
      {/each}
    </div>
  {/if}
</div>

<style>
  .menu {
    position: relative;
    display: inline-flex;
  }

  .trigger {
    width: 44px;
    height: 44px;
    padding: 0;
    justify-content: center;
    background: rgb(0 0 0 / 0.45);
    border-color: rgb(255 255 255 / 0.22);
  }

  .list {
    position: absolute;
    top: calc(100% + 8px);
    right: 0;
    z-index: 30;
    min-width: 230px;
    display: flex;
    flex-direction: column;
    padding: 6px;
    background: var(--panel-hi);
    border: 1px solid var(--line-strong);
    border-radius: var(--radius-sm);
    box-shadow: var(--shadow-sm);
  }

  .list button {
    justify-content: flex-start;
    border: 0;
    border-radius: 7px;
    background: none;
    padding: 9px 12px;
    font-weight: 500;
    white-space: nowrap;
  }

  .list button:hover,
  .list button:focus-visible {
    background: rgb(255 255 255 / 0.08);
  }

  .list .danger {
    color: var(--err);
  }

  hr {
    width: 100%;
    margin: 6px 0;
    border: 0;
    border-top: 1px solid var(--line);
  }
</style>
