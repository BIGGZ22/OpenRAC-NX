// The UI's shared state: settings, the library, navigation, toasts and the
// job queue. Pages read `app` and call the functions below; nothing else
// talks to the Rust side except through api.ts.
import {
  api,
  pickIso,
  onDiscCopy,
  onAndroid,
  errorText,
  onJobEvent,
  type ActionView,
  type DiscCopy,
  type AppInfo,
  type Config,
  type DiscChoice,
  type FoundDisc,
  type GameStatus,
  type Library,
  type RunInputs,
  type Scope,
  type Started,
  type UpdateChannel,
  type VersionView,
  updateChannelOf,
} from "./api";
import { Queue, type Job } from "./queue";
import { backgroundStatus } from "./progress";
import { lastLines, PLAY_ACTIONS, unseenRecent, type StopNotice } from "./reports";
import { checkForUpdate, type UpdateInfo } from "./updates";

export type Page = "library" | "game" | "mods" | "tasks" | "develop" | "settings";

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
  /** What Develop shows: "checkout", "console", or a version's key (`rac1/pal`). */
  develop: string;
  toasts: { id: number; text: string; kind: "info" | "error" }[];
  update: UpdateInfo | null;
  showUpdateModal: boolean;
  /** The game stopped unexpectedly: the notice on its Play page, until dismissed. */
  stopped: StopNotice | null;
  /** Bumped when the logs may have changed (the game ended): the logs list reads them again. */
  reportsChanged: number;
  /** Each set-up game's crash reports: how many, and how many in the last three days (the tiles' mark). */
  crashes: Record<string, { total: number; recent: number }>;
  /** A disc image being copied into the Android app's storage, or a .chd being unpacked. */
  discCopy: DiscCopy | null;
  /** Android: the game running beside the launcher (readGame). */
  game: GameStatus | null;
  /** Android: the tools the launcher carries being unpacked, after an install or an update. */
  preparing: boolean;
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
  develop: "checkout",
  toasts: [],
  update: null,
  showUpdateModal: false,
  stopped: null,
  reportsChanged: 0,
  crashes: {},
  discCopy: null,
  game: null,
  preparing: false,
});

/** Android: whether the game still runs beside the launcher, read when the launcher comes back
 *  in front and after a start. */
export async function readGame() {
  if (!onAndroid) return;
  try {
    app.game = await api.gameStatus();
  } catch {
    app.game = null;
  }
}

/** After a start: the game's activity takes a moment to be there. */
async function readGameSoon() {
  for (const wait of [500, 1500, 3000]) {
    await new Promise((resolve) => setTimeout(resolve, wait));
    await readGame();
    if (app.game?.running) return;
  }
}

/** Android: back to the running game, or end it. */
export async function gameWindow(stop: boolean) {
  await guard(api.gameWindow(stop));
  if (stop) app.game = { running: false, library: null };
}

/** The time, once a second: what a running job's elapsed time is counted against. One timer for
 *  the whole app (started in `start`), so a page shows it moving however it was reached. */
export const clock = $state({ now: Date.now() });

/** The jobs, oldest first; reactive. */
export const jobs = $state<{ list: Job[]; selected: number | null }>({ list: [], selected: null });
const queue = new Queue(jobs.list);

// ---- Android: the set-up goes on in the background -----------------------------------------

/** How long the notification stays after the work ends: the next job of a chain starts within it,
 *  and Android starts the service that keeps the process alive only while the launcher is in front. */
const BACKGROUND_LINGER_MS = 3000;

/** While a disc is copied or a game is set up, Android shows its status in a notification and
 *  keeps the launcher running with its window closed (SetupService.kt). */
function keepAliveInBackground() {
  let sent = "null";
  let stopping: ReturnType<typeof setTimeout> | undefined;
  const send = (status: ReturnType<typeof backgroundStatus>) => {
    const text = JSON.stringify(status);
    if (text === sent) return;
    sent = text;
    void api.backgroundStatus(status).catch(() => undefined);
  };
  $effect.root(() => {
    $effect(() => {
      const status = backgroundStatus(runningJob(), pendingJobs(), app.discCopy);
      clearTimeout(stopping);
      if (status) send(status);
      else
        stopping = setTimeout(() => {
          send(null);
        }, BACKGROUND_LINGER_MS);
    });
  });
}

// ---- toasts ---------------------------------------------------------------------------

let toastId = 0;
/** The time an information toast has left, paused while the pointer or the focus is on the toasts. */
// Not state: nothing is drawn from it.
// eslint-disable-next-line svelte/prefer-svelte-reactivity
const toastTimers = new Map<number, { left: number; since: number; handle: ReturnType<typeof setTimeout> | null }>();
let toastsHeld = false;

function expire(id: number) {
  const t = toastTimers.get(id);
  if (t?.handle) clearTimeout(t.handle);
  toastTimers.delete(id);
  app.toasts = app.toasts.filter((t) => t.id !== id);
}

/** Says something for a moment; an error stays until it is dismissed, as it may need reading twice. */
export function toast(text: string, kind: "info" | "error" = "info") {
  const id = ++toastId;
  app.toasts.push({ id, text, kind });
  // No more than four errors at once: the oldest gives way.
  const errors = app.toasts.filter((t) => t.kind === "error");
  if (errors.length > 4 && errors[0]) expire(errors[0].id);
  if (kind === "info") {
    toastTimers.set(id, {
      left: 4000,
      since: Date.now(),
      handle: toastsHeld
        ? null
        : setTimeout(() => {
            expire(id);
          }, 4000),
    });
  }
}

/** Puts a toast away. */
export function dismissToast(id: number) {
  expire(id);
}

/** While the pointer or the focus is on the toasts, they wait to be read. */
export function holdToasts(held: boolean) {
  toastsHeld = held;
  for (const [id, t] of toastTimers) {
    if (held && t.handle) {
      clearTimeout(t.handle);
      t.handle = null;
      t.left -= Date.now() - t.since;
    } else if (!held && !t.handle) {
      t.since = Date.now();
      t.handle = setTimeout(
        () => {
          expire(id);
        },
        Math.max(t.left, 1500),
      );
    }
  }
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
    // A job that ended: the exit event, whether it came before the start call
    // returned (the queue kept it until the id was known) or after it.
    const ended = queue.takeEnded();
    if (ended) handleExit(ended);
  });
  if (onAndroid) {
    keepAliveInBackground();
    void readGame();
    // Back in front (from the game, or from Home): is the game still running?
    document.addEventListener("visibilitychange", () => {
      if (document.visibilityState === "visible") void readGame();
    });
  }
  if (onAndroid) await prepareTools();
  app.info = await api.appInfo();
  app.config = await firstRun(await api.getConfig());
  if (app.config.lastVersion) app.version = app.config.lastVersion;
  await refresh();
  void syncProgressWeb();
  void adoptGodot();
  // A new launcher is looked for at start unless the player turned that off (Settings).
  if (app.info.version && app.config.checkUpdates) {
    lookForUpdate().catch(() => undefined); // told only when the player asks (Settings' "Check now")
  }
}

/** Android: the tools the launcher carries are unpacked after each install or update, beside the
 *  page; nothing reads them before they are there. A second's wait shows why. */
async function prepareTools() {
  const shown = setTimeout(() => (app.preparing = true), 1000);
  try {
    await api.toolsReady();
  } catch (e) {
    toast(String(e), "error");
  } finally {
    clearTimeout(shown);
    app.preparing = false;
  }
}

/**
 * A job ended: what the launcher does about it. Called once per job, whether
 * its exit event came before the start call returned or after it (the queue
 * keeps the end until it is taken).
 */
function handleExit(job: Job) {
  const version = job.scope.kind === "version" ? job.scope.key : null;
  const played = version !== null && PLAY_ACTIONS.includes(job.actionId);
  if (played) {
    app.reportsChanged++;
    void countCrashes();
  }
  // The game stopping on its own is not a failed task: the Play page says what happened.
  if (job.state === "failed" && played) void gameStopped(job);
  else if (job.state === "failed") toast(`${job.title} failed`, "error");
  // Played to the end since: the notice folds to a line (CrashNotice), the report still a click away.
  if (job.state === "succeeded" && played && app.stopped && version.startsWith(`${app.stopped.game}/`)) {
    app.stopped = { ...app.stopped, playedSince: true };
  }
  settle(job.key, job.state === "succeeded");
  void refresh();
  updateDiscordPresence();
  pump();
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

/** Looks for a new launcher now (at start, or Settings' "Check now") on CHANNEL, by default the
 *  player's; true when one is out. Throws when there was nothing to compare with (no release where
 *  the launcher looks, no network). */
export async function lookForUpdate(channel?: UpdateChannel): Promise<boolean> {
  if (!app.info?.version) return false;
  channel ??= updateChannelOf(app.info, app.config);
  const up = await checkForUpdate(app.info.version, app.info.updateChannels[channel], channel);
  app.update = up;
  return !!up;
}

/** Reads the library again: after a job, on focus, after settings change. */
// What the library and the disc scan last were, as JSON: refresh runs on every focus of the
// window, and an answer that did not change is not put in `app`, so nothing that shows it is
// worked out and drawn again.
const seen = { library: "", discs: "" };

function same(key: keyof typeof seen, value: unknown): boolean {
  const json = JSON.stringify(value);
  if (json === seen[key]) return true;
  seen[key] = json;
  return false;
}

function setLibrary(library: Library | null) {
  if (!same("library", library)) app.library = library;
}

export async function refresh() {
  if (!app.config?.root) {
    setLibrary(null);
    app.libraryError = "Choose your OpenRAC folder in Settings.";
    return;
  }
  try {
    setLibrary(await api.library());
    app.libraryError = null;
  } catch (e) {
    app.libraryError = errorText(e);
  }
  void countCrashes();
  let scan: { found: FoundDisc[]; choices: DiscChoice[] } = { found: [], choices: [] };
  try {
    if (app.config.discsDir) scan = await api.scanDiscs();
  } catch {
    // A folder that went away: the game pages ask for an image as before.
  }
  if (!same("discs", scan)) {
    app.discs = scan.found;
    app.discChoices = scan.choices;
  }
}

/** Fetches every game's latest progress from bytematch.dev (the matched figures and the functional
 *  C beside them); the Rust side fetches and keeps a copy under build/ for offline use. */
export async function syncProgressWeb() {
  if (!app.config?.root) return;
  try {
    setLibrary(await api.syncProgressFromWeb());
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

/** Opens Develop on TARGET: "checkout", "console" or a version's key; by default the version
 *  the game page shows, else the checkout. */
export function openDevelop(target?: string) {
  app.develop = target ?? app.version ?? "checkout";
  app.page = "develop";
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
  const picked =
    found ??
    (await copyingDisc((progress) => pickIso(`Select your legitimately obtained disc image of ${title}`, progress)));
  if (!picked) return false;
  // A .chd is unpacked into a plain image first (on Android the picker already did).
  let image = picked;
  if (/\.chd$/i.test(picked)) {
    const plain = await copyingDisc(async (progress) => {
      const off = await onDiscCopy(progress);
      try {
        return await guard(api.prepareDisc(picked));
      } finally {
        off();
      }
    });
    if (!plain) return false;
    image = plain;
  }
  const job = queue.add({ kind: "repository" }, `install:${game}`, `Set up ${title} from your disc`);
  starters[job.key] = () => api.installGame(game, image);
  jobs.selected = job.key;
  const ended = new Promise<boolean>((resolve) => {
    waiting[job.key] = resolve;
  });
  pump();
  const ok = await ended;
  toast(
    ok ? `${title} is set up from your disc.` : `Setting ${title} up stopped: Activity has what it printed.`,
    ok ? "info" : "error",
  );
  return ok;
}

/** Runs a disc image's copy into the Android app's storage, or a .chd's unpacking, with its
 *  progress in a toast at each quarter and in `app.discCopy` (Android's notification). */
export async function copyingDisc<T>(run: (progress: (progress: DiscCopy) => void) => Promise<T>): Promise<T> {
  let shown = -1;
  try {
    return await run((progress) => {
      const { name, copied, total } = progress;
      app.discCopy = progress;
      const quarter = total > 0 ? Math.floor((copied / total) * 4) : 0;
      if (quarter === shown) return;
      shown = quarter;
      toast(quarter >= 4 ? `${name} is ready.` : `Preparing ${name}: ${quarter * 25}%`);
    });
  } finally {
    app.discCopy = null;
  }
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
      if (started.detached) {
        queue.launched(job, started.command);
        settle(job.key, true);
        if (PLAY_ACTIONS.includes(job.actionId)) void readGameSoon();
        updateDiscordPresence();
        pump();
        return;
      }
      queue.started(job, started.id, started.command);
      // The job may have ended before the start call returned: its exit event was kept
      // until the id was known, and the end is acted on now.
      const ended = queue.takeEnded();
      if (ended) handleExit(ended);
      else updateDiscordPresence();
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
