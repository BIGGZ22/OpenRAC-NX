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
  // to its right edge (above it when there is no room below). The list is a
  // popover, in the browser's top layer, so no box it sits in can cut it off.
  // Closes on a choice, a click elsewhere, Escape, or when the page moves. From the keyboard, as
  // a menu is: its first item takes the focus, the arrows, Home and End move in it, and Escape or
  // Tab goes back to the button.
  import Icon, { type IconName } from "./Icon.svelte";

  let { items, label = "More actions" }: { items: MenuItem[]; label?: string } = $props();

  let open = $state(false);
  let root = $state<HTMLElement | null>(null);
  let list = $state<HTMLElement | null>(null);
  let trigger = $state<HTMLElement | null>(null);

  /** Puts the list by the button, inside the window. */
  function place() {
    if (!root || !list) return;
    const at = root.getBoundingClientRect();
    const gap = 8;
    const below = at.bottom + gap + list.offsetHeight <= window.innerHeight - gap;
    const top = below ? at.bottom + gap : Math.max(gap, at.top - gap - list.offsetHeight);
    const left = Math.min(Math.max(gap, at.right - list.offsetWidth), window.innerWidth - list.offsetWidth - gap);
    list.style.top = `${top}px`;
    list.style.left = `${left}px`;
  }

  $effect(() => {
    if (!list) return;
    list.showPopover();
    place();
    list.querySelector<HTMLElement>('[role="menuitem"]')?.focus();
  });
  const plain = $derived(items.filter((i) => !i.danger));
  const danger = $derived(items.filter((i) => i.danger));

  $effect(() => {
    if (!open) return;
    const outside = (e: MouseEvent) => {
      if (root && !root.contains(e.target as Node)) open = false;
    };
    const key = (e: KeyboardEvent) => {
      if (e.key === "Escape" || e.key === "Tab") {
        if (e.key === "Escape") e.preventDefault();
        open = false;
        trigger?.focus();
      }
    };
    const moved = () => (open = false);
    window.addEventListener("mousedown", outside);
    window.addEventListener("keydown", key);
    window.addEventListener("resize", moved);
    // Capturing: the page scrolls inside <main>, and scroll does not bubble.
    window.addEventListener("scroll", moved, true);
    return () => {
      window.removeEventListener("mousedown", outside);
      window.removeEventListener("keydown", key);
      window.removeEventListener("resize", moved);
      window.removeEventListener("scroll", moved, true);
    };
  });

  /** Up, Down, Home and End in the list, round from one end to the other. */
  function move(e: KeyboardEvent) {
    const all = [...(list?.querySelectorAll<HTMLElement>('[role="menuitem"]') ?? [])];
    const at = all.indexOf(document.activeElement as HTMLElement);
    const to =
      e.key === "ArrowDown"
        ? (at + 1) % all.length
        : e.key === "ArrowUp"
          ? (at - 1 + all.length) % all.length
          : e.key === "Home"
            ? 0
            : e.key === "End"
              ? all.length - 1
              : null;
    if (to === null) return;
    e.preventDefault();
    all[to]?.focus();
  }

  function choose(item: MenuItem) {
    open = false;
    // The list goes away with the focus in it: back to its button, before the item moves it on.
    trigger?.focus();
    item.run();
  }
</script>

<div class="menu" bind:this={root}>
  <button
    class="trigger"
    bind:this={trigger}
    aria-label={label}
    title={label}
    aria-haspopup="menu"
    aria-expanded={open}
    onclick={() => (open = !open)}><Icon name="more" size={20} /></button
  >
  {#if open}
    <div class="list" role="menu" popover="manual" tabindex="-1" bind:this={list} onkeydown={move}>
      {#each plain as item (item.label)}
        <button
          role="menuitem"
          tabindex="-1"
          onclick={() => {
            choose(item);
          }}><Icon name={item.icon} size={16} />{item.label}</button
        >
      {/each}
      {#if danger.length && plain.length}<hr />{/if}
      {#each danger as item (item.label)}
        <button
          role="menuitem"
          tabindex="-1"
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
    position: fixed;
    inset: auto;
    margin: 0;
    min-width: 230px;
    color: inherit;
    flex-direction: column;
    padding: 6px;
    background: var(--panel-hi);
    border: 1px solid var(--line-strong);
    border-radius: var(--radius-sm);
    box-shadow: var(--shadow-sm);
  }

  /* Not on .list itself: a popover that is not showing must stay display: none. */
  .list:popover-open {
    display: flex;
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

  .list button:focus-visible {
    background: rgb(255 255 255 / 0.08);
  }

  @media (hover: hover) {
    .list button:hover {
      background: rgb(255 255 255 / 0.08);
    }
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
