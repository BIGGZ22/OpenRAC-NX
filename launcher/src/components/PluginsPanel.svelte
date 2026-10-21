<script lang="ts">
  // Gameplay mods: the game's native mods (docs/port/MODS.md, core/src/plugins.rs), the first thing
  // on Mods for a game with a port. Each in load order with a switch, its state in a few words, its
  // settings from its manifest, up and down, and Remove in its menu; Add a mod takes a .zip or a
  // folder (a texture and model pack goes to the packs below: lib/addMods.ts), and Purist mode,
  // the launcher's one switch, starts the game with none. Every change is written at once to the
  // list the game reads at its next start (plugins.json). With developer tools on: ids, folders
  // and the hook points each mod takes.
  import { api, inTauri, pickFolder, pickZip, type PluginSetting, type PluginView, type VersionView } from "$lib/api";
  import { addMods } from "$lib/addMods";
  import { app, guard, saveConfig } from "$lib/app.svelte";
  import { pluginState, settingControl, settingNumber, settingRange, sharedWith } from "$lib/plugins";
  import Icon from "./Icon.svelte";
  import Menu from "./Menu.svelte";

  const MODS_URL = "https://github.com/OpenRAC/rac1-mods";

  let {
    version: v,
    reload = 0,
    onpack = () => undefined,
  }: {
    version: VersionView;
    /** Bumped when a mod was added elsewhere on the page (dropped on it): the list is read again. */
    reload?: number;
    /** A texture and model pack was added from here: the packs' list is read again. */
    onpack?: () => void;
  } = $props();

  const developer = $derived(app.config?.developer ?? false);
  const purist = $derived(app.config?.purist ?? false);

  let mods = $state<PluginView[]>([]);
  let listError = $state<string | null>(null);
  let adding = $state(false);
  /** What the last add or change said, in a sentence. */
  let said = $state<{ text: string; failed: boolean } | null>(null);

  async function load() {
    try {
      mods = await api.pluginsList(v.game, v.name);
      listError = null;
    } catch (e) {
      listError = typeof e === "string" ? e : String(e);
    }
  }

  $effect(() => {
    // Again whenever RELOAD is bumped, and for another game.
    if (reload >= 0 && v.key) void load();
  });

  async function add(paths: string[]) {
    adding = true;
    const added = await addMods(v, paths);
    said = { text: added.text, failed: added.failed };
    if (added.pack) onpack();
    adding = false;
    await load();
  }

  async function addZip() {
    const zip = await pickZip("Choose a mod (.zip)");
    if (zip) await add([zip]);
  }

  async function addFolder() {
    const folder = inTauri ? await pickFolder("Choose a mod's folder") : "/home/you/Downloads/lock-strafe";
    if (folder) await add([folder]);
  }

  /** A change to the list: the list as it now is, or the reason it was refused, said under the head. */
  async function change(work: Promise<PluginView[]>) {
    try {
      mods = await work;
      said = null;
    } catch (e) {
      said = { text: typeof e === "string" ? e : String(e), failed: true };
      await load();
    }
  }

  function remove(mod: PluginView) {
    if (!confirm(`Remove ${mod.name}? The launcher's copy and its settings are deleted.`)) return;
    void change(api.pluginsRemove(v.game, v.name, mod.id));
  }

  const setValue = (mod: PluginView, s: PluginSetting, value: unknown) =>
    void change(api.pluginsSetSetting(v.game, v.name, mod.id, s.key, value));

  /** A number typed: kept when the setting takes it, else put back as it was. */
  function setNumber(mod: PluginView, s: PluginSetting, field: HTMLInputElement) {
    const n = settingNumber(s, field.value);
    if (n === null) {
      field.value = String(s.value);
      said = {
        text: `${s.name} takes ${settingRange(s) || (s.type === "int" ? "a whole number" : "a number")}.`,
        failed: true,
      };
      return;
    }
    setValue(mod, s, n);
  }

  async function setPurist(on: boolean) {
    if (app.config) await saveConfig({ ...app.config, purist: on });
  }
</script>

<section class="panel plugins" aria-label="Gameplay mods">
  <div class="row head">
    <h2 class="grow">Gameplay mods</h2>
    <button class="small" disabled={adding} onclick={() => void addZip()}
      >{#if adding}<span class="spinner"></span>{:else}<Icon name="library" size={14} />{/if}Add a .zip…</button
    >
    <button class="small" disabled={adding} onclick={() => void addFolder()}
      ><Icon name="folder" size={14} />Add a folder…</button
    >
  </div>

  <label class="setting purist">
    <span class="text">
      <strong>Purist mode</strong>
      <small>Play the game as the PS2 plays it: no gameplay mod is loaded, whatever is on below.</small>
    </span>
    <input
      type="checkbox"
      role="switch"
      class="switch"
      checked={purist}
      onchange={(e) => void setPurist(e.currentTarget.checked)}
    />
  </label>

  {#if said}<p class="said" class:failed={said.failed} role="status">{said.text}</p>{/if}

  {#if listError}
    <p class="failed">{listError}</p>
  {:else if mods.length === 0}
    <p class="dim empty">
      No gameplay mods yet. They change how the game plays, as a lock-strafe does. Ratchet &amp; Clank's are at
      <button class="link" onclick={() => void guard(api.openUrl(MODS_URL))}>OpenRAC/rac1-mods</button>: add the .zip
      for your computer here.
    </p>
  {:else}
    <ul>
      {#each mods as mod, i (mod.id)}
        {@const state = pluginState(mod, purist)}
        {@const shared = sharedWith(mod)}
        <li class="mod" class:off={!mod.enabled || purist}>
          <div class="line">
            <input
              type="checkbox"
              role="switch"
              class="switch"
              aria-label={`Use ${mod.name}`}
              checked={mod.enabled}
              disabled={!mod.enabled && mod.blocked !== ""}
              title={!mod.enabled && mod.blocked ? mod.blocked : undefined}
              onchange={(e) => void change(api.pluginsSetEnabled(v.game, v.name, mod.id, e.currentTarget.checked))}
            />
            <div class="grow">
              <div class="name">
                {mod.name}<span class="by"
                  >{mod.version}{#if mod.author}&nbsp;· {mod.author}{/if}</span
                >
              </div>
              {#if mod.description}<div class="dim small-text">{mod.description}</div>{/if}
            </div>
            <span class={`state ${state.tone}`}><span class="dot"></span>{state.text}</span>
            <div class="order">
              <button
                class="icon"
                aria-label={`Move ${mod.name} up`}
                title="Move up: it loads earlier"
                disabled={i === 0}
                onclick={() => void change(api.pluginsMove(v.game, v.name, mod.id, true))}
                ><Icon name="up" size={14} /></button
              >
              <button
                class="icon"
                aria-label={`Move ${mod.name} down`}
                title="Move down: it loads later, and goes first where it changes what a mod above it does"
                disabled={i === mods.length - 1}
                onclick={() => void change(api.pluginsMove(v.game, v.name, mod.id, false))}
                ><Icon name="down" size={14} /></button
              >
              <Menu
                label={`More for ${mod.name}`}
                items={[
                  ...(mod.homepage
                    ? [
                        {
                          label: "Its page",
                          icon: "external" as const,
                          run: () => void guard(api.openUrl(mod.homepage)),
                        },
                      ]
                    : []),
                  {
                    label: "Remove",
                    icon: "trash" as const,
                    danger: true,
                    run: () => {
                      remove(mod);
                    },
                  },
                ]}
              />
            </div>
          </div>

          {#if mod.enabled && (mod.state === "refused" || mod.state === "failed")}
            <p class="why failed">{mod.reason}</p>
          {:else if !mod.enabled && mod.blocked}
            <p class="why dim">Cannot be switched on: {mod.blocked}</p>
          {/if}
          {#if shared && mod.enabled}<p class="why warn">{shared}</p>{/if}

          {#if mod.settings.length && mod.enabled}
            <div class="settings">
              {#each mod.settings as s (s.key)}
                {@const control = settingControl(s)}
                {#if control === "switch"}
                  <label class="setting">
                    <span class="text"
                      ><strong>{s.name}</strong>{#if s.description}<small>{s.description}</small>{/if}</span
                    >
                    <input
                      type="checkbox"
                      role="switch"
                      class="switch"
                      checked={s.value === true}
                      onchange={(e) => {
                        setValue(mod, s, e.currentTarget.checked);
                      }}
                    />
                  </label>
                {:else if control === "segmented"}
                  <div class="setting">
                    <span class="text"
                      ><strong>{s.name}</strong>{#if s.description}<small>{s.description}</small>{/if}</span
                    >
                    <div class="segmented" role="radiogroup" aria-label={s.name}>
                      {#each s.choices ?? [] as c (c.value)}
                        <button
                          role="radio"
                          aria-checked={s.value === c.value}
                          onclick={() => {
                            setValue(mod, s, c.value);
                          }}>{c.name}</button
                        >
                      {/each}
                    </div>
                  </div>
                {:else if control === "radios"}
                  <div class="setting">
                    <span class="text"
                      ><strong>{s.name}</strong>{#if s.description}<small>{s.description}</small>{/if}</span
                    >
                    <div class="radios" role="radiogroup" aria-label={s.name}>
                      {#each s.choices ?? [] as c (c.value)}
                        <label class="radio"
                          ><input
                            type="radio"
                            name={`${mod.id}-${s.key}`}
                            checked={s.value === c.value}
                            onchange={() => {
                              setValue(mod, s, c.value);
                            }}
                          />{c.name}</label
                        >
                      {/each}
                    </div>
                  </div>
                {:else if control === "number"}
                  {@const line = [s.description, settingRange(s) && `(${settingRange(s)})`].filter(Boolean).join(" ")}
                  <label class="setting">
                    <span class="text"
                      ><strong>{s.name}</strong>{#if line}<small>{line}</small>{/if}</span
                    >
                    <input
                      class="number"
                      type="number"
                      min={s.min}
                      max={s.max}
                      step={s.step ?? (s.type === "int" ? 1 : "any")}
                      value={s.value}
                      onchange={(e) => {
                        setNumber(mod, s, e.currentTarget);
                      }}
                    />
                  </label>
                {:else}
                  <label class="setting">
                    <span class="text"
                      ><strong>{s.name}</strong>{#if s.description}<small>{s.description}</small>{/if}</span
                    >
                    <input
                      class="text-field"
                      type="text"
                      maxlength={s.maxLength}
                      value={typeof s.value === "string" ? s.value : ""}
                      onchange={(e) => {
                        setValue(mod, s, e.currentTarget.value);
                      }}
                    />
                  </label>
                {/if}
              {/each}
            </div>
          {/if}

          {#if developer}
            <div class="dev dim">
              <code>{mod.id}</code> · <span class="mono">{mod.path}</span>
              {#if mod.hooks.length}<br />Hooks: <span class="mono">{mod.hooks.join(", ")}</span>{/if}
              {#if mod.license}<br />License: {mod.license}{/if}
            </div>
          {/if}
        </li>
      {/each}
    </ul>
    <p class="dim small-text note">
      Mods load from the top down; where two change the same thing, the lower one goes first. Changes count from the
      game's next start. A mod is a program: add only those you trust.
    </p>
  {/if}
</section>

<style>
  .plugins {
    display: flex;
    flex-direction: column;
    gap: 8px;
  }

  h2 {
    font-size: 1rem;
  }

  .purist {
    border-bottom: 1px solid var(--line);
  }

  ul {
    margin: 0;
    padding: 0;
    list-style: none;
  }

  .mod {
    padding: 11px 0;
    border-top: 1px solid var(--line);
  }

  .mod:first-child {
    border-top: 0;
  }

  .mod:last-child {
    border-bottom: 1px solid var(--line);
  }

  .line {
    display: flex;
    align-items: center;
    gap: 12px;
  }

  .line input.switch:focus-visible,
  .settings input:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .name {
    font-size: 0.875rem;
    font-weight: 600;
  }

  .by {
    margin-left: 0.5em;
    font-weight: 400;
    color: var(--dim);
  }

  .mod.off .name {
    color: var(--dim);
  }

  .small-text {
    font-size: 0.8125rem;
  }

  /* A state: a dot and a few words (STYLE.md). */
  .state {
    flex: none;
    display: inline-flex;
    align-items: center;
    gap: 7px;
    font-size: 0.8125rem;
    color: var(--soft);
    white-space: nowrap;
  }

  .state .dot {
    width: 7px;
    height: 7px;
    border-radius: 50%;
    background: currentColor;
  }

  .state.ok {
    color: var(--ok);
  }

  .state.err {
    color: var(--err);
  }

  .state.warn {
    color: var(--warn);
  }

  .state.off {
    color: var(--dim);
  }

  .order {
    flex: none;
    display: flex;
    align-items: center;
    gap: 6px;
  }

  .icon {
    padding: 4px 9px;
    font-size: 0.75rem;
  }

  .why {
    margin: 6px 0 0 54px;
    font-size: 0.8125rem;
  }

  .warn {
    color: var(--amber);
  }

  .settings {
    margin: 6px 0 0 54px;
  }

  .radios {
    flex: none;
    display: flex;
    flex-direction: column;
    gap: 4px;
  }

  .radio {
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 0.8125rem;
    cursor: pointer;
  }

  .radio input {
    accent-color: var(--amber);
    margin: 0;
  }

  input.number {
    flex: none;
    width: 96px;
  }

  input.text-field {
    flex: none;
    width: min(240px, 40%);
  }

  .dev {
    margin: 6px 0 0 54px;
    font-size: 0.75rem;
    overflow-wrap: anywhere;
  }

  .note {
    padding-top: 2px;
  }

  .empty {
    font-size: 0.875rem;
    padding: 4px 0;
  }

  .said {
    font-size: 0.8125rem;
    color: var(--ok);
  }

  .failed {
    color: var(--err);
  }

  @media (max-width: 860px) {
    .why,
    .settings,
    .dev {
      margin-left: 0;
    }
  }
</style>
