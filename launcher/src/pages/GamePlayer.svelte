<script lang="ts">
  // A game's page. One main button, always in the same place,
  // says the next thing to do (set up from your disc, build, play); what is done
  // less often is in the menu beside it. Below: the saves and, beside them, Help
  // (a recent crash, the logs); the level editor is under Mods. Each button runs
  // the actions the Develop tab lists (launcher/actions.json), found by their ids.
  // With developer tools on, one row of chips under the band leads there.
  import CountUp from "$components/CountUp.svelte";
  import CrashNotice from "$components/CrashNotice.svelte";
  import DiscSwitch from "$components/DiscSwitch.svelte";
  import FolderPrompt from "$components/FolderPrompt.svelte";
  import GamesFrame from "$components/GamesFrame.svelte";
  import Icon from "$components/Icon.svelte";
  import JobProgress from "$components/JobProgress.svelte";
  import ProgressBar from "$components/ProgressBar.svelte";
  import Menu, { type MenuItem } from "$components/Menu.svelte";
  import ReportsBlock from "$components/ReportsBlock.svelte";
  import SaveManagerBlock from "$components/SaveManagerBlock.svelte";
  import { api, type ActionView, type ReportEntry } from "$lib/api";
  import {
    app,
    clock,
    currentVersion,
    gameWindow,
    guard,
    jobs,
    openDevelop,
    run,
    uninstallGame,
  } from "$lib/app.svelte";
  import { choiceFor, programOf } from "$lib/discChoice";
  import { pct } from "$lib/format";
  import { SITE_URL } from "$lib/links";
  import { gameArt, gameClip, hideMissing } from "$lib/media";
  import { enter, reduced } from "$lib/motion";
  import { fly } from "svelte/transition";
  import { place, when } from "$lib/reports";
  import { isBuilt, playerAct, playerState, playerVersion } from "$lib/player.svelte";
  import { themeStyle } from "$lib/themes";

  // One game, whatever the region: the page plays the version your disc is (playerVersion),
  // always. Each disc's decompilation and actions are in the Develop tab.
  const developer = $derived(app.config?.developer ?? false);
  const opened = $derived(currentVersion());
  const game = $derived(app.library?.games.find((g) => g.id === opened?.game) ?? null);
  const v = $derived(game ? playerVersion(game) : opened);
  const scope = $derived({ kind: "version" as const, key: v?.key ?? "" });
  // Developer tools on and two of the game's discs found: the switch beside Play (DiscSwitch).
  const choice = $derived(developer && game ? choiceFor(app.discChoices, game.id) : null);
  const action = (id: string): ActionView | null => v?.actions.find((a) => a.id === id && a.thisPlatform) ?? null;

  const build = $derived(action("port-build"));
  const levels = $derived(action("port-levels"));
  const levelsCheck = $derived(action("port-levels-check"));
  /** The levels were read from the disc and, where the version has the check, the game read them all. */
  const levelsReady = $derived(levelsCheck ? levelsCheck.done : !!levels?.done);
  const playGodot = $derived(action("editor-preview"));
  const extract = $derived(action("editor-extract"));

  // Each game's still picture, under its band and behind the page.
  const art = $derived(v ? gameArt(v.game) : null);

  // The game's clip plays over its picture while the pointer is on the band or focus is in it, and
  // stops (it is taken away, so it decodes nothing) when they leave. Never with reduced motion.
  /** Games whose clip is not there: not asked for again. */
  let noClip = $state<Record<string, boolean>>({});
  const clip = $derived(v && !reduced && !noClip[v.game] ? gameClip(v.game) : null);
  let hero = $state<HTMLElement | null>(null);
  let clipOn = $state(false);
  $effect(() => {
    const el = hero;
    clipOn = false;
    if (!el || !clip) return;
    const on = () => (clipOn = true);
    const off = () => (clipOn = false);
    const blur = (e: FocusEvent) => {
      if (!(e.relatedTarget instanceof Node && el.contains(e.relatedTarget))) clipOn = false;
    };
    el.addEventListener("pointerenter", on);
    el.addEventListener("pointerleave", off);
    el.addEventListener("focusin", on);
    el.addEventListener("focusout", blur);
    return () => {
      el.removeEventListener("pointerenter", on);
      el.removeEventListener("pointerleave", off);
      el.removeEventListener("focusin", on);
      el.removeEventListener("focusout", blur);
    };
  });
  /** The game has a PC port to play (or to build and play); without one, the page says how far it is. */
  const portReady = $derived(!!action("play") && action("play")?.state !== "planned");

  const next = $derived(v && game ? playerState(game, v) : null);
  const setUp = $derived(game?.install?.extracted ?? null);
  const built = $derived(!!v && !!game && isBuilt(game, v));
  /** The steps are shown while there is one left that the player can take. */
  const showSteps = $derived(!!next && next.kind !== "play" && next.kind !== "playing" && next.kind !== "return");

  async function removeSetUp() {
    if (!v) return;
    const sure = confirm(
      `Remove what was set up from your ${v.title} disc? Your disc image and your saves are kept; set it up again any time.`,
    );
    if (sure) await uninstallGame(v.game);
  }

  /** The last step towards playing that stopped with an error (set-up, build, levels), while nothing
   *  runs: the page says so under the main button, with the way to what it printed. */
  const stopped = $derived.by(() => {
    if (!v || next?.job) return null;
    const ids = [`install:${v.game}`, "port-build", "port-levels", "port-levels-check"];
    const mine = jobs.list.filter(
      (j) => ids.includes(j.actionId) && (j.scope.kind === "repository" || j.scope.key === v.key),
    );
    const last = mine.at(-1);
    return last?.state === "failed" ? last : null;
  });

  /** The newest log or crash report: "Last played today at 18:15 on Batalia", under the main button. */
  let last = $state<ReportEntry | null>(null);
  /** The newest read wins: switching games while one is read drops the older answer. */
  let reading = 0;
  $effect(() => {
    // Again whenever the game ends (app.reportsChanged) or another game is shown.
    const game = v?.game;
    const mine = ++reading;
    last = null;
    if (!game || !setUp || app.reportsChanged < 0) return;
    void api
      .reportsList(game)
      .then((entries) => {
        if (mine === reading) last = entries[0] ?? null;
      })
      .catch(() => undefined);
  });
  const lastPlayed = $derived.by(() => {
    if (!last) return "";
    const day = when(last.time);
    return `Last played ${day.charAt(0).toLowerCase()}${day.slice(1)} ${place(last)}`;
  });

  /** The menu's way to the logs: Help, with its logs unfolded. */
  function showLogs() {
    document.getElementById("logs-list")?.setAttribute("open", "");
    const logs = document.getElementById("logs");
    logs?.scrollIntoView({ behavior: "smooth", block: "start" });
    logs?.focus({ preventScroll: true });
  }

  /** What is done less often than playing, in the menu beside the main button. */
  const menu = $derived.by<MenuItem[]>(() => {
    if (!v) return [];
    const dir = v.dir;
    const items: MenuItem[] = [];
    if (next?.kind === "return") {
      items.push({ label: "Stop the game", icon: "x", danger: true, run: () => void gameWindow(true) });
    }
    if (build?.runnable && built && next?.kind === "play") {
      items.push({ label: "Rebuild the game", icon: "build", run: () => void run(scope, build) });
    }
    if (playGodot?.runnable) {
      items.push({ label: "Preview a level in Godot", icon: "play", run: () => void run(scope, playGodot) });
    }
    if (extract && extract.state !== "planned") {
      items.push({ label: "Edit its levels (Mods)", icon: "edit", run: () => (app.page = "mods") });
    }
    if (setUp) {
      items.push({
        label: "Logs and crash reports",
        icon: "book",
        run: showLogs,
      });
    }
    items.push({ label: "Open the game's folder", icon: "folder", run: () => void guard(api.openPath(dir)) });
    items.push({ label: "Activity log", icon: "terminal", run: () => (app.page = "tasks") });
    if (setUp) items.push({ label: "Remove the set-up", icon: "trash", danger: true, run: () => void removeSetUp() });
    return items;
  });
</script>

<GamesFrame>
  {#if !v || !game || !next}
    <div class="page">
      <p class="muted">That game is not in this OpenRAC folder.</p>
      <button
        onclick={() => {
          app.version = null;
          app.page = "library";
        }}>Open the first game</button
      >
    </div>
  {:else}
    <!-- Keyed by the game: switching games builds its page afresh (its saves are read again) and
         brings it in, the band first, then what is under it. -->
    {#key v.key}
      <div class="page" style={themeStyle(v.game)}>
        {#if art}
          <div class="page-ambient-bg" aria-hidden="true">
            <img src={art} alt="" decoding="async" onerror={hideMissing} />
          </div>
        {/if}

        <FolderPrompt />
        {#if app.library?.actionsError}
          <div class="panel notice">
            <Icon name="warn" />
            <p class="grow">The games show, but no actions: {app.library.actionsError}</p>
          </div>
        {/if}
        {#if app.stopped?.game === v.game}
          <CrashNotice notice={app.stopped} title={v.title} />
        {/if}

        <section class="hero" aria-label={v.title} in:fly={enter()} bind:this={hero}>
          {#if art ?? clip}
            <div class="hero-backdrop" aria-hidden="true">
              {#if art}
                <img src={art} alt="" class="hero-bg" decoding="async" fetchpriority="high" onerror={hideMissing} />
              {/if}
              {#if clipOn && clip}
                {@const id = v.game}
                <img
                  src={clip}
                  alt=""
                  class="hero-clip"
                  decoding="async"
                  onload={(e) => {
                    e.currentTarget.classList.add("shown");
                  }}
                  onerror={() => (noClip[id] = true)}
                />
              {/if}
              <div class="hero-overlay"></div>
            </div>
          {/if}
          <div class="title box">
            {v.title}
            {#if game.year}<small>{game.year}</small>{/if}
          </div>

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
            {#if choice}<DiscSwitch {game} {choice} busy={!!next.job} />{/if}
            {#if next.hint && !next.job}<p class="line">{next.hint}</p>{/if}
            {#if lastPlayed && next.kind === "play"}<p class="line last">{lastPlayed}</p>{/if}
            {#if stopped}
              <p class="stopped">
                <Icon name="warn" size={14} />{stopped.title} stopped with an error.
                <button class="link" onclick={() => (app.page = "tasks")}>See what it printed</button>
              </p>
            {/if}
          </div>

          {#if next.job}<JobProgress job={next.job} now={clock.now} />{/if}

          {#if showSteps}
            <ol class="steps" aria-label="Setting the game up">
              <li class:done={!!setUp}>Disc set up</li>
              {#if next.kind !== "unavailable" && (next.kind !== "set-up" || next.primary)}
                <li class:done={built}>Game built</li>
                {#if levels}<li class:done={levelsReady}>Levels ready</li>{/if}
              {/if}
            </ol>
          {/if}
        </section>

        {#if developer}
          <!-- A contributor's facts about this disc, as chips; the rest is in Develop. -->
          <div class="chips" aria-label="Developer details" in:fly|global={enter(60)}>
            <span class="chip mono">{v.key}</span>
            {#if v.serial}<span class="chip mono">{v.serial}</span>{/if}
            {#if v.progress}<span class="chip mono">{pct(v.progress.percent)} matched</span>{/if}
            <button
              class="chip link"
              onclick={() => {
                openDevelop(v.key);
              }}>Open in Develop ›</button
            >
          </div>
        {/if}
        {#if !portReady}
          <!-- Every game has this page; one without a port yet says so, and how far it is. -->
          <section class="panel coming" aria-labelledby="coming-title" in:fly|global={enter(90)}>
            <h2 id="coming-title">Not playable yet</h2>
            <p class="muted">
              OpenRAC is rebuilding {v.title} so it runs natively on your computer. Play appears here when it is ready{setUp
                ? "; your disc is already set up for it."
                : ". You can set your disc up now, so it is ready too."}
            </p>
            {#if v.progress}
              <div class="progress">
                <span class="grow">Code rebuilt so far</span>
                <strong><CountUp value={v.progress.percent} format={pct} /></strong>
              </div>
              <ProgressBar percent={v.progress.percent} label={`${v.title}: code rebuilt so far`} />
            {/if}
            <p>
              <button class="link" onclick={() => void guard(api.openUrl(SITE_URL))}
                >Follow the progress on openrac.dev</button
              >
            </p>
          </section>
        {/if}
        {#if !!setUp || portReady}
          <div class="below" class:pair={!!setUp} in:fly|global={enter(90)}>
            <SaveManagerBlock version={v} />
            {#if setUp}<ReportsBlock game={v.game} program={choice ? programOf(v.key) : null} />{/if}
          </div>
        {/if}
      </div>
    {/key}
  {/if}
</GamesFrame>

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
    padding: var(--page-top, 20px) var(--page-x, 28px) var(--page-bottom, 60px);
    display: flex;
    flex-direction: column;
    gap: var(--page-gap, 20px);
  }

  .page-ambient-bg {
    position: absolute;
    top: -20px;
    left: 0;
    width: 100%;
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
    /* The game's picture settles into the band as the page opens. */
    animation: settle 1.4s cubic-bezier(0.2, 0.8, 0.2, 1) both;
    will-change: transform, opacity;
  }

  @keyframes settle {
    from {
      opacity: 0;
      transform: scale(1.08);
    }
  }

  /* The clip fades in over the picture once its first frame is there. */
  .hero-clip {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    object-fit: cover;
    opacity: 0;
    transition: opacity 0.35s ease-out;
  }

  .hero-clip:global(.shown) {
    opacity: 0.55;
  }

  .page-ambient-bg img {
    animation: ambient 0.8s ease-out both;
  }

  @keyframes ambient {
    from {
      opacity: 0;
    }
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
    transition:
      transform 0.15s ease,
      box-shadow 0.2s ease,
      filter 0.15s ease;
  }

  /* The main button lifts towards the pointer, and gives under it. */
  .big.primary:hover:not(:disabled) {
    transform: translateY(-2px);
    box-shadow: 0 12px 28px color-mix(in srgb, var(--bar-top) 38%, transparent);
  }

  .big:active:not(:disabled) {
    transform: translateY(0) scale(0.98);
    box-shadow: none;
  }

  .last {
    color: var(--lav);
  }

  .notice {
    position: relative;
    z-index: 1;
    display: flex;
    align-items: center;
    gap: 12px;
    border-color: color-mix(in srgb, var(--amber) 45%, transparent);
    color: var(--amber);
  }

  .notice p {
    margin: 0;
    color: var(--text);
  }

  /* The last step that failed, under the main button: what stopped, and the way to its output. */
  .stopped {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 4px 8px;
    margin: 0;
    font-size: 13px;
    color: var(--err);
  }

  .stopped .link {
    font-size: 13px;
  }

  /* A game without a port yet: what is happening, how far it is, and where to follow it. */
  .coming {
    position: relative;
    z-index: 1;
    display: flex;
    flex-direction: column;
    gap: 12px;
  }

  .coming p {
    margin: 0;
    max-width: 68ch;
  }

  .coming .progress {
    display: flex;
    align-items: baseline;
    gap: 12px;
    font-size: 14px;
    color: var(--soft);
  }

  .coming .progress strong {
    font-family: var(--font-head);
    font-size: 18px;
    color: var(--gold);
    font-variant-numeric: tabular-nums;
  }

  .link {
    padding: 0;
    border: 0;
    background: none;
    color: var(--amber);
    font-weight: 600;
  }

  /* With developer tools on: one row of small chips, the last one a way to Develop. */
  .chips {
    position: relative;
    z-index: 1;
    display: flex;
    flex-wrap: wrap;
    gap: 6px;
    margin-top: -6px;
  }

  .chip {
    font-size: 11.5px;
    color: var(--soft);
    background: var(--panel-hi);
    border: 1px solid var(--line-strong);
    border-radius: 7px;
    padding: 3px 9px;
    white-space: nowrap;
  }

  button.chip {
    font-family: var(--font-sans);
    font-weight: 600;
    color: var(--amber);
  }

  /* The saves and Help: side by side where the window is wide enough, else one under the other. */
  .below {
    position: relative;
    z-index: 1;
    display: grid;
    grid-template-columns: minmax(0, 1fr);
    gap: 16px;
    align-items: start;
  }

  @media (min-width: 1000px) {
    .below.pair {
      grid-template-columns: minmax(0, 1.35fr) minmax(0, 1fr);
    }
  }

  .line {
    font-size: 14px;
    color: var(--tx2);
    text-align: center;
    max-width: 640px;
  }
</style>
