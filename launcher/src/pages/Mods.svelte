<script lang="ts">
  // Mods, one game at a time, with the game list on the left as in Games: every game gets the
  // same page. For a game with a port, its gameplay mods first (PluginsPanel: native mods, and
  // Purist mode). "Your mods" next: the game's packs (PacksPanel) and Play with mods, the page's one
  // main button. Then "Make your own", in two tabs: Levels (edit them in Godot, pack the edits)
  // and Models and textures (AssetsPanel). A game the editor cannot open yet says so, and where to
  // follow it. A game it reads in early access (Going Commando, Up Your Arsenal, Deadlocked: no
  // pack or play action of theirs yet) says what works and what does not. The disc image is never
  // changed.
  // The version is the one the game page plays (playerVersion: the player's own disc), so every
  // disc goes the same way and no region is named; packs are shared by the discs
  // (editor/overrides.py stacks them for the one played). With developer tools on: the version,
  // its folders, and the command behind each step under a "Show command" toggle.
  import AssetsPanel from "$components/AssetsPanel.svelte";
  import CommandBox from "$components/CommandBox.svelte";
  import GamesFrame from "$components/GamesFrame.svelte";
  import Icon from "$components/Icon.svelte";
  import PacksPanel from "$components/PacksPanel.svelte";
  import PluginsPanel from "$components/PluginsPanel.svelte";
  import { api, pickFile, type ActionView, type GameView, type VersionView } from "$lib/api";
  import { app, guard, jobs, run, runChain, saveConfig, toast } from "$lib/app.svelte";
  import { levelName, levelsOf, type EntryState } from "$lib/labels";
  import { GODOT_URL, SITE_URL } from "$lib/links";
  import { playerVersion } from "$lib/player.svelte";
  import type { Job } from "$lib/queue";
  import { enter } from "$lib/motion";
  import { tablist } from "$lib/tabs";
  import { themeStyle } from "$lib/themes";
  import { fade, fly } from "svelte/transition";

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

  const standalone = $derived(app.info?.standalone ?? false);

  /** A version's folders: its level projects and packed levels. In a checkout they are its own
   *  (launcher/actions.json); an installed launcher's are in the game's data folder, where
   *  tools/bundle_standalone.py points its actions (install.rs, data_dir). */
  const folders = (v: VersionView) => {
    if (standalone && app.config?.installDir) {
      const data = `${app.config.installDir}/active/${v.game}/data`;
      return {
        projects: `${data}/levels/${v.name}`,
        packed: `${data}/packed/${v.name}`,
        levels: `${data}/port/${v.name}`,
      };
    }
    return {
      projects: `assets/levels/${v.game}/${v.name}`,
      packed: `assets/packed/${v.game}/${v.name}`,
      levels: `assets/port/${v.game}/${v.name}`,
    };
  };

  /** Opens one of those folders: a checkout's through its root, an installed launcher's in the data folder. */
  const openFolder = (v: VersionView, path: string) =>
    guard(standalone ? api.openGameFolder(v.game, path) : api.openPath(path));

  const live = (j: Job) => j.state === "queued" || j.state === "running";
  const jobsOf = (v: VersionView, ids: string[]) =>
    jobs.list.filter((j) => j.scope.kind === "version" && j.scope.key === v.key && ids.includes(j.actionId));
  const running = (v: VersionView, ids: string[]) => jobsOf(v, ids).find(live) ?? null;
  const needs = (a: ActionView | null) => (a?.blockers ?? []).map((b) => b.replace(/^needs /, ""));

  /** Early access: the editor reads the game's levels, but they are not packed back and the game
   *  is not playable yet (the version has no editor-pack, and its play is planned). */
  const early = (v: VersionView) => !action(v, "editor-pack");
  const playable = (v: VersionView) => {
    const play = action(v, "play");
    return !!play && play.state !== "planned";
  };

  /** What the game list says for each game on this tab. */
  function modState(game: GameView): EntryState {
    const v = editable(game);
    if (!v) {
      const pv = playerVersion(game);
      return pv && playable(pv) ? { text: "Gameplay mods", tone: "ok" } : { text: "Not moddable yet", tone: "info" };
    }
    if (
      early(v) &&
      v.status.disc.state === "found" &&
      !running(v, ["editor-extract", "assets-export", "assets-import", "port-levels"])
    ) {
      return { text: "Early access", tone: "info" };
    }
    if (running(v, ["editor-extract", "editor-import", "editor-pack", "assets-export", "assets-import"])) {
      return { text: "Working…", tone: "warn", busy: true };
    }
    if (v.status.disc.state !== "found") return { text: "Set it up first", tone: "warn" };
    return { text: "Ready to mod", tone: "ok" };
  }

  // The game shown: the one picked in the list, else the one the player last opened if it can be
  // modded, else the first that can, else the first.
  let picked = $state<string | null>(null);
  const shown = $derived.by(() => {
    const byId = (id: string | null | undefined) => games.find((g) => g.id === id) ?? null;
    const last = byId(app.version?.split("/")[0]);
    return byId(picked) ?? (last && editable(last) ? last : null) ?? games.find((g) => editable(g)) ?? games[0] ?? null;
  });
  const v = $derived(shown ? editable(shown) : null);
  /** The version the game page plays, when it has a port: its gameplay mods need no level editor,
   *  so an installed launcher without the editor's tools still shows them. */
  const played = $derived.by(() => {
    const pv = shown ? playerVersion(shown) : null;
    return pv && playable(pv) ? pv : null;
  });

  /** Make your own: the level editor, or models and textures. */
  let tab = $state<"levels" | "assets">("levels");
  /** Bumped when Import made a pack: Your mods reads its list again. */
  let reload = $state(0);
  /** Bumped when a native mod was dropped on the packs: Gameplay mods reads its list again. */
  let pluginsReload = $state(0);
  let opening = $state<string | null>(null);
  /** The level the editor opens, per game. Each level is a Godot project of its own. */
  const level = $state<Record<string, number>>({});

  /** Puts the packed levels and the packs that are on together, then starts the game with them. */
  async function playModded(v: VersionView) {
    const play = action(v, "play-edited");
    const stack = action(v, "overrides-stack");
    if (!play) return;
    await runChain({ kind: "version", key: v.key }, stack ? [stack, play] : [play]);
  }

  async function chooseGodot() {
    if (!app.config) return;
    const picked = await pickFile("Godot 4");
    if (!picked) return;
    // Settings checks a pick the same way: the zip, a Godot 3 or another program is said at once,
    // not when a level does not open.
    const check = await guard(api.checkTool("godot", picked));
    if (!check) return;
    if (!check.ok) {
      toast(`That is not a Godot 4 the editor can use: ${check.message}`, "error");
      return;
    }
    await saveConfig({ ...app.config, godot: picked });
  }

  /** Reads the levels from the disc when that has not been done yet, then opens the chosen one in Godot. */
  async function editLevels(v: VersionView) {
    const edit = action(v, "editor-open");
    if (!edit) return;
    const which = level[v.game] ?? levelsOf(v.game)[0]?.number ?? 0;
    opening = v.key;
    const steps = edit.runnable ? [edit] : [action(v, "editor-extract"), edit].filter((a) => a !== null);
    const done = await runChain({ kind: "version", key: v.key }, steps, which);
    opening = null;
    if (done) toast(`Godot is opening ${levelName(v.game, which)}.`);
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

<GamesFrame label="Mods" selected={shown?.id ?? null} onselect={(g) => (picked = g.id)} state={modState}>
  {#if shown}
    {#key shown.id}
      <div class="page" style={themeStyle(shown.id)} in:fly={enter()}>
        <div class="intro">
          <h1>{shown.title}</h1>
          <p class="muted lead">
            {#if v && early(v)}
              Look around its levels in Godot, and make texture and model packs for when it can be played. Your disc
              image is never changed.
            {:else}
              Edit its levels, pack your edits, and play with them. Your disc image is never changed.
            {/if}
          </p>
        </div>

        {#if !v && played}
          <!-- The level editor cannot open it here (an installed launcher without its tools, say), but the
               game plays: its gameplay mods. -->
          <PluginsPanel version={played} reload={pluginsReload} onpack={() => (reload += 1)} />
        {:else if !v}
          <!-- Every game has this page; one the editor cannot open yet says so. -->
          <section class="panel none">
            <h2>Mods are not possible for this game yet</h2>
            <p class="muted">
              The level editor reads Ratchet &amp; Clank, Going Commando, Up Your Arsenal and Deadlocked so far. When it
              can open {shown.title}'s, its packs and the editor appear here, as they do for the others.
            </p>
            <p>
              <button class="link" onclick={() => void guard(api.openUrl(SITE_URL))}
                >Follow the progress on openrac.dev</button
              >
            </p>
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
          {@const dirs = folders(v)}

          {#if early(v)}
            <!-- Honest about early access: what the editor reads, what it does not do yet. -->
            <section class="panel early">
              <h2>Early access</h2>
              <p class="muted">
                The editor reads this game's levels: its scenery, objects and textures open in Godot, and you can make
                texture and model packs from them. Edits in Godot are not packed back into the game yet, and the game is
                not playable yet{playable(v) ? "" : ": your packs are kept for when it is"}. Some objects and pieces of
                scenery are not read yet; each level lists them in its <code>level.json</code>.
              </p>
            </section>
          {/if}

          {#if playable(v)}
            <!-- Native mods need the game's port: they plug into it (docs/port/MODS.md). -->
            <PluginsPanel version={v} reload={pluginsReload} onpack={() => (reload += 1)} />
          {/if}

          <PacksPanel
            version={v}
            {reload}
            onplugin={() => (pluginsReload += 1)}
            commands={[`python3 editor/overrides.py add <pack.zip> --into <data>/overrides`]}
          />

          <section class="panel play">
            <div class="grow">
              <h2>Play with your mods</h2>
              <p class="muted">
                {#if !play && early(v)}
                  {shown.title} is not playable yet. Your packs stay in Your mods until it is.
                {:else if !play}
                  Playing is not available on this computer yet.
                {:else if play.runnable}
                  Starts the game with your packed levels and the packs that are on.
                {:else}
                  Build the game first, on its page.
                {/if}
              </p>
            </div>
            <button class="primary" disabled={!play?.runnable} onclick={() => void playModded(v)}
              ><Icon name="play" size={16} />Play with mods</button
            >
            {#if developer}
              <div class="span">
                <CommandBox
                  lines={[
                    `python3 editor/overrides.py stack --into <data>/overrides --out <data>/mods/${v.name} --packed ${dirs.packed} --levels ${dirs.levels}`,
                    `openrac-${v.key.replace("/", "-")} --data <data> --levels ${dirs.levels} --mods <data>/mods/${v.name}`,
                  ]}
                />
              </div>
            {/if}
          </section>

          <section class="panel make">
            <div class="row head">
              <h2 class="grow">Make your own</h2>
              <div class="tabs" role="tablist" aria-label="Make your own" use:tablist>
                <button
                  role="tab"
                  id="make-tab-levels"
                  aria-controls="make-panel"
                  aria-selected={tab === "levels"}
                  tabindex={tab === "levels" ? 0 : -1}
                  class:on={tab === "levels"}
                  onclick={() => (tab = "levels")}>Levels</button
                >
                <button
                  role="tab"
                  id="make-tab-assets"
                  aria-controls="make-panel"
                  aria-selected={tab === "assets"}
                  tabindex={tab === "assets" ? 0 : -1}
                  class:on={tab === "assets"}
                  onclick={() => (tab = "assets")}>Models and textures</button
                >
              </div>
            </div>

            {#key tab}
              <div
                id="make-panel"
                role="tabpanel"
                aria-labelledby={`make-tab-${tab}`}
                in:fade={{ duration: enter().duration ?? 0 }}
              >
                {#if tab === "levels"}
                  <ol class="steps">
                    <li>
                      <div class="text">
                        <h3>Edit the levels</h3>
                        <p class="muted">
                          {#if noDisc}
                            Set the game up from your disc first; the editor reads the levels from it.
                          {:else if lacksGodot}
                            The levels open in Godot 4, a free editor.
                            <button class="link" onclick={() => void guard(api.openUrl(GODOT_URL))}>Get Godot</button>,
                            then choose where it is.
                          {:else if preparing}
                            {preparing.title}: {preparing.lines.at(-1)?.line ?? "starting…"}
                          {:else if edit?.runnable && early(v)}
                            Look around a level, its objects and textures, in Godot. Each level opens on its own.
                          {:else if edit?.runnable}
                            Move, turn or scale a level's objects in Godot, and save. Each level opens on its own.
                          {:else if early(v)}
                            The first time, the levels are read from your disc: a few minutes and a few GB.
                          {:else}
                            The first time, the levels are read from your disc: about 20 seconds and 800 MB.
                          {/if}
                        </p>
                      </div>
                      <div class="controls">
                        {#if lacksGodot && !noDisc}
                          <button onclick={() => void chooseGodot()}
                            ><Icon name="folder" size={16} />Choose Godot</button
                          >
                        {:else}
                          <select
                            aria-label="Level to edit"
                            disabled={noDisc}
                            value={level[v.game] ?? levelsOf(v.game)[0]?.number ?? 0}
                            onchange={(e) => (level[v.game] = Number(e.currentTarget.value))}
                          >
                            {#each levelsOf(v.game) as l (l.number)}<option value={l.number}>{l.name}</option>{/each}
                          </select>
                          <button
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
                      </div>
                      {#if developer}
                        <div class="span">
                          <CommandBox lines={[`python3 editor/extract.py godot <disc.iso> ${dirs.projects} --split`]} />
                        </div>
                      {/if}
                    </li>
                    <li>
                      <div class="text">
                        <h3>Pack your edits</h3>
                        <p class="muted" class:failed={result?.failed}>
                          {#if !pack && early(v)}
                            Not yet for {shown.title}: packing edits back into the game is Ratchet &amp; Clank's only so
                            far. Texture and model packs work (Models and textures).
                          {:else if packing}
                            {packing.lines.at(-1)?.line ?? "Reading your levels…"}
                          {:else if result}
                            {result.text}
                          {:else}
                            Turns what you changed into the game's own level data, in a folder of its own.
                          {/if}
                        </p>
                      </div>
                      <div class="controls">
                        <button disabled={!pack?.runnable || !!packing} onclick={() => pack && void run(scope, pack)}>
                          {#if packing}<span class="spinner"></span>{:else}<Icon name="save" size={16} />{/if}Pack
                        </button>
                      </div>
                      {#if developer}
                        <div class="span">
                          <CommandBox
                            lines={[`python3 editor/extract.py pack <disc.iso> ${dirs.projects} ${dirs.packed}`]}
                          />
                        </div>
                      {/if}
                    </li>
                  </ol>
                {:else}
                  <AssetsPanel version={v} onimported={() => reload++} />
                {/if}
              </div>
            {/key}

            {#if developer}
              <div class="sdk">
                <p class="dim">
                  This is <code>{v.key}</code>, the version of the disc set up. Level projects:
                  <code>{dirs.projects}/level_NN</code>; packed levels: <code>{dirs.packed}</code>.
                </p>
                <div class="row">
                  <button class="small" onclick={() => void openFolder(v, dirs.projects)}
                    ><Icon name="folder" size={14} />Level projects</button
                  >
                  <button class="small" onclick={() => void openFolder(v, dirs.packed)}
                    ><Icon name="folder" size={14} />Packed levels</button
                  >
                  <button class="small" onclick={() => void guard(api.openPath("editor/README.md"))}
                    ><Icon name="book" size={14} />Editor documentation</button
                  >
                </div>
              </div>
            {/if}
          </section>
        {/if}
      </div>
    {/key}
  {/if}
</GamesFrame>

<style>
  .page {
    max-width: 960px;
    margin: 0 auto;
    padding: var(--page-top, 24px) var(--page-x, 24px) var(--page-bottom, 48px);
    display: flex;
    flex-direction: column;
    gap: var(--page-gap, 16px);
  }

  .intro {
    display: flex;
    flex-direction: column;
    gap: 4px;
  }

  /* The game's name in its own typeface and colour, as on its page. */
  h1 {
    font-family: var(--title-font);
    font-weight: var(--title-weight);
    color: var(--tx);
  }

  .none {
    display: flex;
    flex-direction: column;
    gap: 10px;
  }

  .none p,
  .early p {
    margin: 0;
    max-width: 68ch;
  }

  /* Early access: dashed, as what is planned and not there yet (docs/STYLE.md). */
  .early {
    display: flex;
    flex-direction: column;
    gap: 8px;
    border-style: dashed;
  }

  .early h2 {
    font-size: 1rem;
  }

  /* Play with mods: one row, its button in the same column as the steps' below. */
  .play {
    display: grid;
    grid-template-columns: minmax(0, 1fr) 200px;
    align-items: center;
    gap: 10px 18px;
  }

  .play h2,
  .steps h3 {
    font-family: var(--font-sans);
    font-size: 0.9375rem;
    font-weight: 600;
    color: var(--text);
  }

  .play p,
  .steps p {
    margin: 2px 0 0;
    font-size: 0.875rem;
  }

  .play > button {
    justify-content: center;
  }

  .span {
    grid-column: 1 / -1;
  }

  .make {
    display: flex;
    flex-direction: column;
    gap: 12px;
  }

  /* The size of its neighbour's, Your mods (PacksPanel). */
  .make h2 {
    font-size: 1rem;
  }

  .head {
    flex-wrap: wrap;
    gap: 10px;
  }

  /* Make your own's two tabs: the header's round tabs, on the panel. */
  .tabs {
    display: flex;
    gap: 4px;
    padding: 3px;
    border: 1px solid var(--line-strong);
    border-radius: 999px;
  }

  .tabs button {
    border: 0;
    background: none;
    padding: 5px 14px;
    font-size: 0.8125rem;
    font-weight: 500;
    color: var(--soft);
  }

  .tabs button.on {
    background: var(--panel-hi);
    color: var(--amber);
  }

  /* The steps: numbered rows with a rule between them; the controls in a column of their own. */
  .steps {
    margin: 0;
    padding: 0;
    list-style: none;
    counter-reset: step;
  }

  .steps li {
    display: grid;
    grid-template-columns: 26px minmax(0, 1fr) auto;
    align-items: center;
    gap: 8px 16px;
    padding: 14px 0;
    counter-increment: step;
  }

  .steps li + li {
    border-top: 1px solid var(--line);
  }

  .steps li::before {
    content: counter(step);
    width: 26px;
    height: 26px;
    display: grid;
    place-items: center;
    border-radius: 50%;
    border: 1px solid var(--line-strong);
    font-size: 0.8125rem;
    font-weight: 600;
    color: var(--soft);
  }

  .steps .span {
    grid-column: 2 / -1;
  }

  .controls {
    display: flex;
    flex-wrap: wrap;
    justify-content: flex-end;
    gap: 8px;
  }

  .controls button {
    min-width: 170px;
    justify-content: center;
  }

  select {
    font: inherit;
    font-size: 0.875rem;
    color: var(--text);
    background: var(--ink-deep);
    border: 1px solid var(--line-strong);
    border-radius: var(--radius-sm);
    padding: 7px 10px;
  }

  select:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  @media (max-width: 900px) {
    .steps li {
      grid-template-columns: 26px minmax(0, 1fr);
    }

    .controls {
      grid-column: 2;
      justify-content: flex-start;
    }

    .play {
      grid-template-columns: minmax(0, 1fr);
    }
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
    font-size: 0.8125rem;
  }

  .sdk p {
    margin: 0;
  }

  .link {
    border: 0;
    background: none;
    padding: 0;
    color: var(--amber);
    font-weight: 600;
  }
</style>
