<script lang="ts">
  // Gameplay mods: the game's native and script mods (docs/port/MODS.md, core/src/plugins.rs), on
  // My mods for a game with a port. Each in load order with a switch; a state only when it is not
  // what the switch says (purist, refused, not loaded); its settings folded under a Settings
  // button; up and down, and Remove in its menu. Adding is AddMod's. Purist mode, the launcher's
  // one switch, starts the game with none. Every change is written at once to the list the game
  // reads at its next start (plugins.json). A mod installed from a link keeps its source, offers
  // its updates (checked once a day here) and can be kept at its version. With developer tools
  // on: ids, folders, sources and the hook points each mod takes.
  import {
    api,
    ask,
    type ModSources,
    type ModUpdate,
    type PluginPreset,
    type PluginSetting,
    type PluginView,
    type VersionView,
  } from "$lib/api";
  import { addMods } from "$lib/addMods";
  import { app, guard, saveConfig } from "$lib/app.svelte";
  import { modHint, modKind, pluginState, settingControl, settingNumber, settingRange, sharedWith } from "$lib/plugins";
  import * as T from "$lib/modsText";
  import { isPreset, keybindConflicts, presetChanges, sharedBinds } from "$lib/keybinds";
  import Icon from "./Icon.svelte";
  import KeyBindSetting from "./KeyBindSetting.svelte";
  import Menu from "./Menu.svelte";

  let {
    version: v,
    reload = 0,
    onpack = () => undefined,
  }: {
    version: VersionView;
    /** Bumped when a mod was added elsewhere on the page (AddMod): the list is read again. */
    reload?: number;
    /** A texture and model pack came in from here (an update): the packs' list is read again. */
    onpack?: () => void;
  } = $props();

  const developer = $derived(app.config?.developer ?? false);
  const purist = $derived(app.config?.purist ?? false);

  let mods = $state<PluginView[]>([]);
  let listError = $state<string | null>(null);
  /** What the last add or change said, in a sentence. */
  let said = $state<{ text: string; failed: boolean } | null>(null);

  /** Where each installed mod came from (sources.json), and the updates the last check found. */
  let sources = $state<ModSources["mods"]>({});
  let updates = $state<Record<string, ModUpdate>>({});
  let checking = $state(false);
  let updating = $state<string | null>(null);

  /** The mods whose settings are unfolded. */
  let open = $state<Record<string, boolean>>({});

  const message = (e: unknown) => (typeof e === "string" ? e : String(e));

  async function load() {
    try {
      mods = await api.pluginsList(v.game, v.name);
      listError = null;
    } catch (e) {
      listError = message(e);
    }
    try {
      sources = (await api.pluginsSources(v.game)).mods;
    } catch {
      sources = {};
    }
  }

  /** The last update check, per game, in this browser's storage: once a day by itself. */
  const checkedKey = () => `openrac.mods.checked.${v.game}`;
  const DAY = 24 * 60 * 60 * 1000;

  function lastChecked(): number {
    try {
      return Number(localStorage.getItem(checkedKey()) ?? 0) || 0;
    } catch {
      return 0;
    }
  }

  async function checkUpdates(quiet = false) {
    checking = true;
    try {
      const found = await api.pluginsCheckUpdates(v.game, v.name);
      updates = Object.fromEntries(found.updates.map((u) => [u.id, u]));
      try {
        localStorage.setItem(checkedKey(), String(Date.now()));
      } catch {
        // Private windows: checked again next time.
      }
      if (!quiet) {
        const n = found.updates.length;
        said = { text: T.updatesFound(n, found.errors), failed: found.errors.length > 0 && n === 0 };
      }
    } catch (e) {
      if (!quiet) said = { text: message(e), failed: true };
    }
    checking = false;
  }

  async function runUpdate(id: string) {
    updating = id;
    try {
      const done = await api.pluginsUpdate(v.game, v.name, id);
      if (done.kind === "pack") {
        const added = await addMods(v, [done.path]);
        said = { text: added.text, failed: added.failed };
        onpack();
      } else {
        said = { text: T.updated(done.name, done.version), failed: false };
      }
      updates = Object.fromEntries(Object.entries(updates).filter(([k]) => k !== id));
    } catch (e) {
      said = { text: message(e), failed: true };
    }
    updating = null;
    await load();
  }

  async function setPin(id: string, pin: boolean) {
    try {
      sources = (await api.pluginsSetPin(v.game, id, pin)).mods;
      if (pin) {
        updates = Object.fromEntries(Object.entries(updates).filter(([k]) => k !== id));
      }
    } catch (e) {
      said = { text: message(e), failed: true };
    }
  }

  const hasSources = $derived(Object.keys(sources).length > 0);

  $effect(() => {
    // Once a day by itself, when a mod came from a link.
    if (hasSources && !checking && Date.now() - lastChecked() > DAY) void checkUpdates(true);
  });

  /** "OpenRAC/rac1-mods" from a source's address. */
  const sourceLabel = (url: string) => url.replace(/^https?:\/\/(www\.)?github\.com\//, "");

  $effect(() => {
    // Again whenever RELOAD is bumped, and for another game.
    if (reload >= 0 && v.key) void load();
  });

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

  async function remove(mod: PluginView) {
    if (!(await ask(T.removeConfirm(mod.name), T.PANEL.remove))) return;
    void change(api.pluginsRemove(v.game, v.name, mod.id));
  }

  const setValue = (mod: PluginView, s: PluginSetting, value: unknown) =>
    void change(api.pluginsSetSetting(v.game, v.name, mod.id, s.key, value));

  /** A preset of the mod's keybinds (null: their defaults): each binding it changes, one after the other. */
  function applyPreset(mod: PluginView, preset: PluginPreset | null) {
    const changes = Object.entries(presetChanges(mod.settings, preset));
    if (!changes.length) return;
    void change(
      (async () => {
        let views = mods;
        for (const [key, value] of changes) views = await api.pluginsSetSetting(v.game, v.name, mod.id, key, value);
        return views;
      })(),
    );
  }

  /** A number typed: kept when the setting takes it, else put back as it was. */
  function setNumber(mod: PluginView, s: PluginSetting, field: HTMLInputElement) {
    const n = settingNumber(s, field.value);
    if (n === null) {
      field.value = String(s.value);
      said = { text: T.settingTakes(s.name, settingRange(s), s.type === "int"), failed: true };
      return;
    }
    setValue(mod, s, n);
  }

  async function setPurist(on: boolean) {
    if (app.config) await saveConfig({ ...app.config, purist: on });
  }
</script>

<section class="panel plugins" aria-label={T.PANEL.title}>
  <div class="row head">
    <div class="grow">
      <h2>{T.PANEL.title}</h2>
      <p class="dim sub">
        {T.PANEL.sub}{#if mods.length > 1}<span title={T.PANEL.moveDownTitle}>&nbsp;· {T.PANEL.orderHint}</span>{/if}
      </p>
    </div>
    {#if hasSources}
      <button class="small" disabled={checking} onclick={() => void checkUpdates()}
        >{#if checking}<span class="spinner"></span>{:else}<Icon name="refresh" size={14} />{/if}{T.PANEL
          .checkUpdates}</button
      >
    {/if}
  </div>

  {#if purist && mods.length}
    <div class="notice" role="status">
      <Icon name="info" size={16} /><span class="grow">{T.PURIST.onNotice}</span>
      <button class="small" onclick={() => void setPurist(false)}>{T.PURIST.turnOff}</button>
    </div>
  {/if}

  {#if said}<p class="said" class:failed={said.failed} role="status">{said.text}</p>{/if}

  {#if listError}
    <p class="failed">{listError}</p>
  {:else if mods.length === 0}
    <p class="dim empty">{T.PANEL.empty}</p>
  {:else}
    <ul>
      {#each mods as mod, i (mod.id)}
        {@const state = pluginState(mod, purist)}
        {@const kind = modKind(mod.kind)}
        {@const shared = sharedWith(mod)}
        {@const source = sources[mod.id]}
        {@const update = updates[mod.id]}
        {@const unfolded = !!open[mod.id] && mod.enabled}
        <li class="mod" class:off={!mod.enabled || purist}>
          <div class="line">
            <input
              type="checkbox"
              role="switch"
              class="switch"
              aria-label={T.useAria(mod.name)}
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
              {#if mod.description}<div class="dim small-text desc">{mod.description}</div>{/if}
              <div class="tags small-text">
                <span class={`kind ${kind.tone}`} title={modHint(mod.kind)}><span class="dot"></span>{kind.text}</span
                >{#if source}<span class="dim"
                    >&nbsp;· {T.fromSource(sourceLabel(source.url))}{#if source.pin}, {T.keptAt(
                        source.version,
                      )}{/if}</span
                  >{/if}
                {#if update}
                  <button class="small update" disabled={updating !== null} onclick={() => void runUpdate(mod.id)}
                    >{#if updating === mod.id}<span class="spinner"></span>{:else}<Icon
                        name="refresh"
                        size={14}
                      />{/if}{T.updateTo(update.available)}</button
                  >
                {/if}
              </div>
            </div>
            {#if mod.enabled && !purist && state !== T.STATE_LABEL.on}
              <span class={`state ${state.tone}`}><span class="dot"></span>{state.text}</span>
            {/if}
            <div class="order">
              {#if mod.settings.length && mod.enabled}
                <button
                  class="small ghost"
                  aria-expanded={unfolded}
                  aria-controls={`settings-${mod.id}`}
                  onclick={() => (open[mod.id] = !open[mod.id])}
                  ><Icon name="gear" size={14} />{T.PANEL.settings}</button
                >
              {/if}
              <button
                class="icon"
                aria-label={T.moveUpAria(mod.name)}
                title={T.PANEL.moveUpTitle}
                disabled={i === 0}
                onclick={() => void change(api.pluginsMove(v.game, v.name, mod.id, true))}
                ><Icon name="up" size={14} /></button
              >
              <button
                class="icon"
                aria-label={T.moveDownAria(mod.name)}
                title={T.PANEL.moveDownTitle}
                disabled={i === mods.length - 1}
                onclick={() => void change(api.pluginsMove(v.game, v.name, mod.id, false))}
                ><Icon name="down" size={14} /></button
              >
              <Menu
                label={T.moreAria(mod.name)}
                items={[
                  ...(source
                    ? [
                        {
                          label: source.pin ? T.PANEL.letUpdate : T.PANEL.keepVersion,
                          icon: "save" as const,
                          run: () => void setPin(mod.id, !source.pin),
                        },
                      ]
                    : []),
                  ...(mod.homepage
                    ? [
                        {
                          label: T.PANEL.homepage,
                          icon: "external" as const,
                          run: () => void guard(api.openUrl(mod.homepage)),
                        },
                      ]
                    : []),
                  {
                    label: T.PANEL.remove,
                    icon: "trash" as const,
                    danger: true,
                    run: () => void remove(mod),
                  },
                ]}
              />
            </div>
          </div>

          {#if mod.enabled && (mod.state === "refused" || mod.state === "failed")}
            <p class="why failed">{mod.reason}</p>
          {:else if !mod.enabled && mod.blocked}
            <p class="why dim">{T.cannotSwitchOn(mod.blocked)}</p>
          {/if}
          {#if shared && mod.enabled}<p class="why warn">{shared}</p>{/if}

          {#if unfolded}
            {@const clashes = keybindConflicts(mod.settings)}
            <div class="settings" id={`settings-${mod.id}`}>
              {#if mod.presets?.length}
                <div class="setting">
                  <span class="text"><strong>{T.KEYBIND.presets}</strong><small>{T.KEYBIND.presetsHint}</small></span>
                  <div class="segmented" role="radiogroup" aria-label={T.KEYBIND.presets}>
                    {#each mod.presets.some((p) => p.id === "default") ? mod.presets : [null, ...mod.presets] as p (p?.id ?? "")}
                      <button role="radio" aria-checked={isPreset(mod.settings, p)} onclick={() => applyPreset(mod, p)}
                        >{p?.name ?? T.KEYBIND.defaults}</button
                      >
                    {/each}
                  </div>
                </div>
              {/if}
              {#each mod.settings as s (s.key)}
                {@const control = settingControl(s)}
                {#if control === "keybind"}
                  <KeyBindSetting
                    setting={s}
                    conflicts={clashes[s.key] ?? []}
                    clashing={clashes[s.key] ? sharedBinds(mod.settings, s.key) : []}
                    onchange={(value) => setValue(mod, s, value)}
                  />
                {:else if control === "switch"}
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
              {#if mod.hooks.length}<br />{T.PANEL.hooks} <span class="mono">{mod.hooks.join(", ")}</span>{/if}
              {#if mod.license}<br />{T.PANEL.license} {mod.license}{/if}
              {#if source}<br />{T.PANEL.source} <span class="mono">{source.url}</span>{#if source.path}&nbsp;(<span
                    class="mono">{source.path}/</span
                  >){/if}{#if source.tag}, {source.tag}{/if}{/if}
            </div>
          {/if}
        </li>
      {/each}
    </ul>
  {/if}

  <label class="setting purist">
    <span class="text">
      <strong>{T.PURIST.title}</strong>
      <small>{T.PURIST.panelLine}</small>
    </span>
    <input
      type="checkbox"
      role="switch"
      class="switch"
      checked={purist}
      onchange={(e) => void setPurist(e.currentTarget.checked)}
    />
  </label>
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

  .head {
    align-items: flex-start;
  }

  .sub {
    margin: 2px 0 0;
    font-size: 0.8125rem;
  }

  /* Purist mode: the panel's last row, under the list's rule. */
  .purist {
    padding-bottom: 0;
    border-top: 1px solid var(--line);
  }

  /* Purist mode on: says so above the list, with the way back. */
  .notice {
    display: flex;
    align-items: center;
    gap: 10px;
    padding: 8px 10px 8px 12px;
    border-radius: var(--radius-sm);
    background: var(--panel-hi);
    font-size: 0.8125rem;
    color: var(--body);
  }

  .notice > :global(svg) {
    flex: none;
    color: var(--amber);
  }

  .update {
    margin-left: 10px;
    padding: 2px 10px;
  }

  button[aria-expanded="true"] {
    color: var(--amber);
    background: var(--panel-hi);
    border-color: var(--line);
  }

  .desc {
    margin-top: 1px;
  }

  .tags {
    margin-top: 3px;
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

  /* What a mod runs: native code (amber), scripts in the sandbox, or data only. */
  .kind {
    display: inline-flex;
    align-items: center;
    gap: 7px;
    color: var(--dim);
    cursor: help;
  }

  .kind .dot {
    width: 7px;
    height: 7px;
    border-radius: 50%;
    background: currentColor;
  }

  .kind.warn {
    color: var(--warn);
  }

  .kind.ok {
    color: var(--soft);
  }

  .kind.ok .dot {
    background: var(--ok);
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
