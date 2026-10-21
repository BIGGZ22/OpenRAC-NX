<script lang="ts">
  // Make your own → Models and textures: export a level's assets to edit them in Blender or an
  // image editor, and import the edits back as an overrides pack (it joins Your mods, above). The
  // Python side does the work (editor/assets.py, through actions.json). The packs are the game's,
  // shared by its discs; the version (the one the game page plays) only picks the levels exported
  // from. editor/MODDING.md is the modder's guide.
  import Icon from "$components/Icon.svelte";
  import { api, inTauri, pickFolder, type ActionView, type VersionView } from "$lib/api";
  import { app, guard, jobs, runChain, toast } from "$lib/app.svelte";
  import { levelName, levelsOf } from "$lib/labels";
  import CommandBox from "./CommandBox.svelte";

  let { version: v, onimported }: { version: VersionView; onimported?: () => void } = $props();

  const developer = $derived(app.config?.developer ?? false);
  const scope = $derived({ kind: "version" as const, key: v.key });
  const action = (id: string): ActionView | null => v.actions.find((a) => a.id === id && a.thisPlatform) ?? null;
  const exporter = $derived(action("assets-export"));
  const importer = $derived(action("assets-import"));
  /** Reads the levels into the layout the exports are made from (on the game's page, for a game
   *  that plays; here, for one in early access whose page has no such step). */
  const reader = $derived(action("port-levels"));

  const WHAT = [
    { id: "mobys", label: "Characters and objects" },
    { id: "textures", label: "Textures" },
    { id: "scenery", label: "Scenery (ties and shrubs)" },
    { id: "terrain", label: "Terrain (the ground and walls)" },
    { id: "all", label: "Everything" },
  ];

  // svelte-ignore state_referenced_locally
  let level = $state(levelsOf(v.game)[0]?.number ?? 0);
  let what = $state("mobys");
  let busy = $state<string | null>(null);
  /** What the last import said, in a sentence. */
  let said = $state<{ text: string; failed: boolean } | null>(null);

  /** The levels the exports are read from: a checkout's under assets/, an installed launcher's in the
   *  game's data folder (its own folder is read-only; tools/bundle_standalone.py). */
  const levels = $derived(
    app.info?.standalone && app.config?.installDir
      ? `${app.config.installDir}/active/${v.game}/data/port/${v.name}`
      : `assets/port/${v.game}/${v.name}`,
  );

  /** The last line a finished job of ACTION printed, and whether it succeeded. */
  function ending(id: string): { line: string; ok: boolean; refused: number } | null {
    const job = jobs.list.filter((j) => j.scope.kind === "version" && j.actionId === id).at(-1);
    if (!job) return null;
    const lines = job.lines.map((l) => l.line);
    return {
      line: lines.findLast((l) => l.trim() !== "") ?? "",
      ok: job.state === "succeeded",
      refused: lines.filter((l) => l.startsWith("refused: ")).length,
    };
  }

  async function exportAssets() {
    if (!exporter) return;
    busy = "export";
    const ok = await runChain(scope, [exporter], { level, option: what });
    busy = null;
    const end = ending("assets-export");
    const folder = /: (\/.*|[A-Za-z]:\\.*)$/.exec(end?.line ?? "")?.[1];
    if (ok && folder) {
      toast(`Exported ${levelName(v.game, level)}: the folder is opening.`);
      await guard(api.openGameFolder(v.game, folder));
    } else if (!ok) {
      toast("The export stopped. The activity log has what it said.", "error");
    }
  }

  /** The game page's play action is planned: its levels are read here, for packs. */
  const playable = $derived.by(() => {
    const play = action("play");
    return !!play && play.state !== "planned";
  });

  async function readLevels() {
    if (!reader) return;
    busy = "read";
    const ok = await runChain(scope, [reader]);
    busy = null;
    if (!ok) toast("Reading the levels stopped. The activity log has what it said.", "error");
  }

  async function importEdits() {
    if (!importer) return;
    const folder = inTauri
      ? await pickFolder("Choose the export folder you edited")
      : "/home/you/exports/level_00-mobys";
    if (!folder) return;
    busy = "import";
    const ok = await runChain(scope, [importer], { path: folder });
    busy = null;
    const end = ending("assets-import");
    if (ok) {
      said = {
        text: end?.refused
          ? `Imported, but ${end.refused} change${end.refused === 1 ? " was" : "s were"} refused: the activity log says why and what to do.`
          : "Imported: your edits are a pack, switched on, at the end of Your mods.",
        failed: !!end?.refused,
      };
    } else {
      const why = end?.line.replace(/^assets: /, "") ?? "";
      said = { text: why.length > 0 ? why : "The import stopped. The activity log has what it said.", failed: true };
    }
    onimported?.();
  }
</script>

{#if exporter}
  <div class="assets">
    <p class="muted intro">
      Export a level's models and textures, edit them in Blender or an image editor, and bring them back as a pack the
      game draws in place of its own. Upscaled textures are welcome.
      <button class="link" onclick={() => void guard(api.openPath("editor/MODDING.md"))}>How it works</button>
    </p>
    <ol class="steps">
      <li class="wide">
        <div class="text">
          <h3>Export</h3>
          <p class="muted">
            {#if busy === "export"}
              Writing the files…
            {:else if exporter.runnable}
              A folder of glTF models (with their skeletons) and PNG textures opens when it is ready.
            {:else if reader && !reader.done && !playable}
              Read the levels from your disc first: a few minutes, once.
            {:else}
              Get the levels ready first, on the game's page.
            {/if}
          </p>
        </div>
        <div class="controls">
          <select aria-label="Level to export" bind:value={level} disabled={!exporter.runnable}>
            {#each levelsOf(v.game) as l (l.number)}<option value={l.number}>{l.name}</option>{/each}
          </select>
          <select aria-label="What to export" bind:value={what} disabled={!exporter.runnable}>
            {#each WHAT as w (w.id)}<option value={w.id}>{w.label}</option>{/each}
          </select>
          {#if reader && !reader.done && !playable}
            <button disabled={!reader.runnable || busy !== null} onclick={() => void readLevels()}>
              {#if busy === "read"}<span class="spinner"></span>{:else}<Icon name="build" size={16} />{/if}Read the
              levels
            </button>
          {/if}
          <button disabled={!exporter.runnable || busy !== null} onclick={() => void exportAssets()}>
            {#if busy === "export"}<span class="spinner"></span>{:else}<Icon name="folder" size={16} />{/if}Export
          </button>
        </div>
        {#if developer}
          <div class="span">
            <CommandBox lines={[`python3 editor/assets.py export ${levels} <out> --level ${level} --what ${what}`]} />
          </div>
        {/if}
      </li>
      <li>
        <div class="text">
          <h3>Import your edits</h3>
          <p class="muted" class:failed={said?.failed}>
            {#if busy === "import"}
              Checking what you changed…
            {:else if said}
              {said.text}
            {:else}
              Choose the export folder you edited: what changed becomes a pack in Your mods.
            {/if}
          </p>
        </div>
        <div class="controls">
          <button disabled={!importer?.runnable || busy !== null} onclick={() => void importEdits()}>
            {#if busy === "import"}<span class="spinner"></span>{:else}<Icon name="save" size={16} />{/if}Import…
          </button>
        </div>
        {#if developer}
          <div class="span">
            <CommandBox lines={[`python3 editor/assets.py import <out> --levels ${levels} --into <data>/overrides`]} />
          </div>
        {/if}
      </li>
    </ol>
  </div>
{/if}

<style>
  .assets {
    display: flex;
    flex-direction: column;
    gap: 4px;
  }

  .intro {
    font-size: 0.875rem;
    padding-bottom: 6px;
  }

  .steps h3 {
    font-family: var(--font-sans);
    font-size: 0.9375rem;
    font-weight: 600;
    color: var(--text);
  }

  .failed {
    color: var(--err);
  }

  .link {
    border: 0;
    background: none;
    padding: 0;
    font-size: inherit;
    color: var(--amber);
    font-weight: 600;
  }
</style>
