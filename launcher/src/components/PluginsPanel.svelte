<script lang="ts">
  // Gameplay mods: the game's native and script mods (docs/port/MODS.md, core/src/plugins.rs), on
  // My mods for a game with a port. The list is an overview: a row per mod in load order with its
  // switch, its name and a line, one badge (it cannot load, an update, what it runs) and the way
  // into its own page, where its settings are (ModPage.svelte, opened through lib/modNav so Back
  // and Escape return here). Up, down and Remove are in its menu. Adding is AddMod's. The Mods switch under Play
  // on the game's page starts the game with them or with none; off, a notice here says so. Every change is written at once to the
  // list the game reads at its next start (plugins.json). A mod installed from a link keeps its
  // source, offers its updates (checked once a day here) and can be kept at its version. No folder
  // shows here: a mod's is in its page's folded About. Above the mods, for a version that has it,
  // the built-in Enhanced graphics entry (core/src/enhanced.rs): a row and a page as a mod's, its
  // values in the game's own settings file, and the lighting bake it needs (the probe-bake action),
  // started when it is first switched on.
  import {
    api,
    ask,
    type EnhancedView,
    type ModSources,
    type ModUpdate,
    type PluginView,
    type VersionView,
  } from "$lib/api";
  import { addMods } from "$lib/addMods";
  import { app, guardShown, jobs, run, saveConfig } from "$lib/app.svelte";
  import { modBadge, hasSettings } from "$lib/plugins";
  import * as T from "$lib/modsText";
  import { closeMod, modNav, openMod } from "$lib/modNav.svelte";
  import { tick } from "svelte";
  import Icon from "./Icon.svelte";
  import Menu from "./Menu.svelte";
  import ModPage from "./ModPage.svelte";
  import Reason from "./Reason.svelte";
  import { hidePaths } from "$lib/paths";

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
  /** The Mods switch under Play is off (the default): Play starts the game without these. */
  const purist = $derived(!(app.config?.playWithMods ?? false));

  let mods = $state<PluginView[]>([]);
  let listError = $state<string | null>(null);
  /** What the last add or change said, in a sentence. */
  let said = $state<{ text: string; failed: boolean } | null>(null);

  /** Where each installed mod came from (sources.json), and the updates the last check found. */
  let sources = $state<ModSources["mods"]>({});
  let updates = $state<Record<string, ModUpdate>>({});
  let checking = $state(false);
  let updating = $state<string | null>(null);

  /** The built-in Enhanced graphics entry, for a version that has it. */
  let enhanced = $state<EnhancedView | null>(null);
  const isEnhanced = (id: string | null) => id === T.ENHANCED.id;

  /** The mod whose page is open, when it is one of this game's. */
  const page = $derived(
    isEnhanced(modNav.mod) && enhanced ? enhanced.mod : (mods.find((m) => m.id === modNav.mod) ?? null),
  );

  /** The lighting bake running or waiting for this version, if one is. */
  const bakeJob = $derived(
    jobs.list.find(
      (j) =>
        j.actionId === "probe-bake" &&
        j.scope.kind === "version" &&
        j.scope.key === v.key &&
        (j.state === "queued" || j.state === "running"),
    ) ?? null,
  );
  const bakeAction = $derived(v.actions.find((a) => a.id === "probe-bake" && a.thisPlatform) ?? null);

  async function loadEnhanced() {
    try {
      enhanced = await api.enhancedView(v.game, v.name);
    } catch {
      enhanced = null;
    }
  }

  function bake() {
    if (bakeAction && !bakeJob) void run({ kind: "version", key: v.key }, bakeAction);
  }

  /** Every bake of this version gone, after the player says yes. */
  async function deleteBakes() {
    const b = enhanced?.bake;
    if (!b || bakeJob) return;
    if (!(await ask(T.ENHANCED.removeAsk(b.ready + b.partial + b.failed), T.ENHANCED.removeYes))) return;
    try {
      enhanced = await api.enhancedDeleteBakes(v.game, v.name);
      said = { text: T.ENHANCED.removed, failed: false };
    } catch (e) {
      said = { text: message(e), failed: true };
    }
  }

  async function setEnhanced(on: boolean) {
    try {
      enhanced = await api.enhancedSetEnabled(v.game, v.name, on);
      said = null;
    } catch (e) {
      said = { text: message(e), failed: true };
      return;
    }
    // Switched on: the lighting is baked for the levels that have none yet.
    const b = enhanced?.bake;
    if (on && b && b.available && b.ready < b.levels) bake();
  }

  // The bake ended: its status again.
  let wasBaking = false;
  $effect(() => {
    const now = bakeJob !== null;
    if (wasBaking && !now) void loadEnhanced();
    wasBaking = now;
  });
  let loaded = $state(false);

  $effect(() => {
    // A mod asked for that is not here (removed, another game's): the list.
    if (loaded && modNav.mod !== null && !page && !isEnhanced(modNav.mod)) closeMod();
  });

  /** The mod whose page was last open: its row takes the focus back. */
  let returning: string | null = null;
  $effect(() => {
    if (page) returning = page.id;
    else if (returning) {
      const id = returning;
      returning = null;
      void tick().then(() => document.getElementById(`open-${id}`)?.focus());
    }
  });

  const message = (e: unknown) => (typeof e === "string" ? e : String(e));

  async function load() {
    void loadEnhanced();
    try {
      mods = await api.pluginsList(v.game, v.name);
      listError = null;
      loaded = true;
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

  /** Every update the last check found, one after another (a pinned mod has none). */
  async function updateAll() {
    const ids = Object.keys(updates).filter((id) => !sources[id]?.pin);
    let failed = 0;
    for (const id of ids) {
      await runUpdate(id);
      if (said?.failed) failed++;
    }
    said = { text: T.updatedAll(ids.length - failed, failed), failed: failed > 0 };
  }

  const pending = $derived(Object.keys(updates).filter((id) => !sources[id]?.pin).length);

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
    if (modNav.mod === mod.id) closeMod();
    void change(api.pluginsRemove(v.game, v.name, mod.id));
  }

  /** Saves one setting from the mod's page; the list as the core now has it. */
  async function saveSetting(mod: PluginView, key: string, value: unknown) {
    if (isEnhanced(mod.id)) {
      enhanced = await api.enhancedSetSetting(v.game, v.name, key, value);
      return;
    }
    mods = await api.pluginsSetSetting(v.game, v.name, mod.id, key, value);
  }

  function openFolder(mod: PluginView) {
    void guardShown(api.openOutput(mod.path));
  }

  async function playWithMods() {
    if (app.config) await saveConfig({ ...app.config, playWithMods: true });
  }
</script>

{#if page}
  {@const mod = page}
  <ModPage
    {mod}
    source={sources[mod.id] ?? null}
    update={updates[mod.id] ?? null}
    {purist}
    {developer}
    updating={updating === mod.id}
    onsave={(key, value) => saveSetting(mod, key, value)}
    onenable={(on) =>
      void (isEnhanced(mod.id) ? setEnhanced(on) : change(api.pluginsSetEnabled(v.game, v.name, mod.id, on)))}
    builtIn={isEnhanced(mod.id) && enhanced
      ? { bake: enhanced.bake, job: bakeJob, onbake: bake, ondelete: deleteBakes }
      : null}
    onupdate={() => void runUpdate(mod.id)}
    onpin={(pin) => void setPin(mod.id, pin)}
    onfolder={() => {
      openFolder(mod);
    }}
    onhomepage={() => void guardShown(api.openUrl(mod.homepage))}
    onremove={() => void remove(mod)}
  />
  {#if said}<Reason text={said.text} tone={said.failed ? "err" : "ok"} role="status" />{/if}
{:else}
  <section class="panel plugins" aria-label={T.PANEL.title}>
    <div class="head">
      <h2>{T.PANEL.title}</h2>
      {#if hasSources}
        {#if pending > 1}
          <button class="small" disabled={checking || updating !== null} onclick={() => void updateAll()}
            ><Icon name="refresh" size={14} />{T.PANEL.updateAll(pending)}</button
          >
        {/if}
        <button class="small ghost" disabled={checking} onclick={() => void checkUpdates()}
          >{#if checking}<span class="spinner"></span>{:else}<Icon name="refresh" size={14} />{/if}{T.PANEL
            .checkUpdates}</button
        >
      {/if}
    </div>

    {#if purist && mods.length}
      <div class="notice" role="status">
        <Icon name="info" size={16} /><span class="grow">{T.WITH_MODS.offNotice}</span>
        <button class="small" onclick={() => void playWithMods()}>{T.WITH_MODS.turnOn}</button>
      </div>
    {/if}

    {#if said}<Reason text={said.text} tone={said.failed ? "err" : "ok"} role="status" />{/if}

    {#if enhanced}
      {@const entry = enhanced.mod}
      {@const badge = modBadge(entry, null, false)}
      <ul class="built-in">
        <li class="mod" class:off={!entry.enabled}>
          <input
            type="checkbox"
            role="switch"
            class="switch"
            aria-label={T.useAria(entry.name)}
            checked={entry.enabled}
            onchange={(e) => void setEnhanced(e.currentTarget.checked)}
          />
          <button
            class="open"
            id={`open-${entry.id}`}
            aria-label={T.settingsAria(entry.name)}
            onclick={() => {
              openMod(entry.id);
            }}
          >
            <span class="words">
              <span class="name">{entry.name}</span>
              <span class="desc">{entry.description}</span>
            </span>
            <span class={`badge ${badge.tone}`} title={badge.hint}><span class="dot"></span>{badge.text}</span>
            <span class="go">{T.PANEL.settings}<Icon name="next" size={16} /></span>
          </button>
        </li>
      </ul>
    {/if}

    {#if listError}
      <Reason text={listError} tone="err" />
    {:else if mods.length === 0}
      <p class="empty">{T.PANEL.empty}</p>
    {:else}
      <ul>
        {#each mods as mod, i (mod.id)}
          {@const update = sources[mod.id]?.pin ? null : (updates[mod.id]?.available ?? null)}
          {@const badge = modBadge(mod, update, purist)}
          <li class="mod" class:off={!mod.enabled}>
            <input
              type="checkbox"
              role="switch"
              class="switch"
              aria-label={T.useAria(mod.name)}
              checked={mod.enabled}
              disabled={!mod.enabled && mod.blocked !== ""}
              title={!mod.enabled && mod.blocked ? hidePaths(mod.blocked).text : undefined}
              onchange={(e) => void change(api.pluginsSetEnabled(v.game, v.name, mod.id, e.currentTarget.checked))}
            />
            <button
              class="open"
              id={`open-${mod.id}`}
              aria-label={T.settingsAria(mod.name)}
              onclick={() => {
                openMod(mod.id);
              }}
            >
              <span class="words">
                <span class="name">{mod.name}</span>
                {#if mod.description}<span class="desc">{mod.description}</span>{/if}
              </span>
              <span class={`badge ${badge.tone}`} title={hidePaths(badge.hint).text}
                ><span class="dot"></span>{badge.text}</span
              >
              <span class="go"
                >{hasSettings(mod) ? T.PANEL.settings : T.PANEL.details}<Icon name="next" size={16} /></span
              >
            </button>
            {#if update}
              <button
                class="small update"
                disabled={updating !== null}
                aria-label={T.MOD_PAGE.update(update)}
                onclick={() => void runUpdate(mod.id)}
                >{#if updating === mod.id}<span class="spinner"></span>{:else}<Icon name="refresh" size={14} />{/if}{T
                  .PANEL.updateRow}</button
              >
            {/if}
            <Menu
              label={T.moreAria(mod.name)}
              items={[
                ...(update && updating !== mod.id
                  ? [
                      {
                        label: T.MOD_PAGE.update(update),
                        icon: "refresh" as const,
                        run: () => void runUpdate(mod.id),
                      },
                    ]
                  : []),
                ...(i > 0
                  ? [
                      {
                        label: T.PANEL.moveUp,
                        icon: "up" as const,
                        run: () => void change(api.pluginsMove(v.game, v.name, mod.id, true)),
                      },
                    ]
                  : []),
                ...(i < mods.length - 1
                  ? [
                      {
                        label: T.PANEL.moveDown,
                        icon: "down" as const,
                        run: () => void change(api.pluginsMove(v.game, v.name, mod.id, false)),
                      },
                    ]
                  : []),
                {
                  label: T.PANEL.openFolder,
                  icon: "folder" as const,
                  run: () => {
                    openFolder(mod);
                  },
                },
                { label: T.PANEL.remove, icon: "trash" as const, danger: true, run: () => void remove(mod) },
              ]}
            />
          </li>
        {/each}
      </ul>
      {#if mods.length > 1}<p class="order" title={T.PANEL.orderTitle}>{T.PANEL.orderTitle}</p>{/if}
    {/if}
  </section>
{/if}

<style>
  .plugins {
    display: flex;
    flex-direction: column;
    gap: 10px;
  }

  .update {
    flex: none;
  }

  .head {
    display: flex;
    flex-direction: row;
    align-items: center;
    justify-content: space-between;
    gap: 12px;
  }

  h2 {
    font-size: 1rem;
  }

  /* The Mods switch off: says so above the list, with the way to turn it on. */
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

  ul {
    margin: 0 -10px;
    padding: 0;
    list-style: none;
  }

  /* The built-in entry: above the installed mods, a rule between them. */
  ul.built-in {
    border-bottom: 1px solid var(--line);
  }

  /* No menu of its own: its row ends where the others' menus start. */
  ul.built-in .mod {
    padding-right: 44px;
  }

  /* A mod: its switch, then one wide target into its page (name, line, badge, Settings ›), then
     its menu. */
  .mod {
    display: flex;
    flex-direction: row;
    flex-wrap: nowrap;
    align-items: center;
    gap: 6px;
    padding: 4px 4px 4px 10px;
    border-top: 1px solid var(--line);
  }

  .mod:first-child {
    border-top: 0;
  }

  .mod > input.switch:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .open {
    flex: 1 1 0;
    min-width: 0;
    display: flex;
    flex-direction: row;
    flex-wrap: nowrap;
    align-items: center;
    justify-content: flex-start;
    gap: 16px;
    min-height: 64px;
    padding: 10px 12px 10px 12px;
    border: 0;
    border-radius: var(--radius-sm);
    background: transparent;
    color: var(--text);
    text-align: left;
    font-weight: 400;
  }

  @media (hover: hover) {
    .open:hover:not(:disabled) {
      background: var(--panel-hi);
    }

    .open:hover .go {
      color: var(--amber);
    }
  }

  .open:active:not(:disabled) {
    transform: none;
    background: var(--panel-hi);
  }

  .words {
    flex: 1 1 0;
    display: flex;
    flex-direction: column;
    align-items: flex-start;
    min-width: 0;
  }

  .words > span {
    max-width: 100%;
  }

  .name {
    font-size: 0.9375rem;
    font-weight: 600;
  }

  .desc {
    font-size: 0.8125rem;
    color: var(--soft);
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }

  .mod.off .name,
  .mod.off .desc {
    color: var(--dim);
  }

  /* One badge: a dot and a few words (STYLE.md). */
  .badge {
    flex: 0 0 auto;
    display: inline-flex;
    align-items: center;
    gap: 7px;
    font-size: 0.8125rem;
    color: var(--soft);
    white-space: nowrap;
  }

  .badge .dot {
    width: 7px;
    height: 7px;
    border-radius: 50%;
    background: currentColor;
  }

  .badge.ok .dot {
    background: var(--ok);
  }

  .badge.warn {
    color: var(--warn);
  }

  .badge.err {
    color: var(--err);
  }

  .badge.off {
    color: var(--dim);
  }

  .go {
    flex: 0 0 auto;
    display: inline-flex;
    align-items: center;
    gap: 2px;
    font-size: 0.875rem;
    font-weight: 600;
    color: var(--lav);
    transition: color var(--fast) var(--ease);
  }

  .order {
    margin: 0;
    font-size: 0.75rem;
    color: var(--dim);
  }

  .empty {
    margin: 0;
    padding: 4px 0;
    font-size: 0.875rem;
    color: var(--soft);
  }
</style>
