<script lang="ts">
  // A game's save files: the five slots of its memory card as rows (planet, bolts,
  // when it was saved), and backups to take and restore. File names, sizes and the
  // card's folder name are a contributor's details: shown with developer tools on.
  import Icon from "$components/Icon.svelte";
  import { api, type GameSaveStatus, type VersionView } from "$lib/api";
  import { app, toast } from "$lib/app.svelte";
  import { onMount } from "svelte";

  let { version }: { version: VersionView } = $props();

  const developer = $derived(app.config?.developer ?? false);

  let loading = $state(true);
  let status = $state<GameSaveStatus | null>(null);
  let backupNote = $state("");
  let creatingBackup = $state(false);
  let restoringBackupName = $state<string | null>(null);

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

  async function createBackup() {
    if (!status) return;
    creatingBackup = true;
    try {
      const res = await api.backupSaves(version.serial, backupNote.trim() || undefined);
      toast(`Backed up as ${res.name}.`, "info");
      backupNote = "";
      await loadStatus();
    } catch (e) {
      toast(`Backup failed: ${String(e)}`, "error");
    } finally {
      creatingBackup = false;
    }
  }

  async function restoreBackup(name: string) {
    if (!confirm(`Restore the backup "${name}"? Your current saves are backed up first, then replaced.`)) {
      return;
    }
    restoringBackupName = name;
    try {
      await api.restoreBackup(version.serial, name);
      toast(`Restored ${name}.`, "info");
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

  function formatBytes(bytes: number): string {
    if (bytes === 0) return "0 B";
    const k = 1024;
    const sizes = ["B", "KB", "MB", "GB"];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return `${parseFloat((bytes / Math.pow(k, i)).toFixed(1))} ${sizes[i]}`;
  }

  onMount(() => {
    void loadStatus();
  });
</script>

<section class="panel saves" aria-label="Save files">
  <div class="row top">
    <h2 class="grow">Save files</h2>
    {#if developer}
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
    {#if developer}
      <p class="dim mono">{status.gameFolderName ?? version.serial}</p>
    {/if}
    <ul class="slots">
      {#each status.slots as slot (slot.slotIndex)}
        <li class:empty={slot.isEmpty}>
          <span class="slot">Slot {slot.slotIndex + 1}</span>
          {#if slot.isEmpty}
            <span class="planet">Empty</span>
          {:else}
            <span class="planet">{slot.planetName ?? "Unknown planet"}</span>
            <span class="bolts"><Icon name="bolt" size={14} />{(slot.bolts ?? 0).toLocaleString()}</span>
            <span class="when">{slot.timestamp ?? ""}</span>
          {/if}
          {#if developer}
            <span class="file mono dim">{slot.filename}{slot.isEmpty ? "" : ` · ${formatBytes(slot.size)}`}</span>
          {/if}
        </li>
      {/each}
    </ul>

    <div class="backups">
      <h3>Backups</h3>
      <div class="row take">
        <input
          type="text"
          aria-label="A name for the backup"
          placeholder="Name it, if you like: before the last boss"
          bind:value={backupNote}
          disabled={creatingBackup}
          onkeydown={(e) => {
            if (e.key === "Enter") void createBackup();
          }}
        />
        <button disabled={creatingBackup} onclick={createBackup}>
          {#if creatingBackup}<span class="spinner"></span>{:else}<Icon name="save" size={16} />{/if}Back up now
        </button>
      </div>
      {#if status.backups.length}
        <ul class="list">
          {#each status.backups as backup (backup.name)}
            <li>
              <span class="grow clip name">{backup.name}</span>
              <span class="dim when"
                >{new Date(backup.createdMillis).toLocaleString()}{developer
                  ? ` · ${formatBytes(backup.totalSize)}`
                  : ""}</span
              >
              <button
                class="ghost small"
                disabled={restoringBackupName === backup.name}
                onclick={() => void restoreBackup(backup.name)}
              >
                {#if restoringBackupName === backup.name}<span class="spinner"></span>{/if}Restore
              </button>
            </li>
          {/each}
        </ul>
      {/if}
    </div>
  {/if}
</section>

<style>
  .saves {
    display: flex;
    flex-direction: column;
    gap: 14px;
  }

  .top {
    gap: 4px;
  }

  ul {
    margin: 0;
    padding: 0;
    list-style: none;
  }

  /* One slot per row, the columns aligned down the list. */
  .slots li {
    display: grid;
    grid-template-columns: 64px minmax(120px, 1.2fr) minmax(90px, 0.7fr) minmax(150px, 1fr) auto;
    align-items: center;
    gap: 16px;
    padding: 10px 2px;
    border-top: 1px solid var(--line);
    font-variant-numeric: tabular-nums;
  }

  .slots li:last-child {
    border-bottom: 1px solid var(--line);
  }

  .slot {
    font-size: 13px;
    color: var(--dim);
  }

  .planet {
    font-weight: 600;
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
  }

  .file {
    justify-self: end;
  }

  .empty .planet {
    font-weight: 400;
    color: var(--dim);
  }

  .backups {
    display: flex;
    flex-direction: column;
    gap: 10px;
    margin-top: 6px;
  }

  .take input {
    flex: 1;
  }

  .list li {
    display: flex;
    align-items: center;
    gap: 14px;
    padding: 6px 2px;
  }

  .name {
    font-weight: 500;
  }
</style>
