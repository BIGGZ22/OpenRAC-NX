<script lang="ts">
  // Make your own → Models and textures: export a level's assets to edit them in Blender or an
  // image editor, and import the edits back as an overrides pack (it joins My mods). The
  // Python side does the work (editor/assets.py, through actions.json). The packs are the game's,
  // shared by its discs; the version (the one the game page plays) only picks the levels exported
  // from. editor/MODDING.md is the modder's guide.
  import Icon from "$components/Icon.svelte";
  import { api, inTauri, pickFolder, type ActionView, type VersionView } from "$lib/api";
  import { app, guardShown, jobs, runChain, toast } from "$lib/app.svelte";
  import { levelName, levelsOf } from "$lib/labels";
  import { ASSETS } from "$lib/modsText";
  import { hidePaths } from "$lib/paths";
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
  function ending(id: string): { line: string; ok: boolean; refused: number; folder: string | null } | null {
    const job = jobs.list.filter((j) => j.scope.kind === "version" && j.actionId === id).at(-1);
    if (!job) return null;
    const lines = job.lines.map((l) => l.line);
    return {
      line: lines.findLast((l) => l.trim() !== "") ?? "",
      ok: job.state === "succeeded",
      refused: lines.filter((l) => l.startsWith("refused: ")).length,
      folder: job.outputs[0] ?? null,
    };
  }

  async function exportAssets() {
    if (!exporter) return;
    busy = "export";
    const ok = await runChain(scope, [exporter], { level, option: what });
    busy = null;
    const end = ending("assets-export");
    const folder = end?.folder ?? /: (\/.*|[A-Za-z]:\\.*)$/.exec(end?.line ?? "")?.[1];
    if (ok && folder) {
      toast(ASSETS.exported(levelName(v.game, level)));
      await guardShown(end?.folder ? api.openOutput(folder) : api.openGameFolder(v.game, folder));
    } else if (!ok) {
      toast(ASSETS.exportStopped, "error");
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
    if (!ok) toast(ASSETS.readStopped, "error");
  }

  async function importEdits() {
    if (!importer) return;
    const folder = inTauri ? await pickFolder(ASSETS.pickImport) : "/home/you/exports/level_00-mobys";
    if (!folder) return;
    busy = "import";
    const ok = await runChain(scope, [importer], { path: folder });
    busy = null;
    const end = ending("assets-import");
    if (ok) {
      said = {
        text: end?.refused ? ASSETS.importedRefused(end.refused) : ASSETS.imported,
        failed: !!end?.refused,
      };
    } else {
      const why = end?.line.replace(/^assets: /, "") ?? "";
      said = { text: why.length > 0 ? why : ASSETS.importStopped, failed: true };
    }
    onimported?.();
  }
</script>

{#if exporter}
  {@const exported = ending("assets-export")?.folder ?? null}
  <div class="assets">
    <p class="muted intro">
      {ASSETS.lead}
      <button class="link" onclick={() => void guardShown(api.openPath("editor/MODDING.md"))}
        >{ASSETS.howItWorks}</button
      >
    </p>
    <ol class="steps">
      <li>
        <div class="text">
          <h3>{ASSETS.exportTitle}</h3>
          <p class="muted">
            {#if busy === "export"}
              {ASSETS.exporting}
            {:else if exporter.runnable}
              {ASSETS.exportLine}
            {:else if reader && !reader.done && !playable}
              {ASSETS.readFirst}
            {:else}
              {ASSETS.readyFirst}
            {/if}
          </p>
        </div>
        <div class="controls">
          <select aria-label={ASSETS.levelLabel} bind:value={level} disabled={!exporter.runnable}>
            {#each levelsOf(v.game) as l (l.number)}<option value={l.number}>{l.name}</option>{/each}
          </select>
          <select aria-label={ASSETS.whatLabel} bind:value={what} disabled={!exporter.runnable}>
            {#each WHAT as w (w.id)}<option value={w.id}>{w.label}</option>{/each}
          </select>
          {#if reader && !reader.done && !playable}
            <button disabled={!reader.runnable || busy !== null} onclick={() => void readLevels()}>
              {#if busy === "read"}<span class="spinner"></span>{:else}<Icon name="build" size={16} />{/if}{ASSETS.read}
            </button>
          {/if}
          {#if exported && busy !== "export"}
            <button class="ghost" onclick={() => void guardShown(api.openOutput(exported))}
              ><Icon name="folder" size={16} />{ASSETS.openFolder}</button
            >
          {/if}
          <button disabled={!exporter.runnable || busy !== null} onclick={() => void exportAssets()}>
            {#if busy === "export"}<span class="spinner"></span>{:else}<Icon
                name="external"
                size={16}
              />{/if}{ASSETS.export}
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
          <h3>{ASSETS.importTitle}</h3>
          <p class="muted" class:failed={said?.failed} class:done={said && !said.failed}>
            {#if busy === "import"}
              {ASSETS.importing}
            {:else if said}
              {hidePaths(said.text).text}
            {:else}
              {ASSETS.importLine}
            {/if}
          </p>
        </div>
        <div class="controls">
          <button disabled={!importer?.runnable || busy !== null} onclick={() => void importEdits()}>
            {#if busy === "import"}<span class="spinner"></span>{:else}<Icon
                name="save"
                size={16}
              />{/if}{ASSETS.import}
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
    margin: 0 0 4px;
    font-size: 0.875rem;
  }

  /* Export's two choices and its buttons wrap under its text when they do not fit beside it. */
  li:first-child {
    grid-template-columns: 26px minmax(0, 1fr);
  }

  li:first-child .controls {
    grid-column: 2;
    justify-content: flex-start;
  }

  .failed {
    color: var(--err);
  }

  .done {
    color: var(--ok);
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
