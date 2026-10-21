<script lang="ts">
  // Mods, one game at a time, with the game list on the left as in Games: every game gets the
  // same page. Its head: the game's name and Play with mods, the page's one main button, in the
  // same place on every tab. Then three tabs, so that each of the player's jobs has a place of
  // its own that can be seen from the top: My mods (AddMod: a link, a file or a folder; the
  // gameplay mods with Purist mode, PluginsPanel; the texture, model and level packs,
  // PacksPanel), Level editor (open a level in Godot, pack the changes) and Models and textures
  // (AssetsPanel). A game the editor cannot open but that plays has My mods alone; one the editor
  // cannot open yet says so, and where to follow it. A game it reads in early access (Going
  // Commando, Up Your Arsenal, Deadlocked: no pack or play action of theirs yet) says so in a line.
  // The words are lib/modsText.ts's. The disc image is never changed.
  // The version is the one the game page plays (playerVersion: the player's own disc), so every
  // disc goes the same way and no region is named; packs are shared by the discs
  // (editor/overrides.py stacks them for the one played). With developer tools on: the version,
  // its folders, and the command behind each step under a "Show command" toggle.
  import AddMod from "$components/AddMod.svelte";
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
  import { EDITOR, GAME_LIST, PAGE } from "$lib/modsText";
  import { foundDisc, playerVersion, setUp } from "$lib/player.svelte";
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
      return pv && playable(pv)
        ? { text: GAME_LIST.gameplayMods, tone: "ok" }
        : { text: GAME_LIST.notModdable, tone: "info" };
    }
    if (
      early(v) &&
      v.status.disc.state === "found" &&
      !running(v, ["editor-extract", "assets-export", "assets-import", "port-levels"])
    ) {
      return { text: GAME_LIST.early, tone: "info" };
    }
    if (running(v, ["editor-extract", "editor-import", "editor-pack", "assets-export", "assets-import"])) {
      return { text: GAME_LIST.working, tone: "warn", busy: true };
    }
    if (v.status.disc.state !== "found") return { text: GAME_LIST.setUp, tone: "warn" };
    return { text: GAME_LIST.ready, tone: "ok" };
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

  /** The page's tab: My mods, the level editor, or models and textures. */
  type Tab = "mine" | "editor" | "assets";
  let tab = $state<Tab>("mine");
  const TABS: { id: Tab; label: string; icon: "library" | "edit" | "folder" }[] = [
    { id: "mine", label: PAGE.tabMine, icon: "library" },
    { id: "editor", label: PAGE.tabEditor, icon: "edit" },
    { id: "assets", label: PAGE.tabAssets, icon: "folder" },
  ];
  /** Bumped when a pack was added or imported: the packs' list is read again. */
  let reload = $state(0);
  /** Bumped when a gameplay mod was added: their list is read again. */
  let pluginsReload = $state(0);
  const added = (what: { plugin: boolean; pack: boolean }) => {
    if (what.plugin) pluginsReload += 1;
    if (what.pack) reload += 1;
  };
  let opening = $state<string | null>(null);
  /** The level the editor opens, per game. Each level is a Godot project of its own. */
  const level = $state<Record<string, number>>({});

  /** Play with mods, the page's main action. */
  const play = $derived(v ? action(v, "play-edited") : null);

  /** Puts the packed levels and the packs that are on together, then starts the game with them. */
  async function playModded(v: VersionView) {
    const play = action(v, "play-edited");
    const stack = action(v, "overrides-stack");
    if (!play) return;
    await runChain({ kind: "version", key: v.key }, stack ? [stack, play] : [play]);
  }

  async function chooseGodot() {
    if (!app.config) return;
    const picked = await pickFile(EDITOR.pickGodot);
    if (!picked) return;
    // Settings checks a pick the same way: the zip, a Godot 3 or another program is said at once,
    // not when a level does not open.
    const check = await guard(api.checkTool("godot", picked));
    if (!check) return;
    if (!check.ok) {
      toast(EDITOR.notGodot(check.message), "error");
      return;
    }
    await saveConfig({ ...app.config, godot: picked });
  }

  /** Opens the chosen level in Godot (api.openLevelInGodot); the first time, reads the levels from
   *  the disc before. */
  async function editLevels(v: VersionView) {
    const edit = action(v, "editor-open");
    if (!edit) return;
    const which = level[v.game] ?? levelsOf(v.game)[0]?.number ?? 0;
    opening = v.key;
    let done: boolean;
    if (edit.runnable) {
      done = (await guard(api.openLevelInGodot(v.key, which))) !== undefined;
    } else {
      const extract = action(v, "editor-extract");
      done = await runChain({ kind: "version", key: v.key }, extract ? [extract, edit] : [edit], which);
    }
    opening = null;
    if (done) toast(EDITOR.opening(levelName(v.game, which)));
  }

  /** The last pack, from the line the packer printed ("Packed 1 level(s), 3 moby edit(s): …"), and
   *  the folder it wrote, for Open folder. */
  function packed(v: VersionView): { text: string; failed: boolean; folder: string | null } | null {
    const last = jobsOf(v, ["editor-pack"]).at(-1);
    if (!last || live(last)) return null;
    if (last.state !== "succeeded") return { text: EDITOR.packStopped, failed: true, folder: null };
    const folder = last.outputs[0] ?? null;
    const line = last.lines.findLast((l) => l.line.startsWith("Packed "))?.line ?? "";
    const found = /^Packed (\d+) level\(s\), (\d+) moby edit\(s\)/.exec(line);
    if (!found) return { text: EDITOR.packed, failed: false, folder };
    const [levels, edits] = [Number(found[1]), Number(found[2])];
    return { text: levels ? EDITOR.packedSome(edits, levels) : EDITOR.packedNone, failed: false, folder };
  }
</script>

<GamesFrame label="Mods" selected={shown?.id ?? null} onselect={(g) => (picked = g.id)} state={modState}>
  {#if shown}
    {#key shown.id}
      <div class="page" style={themeStyle(shown.id)} in:fly={enter()}>
        <div class="intro">
          <div class="grow">
            <h1>{shown.title}</h1>
            {#if v ? !early(v) : played}<p class="muted lead">{PAGE.lead}</p>{/if}
          </div>
          {#if v}
            <!-- The page's one main action, in the same place on every tab. -->
            <div class="play">
              <button class="primary" disabled={!play?.runnable} onclick={() => void playModded(v)}
                ><Icon name="play" size={16} />{PAGE.play}</button
              >
              {#if !play?.runnable}
                <span class="dim why-not"
                  >{!play && early(v) ? PAGE.playEarly : !play ? PAGE.playMissing : PAGE.playBuild}</span
                >
              {/if}
            </div>
          {/if}
        </div>

        {#if !v && played}
          <!-- The level editor cannot open it here (an installed launcher without its tools, say), but the
               game plays: its gameplay mods, which load when it is played from its page. -->
          <AddMod version={played} onadded={added} />
          <PluginsPanel version={played} reload={pluginsReload} onpack={() => (reload += 1)} />
        {:else if !v}
          <!-- Every game has this page; one the editor cannot open yet says so. -->
          <section class="panel none">
            <h2>{PAGE.noneTitle}</h2>
            <p class="muted">{PAGE.noneLine}</p>
            <p>
              <button class="link" onclick={() => void guard(api.openUrl(SITE_URL))}
                >{PAGE.noneLink}<Icon name="external" size={14} /></button
              >
            </p>
          </section>
        {:else}
          {@const scope = { kind: "version" as const, key: v.key }}
          {@const edit = action(v, "editor-open")}
          {@const pack = action(v, "editor-pack")}
          {@const adder = action(v, "overrides-add")}
          {@const preparing = running(v, ["editor-extract", "editor-import"])}
          {@const packing = running(v, ["editor-pack"])}
          {@const result = packed(v)}
          {@const lacksGodot = needs(edit).some((b) => b.startsWith("Godot"))}
          {@const noDisc = v.status.disc.state !== "found"}
          {@const dirs = folders(v)}

          {#if early(v)}
            <!-- Honest about early access, in a line: what works, what comes later. -->
            <p class="early"><strong>{PAGE.earlyTitle}</strong><span class="muted">{PAGE.earlyLine}</span></p>
          {/if}

          <div class="tabs" role="tablist" aria-label={PAGE.tabsLabel} use:tablist>
            {#each TABS as t (t.id)}
              <button
                role="tab"
                id={`mods-tab-${t.id}`}
                aria-controls="mods-panel"
                aria-selected={tab === t.id}
                tabindex={tab === t.id ? 0 : -1}
                class:on={tab === t.id}
                onclick={() => (tab = t.id)}><Icon name={t.icon} size={16} />{t.label}</button
              >
            {/each}
          </div>

          {#key tab}
            <div
              id="mods-panel"
              class="tabpanel"
              role="tabpanel"
              aria-labelledby={`mods-tab-${tab}`}
              in:fade={{ duration: enter().duration ?? 0 }}
            >
              {#if tab === "mine"}
                <AddMod version={v} links={playable(v)} files={playable(v) || !!adder?.runnable} onadded={added} />
                {#if playable(v)}
                  <!-- Gameplay mods need the game's port: they plug into it (docs/port/MODS.md). -->
                  <PluginsPanel version={v} reload={pluginsReload} onpack={() => (reload += 1)} />
                {/if}
                <PacksPanel
                  version={v}
                  {reload}
                  commands={[`python3 editor/overrides.py add <pack.zip> --into <data>/overrides`]}
                />
                <p class="make-own">
                  <span class="muted">{PAGE.makeOwn}</span>
                  <button class="small" onclick={() => (tab = "editor")}
                    ><Icon name="edit" size={14} />{PAGE.makeOwnButton}</button
                  >
                </p>
              {:else if tab === "editor"}
                <section class="panel">
                  <p class="muted tab-lead">{early(v) ? EDITOR.leadEarly : EDITOR.lead}</p>
                  <ol class="steps">
                    <li>
                      <div class="text">
                        <h3>{EDITOR.openTitle}</h3>
                        <p class="muted">
                          {#if noDisc}
                            {EDITOR.noDisc}
                          {:else if lacksGodot}
                            {EDITOR.noGodot}
                          {:else if preparing}
                            {preparing.title}: {preparing.lines.at(-1)?.line ?? "…"}
                          {:else if edit?.runnable}
                            {early(v) ? EDITOR.readyEarly : EDITOR.ready}
                          {:else}
                            {early(v) ? EDITOR.firstTimeEarly : EDITOR.firstTime}
                          {/if}
                        </p>
                      </div>
                      <div class="controls">
                        {#if noDisc}
                          <button onclick={() => void setUp(shown.id, shown.title, foundDisc(shown.id)?.path)}
                            ><Icon name="disc" size={16} />{EDITOR.setUp}</button
                          >
                        {:else if lacksGodot}
                          <button class="ghost" onclick={() => void guard(api.openUrl(GODOT_URL))}
                            ><Icon name="external" size={16} />{EDITOR.getGodot}</button
                          >
                          <button onclick={() => void chooseGodot()}
                            ><Icon name="folder" size={16} />{EDITOR.chooseGodot}</button
                          >
                        {:else}
                          <select
                            aria-label={EDITOR.levelLabel}
                            value={level[v.game] ?? levelsOf(v.game)[0]?.number ?? 0}
                            onchange={(e) => (level[v.game] = Number(e.currentTarget.value))}
                          >
                            {#each levelsOf(v.game) as l (l.number)}<option value={l.number}>{l.name}</option>{/each}
                          </select>
                          <button disabled={opening === v.key || !!preparing} onclick={() => void editLevels(v)}>
                            {#if opening === v.key || preparing}<span class="spinner"></span>{:else}<Icon
                                name="edit"
                                size={16}
                              />{/if}
                            {edit?.runnable ? EDITOR.open : EDITOR.prepare}
                          </button>
                        {/if}
                      </div>
                    </li>
                    <li>
                      <div class="text">
                        <h3>{EDITOR.packTitle}</h3>
                        <p class="muted" class:failed={result?.failed} class:done={result && !result.failed}>
                          {#if !pack && early(v)}
                            {EDITOR.packEarly}
                          {:else if packing}
                            {packing.lines.at(-1)?.line ?? EDITOR.packReading}
                          {:else if result}
                            {result.text}
                          {:else}
                            {EDITOR.packLine}
                          {/if}
                        </p>
                      </div>
                      <div class="controls">
                        {#if result?.folder}
                          {@const folder = result.folder}
                          <button class="ghost" onclick={() => void guard(api.openOutput(folder))}
                            ><Icon name="folder" size={16} />{EDITOR.openFolder}</button
                          >
                        {/if}
                        <button disabled={!pack?.runnable || !!packing} onclick={() => pack && void run(scope, pack)}>
                          {#if packing}<span class="spinner"></span>{:else}<Icon
                              name="save"
                              size={16}
                            />{/if}{EDITOR.pack}
                        </button>
                      </div>
                    </li>
                  </ol>
                </section>
              {:else}
                <section class="panel">
                  <AssetsPanel version={v} onimported={() => reload++} />
                </section>
              {/if}
            </div>
          {/key}

          {#if developer && tab !== "assets"}
            <!-- The developer view: kept apart from the player's, under the tabs. -->
            <section class="panel sdk">
              <h2>{PAGE.developerTitle}</h2>
              <p class="dim">
                This is <code>{v.key}</code>, the version of the disc set up. Level projects:
                <code>{dirs.projects}/level_NN</code>; packed levels: <code>{dirs.packed}</code>.
              </p>
              <CommandBox
                lines={[
                  ...(tab === "editor"
                    ? [
                        `python3 editor/extract.py godot <disc.iso> ${dirs.projects} --split`,
                        `python3 editor/extract.py pack <disc.iso> ${dirs.projects} ${dirs.packed}`,
                      ]
                    : []),
                  `python3 editor/overrides.py stack --into <data>/overrides --out <data>/mods/${v.name} --packed ${dirs.packed} --levels ${dirs.levels}`,
                  `openrac-${v.key.replace("/", "-")} --data <data> --levels ${dirs.levels} --mods <data>/mods/${v.name}`,
                ]}
              />
              <div class="row">
                <button class="small" onclick={() => void openFolder(v, dirs.projects)}
                  ><Icon name="folder" size={14} />{EDITOR.projects}</button
                >
                <button class="small" onclick={() => void openFolder(v, dirs.packed)}
                  ><Icon name="folder" size={14} />{EDITOR.packedFolder}</button
                >
                <button class="small" onclick={() => void guard(api.openPath("editor/README.md"))}
                  ><Icon name="book" size={14} />{EDITOR.docs}</button
                >
              </div>
            </section>
          {/if}
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

  /* The head: the game's name, and Play with mods at its right. */
  .intro {
    display: flex;
    align-items: flex-start;
    gap: 16px 24px;
    flex-wrap: wrap;
  }

  .lead {
    margin: 4px 0 0;
  }

  /* The game's name in its own typeface and colour, as on its page. */
  h1 {
    font-family: var(--title-font);
    font-weight: var(--title-weight);
    color: var(--tx);
  }

  .play {
    display: flex;
    flex-direction: column;
    align-items: flex-end;
    gap: 6px;
  }

  .play button {
    min-width: 200px;
    padding: 10px 22px;
    justify-content: center;
  }

  .why-not {
    font-size: 0.8125rem;
  }

  .none {
    display: flex;
    flex-direction: column;
    gap: 10px;
  }

  .none h2 {
    font-size: 1rem;
  }

  .none p {
    margin: 0;
    max-width: 68ch;
  }

  /* Early access: one line, dashed, as what is not there yet (docs/STYLE.md). */
  .early {
    display: flex;
    flex-wrap: wrap;
    gap: 4px 10px;
    margin: 0;
    padding: 10px 16px;
    border: 1px dashed var(--line-strong);
    border-radius: var(--radius-sm);
    font-size: 0.875rem;
  }

  .early strong {
    color: var(--gold);
  }

  /* The page's tabs: the header's round tabs, larger, with an icon each, so the level editor can be
     seen from the top of the page. */
  .tabs {
    display: flex;
    align-self: flex-start;
    flex-wrap: wrap;
    gap: 4px;
    padding: 4px;
    border: 1px solid var(--line-strong);
    border-radius: 999px;
    background: var(--panel);
  }

  .tabs button {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    border: 0;
    background: none;
    padding: 8px 18px;
    font-size: 0.875rem;
    font-weight: 600;
    color: var(--soft);
    transition:
      color var(--fast) var(--ease),
      background var(--fast) var(--ease);
  }

  .tabs button.on {
    background: var(--panel-hi);
    color: var(--amber);
    box-shadow: inset 0 0 0 1px var(--line-strong);
  }

  @media (hover: hover) {
    .tabs button:hover:not(.on) {
      color: var(--text);
    }
  }

  .tabs button:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .tabpanel {
    display: flex;
    flex-direction: column;
    gap: var(--page-gap, 16px);
  }

  .make-own {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    justify-content: center;
    gap: 10px;
    margin: 4px 0 0;
    font-size: 0.875rem;
  }

  .tab-lead {
    margin: 0 0 4px;
    font-size: 0.875rem;
  }

  .failed {
    color: var(--err);
  }

  .done {
    color: var(--ok);
  }

  .sdk {
    display: flex;
    flex-direction: column;
    gap: 10px;
    border-style: dashed;
    font-size: 0.8125rem;
  }

  .sdk h2 {
    font-size: 0.875rem;
  }

  .sdk p {
    margin: 0;
  }

  .link {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    border: 0;
    background: none;
    padding: 0;
    color: var(--amber);
    font-weight: 600;
  }
</style>
