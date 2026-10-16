<script lang="ts">
  // Models and textures: export a level's assets to edit them in Blender or an image editor,
  // import the edits back as an overrides pack, and manage the packs (on or off, their order,
  // remove, add a .zip or a folder, or drop one on the page). The Python side does the work
  // (editor/assets.py, editor/overrides.py, through actions.json); the list of packs and its
  // changes are instant (core/src/overrides.rs). editor/MODDING.md is the modder's guide.
  import { onMount } from "svelte";
  import Icon from "$components/Icon.svelte";
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
  import { app, guard, jobs, runChain, toast } from "$lib/app.svelte";
  import { RAC1_LEVELS } from "$lib/labels";

  const { version: v }: { version: VersionView } = $props();

  const developer = $derived(app.config?.developer ?? false);
  const scope = $derived({ kind: "version" as const, key: v.key });
  const action = (id: string): ActionView | null => v.actions.find((a) => a.id === id && a.thisPlatform) ?? null;
  const exporter = $derived(action("assets-export"));
  const importer = $derived(action("assets-import"));
  const adder = $derived(action("overrides-add"));

  const WHAT = [
    { id: "mobys", label: "Characters and objects" },
    { id: "textures", label: "Textures" },
    { id: "scenery", label: "Scenery (ties and shrubs)" },
    { id: "all", label: "Everything" },
  ];

  let level = $state(0);
  let what = $state("mobys");
  let busy = $state<string | null>(null);
  let packs = $state<PackView[]>([]);
  let packsError = $state<string | null>(null);
  let dropping = $state(false);
  /** What the last import or add said, in a sentence. */
  let said = $state<{ text: string; failed: boolean } | null>(null);

  async function load() {
    try {
      packs = await api.overridesList(v.game);
      packsError = null;
    } catch (e) {
      packsError = typeof e === "string" ? e : String(e);
    }
  }

  onMount(() => {
    void load();
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
      toast(`Exported ${RAC1_LEVELS[level] ?? "the level"}: the folder is opening.`);
      await guard(api.openGameFolder(v.game, folder));
    } else if (!ok) {
      toast("The export stopped. Tasks has what it said.", "error");
    }
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
          ? `Imported, but ${end.refused} change${end.refused === 1 ? " was" : "s were"} refused: Tasks says why and what to do.`
          : "Imported: your edits are a pack, switched on, at the end of the list.",
        failed: !!end?.refused,
      };
    } else {
      const why = end?.line.replace(/^assets: /, "") ?? "";
      said = { text: why.length > 0 ? why : "The import stopped. Tasks has what it said.", failed: true };
    }
    await load();
  }

  async function addPaths(paths: string[]) {
    if (!adder) return;
    for (const path of paths) {
      busy = "add";
      const ok = await runChain(scope, [adder], { path });
      const end = ending("overrides-add");
      const name = path.split(/[\\/]/).at(-1) ?? path;
      said = ok
        ? { text: `Added ${name}.`, failed: false }
        : { text: `${name}: ${end?.line.replace(/^overrides: /, "") ?? "it could not be added"}`, failed: true };
      if (!ok) break;
    }
    busy = null;
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

{#if exporter}
  <section class="panel game">
    <h2>Models and textures</h2>
    <p class="muted">
      Export a level's models and textures, edit them in Blender or an image editor, and bring them back as a pack the
      game draws in place of its own. Upscaled textures are welcome.
      <button class="link" onclick={() => void guard(api.openPath("editor/MODDING.md"))}>How it works</button>
    </p>
    <ol>
      <li>
        <div class="grow">
          <h3>Export</h3>
          <p class="muted clip">
            {#if busy === "export"}
              Writing the files…
            {:else if exporter.runnable}
              A folder of glTF models (with their skeletons) and PNG textures opens when it is ready.
            {:else}
              Get the levels ready first, on the game's page.
            {/if}
          </p>
        </div>
        <select aria-label="Level to export" bind:value={level} disabled={!exporter.runnable}>
          {#each RAC1_LEVELS as name, i (i)}<option value={i}>{name}</option>{/each}
        </select>
        <select aria-label="What to export" bind:value={what} disabled={!exporter.runnable}>
          {#each WHAT as w (w.id)}<option value={w.id}>{w.label}</option>{/each}
        </select>
        <button disabled={!exporter.runnable || busy !== null} onclick={() => void exportAssets()}>
          {#if busy === "export"}<span class="spinner"></span>{:else}<Icon name="folder" size={16} />{/if}Export
        </button>
      </li>
      <li>
        <div class="grow">
          <h3>Import your edits</h3>
          <p class="muted clip" class:failed={said?.failed}>
            {#if busy === "import"}
              Checking what you changed…
            {:else if said}
              {said.text}
            {:else}
              Choose the export folder you edited: what changed becomes a pack below.
            {/if}
          </p>
        </div>
        <button disabled={!importer?.runnable || busy !== null} onclick={() => void importEdits()}>
          {#if busy === "import"}<span class="spinner"></span>{:else}<Icon name="save" size={16} />{/if}Import…
        </button>
      </li>
    </ol>

    <div class="packs" class:dropping role="list" aria-label="Overrides packs">
      <div class="head row">
        <h3 class="grow">Packs</h3>
        <button class="small" disabled={!adder?.runnable || busy !== null} onclick={() => void addZip()}
          ><Icon name="library" size={14} />Add a .zip…</button
        >
        <button class="small" disabled={!adder?.runnable || busy !== null} onclick={() => void addFolder()}
          ><Icon name="folder" size={14} />Add a folder…</button
        >
      </div>
      {#if packsError}
        <p class="failed">{packsError}</p>
      {:else if packs.length === 0}
        <p class="dim empty">
          No packs yet. Import your edits, add a pack someone made, or drop a .zip or folder here.
        </p>
      {:else}
        {#each packs as pack, i (pack.id)}
          <div class="pack" role="listitem" class:off={!pack.enabled}>
            <input
              type="checkbox"
              aria-label={`Use ${pack.name}`}
              checked={pack.enabled}
              onchange={(e) => void change(api.overridesSetEnabled(v.game, pack.id, e.currentTarget.checked))}
            />
            <div class="grow">
              <div class="name">
                {pack.name}{#if pack.author}<span class="dim"> by {pack.author}</span>{/if}
              </div>
              <div class="dim small-text">
                {summary(pack)}{#if pack.description}. {pack.description}{/if}{#if developer}<code>
                    {pack.id}</code
                  >{/if}
              </div>
            </div>
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
        {/each}
        <p class="dim small-text">
          A pack lower in the list wins where two replace the same thing. Play with your mods (above) uses the packs
          that are on.{#if inTauri}
            Drop a .zip or folder here to add it.{/if}
        </p>
      {/if}
    </div>

    {#if developer}
      <div class="sdk">
        <p class="dim">
          Exports, packs and the stacked folder the game reads are in the game's set-up folder (<code>exports/</code>,
          <code>overrides/</code>, <code>mods/</code>). The same steps from a shell:
        </p>
        <pre>python3 editor/assets.py export assets/port/{v.game} &lt;out&gt; --level N --what mobys
python3 editor/assets.py import &lt;out&gt; --levels assets/port/{v.game} --into &lt;data&gt;/overrides
python3 editor/overrides.py add &lt;pack.zip&gt; --into &lt;data&gt;/overrides
python3 editor/overrides.py stack --into &lt;data&gt;/overrides --out &lt;data&gt;/mods --packed assets/packed/{v.game}</pre>
      </div>
    {/if}
  </section>
{/if}

<style>
  .game {
    display: flex;
    flex-direction: column;
    gap: 6px;
  }

  ol {
    margin: 0;
    padding: 0;
    list-style: none;
    counter-reset: step;
  }

  li {
    display: flex;
    align-items: center;
    gap: 12px;
    padding: 14px 0;
    counter-increment: step;
  }

  li + li {
    border-top: 1px solid var(--line);
  }

  li::before {
    content: counter(step);
    flex: none;
    width: 26px;
    height: 26px;
    display: grid;
    place-items: center;
    border-radius: 50%;
    border: 1px solid var(--line-strong);
    font-size: 13px;
    font-weight: 600;
    color: var(--soft);
  }

  h3 {
    font-family: var(--font-sans);
    font-size: 15px;
    font-weight: 600;
  }

  li p {
    font-size: 14px;
  }

  li > button {
    flex: none;
    min-width: 120px;
    justify-content: center;
  }

  select {
    flex: none;
    font: inherit;
    font-size: 14px;
    color: var(--text);
    background: var(--ink-deep);
    border: 1px solid var(--line-strong);
    border-radius: var(--radius-sm);
    padding: 7px 10px;
    max-width: 170px;
  }

  select:focus-visible,
  input:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .packs {
    display: flex;
    flex-direction: column;
    gap: 0;
    padding: 12px 14px;
    border: 1px dashed var(--line);
    border-radius: var(--radius-sm);
  }

  .packs.dropping {
    border-color: var(--amber);
  }

  .head {
    padding-bottom: 8px;
  }

  .pack {
    display: flex;
    align-items: center;
    gap: 12px;
    padding: 10px 0;
    border-top: 1px solid var(--line);
  }

  .pack.off .name {
    color: var(--dim);
  }

  .name {
    font-size: 14px;
    font-weight: 600;
  }

  .small-text {
    font-size: 13px;
  }

  .empty {
    font-size: 14px;
    padding: 8px 0;
  }

  .icon {
    flex: none;
    padding: 4px 8px;
    font-size: 12px;
  }

  .remove {
    color: var(--err);
  }

  .failed {
    color: var(--err);
  }

  .sdk {
    display: flex;
    flex-direction: column;
    gap: 10px;
    padding-top: 14px;
    border-top: 1px solid var(--line);
    font-size: 13px;
  }

  pre {
    margin: 0;
    padding: 12px 14px;
    overflow-x: auto;
    background: var(--ink-deep);
    border: 1px solid var(--line);
    border-radius: var(--radius-sm);
    font-family: var(--font-mono);
    font-size: 12.5px;
    color: var(--body);
  }

  .link {
    border: 0;
    background: none;
    padding: 0;
    color: var(--amber);
    font-weight: 600;
  }
</style>
