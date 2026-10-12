<script lang="ts">
  // A game as a player meets it. One main button, always in the same place,
  // says the next thing to do (set up from your disc, build, play); what is done
  // less often is in the menu beside it. Below: the save files and the level
  // editor. Each button runs the actions the developer page lists
  // (launcher/actions.json), found by their ids.
  import Icon from "$components/Icon.svelte";
  import Menu, { type MenuItem } from "$components/Menu.svelte";
  import SaveManagerBlock from "$components/SaveManagerBlock.svelte";
  import { api, pickFile, type ActionView } from "$lib/api";
  import {
    app,
    currentVersion,
    guard,
    jobs,
    openVersion,
    run,
    runChain,
    saveConfig,
    toast,
    uninstallGame,
  } from "$lib/app.svelte";
  import { GODOT_URL } from "$lib/links";
  import { isBuilt, playerAct, playerState, regionWords } from "$lib/player.svelte";
  import { themeStyle } from "$lib/themes";

  const v = $derived(currentVersion());
  const game = $derived(app.library?.games.find((g) => g.id === v?.game) ?? null);
  const scope = $derived({ kind: "version" as const, key: v?.key ?? "" });
  const action = (id: string): ActionView | null => v?.actions.find((a) => a.id === id && a.thisPlatform) ?? null;

  const build = $derived(action("port-build"));
  const playGodot = $derived(action("editor-preview"));
  const extract = $derived(action("editor-extract"));
  const bring = $derived(action("editor-import"));
  const edit = $derived(action("editor-open"));

  const GAME_MEDIA: Record<string, { bg: string; gif?: string }> = {
    rac1: { bg: "/img/rac1-bg.webp", gif: "/img/rac1-gameplay.gif" },
    rac2: { bg: "/img/gc-bg.webp", gif: "/img/rac2-gameplay.gif" },
    rac3: { bg: "/img/uya-bg.webp", gif: "/img/rac3-gameplay.gif" },
    rac4: { bg: "/img/deadlocked-bg.webp", gif: "/img/rac4-gameplay.gif" },
  };
  const media = $derived(v ? GAME_MEDIA[v.game] : null);

  const next = $derived(v && game ? playerState(game, v) : null);
  const setUp = $derived(game?.install?.extracted ?? null);
  const built = $derived(!!v && !!game && isBuilt(game, v));
  const disc = $derived(v?.status.disc.state ?? "missing");
  const preparing = $derived(
    jobs.list.find(
      (j) =>
        j.scope.kind === "version" &&
        j.scope.key === v?.key &&
        (j.actionId === "editor-extract" || j.actionId === "editor-import") &&
        (j.state === "queued" || j.state === "running"),
    ),
  );
  /** The steps are shown while there is one left that the player can take. */
  const showSteps = $derived(!!next && next.kind !== "play" && next.kind !== "playing");

  let opening = $state(false);

  async function removeSetUp() {
    if (!v) return;
    const sure = confirm(
      `Remove what was set up from your ${v.title} disc? Your disc image and your saves are kept; set it up again any time.`,
    );
    if (sure) await uninstallGame(v.game);
  }

  async function chooseGodot() {
    if (!app.config) return;
    const picked = await pickFile("Godot 4");
    if (picked) await saveConfig({ ...app.config, godot: picked });
  }

  /** Extracts and imports the levels when the project is not there yet, then opens Godot. */
  async function editLevels() {
    if (!edit) return;
    opening = true;
    const steps = edit.runnable ? [edit] : [extract, bring, edit].filter((a) => a !== null);
    const done = await runChain(scope, steps);
    opening = false;
    if (done) toast("Godot is opening your levels.");
  }

  const lacksGodot = $derived((edit?.blockers ?? []).some((b) => b.replace(/^needs /, "").startsWith("Godot")));

  /** What is done less often than playing, in the menu beside the main button. */
  const menu = $derived.by<MenuItem[]>(() => {
    if (!v) return [];
    const dir = v.dir;
    const items: MenuItem[] = [];
    if (build?.runnable && built && next?.kind === "play") {
      items.push({ label: "Rebuild the game", icon: "build", run: () => void run(scope, build) });
    }
    if (playGodot?.runnable) {
      items.push({ label: "Preview a level in Godot", icon: "play", run: () => void run(scope, playGodot) });
    }
    items.push({ label: "Open the game's folder", icon: "folder", run: () => void guard(api.openPath(dir)) });
    items.push({ label: "Activity log", icon: "terminal", run: () => (app.page = "tasks") });
    if (setUp) items.push({ label: "Remove the set-up", icon: "trash", danger: true, run: () => void removeSetUp() });
    return items;
  });
</script>

{#if !v || !game || !next}
  <div class="page">
    <p class="muted">That game is not in this OpenRAC folder.</p>
    <button onclick={() => (app.page = "library")}>Back to the games</button>
  </div>
{:else}
  <div class="page" style={themeStyle(v.game)}>
    {#if media}
      <div class="page-ambient-bg" aria-hidden="true">
        <img src={media.bg} alt="" />
      </div>
    {/if}

    <button class="ghost back" onclick={() => (app.page = "library")}><Icon name="arrow" size={16} />Games</button>

    <section class="hero rise" aria-label={`${v.title} (${v.region})`}>
      {#if media}
        <div class="hero-backdrop" aria-hidden="true">
          <img src={media.gif ?? media.bg} alt="" class="hero-bg" />
          <div class="hero-overlay"></div>
        </div>
      {/if}
      <div class="title box">
        {v.title}
        {#if game.year}<small>{game.year}</small>{/if}
      </div>
      {#if game.versions.length > 1}
        <div class="versions" role="tablist" aria-label="Your disc's region">
          {#each game.versions as other (other.key)}
            <button
              role="tab"
              aria-selected={other.key === v.key}
              class:on={other.key === v.key}
              onclick={() => {
                openVersion(other.key);
              }}>{regionWords(other)}</button
            >
          {/each}
        </div>
      {/if}

      <div class="go">
        <div class="go-row">
          <button
            class="big"
            class:primary={next.primary}
            disabled={next.disabled}
            onclick={() => void playerAct(v, next)}
          >
            {#if next.job}<span class="spinner"></span>{:else}<Icon name={next.icon} size={22} />{/if}{next.label}
          </button>
          <Menu items={menu} />
        </div>
        {#if next.hint}<p class="line" class:clip={!!next.job}>{next.hint}</p>{/if}
      </div>

      {#if showSteps}
        <ol class="steps" aria-label="Setting the game up">
          <li class:done={!!setUp}>Disc set up</li>
          {#if next.kind !== "unavailable" && (next.kind !== "set-up" || next.primary)}
            <li class:done={built}>Game built</li>
          {/if}
        </ol>
      {/if}
    </section>

    <SaveManagerBlock version={v} />

    <section class="panel editor">
      <div class="grow">
        <h2>Level editor</h2>
        {#if !extract || extract.state === "planned"}
          <p class="muted">The level editor cannot open this game yet.</p>
        {:else if disc !== "found"}
          <p class="muted">Set up your disc first; the editor reads the levels from it.</p>
        {:else if lacksGodot}
          <p class="muted">
            The levels open in Godot 4, a free editor.
            <button class="link" onclick={() => void guard(api.openUrl(GODOT_URL))}>Get Godot</button>, then choose
            where it is.
          </p>
        {:else if preparing}
          <p class="muted clip">{preparing.title}: {preparing.lines.at(-1)?.line ?? "starting…"}</p>
        {:else if edit?.runnable}
          <p class="muted">Move, add or delete objects in any level. Your edits stay on this computer.</p>
        {:else}
          <p class="muted">The first time, the levels are read from your disc: about three minutes and 630 MB.</p>
        {/if}
      </div>
      {#if extract && extract.state !== "planned" && disc === "found"}
        {#if lacksGodot}
          <button onclick={() => void chooseGodot()}><Icon name="folder" size={16} />Choose Godot</button>
        {:else}
          <button disabled={opening || !!preparing} onclick={() => void editLevels()}>
            {#if opening || preparing}<span class="spinner"></span>{:else}<Icon name="edit" size={16} />{/if}
            {edit?.runnable ? "Edit levels" : "Prepare and edit levels"}
          </button>
        {/if}
      {/if}
    </section>
  </div>
{/if}

<style>
  /* What is left to do, as one line: done steps are ticked, the rest are plain. */
  .steps {
    display: flex;
    flex-wrap: wrap;
    gap: 6px 22px;
    margin: 0;
    padding: 0;
    list-style: none;
    font-size: 13px;
    color: var(--tx2);
  }

  .steps li {
    display: flex;
    align-items: center;
    gap: 7px;
  }

  .steps li::before {
    content: "";
    width: 14px;
    height: 14px;
    border-radius: 50%;
    border: 1.5px solid currentColor;
    opacity: 0.7;
  }

  .steps li.done {
    color: var(--ok);
  }

  .steps li.done::before {
    border-color: var(--ok);
    background: var(--ok)
      url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 14 14'%3E%3Cpath d='M3.5 7.3l2.2 2.2 4.8-5' fill='none' stroke='%2312121a' stroke-width='1.8' stroke-linecap='round' stroke-linejoin='round'/%3E%3C/svg%3E")
      center / 14px no-repeat;
    opacity: 1;
  }

  .page {
    position: relative;
    max-width: 920px;
    margin: 0 auto;
    padding: 20px 28px 60px;
    display: flex;
    flex-direction: column;
    gap: 20px;
  }

  .page-ambient-bg {
    position: absolute;
    top: -20px;
    left: 50%;
    transform: translateX(-50%);
    width: 100vw;
    height: 480px;
    pointer-events: none;
    z-index: 0;
    overflow: hidden;
    opacity: 0.18;
  }

  .page-ambient-bg::after {
    content: "";
    position: absolute;
    inset: 0;
    background:
      radial-gradient(ellipse 70% 60% at 50% 20%, transparent 20%, var(--ink) 80%),
      linear-gradient(to bottom, transparent 60%, var(--ink) 100%);
  }

  .page-ambient-bg img {
    width: 100%;
    height: 100%;
    object-fit: cover;
  }

  .back {
    position: relative;
    z-index: 1;
    align-self: flex-start;
  }

  .hero {
    position: relative;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 16px;
    padding: 34px 28px 30px;
    border-radius: 30px;
    background:
      radial-gradient(90% 70% at 70% 0, var(--from), transparent 70%), linear-gradient(160deg, var(--from), var(--to));
    box-shadow: var(--shadow);
    overflow: hidden;
  }

  .hero-backdrop {
    position: absolute;
    inset: 0;
    pointer-events: none;
    overflow: hidden;
    z-index: 0;
  }

  .hero-bg {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    object-fit: cover;
    opacity: 0.38;
  }

  .hero-overlay {
    position: absolute;
    inset: 0;
    background:
      radial-gradient(ellipse 90% 80% at 50% 20%, transparent 20%, rgba(14, 16, 22, 0.7) 100%),
      linear-gradient(180deg, rgba(14, 16, 22, 0.3) 0%, rgba(14, 16, 22, 0.65) 100%);
  }

  .hero > :not(.hero-backdrop) {
    position: relative;
    z-index: 1;
  }

  .box {
    border: 7px solid var(--bd);
    background: var(--box);
    color: var(--tx);
    text-align: center;
    box-shadow: var(--shadow-sm);
  }

  .title {
    font-family: var(--title-font);
    font-weight: var(--title-weight);
    font-size: clamp(26px, 3.4vw, 40px);
    line-height: 1.15;
    padding: 18px 30px;
  }

  .title small {
    display: block;
    margin-top: 6px;
    font-size: 13px;
    letter-spacing: 0.04em;
    color: var(--tx2);
  }

  /* The disc's region: one control with two halves, the chosen one filled. */
  .versions {
    display: inline-flex;
    padding: 3px;
    border-radius: 999px;
    background: rgb(0 0 0 / 0.5);
    border: 1px solid rgb(255 255 255 / 0.18);
  }

  .versions button {
    border: 0;
    background: none;
    font-size: 13px;
    color: var(--tx2);
    padding: 6px 16px;
  }

  .versions button:hover {
    color: #fff;
  }

  .versions button.on {
    background: var(--bd);
    color: #fff;
  }

  .go {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
    margin-top: 8px;
    max-width: 100%;
  }

  .go-row {
    display: flex;
    align-items: center;
    gap: 10px;
  }

  .big {
    white-space: nowrap;
    font-size: 22px;
    font-weight: 700;
    padding: 16px 46px;
    border-radius: 999px;
    gap: 12px;
  }

  .line {
    font-size: 14px;
    color: var(--tx2);
    text-align: center;
    max-width: 640px;
  }

  .editor {
    display: flex;
    align-items: center;
    gap: 20px;
  }

  .editor h2 {
    margin: 0 0 4px;
  }

  .link {
    border: 0;
    background: none;
    padding: 0;
    color: var(--amber);
    font-weight: 600;
  }
</style>
