<script lang="ts">
  // A game's saves: the memory card's used slots as rows (planet, bolts, when it was saved), the
  // empty ones in one line, and the backups folded under a line of their own, each called by its
  // note. File names, sizes and the card's folder name are a contributor's details: a chip and
  // tooltips with developer tools on.
  import Icon from "$components/Icon.svelte";
  import { api, type GameSaveStatus, type SaveBackupInfo, type VersionView } from "$lib/api";
  import { app, toast } from "$lib/app.svelte";
  import { grouped, size } from "$lib/format";
  import { when } from "$lib/reports";
  import { backupLabel, emptySlots, stampSeconds } from "$lib/saves";
  import { fold } from "$lib/motion";
  import { onMount, tick } from "svelte";
  import { slide } from "svelte/transition";

  let { version }: { version: VersionView } = $props();

  const developer = $derived(app.config?.developer ?? false);

  let loading = $state(true);
  let status = $state<GameSaveStatus | null>(null);
  let backupNote = $state("");
  let creatingBackup = $state(false);
  let restoringBackupName = $state<string | null>(null);
  /** The slot open in the save editor. */
  let editing = $state<number | null>(null);
  /** The backups are folded until asked for. */
  let showBackups = $state(false);
  /** "Back up now" asks for an optional name first, in the backups' own space. */
  let naming = $state(false);
  let noteInput = $state<HTMLInputElement | null>(null);

  const used = $derived(status?.slots.filter((s) => !s.isEmpty) ?? []);
  const empty = $derived(emptySlots(status?.slots.filter((s) => s.isEmpty).map((s) => s.slotIndex + 1) ?? []));

  /** A slot's save time in words ("9 October at 17:01"); the core's own text when it cannot be read. */
  function saved(stamp: string | null): string {
    const seconds = stampSeconds(stamp);
    return seconds === null ? (stamp ?? "") : when(seconds);
  }

  async function loadStatus() {
    loading = true;
    try {
      status = await api.inspectSaves(version.serial);
    } catch (e) {
      toast(`Failed to inspect saves: ${String(e)}`, "error");
    } finally {
      loading = false;
    }
  }

  async function startBackup() {
    showBackups = true;
    naming = true;
    await tick();
    noteInput?.focus();
  }

  async function createBackup() {
    if (!status) return;
    creatingBackup = true;
    try {
      const res = await api.backupSaves(version.serial, backupNote.trim() || undefined);
      toast(`Backed up: ${backupLabel(res)}.`, "info");
      backupNote = "";
      naming = false;
      await loadStatus();
    } catch (e) {
      toast(`Backup failed: ${String(e)}`, "error");
    } finally {
      creatingBackup = false;
    }
  }

  async function restoreBackup(backup: SaveBackupInfo) {
    const label = `${backupLabel(backup)} (${when(backup.createdMillis / 1000)})`;
    if (!confirm(`Restore the backup "${label}"? Your current saves are backed up first, then replaced.`)) {
      return;
    }
    restoringBackupName = backup.name;
    try {
      await api.restoreBackup(version.serial, backup.name);
      toast(`Restored ${label}.`, "info");
      await loadStatus();
    } catch (e) {
      toast(`Restore failed: ${String(e)}`, "error");
    } finally {
      restoringBackupName = null;
    }
  }

  async function openFolder() {
    try {
      await api.openSavesFolder(version.serial);
    } catch (e) {
      toast(`Could not open folder: ${String(e)}`, "error");
    }
  }

  onMount(() => {
    void loadStatus();
  });
</script>

{#if editing !== null}
  <!-- The save editor loads the first time a slot is edited. -->
  {#await import("$components/SaveEditor.svelte") then { default: SaveEditor }}
    <SaveEditor
      serial={version.serial}
      slot={editing}
      onclose={(changedCard) => {
        editing = null;
        if (changedCard) void loadStatus();
      }}
    />
  {/await}
{/if}

<section class="panel saves" aria-label="Saves">
  <div class="row top">
    <h2 class="grow">Saves</h2>
    {#if developer && status?.exists}
      <span class="chip mono" title="The memory card's folder">{status.gameFolderName ?? version.serial}</span>
      <button class="ghost small" onclick={loadStatus} disabled={loading} title="Read the memory card again">
        {#if loading}<span class="spinner"></span>{:else}<Icon name="refresh" size={14} />{/if}Refresh
      </button>
    {/if}
    <button class="ghost small" onclick={openFolder}><Icon name="folder" size={14} />Open folder</button>
  </div>

  {#if loading && !status}
    <p class="muted row"><span class="spinner"></span>Reading the memory card…</p>
  {:else if status && !status.exists}
    <p class="muted">Your saves show here once you have played and saved.</p>
  {:else if status}
    {#if used.length}
      <ul class="slots">
        {#each used as slot (slot.slotIndex)}
          <li>
            <span class="slot" title={developer ? `${slot.filename} · ${size(slot.size)}` : undefined}
              >Slot {slot.slotIndex + 1}</span
            >
            <span class="planet">{slot.planetName ?? "Unknown planet"}</span>
            <span class="bolts"><Icon name="bolt" size={14} />{grouped(slot.bolts ?? 0)}</span>
            <span class="when" title={developer ? (slot.timestamp ?? undefined) : undefined}
              >{saved(slot.timestamp)}</span
            >
            {#if slot.editable}
              <button class="small edit" onclick={() => (editing = slot.slotIndex)}
                ><Icon name="edit" size={14} />Edit</button
              >
            {:else}
              <span></span>
            {/if}
          </li>
        {/each}
      </ul>
      {#if empty}<p class="empty">{empty}</p>{/if}
    {:else}
      <p class="muted">No saves on this card yet: they show here once you have saved in the game.</p>
    {/if}

    <div class="backups">
      <div class="row">
        <button
          class="fold grow"
          class:open={showBackups}
          aria-expanded={showBackups}
          aria-controls="backups-list"
          onclick={() => (showBackups = !showBackups)}
        >
          Backups ({status.backups.length})
        </button>
        <button class="small" disabled={creatingBackup || naming} onclick={() => void startBackup()}>
          <Icon name="save" size={14} />Back up now
        </button>
      </div>
      {#if showBackups}
        <div id="backups-list" class="open-list" transition:slide={fold}>
          {#if naming}
            <div class="row take">
              <input
                bind:this={noteInput}
                type="text"
                aria-label="A name for the backup"
                placeholder="Name it, if you like: before the last boss"
                bind:value={backupNote}
                disabled={creatingBackup}
                onkeydown={(e) => {
                  if (e.key === "Enter") void createBackup();
                  if (e.key === "Escape") naming = false;
                }}
              />
              <button class="primary small" disabled={creatingBackup} onclick={createBackup}>
                {#if creatingBackup}<span class="spinner"></span>{/if}Back up
              </button>
              <button class="ghost small" disabled={creatingBackup} onclick={() => (naming = false)}>Cancel</button>
            </div>
          {/if}
          {#if status.backups.length}
            <ul class="list">
              {#each status.backups as backup (backup.name)}
                <li>
                  <span class="grow clip">
                    <strong>{backupLabel(backup)}</strong>
                    <span class="dim">· {when(backup.createdMillis / 1000)}</span>
                  </span>
                  {#if developer}
                    <span class="chip mono" title={backup.name}>{size(backup.totalSize)}</span>
                  {/if}
                  <button
                    class="ghost small restore"
                    disabled={restoringBackupName === backup.name}
                    onclick={() => void restoreBackup(backup)}
                  >
                    {#if restoringBackupName === backup.name}<span class="spinner"></span>{/if}Restore
                  </button>
                </li>
              {/each}
            </ul>
          {:else if !naming}
            <p class="muted small-text">
              No backups yet. The launcher also takes one by itself before the save editor writes.
            </p>
          {/if}
        </div>
      {/if}
    </div>
  {/if}
</section>

<style>
  .saves {
    display: flex;
    flex-direction: column;
    gap: 12px;
  }

  .top {
    gap: 6px;
  }

  ul {
    margin: 0;
    padding: 0;
    list-style: none;
  }

  /* One used slot per row, the columns aligned down the list. */
  .slots li {
    display: grid;
    grid-template-columns: 48px minmax(0, 1fr) auto minmax(0, auto) auto;
    align-items: center;
    gap: 14px;
    padding: 9px 2px;
    border-bottom: 1px solid var(--line);
    font-variant-numeric: tabular-nums;
  }

  .slot {
    font-size: 13px;
    color: var(--dim);
  }

  .planet {
    font-weight: 600;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }

  .bolts {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    color: var(--amber);
    font-weight: 600;
  }

  .when {
    font-size: 13px;
    color: var(--soft);
    white-space: nowrap;
  }

  .empty {
    margin: 0;
    font-size: 13px;
    color: var(--dim);
  }

  /* A contributor's detail: a small mono chip, never glued to a player's words. */
  .chip {
    font-size: 11.5px;
    color: var(--soft);
    background: var(--panel-hi);
    border: 1px solid var(--line-strong);
    border-radius: 7px;
    padding: 2px 8px;
    white-space: nowrap;
  }

  .backups {
    display: flex;
    flex-direction: column;
    gap: 10px;
    padding-top: 12px;
    border-top: 1px solid var(--line);
  }

  /* The backups' fold: a line that opens, like the logs' in Help. */
  .fold {
    justify-content: flex-start;
    gap: 8px;
    padding: 4px 0;
    border: 0;
    background: none;
    color: var(--lav);
    font-weight: 600;
  }

  .fold::before {
    content: "";
    border-left: 5px solid currentColor;
    border-top: 4px solid transparent;
    border-bottom: 4px solid transparent;
    transition: transform 0.15s;
  }

  .fold.open::before {
    transform: rotate(90deg);
  }

  .open-list {
    display: flex;
    flex-direction: column;
    gap: 8px;
    padding-left: 14px;
  }

  .take input {
    flex: 1;
    min-width: 0;
  }

  .list li {
    display: flex;
    align-items: center;
    gap: 12px;
    padding: 4px 0;
  }

  .list strong {
    font-weight: 600;
  }

  .restore {
    color: var(--amber);
  }

  .small-text {
    font-size: 13px;
  }

  @media (max-width: 640px) {
    .slots li {
      grid-template-columns: 48px minmax(0, 1fr) auto;
    }
  }
</style>
