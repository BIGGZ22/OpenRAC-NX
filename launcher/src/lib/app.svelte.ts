// The UI's shared state: settings, the library, navigation, toasts and the
// job queue. Pages read `app` and call the functions below; nothing else
// talks to the Rust side except through api.ts.
import {
  api,
  pickIso,
  errorText,
  onJobEvent,
  type ActionView,
  type AppInfo,
  type Config,
  type DiscChoice,
  type FoundDisc,
  type Library,
  type RunInputs,
  type Scope,
  type Started,
  type VersionView,
} from "./api";
import { Queue, type Job } from "./queue";
import { lastLines, PLAY_ACTIONS, unseenRecent, type StopNotice } from "./reports";
import { checkForUpdate, type UpdateInfo } from "./updates";

export type Page = "library" | "game" | "mods" | "tasks" | "settings";

interface AppState {
  info: AppInfo | null;
  config: Config | null;
  library: Library | null;
  /** Why the library could not be read (no checkout set, a broken game.json). */
  libraryError: string | null;
  /** The games found in the player's games folder (api.scanDiscs). */
  discs: FoundDisc[];
  /** Developers only: the games whose disc can be switched (core/src/disc_choice.rs). */
  discChoices: DiscChoice[];
  page: Page;
  /** The version the game page shows (`rac1/pal`). */
  version: string | null;
  toasts: { id: number; text: string; kind: "info" | "error" }[];
  update: UpdateInfo | null;
  showUpdateModal: boolean;
  /** The game stopped unexpectedly: the notice on its Play page, until dismissed. */
  stopped: StopNotice | null;
  /** Bumped when the logs may have changed (the game ended): the logs list reads them again. */
  reportsChanged: number;
  /** Each set-up game's crash reports: how many, and how many in the last three days (the tiles' mark). */
  crashes: Record<string, { total: number; recent: number }>;
}

export const app = $state<AppState>({
  info: null,
  config: null,
  library: null,
  libraryError: null,
  discs: [],
  discChoices: [],
  page: "library",
  version: null,
  toasts: [],
  update: null,
  showUpdateModal: false,
  stopped: null,
  reportsChanged: 0,
  crashes: {},
});

/** The time, once a second: what a running job's elapsed time is counted against. One timer for
 *  the whole app (started in `start`), so a page shows it moving however it was reached. */
export const clock = $state({ now: Date.now() });

/** The jobs, oldest first; reactive. */
export const jobs = $state<{ list: Job[]; selected: number | null }>({ list: [], selected: null });
const queue = new Queue(jobs.list);

// ---- toasts ---------------------------------------------------------------------------

let toastId = 0;
export function toast(text: string, kind: "info" | "error" = "info") {
  const id = ++toastId;
  app.toasts.push({ id, text, kind });
  setTimeout(
    () => {
      app.toasts = app.toasts.filter((t) => t.id !== id);
    },
    kind === "error" ? 8000 : 4000,
  );
}

/** Runs `work`; a failure becomes an error toast and `undefined`. */
export async function guard<T>(work: Promise<T>): Promise<T | undefined> {
  try {
    return await work;
  } catch (e) {
    toast(errorText(e), "error");
    return undefined;
  }
}

// ---- loading ----------------------------------------------------------------------------

export async function start() {
  setInterval(() => (clock.now = Date.now()), 1000);
  await onJobEvent((event) => {
    queue.apply(event);
    if (event.type === "exit") {
      const job = queue.jobs.find((j) => j.id === event.id);
      const played = !!job && PLAY_ACTIONS.includes(job.actionId) && job.scope.kind === "version";
      if (played) {
        app.reportsChanged++;
        void countCrashes();
      }
      // The game stopping on its own is not a failed task: the Play page says what happened.
      if (job?.state === "failed" && played) void gameStopped(job);
      else if (job?.state === "failed") toast(`${job.title} failed`, "error");
      if (job) settle(job.key, job.state === "succeeded");
      void refresh();
      updateDiscordPresence();
      pump();
    }
  });
  app.info = await api.appInfo();
  app.config = await firstRun(await api.getConfig());
  if (app.config.lastVersion) app.version = app.config.lastVersion;
  await refresh();
  void syncProgressWeb();
  void adoptGodot();
  // A new launcher is looked for at start unless the player turned that off (Settings).
  if (app.info.version && app.config.checkUpdates) {
    void lookForUpdate();
  }
}

/** The game ended with an error status or a signal: find its crash report and show the notice. */
async function gameStopped(job: Job) {
  if (job.scope.kind !== "version") return;
  const versionKey = job.scope.key;
  const game = versionKey.split("/")[0] ?? versionKey;
  const since = Math.floor((job.startedAt ?? job.queuedAt) / 1000) - 2;
  // No install folder: the notice still says it stopped.
  const crash = await api.latestCrash(game, since).catch(() => null);
  app.stopped = { game, versionKey, code: job.code, at: job.endedAt ?? Date.now(), crash, lines: lastLines(job.lines) };
  // On another page, the player is told where to look.
  if (app.page !== "game" || currentVersion()?.game !== game) {
    toast("The game stopped unexpectedly. Its page has a report you can send us.", "error");
  }
}

/** How many crash reports each set-up game has, for the tiles and the Play page's badge. */
export async function countCrashes() {
  const counts: AppState["crashes"] = {};
  const recent = Date.now() / 1000 - 3 * 86400;
  for (const g of app.library?.games ?? []) {
    if (!g.install?.extracted) continue;
    try {
      const crashes = (await api.reportsList(g.id)).filter((e) => e.kind === "crash");
      const seen = app.config?.crashesSeen[g.id] ?? 0;
      counts[g.id] = { total: crashes.length, recent: unseenRecent(crashes, seen, recent).length };
    } catch {
      // no install folder: nothing to count
    }
  }
  app.crashes = counts;
}

/**
 * The player has seen the crashes this game has now: no notice, button or tile mark is shown for
 * them again, only for one that comes later. Kept in the launcher's settings (crashesSeen).
 */
export async function dismissCrashes(game: string) {
  if (!app.config) return;
  let newest = app.config.crashesSeen[game] ?? 0;
  try {
    for (const e of await api.reportsList(game)) if (e.kind === "crash") newest = Math.max(newest, e.time);
  } catch {
    // no install folder: nothing to mark
  }
  const saved = await guard(
    api.saveConfig({ ...app.config, crashesSeen: { ...app.config.crashesSeen, [game]: newest } }),
  );
  if (saved) app.config = saved;
  await countCrashes();
}

/** Puts the notice away; its crash counts as seen. */
export function dismissStopped() {
  const game = app.stopped?.game;
  app.stopped = null;
  if (game) void dismissCrashes(game);
}

/** Looks for a new launcher now (at start, or Settings' "Check now"); true when one is out. */
export async function lookForUpdate(): Promise<boolean> {
  if (!app.info?.version) return false;
  const up = await checkForUpdate(app.info.version);
  app.update = up;
  return !!up;
}

/** Reads the library again: after a job, on focus, after settings change. */
export async function refresh() {
  if (!app.config?.root) {
    app.library = null;
    app.libraryError = "Choose your OpenRAC folder in Settings.";
    return;
  }
  try {
    app.library = await api.library();
    app.libraryError = null;
  } catch (e) {
    app.libraryError = errorText(e);
  }
  void countCrashes();
  try {
    const scan = app.config.discsDir ? await api.scanDiscs() : { found: [], choices: [] };
    app.discs = scan.found;
    app.discChoices = scan.choices;
  } catch {
    app.discs = []; // a folder that went away: the tiles ask for an image as before
    app.discChoices = [];
  }
}

/** Fetches every game's latest progress from bytematch.dev (the matched figures and the functional
 *  C beside them); the Rust side fetches and keeps a copy under build/ for offline use. */
export async function syncProgressWeb() {
  if (!app.config?.root) return;
  try {
    app.library = await api.syncProgressFromWeb();
  } catch {
    // Offline: keep what was read at start (the kept copy)
  }
}

/** Godot installed after the first run, or moved: the Rust side looks for it again (detect.rs). */
async function adoptGodot() {
  try {
    const config = await api.adoptGodot();
    if (config.godot && config.godot !== app.config?.godot) {
      app.config = config;
      await refresh();
    }
  } catch {
    // Not found is not an error: the Mods page offers Get Godot and Choose Godot
  }
}

export async function saveConfig(config: Config) {
  const saved = await guard(api.saveConfig(config));
  if (saved) {
    app.config = saved;
    await refresh();
  }
}

// ---- navigation ---------------------------------------------------------------------------

export function openVersion(key: string) {
  app.version = key;
  app.page = "game";
  if (app.config && app.config.lastVersion !== key) {
    void guard(api.saveConfig({ ...app.config, lastVersion: key })).then((saved) => {
      if (saved) app.config = saved;
    });
  }
}

export function currentVersion(): VersionView | null {
  const versions = app.library?.games.flatMap((g) => g.versions) ?? [];
  return versions.find((v) => v.key === app.version) ?? null;
}

/** What Discord says a long job is doing; null for jobs it does not show. */
function taskOf(actionId: string): string | null {
  if (actionId.startsWith("install:")) return "Setting up from the disc";
  if (actionId === "port-build" || actionId === "build") return "Building the game";
  if (actionId === "editor-extract" || actionId === "editor-import") return "Preparing the level editor";
  return null;
}

export function updateDiscordPresence(page: Page = app.page, version: string | null = app.version) {
  // While the game (the native port) runs, the status stays PlayingGame:
  const playingJob = jobs.list.find((j) => j.actionId === "play" && j.state === "running");
  if (playingJob?.scope.kind === "version") {
    const scope = playingJob.scope;
    const versions = app.library?.games.flatMap((g) => g.versions) ?? [];
    const v = versions.find((ver) => ver.key === scope.key);
    if (v) {
      void api.setDiscordStatus({
        kind: "playingGame",
        title: v.title,
        region: v.region,
        gameId: v.game,
        startTime: playingJob.startedAt ? Math.floor(playingJob.startedAt / 1000) : null,
      });
      return;
    }
  }

  // A long job for a game: what it does, while it runs.
  const working = jobs.list.find((j) => j.state === "running" && taskOf(j.actionId) !== null);
  if (working) {
    const games = app.library?.games ?? [];
    const game =
      working.scope.kind === "version"
        ? games.find((g) => g.versions.some((ver) => working.scope.kind === "version" && ver.key === working.scope.key))
        : games.find((g) => working.actionId === `install:${g.id}`);
    if (game) {
      void api.setDiscordStatus({
        kind: "working",
        task: taskOf(working.actionId) ?? working.title,
        title: game.title,
        gameId: game.id,
        startTime: working.startedAt ? Math.floor(working.startedAt / 1000) : null,
      });
      return;
    }
  }

  if (page === "game" && version) {
    const v = currentVersion();
    if (v) {
      void api.setDiscordStatus({
        kind: "viewingGame",
        title: v.title,
        region: v.region,
        progressPct: v.progress?.percent ?? null,
        gameId: v.game,
      });
      return;
    }
  }
  void api.setDiscordStatus({ kind: "idle" });
}

// ---- actions and jobs ---------------------------------------------------------------------

/** Jobs that are not actions.json actions (setting a game up, an action for one level), by the job's key: how to start each. */
const starters: Record<number, (() => Promise<Started>) | undefined> = {};

/** Runs an action: a detached one (the game, Godot) at once, anything else through the queue. */
export async function run(scope: Scope, action: ActionView, given?: number | RunInputs) {
  const inputs = typeof given === "number" ? { level: given } : given;
  if (action.detached) {
    const started = await guard(api.runAction(scope, action.id, inputs));
    if (started) toast(`Started: ${started.title}`);
    updateDiscordPresence();
    return;
  }
  const job = queue.add(scope, action.id, action.label);
  if (inputs !== undefined) starters[job.key] = () => api.runAction(scope, action.id, inputs);
  jobs.selected = job.key;
  toast(queue.running() ? `Queued: ${action.label}` : `Started: ${action.label}`);
  pump();
}

/** Who waits for a job to end (a chain's next step), by the job's key. Not shown, so not reactive. */
const waiting: Record<number, ((succeeded: boolean) => void) | undefined> = {};

function settle(key: number, succeeded: boolean) {
  waiting[key]?.(succeeded);
  waiting[key] = undefined;
}

/**
 * Runs actions one after another and stops at the first that fails: "extract
 * the levels, import them, open the editor" behind one button. A detached
 * action (the editor) is started and not waited for. Resolves to whether
 * every step ran.
 */
export async function runChain(scope: Scope, actions: ActionView[], given?: number | RunInputs): Promise<boolean> {
  const inputs = typeof given === "number" ? { level: given } : given;
  for (const action of actions) {
    if (action.detached) {
      const started = await guard(api.runAction(scope, action.id, inputs));
      if (!started) return false;
      continue;
    }
    const job = queue.add(scope, action.id, action.label);
    if (inputs !== undefined) starters[job.key] = () => api.runAction(scope, action.id, inputs);
    jobs.selected = job.key;
    const ended = new Promise<boolean>((resolve) => {
      waiting[job.key] = resolve;
    });
    pump();
    if (!(await ended)) return false;
  }
  return true;
}

/**
 * Sets GAME up from the image of the player's own disc, OpenGOAL's way: the
 * player picks the image, the extractor copies its files out and checks them
 * against the build OpenRAC knows. Resolves to whether it worked.
 */
export async function installGame(game: string, title: string, found?: string): Promise<boolean> {
  // An image the games folder holds needs no picking.
  const image = found ?? (await pickIso(`Select your legitimately obtained ISO file of ${title}`));
  if (!image) return false;
  const job = queue.add({ kind: "repository" }, `install:${game}`, `Set up ${title} from your disc`);
  starters[job.key] = () => api.installGame(game, image);
  jobs.selected = job.key;
  const ended = new Promise<boolean>((resolve) => {
    waiting[job.key] = resolve;
  });
  pump();
  const ok = await ended;
  toast(ok ? `${title} is set up from your disc.` : `Setting ${title} up failed: see Tasks.`, ok ? "info" : "error");
  return ok;
}

/** Removes what setting GAME up made, after the player confirmed. */
/** Developers only: plays VERSION's disc of its game from now on (the switch on the game's page). */
export async function chooseDisc(version: string) {
  const saved = await guard(api.chooseDisc(version));
  if (saved) {
    app.config = saved;
    await refresh();
  }
}

export async function uninstallGame(game: string) {
  await guard(api.uninstallGame(game));
  await refresh();
}

/**
 * A first start with nothing to ask: when the OpenRAC folder and a Python
 * were found, they are taken, with Godot and Docker when present, and the
 * welcome screen is skipped. A player then sees their games at once.
 */
async function firstRun(config: Config): Promise<Config> {
  if (config.setupComplete) return config;
  const found = await guard(api.detect());
  const root = config.root ?? found?.roots[0]?.path ?? null;
  const python = config.python ?? found?.pythons[0]?.path ?? null;
  if (!root || !python) return config;
  const saved = await guard(
    api.saveConfig({
      ...config,
      root,
      python,
      godot: config.godot ?? found?.godots[0]?.path ?? null,
      docker: config.docker ?? found?.dockers[0]?.path ?? null,
      setupComplete: true,
    }),
  );
  return saved ?? config;
}

/** Starts the next queued job, if nothing is running. */
function pump() {
  const job = queue.next();
  if (!job) return;
  job.state = "running";
  const start = starters[job.key] ?? (() => api.runAction(job.scope, job.actionId));
  starters[job.key] = undefined;
  start().then(
    (started) => {
      job.title = started.title;
      queue.started(job, started.id, started.command);
      updateDiscordPresence();
    },
    (e: unknown) => {
      queue.refused(job, errorText(e));
      toast(errorText(e), "error");
      settle(job.key, false);
      updateDiscordPresence();
      pump();
    },
  );
}

export async function cancel(job: Job) {
  if (job.state === "queued") {
    queue.dequeue(job);
    settle(job.key, false);
    return;
  }
  if (job.state === "running" && job.id !== null) await guard(api.cancelJob(job.id));
}

export function clearFinished() {
  queue.clear();
}

export function runningJob(): Job | undefined {
  return queue.running();
}

export function pendingJobs(): number {
  return queue.pending();
}
