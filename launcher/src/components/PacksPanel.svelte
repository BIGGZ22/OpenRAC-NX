<script lang="ts">
  // A game's overrides packs (textures, models and levels), on My mods: switch one on or off,
  // change the order (a pack lower in the list wins), remove one. The list and its changes are
  // instant (core/src/overrides.rs). Adding is AddMod's (editor/overrides.py, through lib/addMods.ts).
  // The packs are the game's, shared by its discs.
  import { api, ask, type PackView, type VersionView } from "$lib/api";
  import { app, guard } from "$lib/app.svelte";
  import { PACKS } from "$lib/modsText";
  import CommandBox from "./CommandBox.svelte";
  import Icon from "./Icon.svelte";

  let {
    version: v,
    reload = 0,
    commands = [],
  }: {
    version: VersionView;
    /** Bumped when something else made a pack (AddMod, an import): the list is read again. */
    reload?: number;
    /** With developer tools on: the same steps from a shell. */
    commands?: string[];
  } = $props();

  const developer = $derived(app.config?.developer ?? false);

  let packs = $state<PackView[]>([]);
  let packsError = $state<string | null>(null);

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

  async function change(work: Promise<PackView[]>) {
    const result = await guard(work);
    if (result) packs = result;
  }

  async function remove(pack: PackView) {
    const message = `Remove "${pack.name}"? The launcher's copy is deleted; the .zip or folder you added it from is kept.`;
    if (!(await ask(message, PACKS.remove))) return;
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

<section class="panel packs" aria-label={PACKS.title}>
  <div class="row head">
    <div class="grow">
      <h2>{PACKS.title}</h2>
      <p class="dim sub">
        {PACKS.sub}{#if packs.length > 1}<span title={PACKS.moveDownTitle}>&nbsp;· {PACKS.orderHint}</span>{/if}
      </p>
    </div>
  </div>

  {#if packsError}
    <p class="failed">{packsError}</p>
  {:else if packs.length === 0}
    <p class="dim empty">{PACKS.empty}</p>
  {:else}
    <ul>
      {#each packs as pack, i (pack.id)}
        <li class="pack" class:off={!pack.enabled}>
          <input
            type="checkbox"
            role="switch"
            class="switch"
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
              title={PACKS.moveUpTitle}
              disabled={i === 0}
              onclick={() => void change(api.overridesMove(v.game, pack.id, true))}><Icon name="up" size={14} /></button
            >
            <button
              class="icon"
              aria-label={`Move ${pack.name} down`}
              title={PACKS.moveDownTitle}
              disabled={i === packs.length - 1}
              onclick={() => void change(api.overridesMove(v.game, pack.id, false))}
              ><Icon name="down" size={14} /></button
            >
            <button
              class="icon remove"
              aria-label={`Remove ${pack.name}`}
              title={PACKS.remove}
              onclick={() => {
                void remove(pack);
              }}><Icon name="trash" size={14} /></button
            >
          </div>
        </li>
      {/each}
    </ul>
  {/if}

  {#if developer && commands.length}<CommandBox lines={commands} />{/if}
</section>

<style>
  /* What follows a name on its line, a space after it (Svelte trims the space). */
  .by {
    margin-left: 0.3em;
  }

  .packs {
    display: flex;
    flex-direction: column;
    gap: 8px;
  }

  h2 {
    font-size: 1rem;
  }

  .sub {
    margin: 2px 0 0;
    font-size: 0.8125rem;
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

  .pack input:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .name {
    font-size: 0.875rem;
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
    font-size: 0.8125rem;
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
    font-size: 0.75rem;
    color: var(--soft);
  }

  .order {
    flex: none;
    display: flex;
    gap: 6px;
  }

  .icon {
    padding: 4px 9px;
    font-size: 0.75rem;
  }

  .remove {
    color: var(--err);
  }

  .empty {
    font-size: 0.875rem;
    padding: 4px 0;
  }

  .failed {
    color: var(--err);
  }
</style>
