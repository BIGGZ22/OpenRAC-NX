<script lang="ts">
  // Your mods: a game's overrides packs, the first thing on Mods. Switch one on or off, change the
  // order (a pack lower in the list wins), remove one, add a .zip or a folder, or drop one on the
  // page. The list and its changes are instant (core/src/overrides.rs); adding goes through
  // editor/overrides.py (actions.json). The packs are the game's, shared by its discs.
  import { onMount } from "svelte";
  import {
    api,
    inTauri,
    onDrop,
    pickFolder,
    pickZip,
    type ActionView,
    type PackView,
    type VersionView,
  } from "$lib/api";
  import { app, guard, jobs, runChain } from "$lib/app.svelte";
  import CommandBox from "./CommandBox.svelte";
  import Icon from "./Icon.svelte";

  let {
    version: v,
    reload = 0,
    commands = [],
  }: {
    version: VersionView;
    /** Bumped when something else made a pack (Import your edits): the list is read again. */
    reload?: number;
    /** With developer tools on: the same steps from a shell. */
    commands?: string[];
  } = $props();

  const developer = $derived(app.config?.developer ?? false);
  const scope = $derived({ kind: "version" as const, key: v.key });
  const adder = $derived<ActionView | null>(v.actions.find((a) => a.id === "overrides-add" && a.thisPlatform) ?? null);

  let packs = $state<PackView[]>([]);
  let packsError = $state<string | null>(null);
  let adding = $state(false);
  let dropping = $state(false);
  /** What the last add said, in a sentence. */
  let said = $state<{ text: string; failed: boolean } | null>(null);

  async function load() {
    try {
      packs = await api.overridesList(v.game);
      packsError = null;
    } catch (e) {
      packsError = typeof e === "string" ? e : String(e);
    }
  }

  $effect(() => {
    // Again whenever RELOAD is bumped.
    if (reload >= 0) void load();
  });

  onMount(() => {
    let stop: (() => void) | undefined;
    void onDrop(
      (paths) => {
        dropping = false;
        void addPaths(paths);
      },
      (over) => (dropping = over),
    ).then((off) => (stop = off));
    return () => stop?.();
  });

  /** The last line the add printed. */
  function lastLine(): string {
    const job = jobs.list.filter((j) => j.scope.kind === "version" && j.actionId === "overrides-add").at(-1);
    return job?.lines.map((l) => l.line).findLast((l) => l.trim() !== "") ?? "";
  }

  async function addPaths(paths: string[]) {
    if (!adder) return;
    adding = true;
    for (const path of paths) {
      const ok = await runChain(scope, [adder], { path });
      const name = path.split(/[\\/]/).at(-1) ?? path;
      const why = lastLine().replace(/^overrides: /, "");
      said = ok
        ? { text: `Added ${name}.`, failed: false }
        : { text: `${name}: ${why.length > 0 ? why : "it could not be added"}`, failed: true };
      if (!ok) break;
    }
    adding = false;
    await load();
  }

  async function addZip() {
    const zip = await pickZip("Choose an overrides pack (.zip)");
    if (zip) await addPaths([zip]);
  }

  async function addFolder() {
    const folder = inTauri ? await pickFolder("Choose an overrides pack's folder") : "/home/you/Downloads/HD Veldin";
    if (folder) await addPaths([folder]);
  }

  async function change(work: Promise<PackView[]>) {
    const result = await guard(work);
    if (result) packs = result;
  }

  function remove(pack: PackView) {
    if (
      !confirm(`Remove "${pack.name}"? The launcher's copy is deleted; the .zip or folder you added it from is kept.`)
    )
      return;
    void change(api.overridesRemove(v.game, pack.id));
  }

  function summary(pack: PackView): string {
    const parts = [];
    if (pack.textures) parts.push(`${pack.textures} texture${pack.textures === 1 ? "" : "s"}`);
    if (pack.models) parts.push(`${pack.models} model${pack.models === 1 ? "" : "s"}`);
    if (pack.levelFiles) parts.push(`${pack.levelFiles} level edit${pack.levelFiles === 1 ? "" : "s"}`);
    const where = pack.levels === 1 ? "1 level" : `${pack.levels} levels`;
    return parts.length ? `${parts.join(", ")} in ${where}` : "Replaces nothing the game reads";
  }
</script>

<section class="panel packs" class:dropping aria-label="Your mods">
  <div class="row head">
    <h3 class="grow">Your mods</h3>
    <button class="small" disabled={!adder?.runnable || adding} onclick={() => void addZip()}
      >{#if adding}<span class="spinner"></span>{:else}<Icon name="library" size={14} />{/if}Add a .zip…</button
    >
    <button class="small" disabled={!adder?.runnable || adding} onclick={() => void addFolder()}
      ><Icon name="folder" size={14} />Add a folder…</button
    >
  </div>

  {#if said}<p class="said" class:failed={said.failed}>{said.text}</p>{/if}

  {#if packsError}
    <p class="failed">{packsError}</p>
  {:else if packs.length === 0}
    <p class="dim empty">No packs yet. Make your own below, add a pack someone made, or drop a .zip or folder here.</p>
  {:else}
    <ul>
      {#each packs as pack, i (pack.id)}
        <li class="pack" class:off={!pack.enabled}>
          <input
            type="checkbox"
            aria-label={`Use ${pack.name}`}
            checked={pack.enabled}
            onchange={(e) => void change(api.overridesSetEnabled(v.game, pack.id, e.currentTarget.checked))}
          />
          <div class="grow">
            <div class="name">
              {pack.name}{#if pack.author}<span class="by">by {pack.author}</span>{/if}
            </div>
            <div class="dim small-text">
              {summary(pack)}{#if pack.description}. {pack.description}{/if}
            </div>
          </div>
          {#if developer}<code class="chip" title="The pack's id">{pack.id}</code>{/if}
          <div class="order">
            <button
              class="icon"
              aria-label={`Move ${pack.name} up`}
              title="Earlier: packs below it win"
              disabled={i === 0}
              onclick={() => void change(api.overridesMove(v.game, pack.id, true))}>▲</button
            >
            <button
              class="icon"
              aria-label={`Move ${pack.name} down`}
              title="Later: it wins over the packs above"
              disabled={i === packs.length - 1}
              onclick={() => void change(api.overridesMove(v.game, pack.id, false))}>▼</button
            >
            <button
              class="icon remove"
              aria-label={`Remove ${pack.name}`}
              title="Remove"
              onclick={() => {
                remove(pack);
              }}><Icon name="trash" size={14} /></button
            >
          </div>
        </li>
      {/each}
    </ul>
    <p class="dim small-text note">
      A pack lower in the list wins where two replace the same thing. Play with mods uses the packs that are on.
      {#if inTauri}<span class="after">Drop a .zip or folder here to add it.</span>{/if}
    </p>
  {/if}

  {#if developer && commands.length}<CommandBox lines={commands} />{/if}
</section>

<style>
  /* What follows a name or a sentence on its line, a space after it (Svelte trims the space). */
  .by,
  .after {
    margin-left: 0.3em;
  }

  .packs {
    display: flex;
    flex-direction: column;
    gap: 8px;
  }

  .packs.dropping {
    border-color: var(--amber);
    border-style: dashed;
  }

  h3 {
    font-size: 16px;
    color: var(--gold);
  }

  ul {
    margin: 0;
    padding: 0;
    list-style: none;
  }

  .pack {
    display: flex;
    align-items: center;
    gap: 12px;
    padding: 11px 0;
    border-top: 1px solid var(--line);
  }

  .pack:last-child {
    border-bottom: 1px solid var(--line);
  }

  .pack input {
    flex: none;
    width: 18px;
    height: 18px;
    margin: 0;
    accent-color: var(--amber);
    cursor: pointer;
  }

  .pack input:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .name {
    font-size: 14px;
    font-weight: 600;
  }

  .by {
    font-weight: 400;
    color: var(--dim);
  }

  .pack.off .name {
    color: var(--dim);
  }

  .small-text {
    font-size: 13px;
  }

  .chip {
    flex: none;
    max-width: 30%;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    padding: 2px 8px;
    border-radius: 7px;
    border: 1px solid var(--line-strong);
    background: var(--panel-hi);
    font-size: 11.5px;
    color: var(--soft);
  }

  .order {
    flex: none;
    display: flex;
    gap: 6px;
  }

  .icon {
    padding: 4px 9px;
    font-size: 12px;
  }

  .remove {
    color: var(--err);
  }

  .note {
    padding-top: 2px;
  }

  .empty {
    font-size: 14px;
    padding: 4px 0;
  }

  .said {
    font-size: 13px;
    color: var(--ok);
  }

  .failed {
    color: var(--err);
  }
</style>
