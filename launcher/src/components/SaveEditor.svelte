<script lang="ts">
  // The save editor: one slot of the memory card, opened from the Save files list. It shows only
  // what core/src/save_edit.rs decodes with confidence, in the game's own words, and saves through
  // write_save, which refuses while the game runs with the card and backs the card up first. After
  // a save, Undo puts that backup back.
  import Icon from "$components/Icon.svelte";
  import { api, type Nanotech, type SaveBackupInfo, type SaveFields, type SaveView } from "$lib/api";
  import { toast } from "$lib/app.svelte";
  import { changed, fieldErrors, goldBoltCount, setOwned, skillCount } from "$lib/saveEdit";
  import { onMount } from "svelte";

  let {
    serial,
    slot,
    onclose,
  }: {
    serial: string;
    slot: number;
    /** `true` when the card changed (saved or undone), so the list reads it again. */
    onclose: (changedCard: boolean) => void;
  } = $props();

  let view = $state<SaveView | null>(null);
  let fields = $state<SaveFields | null>(null);
  let loadError = $state<string | null>(null);
  let saveError = $state<string | null>(null);
  let busy = $state(false);
  let saved = $state<SaveBackupInfo | null>(null);
  let touchedCard = false;

  const errors = $derived(view && fields ? fieldErrors(view, fields) : {});
  const dirty = $derived(!!view && !!fields && changed(view, fields));
  const canSave = $derived(dirty && Object.keys(errors).length === 0 && !busy);
  const weapons = $derived(view?.items.filter((i) => i.kind === "weapon") ?? []);
  const gadgets = $derived(view?.items.filter((i) => i.kind === "gadget") ?? []);

  const nanotechChoices: { value: Nanotech; label: string; health: number }[] = [
    { value: "standard", label: "No upgrade", health: 4 },
    { value: "premium", label: "Premium Nanotech", health: 5 },
    { value: "ultra", label: "Ultra Nanotech", health: 8 },
  ];

  async function load() {
    loadError = null;
    try {
      const read = await api.readSave(serial, slot);
      view = read;
      fields = structuredClone(read.fields);
    } catch (e) {
      loadError = String(e);
    }
  }

  async function save() {
    if (!view || !fields || !canSave) return;
    busy = true;
    saveError = null;
    try {
      const result = await api.writeSave(serial, slot, view.fingerprint, $state.snapshot(fields));
      view = result.view;
      fields = structuredClone(result.view.fields);
      saved = result.backup;
      touchedCard = true;
    } catch (e) {
      saveError = String(e);
    } finally {
      busy = false;
    }
  }

  async function undo() {
    if (!saved) return;
    busy = true;
    saveError = null;
    try {
      await api.restoreBackup(serial, saved.name);
      toast(`Slot ${String(slot + 1)} is back as it was before the edit.`, "info");
      onclose(true);
    } catch (e) {
      saveError = String(e);
    } finally {
      busy = false;
    }
  }

  function close() {
    if (busy) return;
    if (dirty && !saved && !confirm("Close without saving your changes?")) return;
    onclose(touchedCard);
  }

  function item(id: number) {
    return fields?.items.find((i) => i.id === id);
  }

  function setAllSkills(on: boolean) {
    if (fields) fields.skillPoints = fields.skillPoints.map(() => on);
  }

  onMount(() => {
    void load();
  });
</script>

<svelte:window
  onkeydown={(e) => {
    if (e.key === "Escape") close();
  }}
/>

<div class="overlay" role="presentation">
  <button class="backdrop" onclick={close} tabindex="-1" aria-label="Close the save editor"></button>
  <div class="modal rise" role="dialog" aria-modal="true" aria-labelledby="save-editor-title">
    <div class="head">
      <div class="grow">
        <h2 id="save-editor-title">Edit slot {slot + 1}</h2>
        {#if view}
          <p class="sub">
            {view.planetName ?? "Unknown planet"}{view.savedAt ? ` · saved ${view.savedAt}` : ""}
          </p>
        {/if}
      </div>
      <button class="ghost small" onclick={close} disabled={busy} aria-label="Close">
        <Icon name="x" size={16} />
      </button>
    </div>

    <div class="body">
      {#if loadError}
        <p class="problem"><Icon name="warn" size={16} />{loadError}</p>
      {:else if !view || !fields}
        <p class="muted row"><span class="spinner"></span>Reading the save…</p>
      {:else}
        <section aria-labelledby="se-ratchet">
          <h3 id="se-ratchet">Ratchet</h3>
          <div class="line">
            <label for="se-bolts">Bolts</label>
            <div class="field">
              <span class="bolt"><Icon name="bolt" size={16} /></span>
              <input
                id="se-bolts"
                type="number"
                min="0"
                max={view.maxBolts}
                step="1"
                bind:value={fields.bolts}
                aria-invalid={!!errors.bolts}
              />
              {#if errors.bolts}<span class="err-text">{errors.bolts}</span>{/if}
            </div>
          </div>
          <div class="line">
            <span id="se-health">Health</span>
            <div class="choices" role="radiogroup" aria-labelledby="se-health">
              {#each nanotechChoices as c (c.value)}
                <label class="choice" class:on={fields.nanotech === c.value}>
                  <input type="radio" name="se-nanotech" value={c.value} bind:group={fields.nanotech} />
                  <b>{c.health}</b>
                  {c.label}
                </label>
              {/each}
            </div>
          </div>
        </section>

        <section aria-labelledby="se-weapons">
          <h3 id="se-weapons">Weapons</h3>
          <ul class="rows">
            {#each weapons as w (w.id)}
              {@const s = item(w.id)}
              {#if s}
                <li>
                  <label class="own">
                    <input
                      type="checkbox"
                      checked={s.owned}
                      onchange={(e) => {
                        if (view && fields) setOwned(view, fields, w.id, e.currentTarget.checked);
                      }}
                    />
                    {w.name}
                  </label>
                  {#if w.maxAmmo !== null && s.ammo !== null}
                    <span class="ammo" class:off={!s.owned}>
                      <input
                        type="number"
                        min="0"
                        max={w.maxAmmo}
                        step="1"
                        aria-label={`${w.name} ammo`}
                        bind:value={s.ammo}
                        aria-invalid={!!errors[`ammo-${String(w.id)}`]}
                      />
                      <span class="dim">/ {w.maxAmmo}</span>
                      <button class="ghost small" onclick={() => (s.ammo = w.maxAmmo)} disabled={s.ammo === w.maxAmmo}
                        >Fill</button
                      >
                    </span>
                  {:else}
                    <span class="dim no-ammo">No ammo</span>
                  {/if}
                  {#if w.hasGold && s.gold !== null}
                    <label class="gold" class:on={s.gold}>
                      <input type="checkbox" bind:checked={s.gold} />
                      Gold
                    </label>
                  {:else}
                    <span></span>
                  {/if}
                </li>
              {/if}
            {/each}
          </ul>
          {#if Object.keys(errors).some((k) => k.startsWith("ammo-"))}
            <p class="err-text">Ammo goes from 0 to the most the weapon holds.</p>
          {/if}
        </section>

        <section aria-labelledby="se-gadgets">
          <h3 id="se-gadgets">Gadgets and items</h3>
          <div class="chips">
            {#each gadgets as g (g.id)}
              {@const s = item(g.id)}
              {#if s}
                <label class="chip" class:on={s.owned}>
                  <input type="checkbox" bind:checked={s.owned} />
                  {g.name}
                </label>
              {/if}
            {/each}
          </div>
          <p class="note">
            Taking away something the story needs, like the Heli-Pack or the Swingshot, can leave Ratchet stuck.
          </p>
        </section>

        <section aria-labelledby="se-planets">
          <h3 id="se-planets">
            Planets <span class="count">{goldBoltCount(fields)[0]} of {goldBoltCount(fields)[1]} gold bolts</span>
          </h3>
          <ul class="rows planets">
            {#each view.planets as p, i (p.id)}
              {@const s = fields.planets[i]}
              {#if s}
                <li>
                  <span class="planet">
                    <b>{p.name}</b>
                    <span class="dim">{p.location}</span>
                  </span>
                  {#if p.canUnlock}
                    <label class="own">
                      <input type="checkbox" bind:checked={s.unlocked} />
                      Unlocked
                    </label>
                  {:else}
                    <span class="dim story">By the story</span>
                  {/if}
                  <span class="bolts" role="group" aria-label={`Gold bolts on ${p.name}`}>
                    {#each s.goldBolts as found, k (k)}
                      <button
                        class="goldbolt"
                        class:on={found}
                        aria-pressed={found}
                        aria-label={`Gold bolt ${String(k + 1)} on ${p.name}`}
                        onclick={() => (s.goldBolts[k] = !found)}
                      >
                        <Icon name="bolt" size={14} />
                      </button>
                    {/each}
                  </span>
                </li>
              {/if}
            {/each}
          </ul>
        </section>

        <section aria-labelledby="se-skills">
          <h3 id="se-skills">
            Skill points <span class="count">{skillCount(fields)[0]} of {skillCount(fields)[1]}</span>
            <span class="grow"></span>
            <button
              class="ghost small"
              onclick={() => {
                setAllSkills(true);
              }}>All</button
            >
            <button
              class="ghost small"
              onclick={() => {
                setAllSkills(false);
              }}>None</button
            >
          </h3>
          <div class="skills" role="group" aria-label="Skill points">
            {#each fields.skillPoints as on, k (k)}
              <button
                class="skill"
                class:on
                aria-pressed={on}
                aria-label={`Skill point ${String(k + 1)}`}
                onclick={() => {
                  if (fields) fields.skillPoints[k] = !on;
                }}>{k + 1}</button
              >
            {/each}
          </div>
          <p class="note">In the order the game keeps them.</p>
        </section>
      {/if}
    </div>

    <div class="foot">
      {#if saveError}
        <p class="problem"><Icon name="warn" size={16} />{saveError}</p>
      {/if}
      <div class="row">
        {#if saved}
          <span class="grow ok"
            ><Icon name="save" size={16} />Saved. Your card was backed up first: Undo puts it back.</span
          >
          <button class="ghost" onclick={undo} disabled={busy}>Undo</button>
          <button
            onclick={() => {
              onclose(true);
            }}
            disabled={busy}>Close</button
          >
        {:else}
          <span class="grow dim">Your memory card is backed up before anything is written.</span>
          <button class="ghost" onclick={close} disabled={busy}>Cancel</button>
          <button class="primary" onclick={save} disabled={!canSave}>
            {#if busy}<span class="spinner"></span>{:else}<Icon name="save" size={16} />{/if}Save
          </button>
        {/if}
      </div>
    </div>
  </div>
</div>

<style>
  .overlay {
    position: fixed;
    inset: 0;
    z-index: 100;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 24px;
  }

  .backdrop {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    border: none;
    border-radius: 0;
    padding: 0;
    background: rgb(10 10 16 / 0.75);
    backdrop-filter: blur(4px);
    cursor: default;
  }

  .modal {
    position: relative;
    z-index: 1;
    width: 100%;
    max-width: 720px;
    max-height: calc(100vh - 48px);
    background: var(--panel);
    border: 1px solid var(--line-strong);
    border-radius: var(--radius);
    box-shadow: var(--shadow);
    display: flex;
    flex-direction: column;
    overflow: hidden;
  }

  .head,
  .foot {
    padding: 16px 22px;
    background: var(--panel-hi);
  }

  .head {
    display: flex;
    align-items: flex-start;
    gap: 12px;
    border-bottom: 1px solid var(--line);
  }

  .head h2 {
    margin: 0;
  }

  .sub {
    margin: 4px 0 0;
    font-size: 13px;
    color: var(--soft);
  }

  .foot {
    border-top: 1px solid var(--line);
    display: flex;
    flex-direction: column;
    gap: 10px;
  }

  .foot .row {
    gap: 8px;
    font-size: 13px;
  }

  .ok {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    color: var(--ok);
  }

  .body {
    overflow-y: auto;
    padding: 6px 22px 18px;
    display: flex;
    flex-direction: column;
    gap: 18px;
  }

  section h3 {
    display: flex;
    align-items: center;
    gap: 10px;
    margin: 14px 0 8px;
  }

  .count {
    font-family: var(--font-sans);
    font-size: 13px;
    font-weight: 500;
    color: var(--soft);
  }

  ul {
    margin: 0;
    padding: 0;
    list-style: none;
  }

  .line {
    display: grid;
    grid-template-columns: 120px minmax(0, 1fr);
    align-items: center;
    gap: 12px;
    padding: 10px 2px;
    border-top: 1px solid var(--line);
  }

  .line:last-child {
    border-bottom: 1px solid var(--line);
  }

  .field {
    display: flex;
    align-items: center;
    gap: 10px;
    flex-wrap: wrap;
  }

  .bolt {
    color: var(--amber);
    display: inline-flex;
  }

  input[type="number"] {
    font: inherit;
    font-size: 14px;
    font-variant-numeric: tabular-nums;
    width: 130px;
    background: var(--ink-deep);
    color: var(--text);
    border: 1px solid var(--line-strong);
    border-radius: var(--radius-sm);
    padding: 6px 10px;
  }

  input[type="number"][aria-invalid="true"] {
    border-color: var(--err);
  }

  input[type="checkbox"],
  input[type="radio"] {
    accent-color: var(--amber);
    margin: 0;
  }

  .err-text {
    font-size: 13px;
    color: var(--err);
  }

  .choices {
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
  }

  .choice,
  .chip,
  .gold {
    display: inline-flex;
    align-items: center;
    gap: 7px;
    font-size: 13px;
    border: 1px solid var(--line-strong);
    border-radius: 999px;
    padding: 5px 12px;
    cursor: pointer;
    color: var(--soft);
    background: var(--panel-hi);
  }

  .choice input {
    position: absolute;
    opacity: 0;
    pointer-events: none;
  }

  .choice:focus-within {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .choice.on,
  .chip.on {
    color: var(--text);
    border-color: var(--amber);
  }

  .choice b {
    color: var(--gold);
  }

  .gold.on {
    color: var(--gold);
    border-color: var(--gold);
  }

  .rows li {
    display: grid;
    grid-template-columns: minmax(0, 1.3fr) minmax(0, 1.4fr) 88px;
    align-items: center;
    gap: 12px;
    padding: 7px 2px;
    border-top: 1px solid var(--line);
  }

  .rows li:last-child {
    border-bottom: 1px solid var(--line);
  }

  .own {
    display: inline-flex;
    align-items: center;
    gap: 9px;
    cursor: pointer;
  }

  .ammo {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    font-size: 13px;
  }

  .ammo input {
    width: 80px;
  }

  .ammo.off {
    opacity: 0.55;
  }

  .no-ammo,
  .story {
    font-size: 13px;
  }

  .chips {
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
  }

  .note {
    margin: 8px 0 0;
    font-size: 12px;
    color: var(--dim);
  }

  .planets li {
    grid-template-columns: minmax(0, 1.6fr) minmax(0, 0.8fr) minmax(0, 0.8fr);
  }

  .planet {
    display: flex;
    flex-direction: column;
    line-height: 1.3;
  }

  .planet .dim {
    font-size: 12px;
  }

  .bolts {
    display: inline-flex;
    gap: 6px;
    justify-self: end;
  }

  .goldbolt,
  .skill {
    padding: 0;
    width: 28px;
    height: 28px;
    justify-content: center;
    color: var(--dim);
  }

  .goldbolt.on {
    color: var(--bar-ink-deep);
    background: var(--gold);
    border-color: var(--gold);
  }

  .skills {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(34px, 1fr));
    gap: 6px;
  }

  .skill {
    width: auto;
    font-size: 12px;
    font-variant-numeric: tabular-nums;
  }

  .skill.on {
    color: var(--bar-ink-deep);
    background: var(--amber);
    border-color: var(--amber);
  }

  .problem {
    display: flex;
    align-items: flex-start;
    gap: 8px;
    margin: 0;
    font-size: 13px;
    color: var(--err);
  }
</style>
