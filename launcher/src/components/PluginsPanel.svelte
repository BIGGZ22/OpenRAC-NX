<script lang="ts">
  // Gameplay mods: the game's native mods (docs/port/MODS.md, core/src/plugins.rs), the first thing
  // on Mods for a game with a port. Each in load order with a switch, its state in a few words, its
  // settings from its manifest, up and down, and Remove in its menu; Add a mod takes a .zip or a
  // folder (a texture and model pack goes to the packs below: lib/addMods.ts), and Purist mode,
  // the launcher's one switch, starts the game with none. Every change is written at once to the
  // list the game reads at its next start (plugins.json). Add from a link reads a repository's
  // address (core/src/remote.rs, docs/port/MODS.md section 10): its mods are shown, with where they
  // come from and whether this computer's game can take them, before one is installed; each mod
  // installed so keeps its source, offers its updates (checked once a day here) and can be kept at
  // its version. With developer tools on: ids, folders, sources and the hook points each mod takes.
  import {
    api,
    inTauri,
    pickFolder,
    pickZip,
    type ModSources,
    type ModUpdate,
    type PluginSetting,
    type PluginView,
    type RemoteMod,
    type RemotePreview,
    type VersionView,
  } from "$lib/api";
  import { addMods } from "$lib/addMods";
  import { app, guard, saveConfig } from "$lib/app.svelte";
  import { modKind, pluginState, settingControl, settingNumber, settingRange, sharedWith } from "$lib/plugins";
  import * as T from "$lib/modsText";
  import Icon from "./Icon.svelte";
  import Menu from "./Menu.svelte";

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

  /** Where each installed mod came from (sources.json), and the updates the last check found. */
  let sources = $state<ModSources["mods"]>({});
  let updates = $state<Record<string, ModUpdate>>({});
  let checking = $state(false);
  let updating = $state<string | null>(null);

  /** Add from a link: the address typed, what it holds, the mod being installed. */
  let link = $state("");
  let reading = $state(false);
  let preview = $state<RemotePreview | null>(null);
  let previewError = $state<string | null>(null);
  let installing = $state<string | null>(null);

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

  async function readLink(e: SubmitEvent) {
    e.preventDefault();
    if (!link.trim()) return;
    reading = true;
    preview = null;
    previewError = null;
    try {
      preview = await api.pluginsPreview(v.game, v.name, link.trim());
    } catch (err) {
      previewError = message(err);
    }
    reading = false;
  }

  async function installRemote(m: RemoteMod) {
    if (!preview) return;
    installing = m.id;
    try {
      const done = await api.pluginsInstallRemote(v.game, v.name, link.trim(), m.id);
      if (done.kind === "pack") {
        // A data pack's files, downloaded: the packs below take them (overrides-add).
        const added = await addMods(v, [done.path]);
        said = { text: added.text, failed: added.failed };
        if (added.pack) onpack();
      } else {
        said = {
          text: done.replaced
            ? T.installedFrom.replaced(done.name, done.version, preview.label)
            : done.enabled
              ? T.installedFrom.on(done.name, preview.label)
              : T.installedFrom.off(done.name, preview.label),
          failed: false,
        };
      }
      preview = null;
      link = "";
    } catch (err) {
      said = { text: T.withName(m.name, message(err)), failed: true };
    }
    installing = null;
    await load();
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

  async function add(paths: string[]) {
    adding = true;
    const added = await addMods(v, paths);
    said = { text: added.text, failed: added.failed };
    if (added.pack) onpack();
    adding = false;
    await load();
  }

  async function addZip() {
    const zip = await pickZip(T.PANEL.pickZip);
    if (zip) await add([zip]);
  }

  async function addFolder() {
    const folder = inTauri ? await pickFolder(T.PANEL.pickFolder) : "/home/you/Downloads/lock-strafe";
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
    if (!confirm(T.removeConfirm(mod.name))) return;
    void change(api.pluginsRemove(v.game, v.name, mod.id));
  }

  const setValue = (mod: PluginView, s: PluginSetting, value: unknown) =>
    void change(api.pluginsSetSetting(v.game, v.name, mod.id, s.key, value));

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
    <h2 class="grow">{T.PANEL.title}</h2>
    <button class="small" disabled={adding} onclick={() => void addZip()}
      >{#if adding}<span class="spinner"></span>{:else}<Icon name="library" size={14} />{/if}{T.PANEL.addZip}</button
    >
    <button class="small" disabled={adding} onclick={() => void addFolder()}
      ><Icon name="folder" size={14} />{T.PANEL.addFolder}</button
    >
    {#if hasSources}
      <button class="small" disabled={checking} onclick={() => void checkUpdates()}
        >{#if checking}<span class="spinner"></span>{/if}{T.PANEL.checkUpdates}</button
      >
    {/if}
  </div>

  <form class="link-row" onsubmit={(e) => void readLink(e)}>
    <label class="grow">
      <span class="sr-only">{T.PANEL.linkLabel}</span>
      <input
        class="link-field"
        type="text"
        inputmode="url"
        spellcheck="false"
        autocomplete="off"
        placeholder={T.PANEL.linkPlaceholder}
        bind:value={link}
      />
    </label>
    <button class="small" type="submit" disabled={reading || !link.trim()}
      >{#if reading}<span class="spinner"></span>{/if}{T.PANEL.look}</button
    >
  </form>

  {#if previewError}<p class="said failed" role="status">{previewError}</p>{/if}

  {#if preview}
    {@const from = T.fromPublisher(preview.label, preview.owner)}
    <div class="preview" aria-label={T.modsAria(preview.label)}>
      <p class="from small-text">
        {from.before}<strong>{from.label}</strong>{from.after}{#if developer}
          <span class="mono dim"
            >· {preview.reference}{#if preview.commit}&nbsp;({preview.commit.slice(0, 10)}){/if}</span
          >{/if}
      </p>
      {#if preview.note}<p class="dim small-text">{preview.note}</p>{/if}
      {#if preview.mods.some((m) => m.kind === "native")}
        <p class="warn small-text">{T.PREVIEW.nativeWarning}</p>
      {/if}
      {#if preview.mods.some((m) => m.kind === "script")}
        <p class="dim small-text">{T.PREVIEW.scriptNote}</p>
      {/if}
      <ul>
        {#each preview.mods as m (m.id)}
          {@const fits = m.compatible ? T.COMPATIBLE_LABEL.yes : T.COMPATIBLE_LABEL.no}
          <li class="mod remote">
            <div class="line">
              <div class="grow">
                <div class="name">
                  {m.name}<span class="by"
                    >{m.version}{#if m.author}&nbsp;· {m.author}{/if}</span
                  >
                </div>
                {#if m.description}<div class="dim small-text">{m.description}</div>{/if}
                <div class={`kind small-text ${modKind(m.kind).tone}`}>
                  <span class="dot"></span>{modKind(m.kind).text}{#if m.installed}<span class="dim"
                      >&nbsp;· {T.installedAlready(m.installed)}</span
                    >{/if}
                </div>
              </div>
              <span class={`state ${fits.tone}`}><span class="dot"></span>{fits.text}</span>
              <button
                class="small"
                disabled={!m.compatible || installing !== null}
                onclick={() => void installRemote(m)}
                >{#if installing === m.id}<span class="spinner"></span>{/if}{m.installed === m.version
                  ? T.PREVIEW.reinstall
                  : m.installed
                    ? T.updateTo(m.version)
                    : T.PREVIEW.install}</button
              >
            </div>
            {#if !m.compatible && m.why}<p class="why failed">{m.why}</p>{/if}
            {#if developer}
              <div class="dev dim">
                <code>{m.id}</code>{#if m.path}&nbsp;· <span class="mono">{m.path}/</span>{/if}{#if m.asset}&nbsp;·
                  <span class="mono">{m.asset}</span> ({m.tag}){/if}{#if m.api}&nbsp;· {T.PANEL.interface} {m.api}{/if}
                · {Object.entries(m.games)
                  .map(([g, d]) => `${g}: ${d.join(", ")}`)
                  .join("; ")}
              </div>
            {/if}
          </li>
        {/each}
      </ul>
      <p><button class="link" onclick={() => (preview = null)}>{T.PANEL.close}</button></p>
    </div>
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

  {#if said}<p class="said" class:failed={said.failed} role="status">{said.text}</p>{/if}

  {#if listError}
    <p class="failed">{listError}</p>
  {:else if mods.length === 0}
    <p class="dim empty">
      {T.PANEL.emptyBefore}
      <button class="link" onclick={() => void guard(api.openUrl(T.PANEL.modsUrl))}>{T.PANEL.modsUrlLabel}</button>{T
        .PANEL.emptyAfter}
    </p>
  {:else}
    <ul>
      {#each mods as mod, i (mod.id)}
        {@const state = pluginState(mod, purist)}
        {@const shared = sharedWith(mod)}
        {@const source = sources[mod.id]}
        {@const update = updates[mod.id]}
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
              {#if mod.description}<div class="dim small-text">{mod.description}</div>{/if}
              <div class={`kind small-text ${modKind(mod.kind).tone}`}>
                <span class="dot"></span>{modKind(mod.kind).text}
              </div>
            </div>
            <span class={`state ${state.tone}`}><span class="dot"></span>{state.text}</span>
            <div class="order">
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
                    run: () => {
                      remove(mod);
                    },
                  },
                ]}
              />
            </div>
          </div>

          {#if source}
            <p class="why dim">
              {T.fromSource(sourceLabel(source.url))}{#if source.pin}&nbsp;· {T.keptAt(source.version)}{/if}
              {#if update}
                <button class="small update" disabled={updating !== null} onclick={() => void runUpdate(mod.id)}
                  >{#if updating === mod.id}<span class="spinner"></span>{/if}{T.updateTo(update.available)}</button
                >
              {/if}
            </p>
          {/if}
          {#if mod.enabled && (mod.state === "refused" || mod.state === "failed")}
            <p class="why failed">{mod.reason}</p>
          {:else if !mod.enabled && mod.blocked}
            <p class="why dim">{T.cannotSwitchOn(mod.blocked)}</p>
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
    <p class="dim small-text note">{T.PANEL.note}</p>
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

  .link-row {
    display: flex;
    align-items: center;
    gap: 10px;
  }

  .link-field {
    width: 100%;
  }

  .link-field:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .sr-only {
    position: absolute;
    width: 1px;
    height: 1px;
    overflow: hidden;
    clip-path: inset(50%);
    white-space: nowrap;
  }

  /* What a link holds: rows under a rule, dashed while it is not installed (STYLE.md). */
  .preview {
    padding: 4px 0 4px 12px;
    border-left: 2px dashed var(--line-strong);
  }

  .preview .from {
    margin: 0 0 4px;
  }

  .update {
    margin-left: 8px;
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

  /* What a mod runs: native code (amber), scripts in the sandbox, or data only. */
  .kind {
    display: inline-flex;
    align-items: center;
    gap: 7px;
    margin-top: 2px;
    color: var(--dim);
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
