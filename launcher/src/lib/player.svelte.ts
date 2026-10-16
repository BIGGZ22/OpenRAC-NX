// What a player can do with a game right now: one answer, used by the library's
// tile and the game's page, so the main button says and does the same thing in both.
import type { ActionView, GameView, VersionView } from "./api";
import type { IconName } from "$components/Icon.svelte";
import { pickFolder } from "./api";
import { app, cancel, installGame, jobs, refresh, run, runChain, saveConfig, toast } from "./app.svelte";
import type { Job } from "./queue";

export type PlayerKind =
  | "installing" // the disc is being read
  | "set-up" // nothing is set up yet: choose the disc image
  | "unavailable" // set up, but this game has no port to play yet
  | "other-disc" // the player's disc is another version of this game
  | "playing"
  | "building"
  | "play"
  | "build"
  | "levels" // built, but the levels the game draws are not read from the disc yet
  | "blocked"; // a port exists but something it needs is missing

export interface PlayerState {
  kind: PlayerKind;
  /** The main button's words. */
  label: string;
  icon: IconName;
  /** One line under the button: what it will do, or why it cannot. */
  hint: string;
  /** The button is the page's main action (orange), not a stop or a dead end. */
  primary: boolean;
  disabled: boolean;
  /** The job the button stops, while one runs. */
  job: Job | null;
  /** A few words for a list: "Ready to play", "Not available yet". */
  status: string;
  tone: "ok" | "warn" | "info";
}

const live = (j: Job) => j.state === "queued" || j.state === "running";

function jobOf(v: VersionView, ids: string[]): Job | null {
  return (
    jobs.list.find((j) => j.scope.kind === "version" && j.scope.key === v.key && ids.includes(j.actionId) && live(j)) ??
    null
  );
}

const action = (v: VersionView, id: string): ActionView | null =>
  v.actions.find((a) => a.id === id && a.thisPlatform) ?? null;

/** The image of GAME in the player's games folder, if it holds one. */
export function foundDisc(game: string) {
  return app.discs.find((d) => d.game === game) ?? null;
}

const fileName = (path: string) => path.split(/[\\/]/).pop() ?? path;

/**
 * Sets a game up and goes on to what playing needs next without being asked: the levels the game
 * draws from, when the set-up's version has that step and it is not done. IMAGE: from the games
 * folder; without one the player picks the image.
 */
export async function setUp(game: string, title: string, image?: string): Promise<boolean> {
  if (!(await installGame(game, title, image))) return false;
  await refresh();
  const g = app.library?.games.find((x) => x.id === game);
  const v = g ? playerVersion(g) : null;
  if (v && levelsNeeded(action(v, "port-levels"), action(v, "port-levels-check"))) await prepareLevels(v);
  return true;
}

/**
 * The player names the folder their PlayStation 2 games are in; every game OpenRAC knows in it
 * that can be played and is not set up yet is set up, one after the other. Resolves to how many
 * games the folder holds (0: none found), or null when no folder was chosen.
 */
export async function chooseGamesFolder(): Promise<number | null> {
  const dir = await pickFolder("Choose the folder with your PlayStation 2 games");
  if (!dir || !app.config) return null;
  await saveConfig({ ...app.config, discsDir: dir });
  await refresh();
  const found = [...app.discs];
  if (!found.length) {
    toast("No Ratchet & Clank disc image (.iso) was found in that folder.", "error");
    return 0;
  }
  for (const disc of found) {
    const game = app.library?.games.find((g) => g.id === disc.game);
    const version = game?.versions.find((x) => x.key === disc.version);
    const play = version ? action(version, "play") : null;
    if (game && !game.install?.extracted && play && play.state !== "planned") {
      await setUp(disc.game, disc.title, disc.path);
    }
  }
  return found.length;
}

/** "PAL (Europe)" -> "Europe": a region in a player's words. */
export function regionWords(v: VersionView): string {
  return /\(([^)]+)\)/.exec(v.region)?.[1] ?? v.region;
}

/** The version a player means by "this game": the one their set-up disc is, else the first one
 *  whose disc image was found, else the first one that can be played. */
export function playerVersion(game: GameView): VersionView | null {
  const own = game.install?.extracted?.version;
  const found = app.discs.filter((d) => d.game === game.id).map((d) => d.version);
  return (
    game.versions.find((v) => v.key === own) ??
    game.versions.find((v) => found.includes(v.key)) ??
    game.versions.find((v) => {
      const play = action(v, "play");
      return play && play.state !== "planned";
    }) ??
    game.versions.at(0) ??
    null
  );
}

/** The game is built for this computer. */
export function isBuilt(game: GameView, v: VersionView): boolean {
  return !!game.install?.compiled || !!action(v, "play")?.runnable;
}

export function playerState(game: GameView, v: VersionView): PlayerState {
  const play = action(v, "play");
  const build = action(v, "port-build");
  const levels = action(v, "port-levels");
  const check = action(v, "port-levels-check");
  const setUp = game.install?.extracted ?? null;
  const other = setUp && setUp.version !== v.key ? (game.versions.find((o) => o.key === setUp.version) ?? null) : null;
  const installing = jobs.list.find((j) => j.actionId === `install:${v.game}` && live(j)) ?? null;
  const base = { primary: false, disabled: false, job: null as Job | null };

  if (installing) {
    return {
      ...base,
      kind: "installing",
      label: installing.state === "queued" ? "Waiting…" : "Stop",
      icon: "x",
      hint: installing.lines.at(-1)?.line ?? "Reading your disc…",
      job: installing,
      status: "Setting up…",
      tone: "warn",
    };
  }
  if (!setUp) {
    const planned = !play || play.state === "planned";
    const found = foundDisc(v.game);
    return {
      ...base,
      kind: "set-up",
      label: found ? "Set up" : "Set up from your disc",
      icon: "disc",
      hint: planned
        ? "This game cannot be played yet. You can set your disc up now, ready for when it can."
        : found
          ? `Found in your games folder: ${fileName(found.path)}`
          : "Choose the image (.iso) of your own disc. OpenRAC never downloads a game.",
      primary: !planned,
      status: planned ? "Not playable yet" : "Not set up",
      tone: planned ? "info" : "warn",
    };
  }
  if (!play || play.state === "planned") {
    return {
      ...base,
      kind: "unavailable",
      label: "Not playable yet",
      icon: "play",
      hint: "Your disc is set up. This game's PC port is not ready yet.",
      disabled: true,
      status: "Not playable yet",
      tone: "info",
    };
  }
  if (other) {
    return {
      ...base,
      kind: "other-disc",
      label: "Play",
      icon: "play",
      hint: `Your disc is the ${regionWords(other)} version. Switch to it to play.`,
      disabled: true,
      status: `Your disc is ${regionWords(other)}`,
      tone: "info",
    };
  }
  const playing = jobOf(v, ["play"]);
  if (playing) {
    return {
      ...base,
      kind: "playing",
      label: playing.state === "queued" ? "Waiting…" : "Stop the game",
      icon: "x",
      hint: playing.lines.at(-1)?.line ?? "Starting the game…",
      job: playing,
      status: "Running",
      tone: "ok",
    };
  }
  const building = jobOf(v, ["port-build", "port-levels", "port-levels-check"]);
  if (building) {
    return {
      ...base,
      kind: "building",
      label: building.state === "queued" ? "Waiting…" : "Stop",
      icon: "x",
      hint:
        building.lines.at(-1)?.line ??
        (building.actionId === "port-levels"
          ? "Reading the levels from your disc…"
          : "Building the game for this computer…"),
      job: building,
      status: building.actionId === "port-build" ? "Building…" : "Getting ready…",
      tone: "warn",
    };
  }
  if (play.runnable) {
    return {
      ...base,
      kind: "play",
      label: "Play",
      icon: "play",
      hint: "",
      primary: true,
      status: "Ready to play",
      tone: "ok",
    };
  }
  // The levels are ready when the game itself has read every one of them (the check's stamp),
  // not when their folder is there: an export cut short was a black world behind the HUD.
  if (play.done && levelsNeeded(levels, check)) {
    const read = !!levels?.done;
    return {
      ...base,
      kind: "levels",
      label: read ? "Check the levels" : "Get the levels ready",
      icon: "disc",
      hint: read
        ? "Makes sure every level was read from your disc whole, and reads them again if not. A few seconds."
        : "Reads the levels from your disc so the game can draw them. About half a minute and 1 GB, once.",
      primary: true,
      status: "Needs its levels",
      tone: "warn",
    };
  }
  if (build?.runnable && !play.done) {
    return {
      ...base,
      kind: "build",
      label: "Build the game",
      icon: "build",
      hint: "Compiles the game for this computer. A few minutes the first time.",
      primary: true,
      status: "Needs building",
      tone: "warn",
    };
  }
  const needs = play.blockers.map((b) => b.replace(/^needs /, "")).join(", ");
  return {
    ...base,
    kind: "blocked",
    label: "Play",
    icon: "play",
    hint: needs ? `Not ready: it needs ${needs}.` : "Not ready yet.",
    disabled: true,
    status: "Not ready",
    tone: "warn",
  };
}

/** Something of the levels is still to do: reading them, or (where the version has it) the check. */
function levelsNeeded(levels: ActionView | null, check: ActionView | null): boolean {
  if (levels?.runnable && !levels.done) return true;
  return !!check && !check.done && (check.runnable || !!levels?.runnable);
}

/**
 * Reads the levels from the disc if that is still to do, then has the game check them. Levels that
 * are there but fail the check are read again, once, and checked again: a player is not left
 * with a button that fails the same way each time.
 */
export async function prepareLevels(v: VersionView): Promise<boolean> {
  const scope = { kind: "version" as const, key: v.key };
  const fresh = () => app.library?.games.flatMap((g) => g.versions).find((x) => x.key === v.key) ?? v;
  const levels = action(v, "port-levels");
  const check = action(v, "port-levels-check");
  if (!check) {
    const ok = !!levels && (await runChain(scope, [levels]));
    await refresh();
    return ok;
  }
  if (levels && !levels.done && !(await runChain(scope, [levels]))) return false;
  await refresh();
  let again = action(fresh(), "port-levels-check");
  if (again && (await runChain(scope, [again]))) {
    await refresh();
    return true;
  }
  // The check named levels it cannot draw: read them all again, in the old ones' place.
  const reread = action(fresh(), "port-levels");
  if (!reread?.runnable) return false;
  toast("Some levels could not be drawn. Reading them from your disc again…");
  if (!(await runChain(scope, [reread]))) return false;
  await refresh();
  again = action(fresh(), "port-levels-check");
  const ok = !!again && (await runChain(scope, [again]));
  await refresh();
  if (!ok) toast("The levels still cannot be drawn: see Activity for which, and why.", "error");
  return ok;
}

/** Does what the main button says. */
export async function playerAct(v: VersionView, state: PlayerState): Promise<void> {
  const scope = { kind: "version" as const, key: v.key };
  if (state.job) {
    await cancel(state.job);
    return;
  }
  if (state.kind === "set-up") {
    await setUp(v.game, v.title, foundDisc(v.game)?.path);
    return;
  }
  if (state.kind === "build") {
    // Building is not enough to play: the levels come with it, the first time.
    const build = action(v, "port-build");
    if (build && (await runChain(scope, [build]))) {
      await refresh();
      const now = app.library?.games.flatMap((g) => g.versions).find((x) => x.key === v.key) ?? v;
      if (levelsNeeded(action(now, "port-levels"), action(now, "port-levels-check"))) await prepareLevels(now);
    }
    return;
  }
  if (state.kind === "levels") {
    await prepareLevels(v);
    return;
  }
  const act = state.kind === "play" ? action(v, "play") : null;
  if (act) await run(scope, act);
}
