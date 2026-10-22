<script lang="ts">
  // A key-binding setting's control (a "keybind" setting, docs/settings.md in the SDK), in its row
  // of ModSetting.svelte, which says what a waiting cap wants and what else has its key: one key cap
  // per bind it may have (one or two), each a button. Pressed, a cap waits for the next key, mouse
  // button or turn of the wheel and takes it; any key counts, Escape and Tab too, so cancelling is a
  // click elsewhere (or Cancel). The cross clears a bind. A bind the setting already has in its other
  // cap swaps over rather than being there twice; one another setting of its group has is said under
  // it. The value goes out through onchange, as the other settings' do.
  import type { PluginSetting } from "$lib/api";
  import {
    bindLabel,
    bindOfKey,
    bindOfMouse,
    bindOfWheel,
    bindSpoken,
    formatBinds,
    mostBinds,
    parseBinds,
    withBind,
    withoutBind,
  } from "$lib/keybinds";
  import * as T from "$lib/modsText";
  import Icon from "./Icon.svelte";

  let {
    setting: s,
    clashing = [],
    capturing = $bindable(null),
    onnote = () => undefined,
    onchange,
  }: {
    setting: PluginSetting;
    /** Its binds that one of those has too. */
    clashing?: string[];
    /** The cap waiting for an input, or null: the row then says to press one. */
    capturing?: number | null;
    /** What the last change did besides binding (a swap), for the row's line; null to clear it. */
    onnote?: (note: string | null) => void;
    onchange: (value: string) => void;
  } = $props();

  const binds = $derived(parseBinds(s.value));
  const slots = $derived(Array.from({ length: mostBinds(s) }, (_, i) => i));

  let caps: HTMLButtonElement[] = $state([]);
  /** When a mouse button was taken: the click that follows it is not a new capture. */
  let boundAt = 0;

  function start(slot: number) {
    if (Date.now() - boundAt < 400) return;
    onnote(null);
    capturing = slot;
  }

  function take(name: string) {
    const slot = capturing;
    if (slot === null) return;
    capturing = null;
    const next = withBind(binds, slot, name);
    onnote(binds.includes(name) && binds.indexOf(name) !== slot ? T.KEYBIND.swapped : null);
    const value = formatBinds(next);
    if (value !== formatBinds(binds)) onchange(value);
    caps[Math.min(slot, next.length - 1)]?.focus();
  }

  function clear(slot: number) {
    onnote(null);
    capturing = null;
    onchange(formatBinds(withoutBind(binds, slot)));
  }

  // While a cap waits: the window's next key, a button pressed on the cap, the wheel on it.
  $effect(() => {
    if (capturing === null) return;
    const cap = caps[capturing];
    const key = (e: KeyboardEvent) => {
      e.preventDefault();
      e.stopPropagation();
      const name = bindOfKey(e);
      if (name) take(name);
    };
    const down = (e: MouseEvent) => {
      if (cap?.contains(e.target as Node)) {
        e.preventDefault();
        e.stopPropagation();
        const name = bindOfMouse(e);
        if (name) {
          boundAt = Date.now();
          take(name);
        }
      } else {
        capturing = null;
      }
    };
    const menu = (e: MouseEvent) => {
      e.preventDefault();
    };
    const wheel = (e: WheelEvent) => {
      e.preventDefault();
      const name = bindOfWheel(e);
      if (name) take(name);
    };
    const away = () => (capturing = null);
    window.addEventListener("keydown", key, true);
    window.addEventListener("mousedown", down, true);
    window.addEventListener("contextmenu", menu, true);
    window.addEventListener("blur", away);
    cap?.addEventListener("wheel", wheel, { passive: false });
    return () => {
      window.removeEventListener("keydown", key, true);
      window.removeEventListener("mousedown", down, true);
      window.removeEventListener("contextmenu", menu, true);
      window.removeEventListener("blur", away);
      cap?.removeEventListener("wheel", wheel);
    };
  });
</script>

<div class="caps" role="group" aria-label={s.name}>
  {#each slots as slot (slot)}
    {@const name = binds[slot]}
    {#if name !== undefined || slot === binds.length || capturing === slot}
      <span class="cap-wrap">
        <button
          bind:this={caps[slot]}
          class="cap"
          class:waiting={capturing === slot}
          class:empty={!name && capturing !== slot}
          class:clash={!!name && clashing.includes(name)}
          aria-label={capturing === slot
            ? T.KEYBIND.press
            : name
              ? T.bindAria(s.name, bindSpoken(name))
              : T.addBindAria(s.name)}
          onclick={() => {
            start(slot);
          }}
          >{#if capturing === slot}{T.KEYBIND.press}{:else if name}{bindLabel(name)}{:else}<span class="add"
              >{slot === 0 ? T.KEYBIND.none : T.KEYBIND.add}</span
            >{/if}</button
        >{#if name && capturing !== slot}<button
            class="clear"
            aria-label={T.clearBindAria(s.name, bindSpoken(name))}
            title={T.clearBindAria(s.name, bindSpoken(name))}
            onclick={() => {
              clear(slot);
            }}><Icon name="x" size={14} /></button
          >{/if}
      </span>
    {/if}
  {/each}
</div>

<style>
  /* Two columns of caps, the same on every row, so the bindings read as a table. */
  .caps {
    flex: none;
    display: grid;
    grid-template-columns: repeat(2, 156px);
    align-items: center;
    gap: 8px;
  }

  .cap-wrap {
    display: flex;
    align-items: stretch;
  }

  .cap {
    flex: 1;
    min-width: 0;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    text-align: center;
    justify-content: center;
    min-height: 38px;
    padding: 6px 12px;
    font-family: var(--font-mono);
    font-size: 0.875rem;
    font-weight: 600;
    color: var(--text);
    background: var(--panel-hi);
    border: 1px solid var(--line-strong);
    border-bottom-width: 3px;
    border-radius: var(--radius-sm);
    cursor: pointer;
  }

  .cap-wrap:has(.clear) .cap {
    border-top-right-radius: 0;
    border-bottom-right-radius: 0;
  }

  @media (hover: hover) {
    .cap:hover {
      border-color: var(--amber);
    }

    .clear:hover {
      color: var(--err);
    }
  }

  .cap.empty {
    background: transparent;
    border-style: dashed;
    border-bottom-width: 1px;
  }

  .cap.waiting {
    font-family: var(--font-sans);
    font-weight: 500;
    color: var(--bar-ink);
    background: var(--amber);
    border-color: var(--amber);
    animation: pulse 1.2s ease-in-out infinite;
  }

  .cap.clash {
    border-color: var(--amber);
    color: var(--amber);
  }

  .add {
    font-family: var(--font-sans);
    font-weight: 400;
    color: var(--dim);
  }

  .clear {
    display: grid;
    place-items: center;
    min-width: 34px;
    padding: 0 8px;
    font-size: 0.9375rem;
    line-height: 1;
    color: var(--soft);
    background: var(--panel-hi);
    border: 1px solid var(--line-strong);
    border-left: none;
    border-bottom-width: 3px;
    border-radius: 0 var(--radius-sm) var(--radius-sm) 0;
    cursor: pointer;
  }

  .cap:focus-visible,
  .clear:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  @media (max-width: 560px) {
    .caps {
      grid-template-columns: repeat(2, minmax(0, 1fr));
      width: 100%;
    }
  }

  @keyframes pulse {
    50% {
      opacity: 0.75;
    }
  }

  @media (prefers-reduced-motion: reduce) {
    .cap.waiting {
      animation: none;
    }
  }
</style>
