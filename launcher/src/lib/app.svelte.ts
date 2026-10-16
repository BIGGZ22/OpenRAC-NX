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
  type FoundDisc,
  type Library,
  type Scope,
  type Started,
  type VersionView,
} from "./api";
import { Queue, type Job } from "./queue";
import { checkGitHubRelease, type UpdateInfo } from "./updates";

export type Page = "library" | "game" | "mods" | "tasks" | "settings";

interface AppState {
  info: AppInfo | null;
  config: Config | null;
  library: Library | null;
  /** Why the library could not be read (no checkout set, a broken game.json). */
  libraryError: string | null;
  /** The games found in the player's games folder (api.scanDiscs). */
  discs: FoundDisc[];
  page: Page;
  /** The version the game page shows (`rac1/pal`). */
  version: string | null;
  toasts: { id: number; text: string; kind: "info" | "error" }[];
  update: UpdateInfo | null;
}

export const app = $state<AppState>({
  info: null,
  config: null,
  library: null,
  libraryError: null,
  discs: [],
  page: "library",
  version: null,
  toasts: [],
  update: null,
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
      if (job?.state === "failed") toast(`${job.title} failed`, "error");
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
  if (app.info.version) {
    void checkGitHubRelease(app.info.version).then((up) => {
      if (up) app.update = up;
    });
  }
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
  try {
    app.discs = app.config.discsDir ? await api.scanDiscs() : [];
  } catch {
    app.discs = []; // a folder that went away: the tiles ask for an image as before
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
export async function run(scope: Scope, action: ActionView, level?: number) {
  if (action.detached) {
    const started = await guard(api.runAction(scope, action.id, level));
    if (started) toast(`Started: ${started.title}`);
    updateDiscordPresence();
    return;
  }
  const job = queue.add(scope, action.id, action.label);
  if (level !== undefined) starters[job.key] = () => api.runAction(scope, action.id, level);
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
export async function runChain(scope: Scope, actions: ActionView[], level?: number): Promise<boolean> {
  for (const action of actions) {
    if (action.detached) {
      const started = await guard(api.runAction(scope, action.id, level));
      if (!started) return false;
      continue;
    }
    const job = queue.add(scope, action.id, action.label);
    if (level !== undefined) starters[job.key] = () => api.runAction(scope, action.id, level);
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
