<script lang="ts">
  // One setting on a mod's page (ModPage.svelte), as a row: its name and one line about it on the
  // left (the rest of the line unfolds with the info button), its control on the right, and Reset
  // when it is not at its default. Every control applies as it changes, with no Enter: a slider and
  // a typed number or word after a moment (the page's saver, lib/autosave.ts, waits for it to
  // settle), a switch, a choice or a key at once. A number with both ends is a slider with its
  // value in a field beside it, on the manifest's steps (lib/settingValues.ts), and a mark where its
  // default is; text is a field the row's width.
  import type { PluginSetting } from "$lib/api";
  import { bindLabel, parseBinds } from "$lib/keybinds";
  import * as T from "$lib/modsText";
  import { settingControl } from "$lib/plugins";
  import { formatValue, parseTyped, percentOf, snapValue, stepOf } from "$lib/settingValues";
  import Icon from "./Icon.svelte";
  import KeyBindSetting from "./KeyBindSetting.svelte";

  let {
    setting: s,
    id,
    saved = 0,
    conflicts = [],
    clashing = [],
    onchange,
  }: {
    /** The setting, with the value its control shows (the one not saved yet, else the core's). */
    setting: PluginSetting;
    /** A page-unique id for the row's label. */
    id: string;
    /** How many times it was saved: each time, the row says so for a moment. */
    saved?: number;
    /** A keybind's: the other keybinds of its group with one of its inputs. */
    conflicts?: string[];
    /** A keybind's: its binds one of those has too. */
    clashing?: string[];
    /** A new value, saved after DELAY ms of no further change. */
    onchange: (value: unknown, delay: number) => void;
  } = $props();

  /** How long a value that moves (a slider, a field being typed in) waits before it is saved. */
  const SETTLE = 350;
  const TYPED = 600;

  const control = $derived(settingControl(s));
  const atDefault = $derived(
    s.type === "keybind" ? parseBinds(s.value).join(",") === parseBinds(s.default).join(",") : s.value === s.default,
  );

  /** The default as the reset button names it. */
  const defaultText = $derived.by(() => {
    switch (s.type) {
      case "bool":
        return s.default === true ? T.SETTING.yes : T.SETTING.no;
      case "choice":
        return s.choices?.find((c) => c.value === s.default)?.name ?? String(s.default);
      case "int":
      case "float":
        return formatValue(s, s.default);
      case "keybind": {
        const binds = parseBinds(s.default);
        return binds.length ? binds.map(bindLabel).join(", ") : T.KEYBIND.none;
      }
      default:
        return typeof s.default === "string" && s.default !== "" ? `"${s.default}"` : T.SETTING.empty;
    }
  });

  // The line about it: one line, the rest unfolded on request.
  let unfolded = $state(false);
  let clamps = $state(false);
  function measure(node: HTMLElement) {
    const check = () => {
      clamps = unfolded || node.scrollHeight > node.clientHeight + 1;
    };
    const watch = new ResizeObserver(check);
    watch.observe(node);
    check();
    return {
      destroy: () => {
        watch.disconnect();
      },
    };
  }

  // A number being typed: the field keeps what was typed until it is left, then writes the value.
  let typing = $state<string | null>(null);
  const fieldText = $derived(typing ?? formatValue(s, s.value));
  const invalid = $derived(typing !== null && parseTyped(s, typing) === null);

  function typed(text: string) {
    typing = text;
    const n = parseTyped(s, text);
    if (n !== null && n !== s.value) onchange(n, TYPED);
  }

  function nudge(e: KeyboardEvent) {
    if (e.key === "ArrowUp" || e.key === "ArrowDown") {
      e.preventDefault();
      const by = stepOf(s) * (e.shiftKey ? 10 : 1) * (e.key === "ArrowUp" ? 1 : -1);
      const n = snapValue(s, (typeof s.value === "number" ? s.value : 0) + by);
      typing = null;
      if (n !== s.value) onchange(n, SETTLE);
    } else if (e.key === "Enter") {
      typing = null;
    }
  }

  // A keybind's waiting cap and what it said, for the row's line.
  let capturing = $state<number | null>(null);
  let note = $state<string | null>(null);

  // Text: what is typed shows at once, saved when it settles.
  const textValue = $derived(typeof s.value === "string" ? s.value : "");
</script>

<div class="setting-row" class:binds={control === "keybind"} role="group" aria-labelledby={`${id}-name`}>
  <div class="text">
    <div class="name-line">
      <span class="name" id={`${id}-name`}>{s.name}</span>
      {#if saved}{#key saved}<span class="tick" aria-hidden="true"
            ><Icon name="check" size={14} />{T.SETTING.saved}</span
          >{/key}{/if}
    </div>
    {#if control === "keybind" && capturing !== null}
      <p class="line hint">
        {T.KEYBIND.pressHint}
        <button class="cancel" onclick={() => (capturing = null)}>{T.KEYBIND.cancel}</button>
      </p>
    {:else if s.description}
      <div class="about">
        <p class="line" class:open={unfolded} id={`${id}-about`} use:measure>{s.description}</p>
        {#if clamps}
          <button
            class="more-about"
            aria-expanded={unfolded}
            aria-controls={`${id}-about`}
            aria-label={unfolded ? T.SETTING.less(s.name) : T.SETTING.more(s.name)}
            title={unfolded ? T.SETTING.less(s.name) : T.SETTING.more(s.name)}
            onclick={() => (unfolded = !unfolded)}><Icon name={unfolded ? "up" : "info"} size={15} /></button
          >
        {/if}
      </div>
    {/if}
    {#if conflicts.length}<p class="line warn">{T.alsoBound(conflicts)}</p>{/if}
    {#if note}<p class="line">{note}</p>{/if}
  </div>

  <div class="control">
    {#if control === "slider"}
      {@const at = percentOf(s, s.value)}
      {@const mark = percentOf(s, s.default)}
      <div class="slider" style={`--at: ${at}%; --mark: ${mark}%`}>
        <input
          type="range"
          min={s.min}
          max={s.max}
          step={stepOf(s)}
          value={typeof s.value === "number" ? s.value : 0}
          aria-labelledby={`${id}-name`}
          aria-valuetext={formatValue(s, s.value)}
          oninput={(e) => {
            typing = null;
            onchange(snapValue(s, Number(e.currentTarget.value)), SETTLE);
          }}
        />
        <span class="mark" class:hidden={atDefault} aria-hidden="true" title={T.SETTING.defaultMark(defaultText)}
        ></span>
      </div>
      <input
        class="num"
        type="text"
        inputmode="decimal"
        spellcheck="false"
        autocomplete="off"
        aria-label={T.SETTING.valueAria(s.name)}
        aria-invalid={invalid}
        value={fieldText}
        oninput={(e) => {
          typed(e.currentTarget.value);
        }}
        onkeydown={nudge}
        onblur={() => (typing = null)}
      />
    {:else if control === "number"}
      <input
        class="num wide-num"
        type="text"
        inputmode="decimal"
        spellcheck="false"
        autocomplete="off"
        aria-labelledby={`${id}-name`}
        aria-invalid={invalid}
        value={fieldText}
        oninput={(e) => {
          typed(e.currentTarget.value);
        }}
        onkeydown={nudge}
        onblur={() => (typing = null)}
      />
    {:else if control === "switch"}
      <input
        type="checkbox"
        role="switch"
        class="switch"
        aria-labelledby={`${id}-name`}
        checked={s.value === true}
        onchange={(e) => {
          onchange(e.currentTarget.checked, 0);
        }}
      />
    {:else if control === "segmented"}
      <div class="segmented" role="radiogroup" aria-labelledby={`${id}-name`}>
        {#each s.choices ?? [] as c (c.value)}
          <button
            role="radio"
            aria-checked={s.value === c.value}
            onclick={() => {
              onchange(c.value, 0);
            }}>{c.name}</button
          >
        {/each}
      </div>
    {:else if control === "select"}
      <select
        aria-labelledby={`${id}-name`}
        value={s.value}
        onchange={(e) => {
          onchange(e.currentTarget.value, 0);
        }}
      >
        {#each s.choices ?? [] as c (c.value)}<option value={c.value}>{c.name}</option>{/each}
      </select>
    {:else if control === "keybind"}
      <KeyBindSetting
        setting={s}
        {clashing}
        bind:capturing
        onnote={(text) => (note = text)}
        onchange={(value) => {
          onchange(value, 0);
        }}
      />
    {:else}
      <input
        class="field"
        type="text"
        spellcheck="false"
        autocomplete="off"
        aria-labelledby={`${id}-name`}
        maxlength={s.maxLength}
        placeholder={T.SETTING.empty}
        value={textValue}
        oninput={(e) => {
          onchange(e.currentTarget.value, TYPED);
        }}
      />
    {/if}
    <button
      class="reset"
      class:hidden={atDefault}
      aria-hidden={atDefault}
      tabindex={atDefault ? -1 : 0}
      aria-label={T.SETTING.reset(s.name, defaultText)}
      title={T.SETTING.reset(s.name, defaultText)}
      onclick={() => {
        typing = null;
        onchange(s.default, 0);
      }}><Icon name="reset" size={16} /></button
    >
  </div>
</div>

<style>
  /* A row, always side by side: the words take what is left (and shrink, never the control), the
     control keeps its width at the right, so the controls line up down the page. Plain flex, no
     grid and no breakpoint, so every webview lays it out the same. */
  .setting-row {
    display: flex;
    flex-direction: row;
    flex-wrap: nowrap;
    align-items: center;
    gap: 24px;
    min-height: 64px;
    padding: 12px 0;
    border-top: 1px solid var(--line);
  }

  .setting-row:first-child {
    border-top: 0;
  }

  .text {
    flex: 1 1 0;
    min-width: 0;
  }

  .name-line {
    display: flex;
    align-items: baseline;
    gap: 10px;
  }

  .name {
    font-size: 0.9375rem;
    font-weight: 600;
    color: var(--text);
  }

  .tick {
    display: inline-flex;
    align-items: center;
    gap: 4px;
    font-size: 0.75rem;
    font-weight: 600;
    color: var(--ok);
    animation: tick 1.6s var(--ease) both;
  }

  @keyframes tick {
    0% {
      opacity: 0;
    }
    12%,
    70% {
      opacity: 1;
    }
    100% {
      opacity: 0;
    }
  }

  .about {
    display: flex;
    align-items: flex-start;
    gap: 6px;
    margin-top: 2px;
  }

  .line {
    margin: 2px 0 0;
    font-size: 0.8125rem;
    line-height: 1.5;
    color: var(--soft);
    max-width: 72ch;
  }

  .about .line {
    margin: 0;
    display: -webkit-box;
    -webkit-box-orient: vertical;
    -webkit-line-clamp: 1;
    line-clamp: 1;
    overflow: hidden;
  }

  .about .line.open {
    display: block;
    -webkit-line-clamp: unset;
    line-clamp: none;
  }

  .more-about {
    flex: none;
    display: grid;
    place-items: center;
    width: 24px;
    height: 20px;
    padding: 0;
    border: 0;
    border-radius: 6px;
    background: none;
    color: var(--dim);
  }

  .more-about[aria-expanded="true"] {
    color: var(--amber);
  }

  .line.warn,
  .line.hint {
    color: var(--amber);
  }

  .cancel {
    padding: 0;
    margin-left: 4px;
    border: 0;
    background: none;
    font: inherit;
    color: var(--lav);
    text-decoration: underline;
    text-underline-offset: 2px;
  }

  .control {
    flex: 0 0 auto;
    display: flex;
    flex-direction: row;
    flex-wrap: nowrap;
    align-items: center;
    justify-content: flex-end;
    gap: 12px;
  }

  /* The slider: a 6 px track filled in the brand's orange up to the value, a 20 px knob, a hit area
     36 px tall; a tick where the default is, while the value is elsewhere. */
  .slider {
    position: relative;
    flex: 0 0 auto;
    width: 260px;
    height: 36px;
    display: flex;
    align-items: center;
  }

  .slider input {
    appearance: none;
    width: 100%;
    height: 36px;
    margin: 0;
    background: transparent;
    cursor: pointer;
    position: relative;
    z-index: 1;
  }

  .slider input::-webkit-slider-runnable-track {
    height: 6px;
    border-radius: 999px;
    background: linear-gradient(to right, var(--brand) var(--at), var(--ink-deep) var(--at));
    box-shadow: inset 0 0 0 1px var(--line-strong);
  }

  .slider input::-moz-range-track {
    height: 6px;
    border-radius: 999px;
    background: var(--ink-deep);
    box-shadow: inset 0 0 0 1px var(--line-strong);
  }

  .slider input::-moz-range-progress {
    height: 6px;
    border-radius: 999px;
    background: var(--brand);
  }

  .slider input::-webkit-slider-thumb {
    appearance: none;
    width: 20px;
    height: 20px;
    margin-top: -7px;
    border-radius: 50%;
    background: var(--text);
    border: 2px solid var(--brand);
    box-shadow: 0 2px 6px rgb(0 0 0 / 0.45);
    transition: transform var(--fast) var(--ease);
  }

  .slider input::-moz-range-thumb {
    width: 16px;
    height: 16px;
    border-radius: 50%;
    background: var(--text);
    border: 2px solid var(--brand);
    box-shadow: 0 2px 6px rgb(0 0 0 / 0.45);
  }

  .slider input:active::-webkit-slider-thumb {
    transform: scale(1.12);
  }

  @media (hover: hover) {
    .slider input:hover::-webkit-slider-thumb {
      transform: scale(1.08);
    }

    .more-about:hover,
    .reset:hover {
      color: var(--text);
    }
  }

  .slider input:focus-visible {
    outline: none;
  }

  .slider input:focus-visible::-webkit-slider-thumb {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .slider input:focus-visible::-moz-range-thumb {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .mark {
    position: absolute;
    left: calc(10px + (100% - 20px) * var(--mark) / 100);
    top: 50%;
    width: 2px;
    height: 14px;
    margin-left: -1px;
    transform: translateY(-50%);
    border-radius: 1px;
    background: var(--soft);
    opacity: 0.7;
  }

  .mark.hidden {
    display: none;
  }

  /* The value beside it, a field to type in: the arrows step, Shift by ten. */
  .num {
    width: 76px;
    height: 36px;
    padding: 6px 10px;
    text-align: right;
    font-variant-numeric: tabular-nums;
    font-size: 0.9375rem;
    font-weight: 600;
  }

  .num.wide-num {
    width: 104px;
  }

  .num[aria-invalid="true"] {
    border-color: var(--err);
  }

  .field {
    flex: 0 0 auto;
    width: 300px;
    height: 40px;
    font-size: 0.9375rem;
  }

  .segmented button {
    min-height: 34px;
    padding: 6px 16px;
    font-size: 0.875rem;
  }

  select {
    min-width: 200px;
    height: 38px;
    font: inherit;
    font-size: 0.875rem;
    color: var(--text);
    background: var(--ink-deep);
    border: 1px solid var(--line-field);
    border-radius: var(--radius-sm);
    padding: 6px 12px;
  }

  select:focus-visible,
  .reset:focus-visible,
  .more-about:focus-visible,
  .cancel:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .reset {
    flex: none;
    display: grid;
    place-items: center;
    width: 36px;
    height: 36px;
    padding: 0;
    border-color: transparent;
    background: transparent;
    color: var(--soft);
  }

  .reset.hidden {
    visibility: hidden;
  }

  /* The keybinds: the caps keep their two columns. */
  .binds .control {
    gap: 8px;
  }

  /* The narrowest windows only (below the launcher's 960 px floor): the control under the words. */
  @media (max-width: 599px) {
    .setting-row {
      flex-wrap: wrap;
    }

    .text {
      flex-basis: 100%;
    }
  }
</style>
