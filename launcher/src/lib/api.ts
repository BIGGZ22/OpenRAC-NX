// Typed wrappers around the Rust commands (src-tauri/src/lib.rs, whose data
// types live in ../core). In a plain browser (`yarn dev` without Tauri)
// every call goes to mock.ts instead, so the UI can be worked on and
// screenshotted without the desktop app, a disc or a toolchain.
//
// Adding a command: the Rust function, its name in generate_handler!, a
// wrapper and its types here, and a case in mock.ts (docs/ARCHITECTURE.md).
import { invoke } from "@tauri-apps/api/core";
import { listen, type UnlistenFn } from "@tauri-apps/api/event";
import { open } from "@tauri-apps/plugin-dialog";

export const inTauri = typeof window !== "undefined" && "__TAURI_INTERNALS__" in window;

// ---- types: the serde output of core/src (camelCase) ------------------------------

export type Platform = "linux" | "macos" | "windows";
export type Tool = "python" | "godot" | "docker";

export interface AppInfo {
  version: string;
  platform: Platform;
  /** Where launcher.json lives. */
  configFile: string | null;
  /** The launcher works on the copy of OpenRAC it carries: only what a player does is offered. */
  standalone: boolean;
}

/** core/src/config.rs */
export interface Config {
  root: string | null;
  python: string | null;
  godot: string | null;
  docker: string | null;
  /** Where games are set up from the player's discs (OpenGOAL's install folder). */
  installDir: string | null;
  /** Show what contributors use (builds, checks, progress); off, a player's three steps. */
  developer: boolean;
  setupComplete: boolean;
  lastVersion: string | null;
  discordRpc: boolean;
  discordClientId: string | null;
  /** "compact", or "classic": the original design (src/layouts/compact.css). */
  layout: "compact" | "classic";
}

export type DiscordStatus =
  | { kind: "idle" }
  | {
      kind: "viewingGame";
      title: string;
      region: string;
      progressPct: number | null;
      gameId: string;
    }
  | {
      kind: "playingGame";
      title: string;
      region: string;
      gameId: string;
      startTime?: number | null;
    }
  | {
      /** A long job for a game: set-up, build, the editor's preparation. */
      kind: "working";
      task: string;
      title: string;
      gameId: string;
      startTime?: number | null;
    };

export interface Candidate {
  path: string;
  source: string;
}

/** core/src/detect.rs */
export interface Detected {
  roots: Candidate[];
  pythons: Candidate[];
  godots: Candidate[];
  dockers: Candidate[];
}

export interface Check {
  ok: boolean;
  version: string | null;
  message: string;
}

/** core/src/actions.rs */
/** What an action works on: the decompilation, the PC port, or the level editor. */
export type ActionArea = "decomp" | "port" | "editor";
export type ActionKind = "setup" | "build" | "check" | "play" | "edit";
export type ActionState = "connected" | "unverified" | "planned";
export type Scope = { kind: "repository" } | { kind: "version"; key: string };

export interface ActionView {
  id: string;
  label: string;
  description: string;
  kind: ActionKind;
  area: ActionArea;
  state: ActionState;
  detached: boolean;
  docs: string | null;
  todo: string | null;
  thisPlatform: boolean;
  runnable: boolean;
  blockers: string[];
  /** Its artifact exists: a step that makes one has been done. */
  done: boolean;
}

/** core/src/catalog.rs */
export interface FileSpec {
  file: string;
  size: number | null;
  sha1: string | null;
}

export interface Input {
  from: string;
  path: string;
  target: string | null;
}

export interface Source {
  name: string;
  repo: string;
  license: string | null;
  commit: string | null;
  date: string | null;
}

export interface Progress {
  matchedCode: number;
  totalCode: number;
  matchedFunctions: number | null;
  totalFunctions: number | null;
  fuzzyPercent: number | null;
  percent: number;
  date: string | null;
  note: string | null;
}

/** core/src/status.rs */
export type DiscState = "found" | "mismatch" | "missing" | "unknown";

export interface VersionStatus {
  disc: { state: DiscState; path: string | null; expected: string | null; message: string };
  boot: { path: string; present: boolean; message: string } | null;
  inputs: { from: string; path: string; present: boolean; message: string }[];
  inputsReady: boolean;
}

/** core/src/library.rs: a version, its status and its actions. */
export interface VersionView {
  key: string;
  game: string;
  name: string;
  title: string;
  region: string;
  serial: string;
  dir: string;
  disc: FileSpec | null;
  boot: FileSpec | null;
  inputs: Input[];
  target: string | null;
  setup: string | null;
  buildHost: string | null;
  readme: string | null;
  source: Source | null;
  progress: Progress | null;
  status: VersionStatus;
  actions: ActionView[];
}

export interface GameView {
  id: string;
  title: string;
  year: number | null;
  /** How far the game is set up from the player's disc. */
  install: InstallState | null;
  versions: VersionView[];
}

export interface Library {
  root: string;
  platform: Platform;
  toolchains: boolean;
  repository: ActionView[];
  games: GameView[];
  actionsError: string | null;
}

/** core/src/jobs.rs */
export interface Started {
  /** 0 for a detached launch (the game, Godot), which has no job. */
  id: number;
  title: string;
  command: string;
  detached: boolean;
}

export type JobEvent =
  | { type: "output"; id: number; stream: "stdout" | "stderr"; line: string }
  | { type: "exit"; id: number; code: number | null; cancelled: boolean };

/** core/src/install.rs: a game set up from the player's disc, OpenGOAL's way. */
export interface BuildInfo {
  serial: string;
  /** The version the disc is, `rac1/pal`. */
  version: string;
  elfSha1: string;
  files: number;
  image: string;
}

export interface InstallState {
  /** `<install folder>/active/<game>/data`. */
  dir: string;
  /** The disc was extracted and validated: which build it is. */
  extracted: BuildInfo | null;
  decompiled: boolean;
  compiled: boolean;
}

export interface SaveSlotInfo {
  slotIndex: number;
  filename: string;
  path: string;
  size: number;
  exists: boolean;
  isEmpty: boolean;
  bolts: number | null;
  planetId: number | null;
  planetName: string | null;
  timestamp: string | null;
  modifiedMillis: number | null;
}

export interface SaveBackupInfo {
  name: string;
  path: string;
  createdMillis: number;
  totalSize: number;
}

export interface GameSaveStatus {
  serial: string;
  memcardDir: string;
  exists: boolean;
  gameFolderName: string | null;
  title: string | null;
  slots: SaveSlotInfo[];
  backups: SaveBackupInfo[];
}

// ---- calls ------------------------------------------------------------------------

async function call<T>(command: string, args?: Record<string, unknown>): Promise<T> {
  if (inTauri) return invoke<T>(command, args);
  // Loaded only in a browser, so the desktop build never carries the mock.
  const { mockCall } = await import("./mock");
  return mockCall(command, args) as Promise<T>;
}

export const api = {
  appInfo: () => call<AppInfo>("app_info"),
  getConfig: () => call<Config>("get_config"),
  saveConfig: (config: Config) => call<Config>("save_config", { config }),
  detect: () => call<Detected>("detect"),
  checkRoot: (path: string) => call<Check>("check_root", { path }),
  checkTool: (tool: Tool, path: string) => call<Check>("check_tool", { tool, path }),
  library: () => call<Library>("library"),
  /** `level`: which level a per-level action is for (the level editor's projects, one each). */
  runAction: (scope: Scope, id: string, level?: number) =>
    call<Started>("run_action", { scope, id, level: level ?? null }),
  cancelJob: (id: number) => call<null>("cancel_job", { id }),
  /** A file or folder in the checkout, relative to it (`games/rac1/pal/README.md`). */
  openPath: (path: string) => call<null>("open_path", { path }),
  openUrl: (url: string) => call<null>("open_url", { url }),
  /** Sets a game up from the image of the player's disc: the extractor, as a job. */
  installGame: (game: string, image: string) => call<Started>("install_game", { game, image }),
  /** Removes what setting a game up made (not the disc image, not the saves). */
  uninstallGame: (game: string) => call<null>("uninstall_game", { game }),
  syncProgressFromWeb: () => call<Library>("sync_progress_from_web"),
  applyProgressJson: (json: string) => call<Library>("apply_progress_json", { json }),
  setDiscordStatus: (status: DiscordStatus) => call<null>("set_discord_status", { status }),
  inspectSaves: (serial: string) => call<GameSaveStatus>("inspect_saves", { serial }),
  backupSaves: (serial: string, note?: string) => call<SaveBackupInfo>("backup_saves", { serial, note }),
  restoreBackup: (serial: string, backupName: string) => call<null>("restore_backup", { serial, backupName }),
  openSavesFolder: (serial: string) => call<null>("open_saves_folder", { serial }),
};

/** A folder picker; null when cancelled (or in the browser preview). */
export async function pickFolder(title: string): Promise<string | null> {
  if (!inTauri) return null;
  const picked = await open({ directory: true, title });
  return typeof picked === "string" ? picked : null;
}

/** A file picker (a program); null when cancelled (or in the browser preview). */
export async function pickFile(title: string): Promise<string | null> {
  if (!inTauri) return null;
  const picked = await open({ directory: false, multiple: false, title });
  return typeof picked === "string" ? picked : null;
}

/** A picker for a disc image; null when cancelled (or in the browser preview). */
/** The player's disc image for setting a game up: an .iso, as OpenGOAL asks for. */
export async function pickIso(title: string): Promise<string | null> {
  if (!inTauri) return "/home/you/Discs/game.iso";
  const picked = await open({
    directory: false,
    multiple: false,
    title,
    filters: [{ name: "ISO", extensions: ["iso", "ISO"] }],
  });
  return typeof picked === "string" ? picked : null;
}

/** Job output and exits, from `job-output` and `job-exit` events. */
export async function onJobEvent(handler: (event: JobEvent) => void): Promise<UnlistenFn> {
  if (!inTauri) {
    const { mockListen } = await import("./mock");
    return mockListen(handler);
  }
  const offOutput = await listen<JobEvent>("job-output", (e) => {
    handler(e.payload);
  });
  const offExit = await listen<JobEvent>("job-exit", (e) => {
    handler(e.payload);
  });
  return () => {
    offOutput();
    offExit();
  };
}

/** A command's error as text: Tauri rejects with the Rust side's String. */
export function errorText(e: unknown): string {
  if (typeof e === "string") return e;
  if (e instanceof Error) return e.message;
  return JSON.stringify(e);
}
