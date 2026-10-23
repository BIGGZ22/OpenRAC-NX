<script lang="ts">
  // The list on the left of the Games, Mods and Develop tabs: one entry per place, all the same
  // size, the open one marked (the header's selected-tab colours on the list's dark ground).
  // Each entry is a picture (a game's, framed in its colours) or an icon, a name, and a line on
  // where it stands. In a narrow window only the pictures show, the names in their tooltips. On
  // Android it is a row of pictures above the page (SideFrame), each named across its foot.
  import { onAndroid } from "$lib/api";
  import { shortTitle, type SideEntry } from "$lib/labels";
  import { hideMissing } from "$lib/media";
  import Icon from "./Icon.svelte";

  let {
    entries,
    selected,
    onpick,
    label,
  }: { entries: SideEntry[]; selected: string | null; onpick: (key: string) => void; label: string } = $props();
</script>

<nav class="list" aria-label={label}>
  {#each entries as e, i (e.key)}
    {#if e.group}<span class="group">{e.group}</span>{/if}
    <button
      class="entry"
      class:on={selected === e.key}
      aria-current={selected === e.key ? "page" : undefined}
      title={e.state ? `${e.name}: ${e.state.text}` : e.name}
      style={`${e.style ?? ""}; --i: ${i}`}
      onclick={() => {
        onpick(e.key);
      }}
    >
      <span class="thumb" class:icon={!e.art}>
        {#if e.art}<img src={e.art} alt="" decoding="async" onerror={hideMissing} />{:else if e.icon}<Icon
            name={e.icon}
            size={20}
          />{/if}
        {#if e.mark}<span class="mark" aria-hidden="true"></span>{/if}
      </span>
      <span class="text">
        <span class="name">{onAndroid ? shortTitle(e.name) : e.name}</span>
        {#if e.mark}<span class="sr-only">, crashed recently</span>{/if}
        {#if e.state}
          <span class={`state ${e.state.tone}`}>
            {#if e.state.busy}<span class="spinner"></span>{:else}<span class="dot"></span>{/if}{e.state.text}
          </span>
        {/if}
      </span>
    </button>
  {/each}
</nav>

<style>
  .list {
    display: flex;
    flex-direction: column;
    gap: 4px;
    padding: 20px 10px;
    background: var(--ink-deep);
    border-right: 1px solid var(--line);
    min-height: 100%;
  }

  .group {
    padding: 14px 8px 6px;
    font: 600 0.75rem/1 var(--font-head);
    letter-spacing: 0.14em;
    text-transform: uppercase;
    color: var(--dim);
  }

  .group:first-child {
    padding-top: 0;
  }

  .entry {
    display: flex;
    align-items: center;
    gap: 12px;
    width: 100%;
    padding: 8px;
    border: 1px solid transparent;
    border-radius: 12px;
    background: none;
    color: var(--text);
    text-align: left;
    font-weight: 400;
    transition:
      background-color 0.18s ease,
      border-color 0.18s ease,
      transform 0.18s ease;
    /* The entries come in one after the other when the list shows. */
    animation: list-in 0.36s cubic-bezier(0.2, 0.8, 0.2, 1) both;
    animation-delay: calc(var(--i, 0) * 35ms);
  }

  @keyframes list-in {
    from {
      opacity: 0;
      transform: translateX(-8px);
    }
  }

  @media (hover: hover) {
    .entry:hover:not(.on) {
      background: color-mix(in srgb, var(--panel-hi) 60%, transparent);
      transform: translateX(2px);
    }
  }

  .entry:active {
    transform: scale(0.98);
  }

  .entry.on {
    background: var(--panel-hi);
    border-color: var(--line-strong);
  }

  .entry.on .name {
    color: var(--amber);
  }

  /* Every picture the same size, in the game's own frame colour. */
  .thumb {
    position: relative;
    flex: none;
    display: grid;
    place-items: center;
    width: 64px;
    height: 44px;
    border-radius: 8px;
    overflow: hidden;
    background: linear-gradient(160deg, var(--from, var(--panel)), var(--to, var(--ink-deep)));
    border: 1px solid var(--bd, var(--line-strong));
    color: var(--soft);
    transition: box-shadow 0.2s ease;
  }

  .entry.on .thumb {
    box-shadow: 0 0 0 2px var(--amber);
  }

  .thumb.icon {
    background: var(--panel);
    border-color: var(--line-strong);
  }

  .thumb img {
    width: 100%;
    height: 100%;
    object-fit: cover;
    filter: saturate(0.85);
    transition:
      filter 0.2s ease,
      transform 0.3s ease;
  }

  @media (hover: hover) {
    .entry:hover .thumb img {
      transform: scale(1.06);
      filter: none;
    }
  }

  .entry.on .thumb img {
    filter: none;
  }

  .mark {
    position: absolute;
    top: 4px;
    right: 4px;
    width: 9px;
    height: 9px;
    border-radius: 50%;
    background: var(--err);
    box-shadow: 0 0 0 2px var(--ink-deep);
  }

  .text {
    display: flex;
    flex-direction: column;
    gap: 2px;
    min-width: 0;
  }

  .name {
    font-size: 0.8438rem;
    font-weight: 600;
    line-height: 1.3;
    color: var(--text);
  }

  .state {
    display: flex;
    align-items: center;
    gap: 6px;
    font-size: 0.75rem;
    color: var(--dim);
  }

  .state.ok {
    color: var(--ok);
  }

  .state.warn {
    color: var(--amber);
  }

  .dot {
    width: 6px;
    height: 6px;
    border-radius: 50%;
    background: currentColor;
    flex: none;
  }

  /* A narrow window: the pictures only. */
  @media (max-width: 1000px) {
    .list {
      padding: 16px 8px;
    }

    .group {
      display: none;
    }

    .entry {
      justify-content: center;
      padding: 6px;
    }

    .text {
      position: absolute;
      width: 1px;
      height: 1px;
      overflow: hidden;
      clip-path: inset(50%);
    }

    .thumb {
      width: 52px;
      height: 40px;
    }
  }

  :global(html[data-platform="android"]) .list {
    flex-direction: row;
    gap: 10px;
    width: max-content;
    min-width: 100%;
    min-height: 0;
    padding: var(--page-top) var(--page-x) 0;
    background: none;
    border-right: none;
  }

  :global(html[data-platform="android"]) .group {
    display: none;
  }

  :global(html[data-platform="android"]) .entry {
    position: relative;
    flex: none;
    width: auto;
    padding: 0;
    border: none;
    border-radius: 10px;
  }

  :global(html[data-platform="android"]) .entry.on {
    background: none;
    transform: none;
  }

  @media (hover: hover) {
    :global(html[data-platform="android"]) .entry:hover:not(.on) {
      background: none;
      transform: none;
    }
  }

  :global(html[data-platform="android"]) .thumb {
    width: 112px;
    height: 63px;
    border-radius: 10px;
  }

  :global(html[data-platform="android"]) .entry:not(.on) .thumb {
    opacity: 0.7;
  }

  /* The name across the picture's foot, the state left to the page. */
  :global(html[data-platform="android"]) .text {
    position: absolute;
    inset: auto 0 0;
    width: auto;
    height: auto;
    padding: 16px 8px 5px;
    clip-path: none;
    border-radius: 0 0 10px 10px;
    background: linear-gradient(transparent, rgb(0 0 0 / 0.8));
    pointer-events: none;
  }

  :global(html[data-platform="android"]) .name {
    overflow: hidden;
    font-size: 0.75rem;
    line-height: 1.2;
    white-space: nowrap;
    text-overflow: ellipsis;
    color: var(--on-game);
  }

  :global(html[data-platform="android"]) .state {
    display: none;
  }
</style>
