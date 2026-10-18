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

/** core/src/discs.rs: a disc image in the games folder that is a game OpenRAC knows. */
export interface FoundDisc {
  path: string;
  serial: string;
  game: string;
  /** `rac1/pal`. */
  version: string;
  title: string;
  region: string;
}

/** core/src/config.rs */
export interface Config {
  root: string | null;
  python: string | null;
  godot: string | null;
  docker: string | null;
  /** Where games are set up from the player's discs (OpenGOAL's install folder). */
  installDir: string | null;
  /** The player's folder of PlayStation 2 disc images, looked through for the games OpenRAC knows. */
  discsDir: string | null;
  /** Show what contributors use (builds, checks, progress); off, a player's three steps. */
  developer: boolean;
  /** The save editor shows every field of a save, raw bytes included (for debugging). */
  saveEditorAdvanced: boolean;
  setupComplete: boolean;
  lastVersion: string | null;
  discordRpc: boolean;
  /** Look for a new launcher at start (Settings). */
  checkUpdates: boolean;
  discordClientId: string | null;
  /** "compact", or "classic": the original design (src/layouts/compact.css). */
  layout: "compact" | "classic";
  /** The game writes a detailed log (OPENRAC_LOG=verbose); off keeps logs small. */
  detailedGameLog: boolean;
}

/** One of a game's logs or crash reports (core/src/reports.rs). */
export interface ReportEntry {
  kind: "crash" | "session";
  /** Its path in the game's data folder: `crashes/crash-....txt`, `openrac-rac1-pal.log`. */
  file: string;
  program: string | null;
  /** When it happened (a crash) or began (a session), seconds since 1970. */
  time: number;
  /** How long the session ran, in seconds. */
  duration: number | null;
  /** A crash's reason, in the port's words: for the Details. */
  reason: string | null;
  level: number | null;
  planet: string | null;
  crashed: boolean;
  session: string | null;
  size: number;
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
  /** Functional C: does what retail does without matching its bytes; never counted as matched. */
  functional?: { functions: number; code: number; percent: number } | null;
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

/** What a run gives an action besides its name: `{level}`, `{path}`, `{option}`. */
export interface RunInputs {
  level?: number;
  path?: string;
  option?: string;
}

/** core/src/overrides.rs: an overrides pack (replacement textures and models). */
export interface PackView {
  id: string;
  name: string;
  author: string;
  description: string;
  enabled: boolean;
  levels: number;
  textures: number;
  models: number;
  levelFiles: number;
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
  /** The save editor can open it. */
  editable: boolean;
}

// ---- the save editor (core/src/save_edit.rs) ---------------------------------------

export type Nanotech = "standard" | "premium" | "ultra";

export interface ItemState {
  id: number;
  owned: boolean;
  /** Ammo, for an item that has it. */
  ammo: number | null;
  /** The gold version, for an item that has one. */
  gold: boolean | null;
}

export interface PlanetState {
  id: number;
  unlocked: boolean;
  /** One per gold bolt on the planet. */
  goldBolts: boolean[];
}

/** What the editor changes; sent back whole. */
export interface SaveFields {
  /** The planet the save loads on, a level number. */
  planet: number;
  bolts: number;
  nanotech: Nanotech;
  items: ItemState[];
  planets: PlanetState[];
  skillPoints: boolean[];
}

export interface ItemInfo {
  id: number;
  name: string;
  kind: "weapon" | "gadget";
  maxAmmo: number | null;
  hasGold: boolean;
}

export interface PlanetInfo {
  id: number;
  name: string;
  location: string;
  /** A ship destination the player may lock or unlock. */
  canUnlock: boolean;
  /** The save can be moved there in the default mode. */
  canLand: boolean;
}

/** One chunk of the file, for the advanced editor. */
export interface ChunkView {
  /** `null`: the global section; else the planet's level number. */
  section: number | null;
  id: number;
  size: number;
  name: string;
  /** Its meaning is decoded; otherwise raw bytes only. */
  known: boolean;
  hex: string;
}

export interface RawPatch {
  section: number | null;
  id: number;
  hex: string;
}

/** What a write changes: the default mode's fields, or raw chunks (advanced mode). */
export type SaveChange = { kind: "fields"; fields: SaveFields } | { kind: "raw"; chunks: RawPatch[] };

export interface SaveView {
  serial: string;
  slot: number;
  /** The file as read; a write is refused if it changed since. */
  fingerprint: string;
  planetName: string | null;
  savedAt: string | null;
  fields: SaveFields;
  items: ItemInfo[];
  planets: PlanetInfo[];
  /** Every level, 0..18. */
  levels: PlanetInfo[];
  maxBolts: number;
  chunks: ChunkView[];
  /** Time played as the game counts it, in seconds. */
  playedSeconds: number;
  /** The 30 skill points' names from the game's text, in save order; empty when not readable. */
  skillNames: string[];
}

export interface WriteResult {
  backup: SaveBackupInfo;
  view: SaveView;
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
  /** `inputs`: which level a per-level action is for (the level editor's projects, one each), a
   *  file or folder the player chose (`{path}`), one of the action's options (`{option}`). */
  runAction: (scope: Scope, id: string, inputs: RunInputs = {}) =>
    call<Started>("run_action", {
      scope,
      id,
      level: inputs.level ?? null,
      path: inputs.path ?? null,
      option: inputs.option ?? null,
    }),
  cancelJob: (id: number) => call<null>("cancel_job", { id }),
  /** A file or folder in the checkout, relative to it (`games/rac1/README.md`). */
  openPath: (path: string) => call<null>("open_path", { path }),
  openUrl: (url: string) => call<null>("open_url", { url }),
  /** Sets a game up from the image of the player's disc: the extractor, as a job. */
  installGame: (game: string, image: string) => call<Started>("install_game", { game, image }),
  /** Removes what setting a game up made (not the disc image, not the saves). */
  uninstallGame: (game: string) => call<null>("uninstall_game", { game }),
  /** The games OpenRAC knows among the images in the games folder. */
  scanDiscs: () => call<FoundDisc[]>("scan_discs"),
  syncProgressFromWeb: () => call<Library>("sync_progress_from_web"),
  applyProgressJson: (json: string) => call<Library>("apply_progress_json", { json }),
  setDiscordStatus: (status: DiscordStatus) => call<null>("set_discord_status", { status }),
  inspectSaves: (serial: string) => call<GameSaveStatus>("inspect_saves", { serial }),
  backupSaves: (serial: string, note?: string) => call<SaveBackupInfo>("backup_saves", { serial, note }),
  restoreBackup: (serial: string, backupName: string) => call<null>("restore_backup", { serial, backupName }),
  openSavesFolder: (serial: string) => call<null>("open_saves_folder", { serial }),
  /** A game's session logs and crash reports, the newest first. */
  reportsList: (game: string) => call<ReportEntry[]>("reports_list", { game }),
  /** The newest crash report written since `since` (seconds since 1970). */
  latestCrash: (game: string, since: number) => call<ReportEntry | null>("latest_crash", { game, since }),
  /** The short text to paste in an issue or on Discord (home folder as ~). */
  reportText: (game: string, file: string) => call<string>("report_text", { game, file }),
  /** Saves the report's .zip in Downloads and shows it; returns where (home folder as ~). */
  reportSave: (game: string, file: string) => call<string>("report_save", { game, file }),
  /** Opens the folder of a game's logs, or of its crash reports. */
  openReportsFolder: (game: string, crashes: boolean) => call<null>("open_reports_folder", { game, crashes }),
  /** One slot for the save editor. */
  readSave: (serial: string, slot: number) => call<SaveView>("read_save", { serial, slot }),
  /** Writes the editor's change: refused while the game runs with the card; backs the card up first. */
  writeSave: (serial: string, slot: number, fingerprint: string, change: SaveChange) =>
    call<WriteResult>("write_save", { serial, slot, fingerprint, change }),
  /** A game's overrides packs (core/src/overrides.rs), in the order they apply. */
  overridesList: (game: string) => call<PackView[]>("overrides_list", { game }),
  overridesSetEnabled: (game: string, id: string, enabled: boolean) =>
    call<PackView[]>("overrides_set_enabled", { game, id, enabled }),
  overridesMove: (game: string, id: string, up: boolean) => call<PackView[]>("overrides_move", { game, id, up }),
  overridesRemove: (game: string, id: string) => call<PackView[]>("overrides_remove", { game, id }),
  /** A folder inside the game's set-up folder (an export), opened with the system. */
  openGameFolder: (game: string, path: string) => call<null>("open_game_folder", { game, path }),
};

/** A folder picker; null when cancelled (or in the browser preview). */
export async function pickFolder(title: string): Promise<string | null> {
  if (!inTauri) return null;
  const picked = await open({ directory: true, title });
  return typeof picked === "string" ? picked : null;
}

/** Restart the launcher application (e.g. after installing an update). */
export async function relaunchApp(): Promise<void> {
  if (inTauri) {
    await invoke("relaunch_app");
  } else {
    window.location.reload();
  }
}

/** A file picker (a program); null when cancelled (or in the browser preview). */
export async function pickFile(title: string): Promise<string | null> {
  if (!inTauri) return null;
  const picked = await open({ directory: false, multiple: false, title });
  return typeof picked === "string" ? picked : null;
}

/** A picker for an overrides pack's .zip; null when cancelled (in the browser preview, a made-up one). */
export async function pickZip(title: string): Promise<string | null> {
  if (!inTauri) return "/home/you/Downloads/HD Ratchet.zip";
  const picked = await open({
    directory: false,
    multiple: false,
    title,
    filters: [{ name: "Zip", extensions: ["zip"] }],
  });
  return typeof picked === "string" ? picked : null;
}

/** Files and folders dropped on the window, while `handler` is listening (the desktop app only). */
export async function onDrop(
  handler: (paths: string[]) => void,
  hovering: (over: boolean) => void = () => undefined,
): Promise<() => void> {
  if (!inTauri) return () => undefined;
  const { getCurrentWebview } = await import("@tauri-apps/api/webview");
  return getCurrentWebview().onDragDropEvent((event) => {
    if (event.payload.type === "drop") handler(event.payload.paths);
    else hovering(event.payload.type !== "leave");
  });
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
