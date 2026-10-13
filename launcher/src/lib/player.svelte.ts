// What a player can do with a game right now: one answer, used by the library's
// tile and the game's page, so the main button says and does the same thing in both.
import type { ActionView, GameView, VersionView } from "./api";
import type { IconName } from "$components/Icon.svelte";
import { cancel, installGame, jobs, refresh, run, runChain } from "./app.svelte";
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

/** "PAL (Europe)" -> "Europe": a region in a player's words. */
export function regionWords(v: VersionView): string {
  return /\(([^)]+)\)/.exec(v.region)?.[1] ?? v.region;
}

/** The version a player means by "this game": the one their disc is, else the first one that can be played. */
export function playerVersion(game: GameView): VersionView | null {
  const own = game.install?.extracted?.version;
  return (
    game.versions.find((v) => v.key === own) ??
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
    return {
      ...base,
      kind: "set-up",
      label: "Set up from your disc",
      icon: "disc",
      hint: planned
        ? "This game cannot be played yet. You can set your disc up now, ready for when it can."
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
  const building = jobOf(v, ["port-build", "port-levels"]);
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
      status: building.actionId === "port-levels" ? "Getting ready…" : "Building…",
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
  if (play.done && levels?.runnable && !levels.done) {
    return {
      ...base,
      kind: "levels",
      label: "Get the levels ready",
      icon: "disc",
      hint: "Reads the levels from your disc so the game can draw them. About half a minute and 1 GB, once.",
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

/** Does what the main button says. */
export async function playerAct(v: VersionView, state: PlayerState): Promise<void> {
  const scope = { kind: "version" as const, key: v.key };
  if (state.job) {
    await cancel(state.job);
    return;
  }
  if (state.kind === "set-up") {
    await installGame(v.game, v.title);
    await refresh();
    return;
  }
  const levels = action(v, "port-levels");
  if (state.kind === "build") {
    // Building is not enough to play: the levels come with it, the first time.
    const build = action(v, "port-build");
    const steps = [build, levels && !levels.done ? levels : null].filter((a) => a !== null);
    if (await runChain(scope, steps)) await refresh();
    return;
  }
  const act = state.kind === "play" ? action(v, "play") : state.kind === "levels" ? levels : null;
  if (act) await run(scope, act);
}
