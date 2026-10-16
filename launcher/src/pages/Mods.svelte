<script lang="ts">
  // Modding: for each game that can be edited, the three things a modder does,
  // in order. Edit the levels in Godot, pack the edits into the game's own
  // level data, play the game with them. The disc image is never changed.
  // The version is the one the game page plays (playerVersion: the player's own
  // disc), so every disc goes the same way and no region is named; packs are
  // shared by the discs (editor/overrides.py stacks them for the one played).
  // With developer tools on: the version, the folders and the commands behind each step.
  import AssetsPanel from "$components/AssetsPanel.svelte";
  import Icon from "$components/Icon.svelte";
  import { api, pickFile, type ActionView, type GameView, type VersionView } from "$lib/api";
  import { app, guard, jobs, run, runChain, saveConfig, toast } from "$lib/app.svelte";
  import { RAC1_LEVELS } from "$lib/labels";
  import { GODOT_URL } from "$lib/links";
  import { playerVersion } from "$lib/player.svelte";
  import type { Job } from "$lib/queue";

  const developer = $derived(app.config?.developer ?? false);
  const games = $derived(app.library?.games ?? []);

  const action = (v: VersionView, id: string): ActionView | null =>
    v.actions.find((a) => a.id === id && a.thisPlatform) ?? null;

  /** The version of a game the player modifies, the one its page plays, if the level editor can open it. */
  function editable(game: GameView): VersionView | null {
    const v = playerVersion(game);
    const extract = v ? action(v, "editor-extract") : null;
    return v && extract && extract.state !== "planned" ? v : null;
  }

  /** A version's folders in a checkout (launcher/actions.json): its level projects and packed levels. */
  const folders = (v: VersionView) => {
    const tree = v.name === "pal" ? v.game : `${v.game}-${v.name}`;
    return { projects: `assets/levels/${tree}`, packed: `assets/packed/${tree}` };
  };

  const live = (j: Job) => j.state === "queued" || j.state === "running";
  const jobsOf = (v: VersionView, ids: string[]) =>
    jobs.list.filter((j) => j.scope.kind === "version" && j.scope.key === v.key && ids.includes(j.actionId));
  const running = (v: VersionView, ids: string[]) => jobsOf(v, ids).find(live) ?? null;
  const needs = (a: ActionView | null) => (a?.blockers ?? []).map((b) => b.replace(/^needs /, ""));

  let opening = $state<string | null>(null);

  /** Puts the packed levels and the packs that are on together, then starts the game with them. */
  async function playModded(v: VersionView) {
    const play = action(v, "play-edited");
    const stack = action(v, "overrides-stack");
    if (!play) return;
    await runChain({ kind: "version", key: v.key }, stack ? [stack, play] : [play]);
  }
  /** The level the editor opens, per game. Each level is a Godot project of its own. */
  const level = $state<Record<string, number>>({});

  async function chooseGodot() {
    if (!app.config) return;
    const picked = await pickFile("Godot 4");
    if (picked) await saveConfig({ ...app.config, godot: picked });
  }

  /** Reads the levels from the disc when that has not been done yet, then opens the chosen one in Godot. */
  async function editLevels(v: VersionView) {
    const edit = action(v, "editor-open");
    if (!edit) return;
    const which = level[v.game] ?? 0;
    opening = v.key;
    const steps = edit.runnable ? [edit] : [action(v, "editor-extract"), edit].filter((a) => a !== null);
    const done = await runChain({ kind: "version", key: v.key }, steps, which);
    opening = null;
    if (done) toast(`Godot is opening ${RAC1_LEVELS[which] ?? "your level"}.`);
  }

  /** The last line the packer printed: "Packed 1 level(s), 3 moby edit(s): …". */
  function packed(v: VersionView): { text: string; failed: boolean } | null {
    const last = jobsOf(v, ["editor-pack"]).at(-1);
    if (!last || live(last)) return null;
    if (last.state !== "succeeded") return { text: "Packing stopped. The activity log has why.", failed: true };
    const line = last.lines.findLast((l) => l.line.startsWith("Packed "))?.line ?? "";
    const found = /^Packed (\d+) level\(s\), (\d+) moby edit\(s\)/.exec(line);
    if (!found) return { text: "Packed.", failed: false };
    const [levels, edits] = [Number(found[1]), Number(found[2])];
    return {
      text: levels
        ? `Packed ${edits} edited object${edits === 1 ? "" : "s"} in ${levels} level${levels === 1 ? "" : "s"}.`
        : "Nothing to pack: your levels are as they are on the disc.",
      failed: false,
    };
  }
</script>

<div class="page">
  <h1>Mods</h1>
  <p class="muted lead">Edit a game's levels, pack your edits, and play with them. Your disc image is never changed.</p>

  {#each games as game (game.id)}
    {@const v = editable(game)}
    {#if !v}
      <section class="row closed">
        <h2 class="grow">{game.title}</h2>
        <span class="dim">Cannot be modded yet</span>
      </section>
    {:else}
      {@const scope = { kind: "version" as const, key: v.key }}
      {@const edit = action(v, "editor-open")}
      {@const pack = action(v, "editor-pack")}
      {@const play = action(v, "play-edited")}
      {@const preparing = running(v, ["editor-extract", "editor-import"])}
      {@const packing = running(v, ["editor-pack"])}
      {@const result = packed(v)}
      {@const lacksGodot = needs(edit).some((b) => b.startsWith("Godot"))}
      {@const noDisc = v.status.disc.state !== "found"}
      <section class="panel game">
        <h2>{game.title}</h2>
        <ol>
          <li>
            <div class="grow">
              <h3>Edit the levels</h3>
              <p class="muted">
                {#if noDisc}
                  Set the game up from your disc first; the editor reads the levels from it.
                {:else if lacksGodot}
                  The levels open in Godot 4, a free editor.
                  <button class="link" onclick={() => void guard(api.openUrl(GODOT_URL))}>Get Godot</button>, then
                  choose where it is.
                {:else if preparing}
                  {preparing.title}: {preparing.lines.at(-1)?.line ?? "starting…"}
                {:else if edit?.runnable}
                  Move, turn or scale a level's objects in Godot, and save. Each level opens on its own.
                {:else}
                  The first time, the levels are read from your disc: about 20 seconds and 800 MB.
                {/if}
              </p>
            </div>
            {#if lacksGodot && !noDisc}
              <button onclick={() => void chooseGodot()}><Icon name="folder" size={16} />Choose Godot</button>
            {:else}
              <select
                aria-label="Level to edit"
                disabled={noDisc}
                value={level[v.game] ?? 0}
                onchange={(e) => (level[v.game] = Number(e.currentTarget.value))}
              >
                {#each RAC1_LEVELS as name, i (i)}<option value={i}>{name}</option>{/each}
              </select>
              <button
                class:primary={!result}
                disabled={noDisc || opening === v.key || !!preparing}
                onclick={() => void editLevels(v)}
              >
                {#if opening === v.key || preparing}<span class="spinner"></span>{:else}<Icon
                    name="edit"
                    size={16}
                  />{/if}
                {edit?.runnable ? "Open the editor" : "Prepare the editor"}
              </button>
            {/if}
          </li>
          <li>
            <div class="grow">
              <h3>Pack your edits</h3>
              <p class="muted clip" class:failed={result?.failed}>
                {#if packing}
                  {packing.lines.at(-1)?.line ?? "Reading your levels…"}
                {:else if result}
                  {result.text}
                {:else}
                  Turns what you changed into the game's own level data, in a folder of its own.
                {/if}
              </p>
            </div>
            <button disabled={!pack?.runnable || !!packing} onclick={() => pack && void run(scope, pack)}>
              {#if packing}<span class="spinner"></span>{:else}<Icon name="save" size={16} />{/if}Pack
            </button>
          </li>
          <li>
            <div class="grow">
              <h3>Play with your mods</h3>
              <p class="muted clip">
                {#if !play}
                  Playing is not available on this computer yet.
                {:else if play.runnable}
                  Starts the game with your packed levels and the packs that are on (below).
                {:else}
                  Build the game first, on its page.
                {/if}
              </p>
            </div>
            <button
              class:primary={!!result && !result.failed}
              disabled={!play?.runnable}
              onclick={() => void playModded(v)}><Icon name="play" size={16} />Play</button
            >
          </li>
        </ol>

        {#if developer}
          {@const dirs = folders(v)}
          <div class="sdk">
            <p class="dim">
              This is <code>{v.key}</code>, the version of the disc set up. The projects are
              <code>{dirs.projects}/level_NN</code>, the packed levels <code>{dirs.packed}</code> (a list of disc sectors
              and the files that replace them). The same steps from a shell:
            </p>
            <pre>python3 editor/extract.py godot &lt;disc.iso&gt; {dirs.projects} --split
python3 editor/extract.py pack &lt;disc.iso&gt; {dirs.projects} {dirs.packed}
python3 editor/overrides.py stack --into &lt;data&gt;/overrides --out &lt;data&gt;/mods/{v.name} --packed {dirs.packed} --levels assets/port/{v.game}/{v.name}
openrac-{v.key.replace(
                "/",
                "-",
              )} --data &lt;data&gt; --levels assets/port/{v.game}/{v.name} --mods &lt;data&gt;/mods/{v.name}</pre>
            <div class="row">
              <button class="small" onclick={() => void guard(api.openPath(dirs.projects))}
                ><Icon name="folder" size={14} />Level projects</button
              >
              <button class="small" onclick={() => void guard(api.openPath(dirs.packed))}
                ><Icon name="folder" size={14} />Packed levels</button
              >
              <button class="small" onclick={() => void guard(api.openPath("editor/README.md"))}
                ><Icon name="book" size={14} />Editor documentation</button
              >
            </div>
          </div>
        {/if}
      </section>
      <AssetsPanel version={v} />
    {/if}
  {/each}
</div>

<style>
  .page {
    max-width: 920px;
    margin: 0 auto;
    padding: 28px 28px 60px;
    display: flex;
    flex-direction: column;
    gap: 16px;
  }

  .lead {
    margin-top: -8px;
  }

  .game {
    display: flex;
    flex-direction: column;
    gap: 6px;
  }

  /* The three steps: numbered rows with a rule between them, the button at the end of each. */
  ol {
    margin: 0;
    padding: 0;
    list-style: none;
    counter-reset: step;
  }

  li {
    display: flex;
    align-items: center;
    gap: 18px;
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

  li h3 {
    font-family: var(--font-sans);
    font-size: 15px;
    font-weight: 600;
  }

  li p {
    font-size: 14px;
  }

  li > button {
    flex: none;
    min-width: 170px;
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
    max-width: 190px;
  }

  select:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .failed {
    color: var(--err);
  }

  .closed {
    padding: 12px 22px;
    border: 1px dashed var(--line);
    border-radius: var(--radius);
  }

  .closed h2 {
    font-size: 15px;
    color: var(--soft);
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
