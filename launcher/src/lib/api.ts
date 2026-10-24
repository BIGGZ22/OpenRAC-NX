// Typed wrappers around the Rust commands (src-tauri/src/lib.rs, whose data
// types live in ../core). In a plain browser (`yarn dev` without Tauri)
// every call goes to mock.ts instead, so the UI can be worked on and
// screenshotted without the desktop app, a disc or a toolchain.
//
// Adding a command: the Rust function, its name in generate_handler!, a
// wrapper and its types here, and a case in mock.ts (docs/ARCHITECTURE.md).
import { invoke } from "@tauri-apps/api/core";
import { listen, type UnlistenFn } from "@tauri-apps/api/event";
import { confirm, open } from "@tauri-apps/plugin-dialog";
import { REPO_URL } from "./links";
import type { BackgroundStatus } from "./progress";
import type { ActionArea } from "./generated/ActionArea";
import type { ActionKind } from "./generated/ActionKind";
import type { ActionState } from "./generated/ActionState";
import type { ActionView } from "./generated/ActionView";
import type { AndroidRelease } from "./generated/AndroidRelease";
import type { BakeStatus } from "./generated/BakeStatus";
import type { Candidate } from "./generated/Candidate";
import type { Check } from "./generated/Check";
import type { Config } from "./generated/Config";
import type { Detected } from "./generated/Detected";
import type { DifferenceKind } from "./generated/DifferenceKind";
import type { DifferenceView } from "./generated/DifferenceView";
import type { Differences } from "./generated/Differences";
import type { DiscChoice } from "./generated/DiscChoice";
import type { DiscState } from "./generated/DiscState";
import type { EnhancedView } from "./generated/EnhancedView";
import type { FoundDisc } from "./generated/FoundDisc";
import type { GameSaveStatus } from "./generated/GameSaveStatus";
import type { GameView } from "./generated/GameView";
import type { JobEvent } from "./generated/JobEvent";
import type { Library } from "./generated/Library";
import type { ModSources } from "./generated/ModSources";
import type { ModUpdate } from "./generated/ModUpdate";
import type { Nanotech } from "./generated/Nanotech";
import type { PackView } from "./generated/PackView";
import type { Platform } from "./generated/Platform";
import type { PluginInstalled } from "./generated/PluginInstalled";
import type { PluginPreset } from "./generated/PluginPreset";
import type { PluginSetting } from "./generated/PluginSetting";
import type { PluginStatus } from "./generated/PluginStatus";
import type { PluginView } from "./generated/PluginView";
import type { RawPatch } from "./generated/RawPatch";
import type { RemoteInstalled } from "./generated/RemoteInstalled";
import type { RemoteMod } from "./generated/RemoteMod";
import type { RemotePreview } from "./generated/RemotePreview";
import type { ReportEntry } from "./generated/ReportEntry";
import type { SaveBackupInfo } from "./generated/SaveBackupInfo";
import type { SaveChange } from "./generated/SaveChange";
import type { SaveFields } from "./generated/SaveFields";
import type { SaveView } from "./generated/SaveView";
import type { Scope } from "./generated/Scope";
import type { Started } from "./generated/Started";
import type { Tool } from "./generated/Tool";
import type { UpdateChannel } from "./generated/UpdateChannel";
import type { UpdateSource } from "./generated/UpdateSource";
import type { VersionView } from "./generated/VersionView";
import type { WriteResult } from "./generated/WriteResult";

export const inTauri = typeof window !== "undefined" && "__TAURI_INTERNALS__" in window;

/** The Android launcher (docs/port/ANDROID.md): files come from the system's picker, which gives
 *  no folders and no paths, and the GPU driver packages are its own. `?android` in the browser
 *  preview shows its pages. */
export const onAndroid =
  typeof navigator !== "undefined" &&
  (inTauri
    ? /android/i.test(navigator.userAgent)
    : typeof location !== "undefined" && new URLSearchParams(location.search).has("android"));

export type { ActionArea };
export type { ActionKind };
export type { ActionState };
export type { ActionView };
export type { AndroidRelease };
export type { BakeStatus };
export type { Candidate };
export type { Check };
export type { Config };
export type { Detected };
export type { DifferenceKind };
export type { DifferenceView };
export type { Differences };
export type { DiscChoice };
export type { DiscState };
export type { EnhancedView };
export type { FoundDisc };
export type { GameSaveStatus };
export type { GameView };
export type { JobEvent };
export type { Library };
export type { ModSources };
export type { ModUpdate };
export type { Nanotech };
export type { PackView };
export type { PluginPreset };
export type { PluginSetting };
export type { PluginView };
export type { RawPatch };
export type { RemoteMod };
export type { RemotePreview };
export type { ReportEntry };
export type { SaveBackupInfo };
export type { SaveChange };
export type { SaveFields };
export type { SaveView };
export type { Scope };
export type { Started };
export type { UpdateChannel };
export type { UpdateSource };
export type { VersionView };

// ---- types ------------------------------------------------------------------------
//
// The types of what the Tauri commands send and take are generated from the Rust types in core/src
// (ts-rs, `cargo test -p openrac-launcher-core --test generated_types`; docs/ARCHITECTURE.md, "The
// page's types") and live in ./generated/, never edited by hand. They are re-exported from here
// so the rest of the page imports every type from "./api". What is written out below is what the
// generator does not reach: the types of src-tauri's own commands, and what the page sends.

/** What the app says about itself (src-tauri/src/lib.rs, AppInfo). */
export interface AppInfo {
  version: string;
  platform: Platform;
  /** Where launcher.json lives. */
  configFile: string | null;
  /** The launcher works on the copy of OpenRAC it carries: only what a player does is offered. */
  standalone: boolean;
  /** Where its updates come from: the repository that built it, set by the build. */
  updates: UpdateSource;
  /** That repository's two channels, which the player chooses between (Config.updateChannel). */
  updateChannels: Record<UpdateChannel, UpdateSource>;
}

/** The games folder looked through (scan_discs, src-tauri/src/lib.rs). */
export interface DiscScan {
  found: FoundDisc[];
  /** Only with developer tools on: the games whose disc can be switched. */
  choices: DiscChoice[];
}

/** What the page tells Discord it is doing (core/src/discord.rs, DiscordStatus, whose fields
 *  are read in this camelCase since `rename_all_fields`; its test holds the two together).
 *  Written by hand, not generated yet: generating it is a follow-up in PROGRESS.md. */
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

/** What a run gives an action besides its name: `{level}`, `{path}`, `{option}`. */
export interface RunInputs {
  level?: number;
  path?: string;
  option?: string;
}

/** What a gameplay mod is: code the game loads (a library), Luau scripts the game runs sandboxed,
 *  or data only (docs/port/MODS.md, section 11). */
export type ModKind = PluginView["kind"];

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
  /** Takes a Godot that works when none is set or the one set is gone; the config as it is then. */
  adoptGodot: () => call<Config>("adopt_godot"),
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
  /** A file or folder in the checkout, relative to it (`games/rac1/README.md`). Android opens no
   *  app's own files: a document there is read on GitHub, and a folder cannot be shown. */
  openPath: (path: string) =>
    onAndroid
      ? /^[\w./-]+\.\w+$/.test(path) && !path.startsWith("/") && !path.includes("..")
        ? call<null>("open_url", { url: `${REPO_URL}/blob/master/${path}` })
        : Promise.reject(new Error("Android cannot show the launcher's own folders."))
      : call<null>("open_path", { path }),
  openUrl: (url: string) => call<null>("open_url", { url }),
  /** Sets a game up from the image of the player's disc: the extractor, as a job. */
  installGame: (game: string, image: string) => call<Started>("install_game", { game, image }),
  /** The plain image a set-up reads: an .iso as it is, a .chd unpacked once (`disc-copy` events). */
  prepareDisc: (image: string) => call<string>("prepare_disc", { image }),
  /** Removes what setting a game up made (not the disc image, not the saves). */
  uninstallGame: (game: string) => call<null>("uninstall_game", { game }),
  /** The games OpenRAC knows among the images in the games folder, and a developer's disc choices. */
  scanDiscs: () => call<DiscScan>("scan_discs"),
  /** Developers only: plays this version's disc of its game; the other's set-up is kept aside. */
  chooseDisc: (version: string) => call<Config>("choose_disc", { version }),
  syncProgressFromWeb: () => call<Library>("sync_progress_from_web"),
  applyProgressJson: (json: string) => call<Library>("apply_progress_json", { json }),
  setDiscordStatus: (status: DiscordStatus) => call<null>("set_discord_status", { status }),
  inspectSaves: (serial: string) => call<GameSaveStatus>("inspect_saves", { serial }),
  backupSaves: (serial: string, note?: string) => call<SaveBackupInfo>("backup_saves", { serial, note }),
  restoreBackup: (serial: string, backupName: string) => call<null>("restore_backup", { serial, backupName }),
  openSavesFolder: (serial: string) => call<null>("open_saves_folder", { serial }),
  /** Android: OpenRAC's place in the system's Files app (the saves, settings, games, discs). */
  showFiles: () => call<null>("show_files"),
  /** A game's session logs and crash reports, the newest first. */
  reportsList: (game: string) => call<ReportEntry[]>("reports_list", { game }),
  /** The newest crash report written since `since` (seconds since 1970). */
  latestCrash: (game: string, since: number) => call<ReportEntry | null>("latest_crash", { game, since }),
  /** The short text to paste in an issue or on Discord (home folder as ~). */
  reportText: (game: string, file: string) => call<string>("report_text", { game, file }),
  /** Saves the report's .zip in Downloads and shows it; returns where (home folder as ~). */
  reportSave: (game: string, file: string) => call<string>("report_save", { game, file }),
  /** Deletes one log or crash report from the game's data folder (core refuses any other file). */
  reportDelete: (game: string, file: string) => call<null>("report_delete", { game, file }),
  /** What the port does otherwise than the PS2 game, for one version (`rac1`, `pal`). */
  differences: (game: string, version: string) => call<Differences>("differences", { game, version }),
  /** Switches a game fix on (the port's) or off (as the original plays) in the version's settings. */
  setGameFix: (game: string, version: string, id: string, on: boolean) =>
    call<Differences>("set_game_fix", { game, version, id, on }),
  /** Deletes every log and crash report of the game; returns how many files went. */
  reportsDeleteAll: (game: string) => call<number>("reports_delete_all", { game }),
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
  /** A game's native mods (core/src/plugins.rs) for its disc `version` ("pal"), in load order. */
  pluginsList: (game: string, version: string) => call<PluginView[]>("plugins_list", { game, version }),
  pluginsSetEnabled: (game: string, version: string, id: string, enabled: boolean) =>
    call<PluginView[]>("plugins_set_enabled", { game, version, id, enabled }),
  pluginsMove: (game: string, version: string, id: string, up: boolean) =>
    call<PluginView[]>("plugins_move", { game, version, id, up }),
  pluginsSetSetting: (game: string, version: string, id: string, key: string, value: unknown) =>
    call<PluginView[]>("plugins_set_setting", { game, version, id, key, value }),
  /** The built-in Enhanced graphics entry of a version (core/src/enhanced.rs); null where it has none. */
  enhancedView: (game: string, version: string) => call<EnhancedView | null>("enhanced_view", { game, version }),
  enhancedSetEnabled: (game: string, version: string, enabled: boolean) =>
    call<EnhancedView | null>("enhanced_set_enabled", { game, version, enabled }),
  enhancedSetSetting: (game: string, version: string, key: string, value: unknown) =>
    call<EnhancedView | null>("enhanced_set_setting", { game, version, key, value }),
  pluginsRemove: (game: string, version: string, id: string) =>
    call<PluginView[]>("plugins_remove", { game, version, id }),
  pluginsInstall: (game: string, version: string, path: string) =>
    call<PluginInstalled>("plugins_install", { game, version, path }),
  /** The mods a chosen folder holds: itself, or each mod of a repository's checkout (its build first). */
  pluginsModFolders: (path: string) => call<string[]>("plugins_mod_folders", { path }),
  pluginsStatus: (game: string) => call<PluginStatus | null>("plugins_status", { game }),
  /** Mods from a repository's address (core/src/remote.rs): what it holds, before anything is installed. */
  pluginsPreview: (game: string, version: string, url: string) =>
    call<RemotePreview>("plugins_preview", { game, version, url }),
  pluginsInstallRemote: (game: string, version: string, url: string, id: string) =>
    call<RemoteInstalled>("plugins_install_remote", { game, version, url, id }),
  pluginsCheckUpdates: (game: string, version: string) =>
    call<{ updates: ModUpdate[]; errors: string[] }>("plugins_check_updates", { game, version }),
  pluginsUpdate: (game: string, version: string, id: string) =>
    call<RemoteInstalled>("plugins_update", { game, version, id }),
  pluginsSources: (game: string) => call<ModSources>("plugins_sources", { game }),
  pluginsSetPin: (game: string, id: string, pin: boolean) => call<ModSources>("plugins_set_pin", { game, id, pin }),
  /** A folder inside the game's set-up folder (an export), opened with the system. */
  openGameFolder: (game: string, path: string) => call<null>("open_game_folder", { game, path }),
  /** A folder a job wrote (a job exit's `outputs`), opened with the system; a file opens its folder. */
  openOutput: (path: string) => call<null>("open_output", { path }),
  /** One level's Godot project in the Godot editor (the version's editor-open action). */
  openLevelInGodot: (version: string, level: number) => call<Started>("open_level_in_godot", { version, level }),
  /** Android: the player's GPU driver packages (src-tauri/src/android.rs). */
  gpuDrivers: () => call<GpuDrivers>("gpu_drivers"),
  /** Android: the player picks a package's zip; null when they cancelled. */
  gpuDriverImport: () => call<{ driver?: GpuDriver }>("gpu_driver_import").then((r) => r.driver ?? null),
  gpuDriverSelect: (id: string | null) => call<null>("gpu_driver_select", { id }),
  gpuDriverRemove: (id: string) => call<null>("gpu_driver_remove", { id }),
  /** Android: what the game draws with from its next start. */
  gpuRendererSelect: (renderer: Renderer) => call<null>("gpu_renderer_select", { renderer }),
  /** Android: what the set-up's notification shows while the launcher is in the background; null:
   *  the set-up is over (SetupService.kt). */
  backgroundStatus: (status: BackgroundStatus | null) => call<null>("background_status", { ...status }),
  toolsReady: () => call<null>("tools_ready"),
  gameStatus: () => call<GameStatus>("game_status"),
  /** Back to the running game, or (`stop`) end it. */
  gameWindow: (stop: boolean) => call<null>("game_window", { stop }),
  /** Android: a newer launcher on the releases, or null (desktops use Tauri's updater). */
  updateCheck: (channel: UpdateChannel) => call<AndroidRelease | null>("update_check", { channel }),
  /** A desktop: Tauri's updater asked at the channel's manifest; its update, or null. */
  launcherUpdateCheck: (channel: UpdateChannel) => call<LauncherUpdate | null>("launcher_update_check", { channel }),
  /** Android: downloads the release's APK, checks its signature, opens Android's installer. */
  updateInstall: (url: string, signature: string) => call<null>("update_install", { url, signature }),
};

/** A desktop's update as Tauri's updater holds it: plugin-updater's `Update` is made from it. */
export interface LauncherUpdate {
  rid: number;
  currentVersion: string;
  version: string;
  date?: string;
  body?: string;
  rawJson: Record<string, unknown>;
}

/** The channel a launcher updates on: the player's choice, or the one it was built from. */
export function updateChannelOf(info: AppInfo, config: Config | null): UpdateChannel {
  return config?.updateChannel ?? (info.updates.channel ? "dev" : "stable");
}

/** A GPU driver package the Android launcher keeps (OpenracPlugin.kt): its meta.json. */
export interface GpuDriver {
  id: string;
  name: string;
  description: string;
  author: string;
  version: string;
  vendor: string;
  driverVersion: string;
  minApi: number;
  /** The driver library in the package (meta.json's libraryName). */
  library: string;
}

/** Android: the game's activity, while it runs beside the launcher. */
export interface GameStatus {
  running: boolean;
  /** Its library ("openrac-rac1-pal"); absent when the launcher restarted while it ran. */
  library?: string | null;
}

export type Renderer = "gles" | "vulkan";

export interface GpuDrivers {
  drivers: GpuDriver[];
  /** The package the game loads; absent or null: the system's driver (Android's JSON drops a
   *  null value). */
  selected?: string | null;
  /** What the game draws with: OpenGL ES (the default) or its Vulkan renderer. */
  renderer: Renderer;
  device: {
    model: string;
    api: number;
    soc: string;
    /** Which kernel GPU driver the device has: packages load on Adreno's only. */
    gpu: "adreno" | "mali" | "other";
  };
}

/** How far a disc image's copy into the Android app's storage, or a CHD's unpacking, is (`disc-copy`). */
export interface DiscCopy {
  name: string;
  copied: number;
  total: number;
}

/** Asks before something that cannot be undone, with `yes` on the button that goes ahead. The
 *  system's dialog: in the desktop app the page's own confirm() says yes on Linux (WebKitGTK)
 *  without asking anything. The browser preview asks with the page's. */
export async function ask(message: string, yes: string): Promise<boolean> {
  if (!inTauri) return window.confirm(message);
  return confirm(message, { title: "OpenRAC", kind: "warning", okLabel: yes, cancelLabel: "Cancel" });
}

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
  // Android's picker hands over a file, not a path: it is copied into the app's cache first.
  if (onAndroid) return await call<string | null>("pick_zip");
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

/** The player's disc image for setting a game up: an .iso, as OpenGOAL asks for, or a .chd. */
export async function pickIso(title: string, copying?: (progress: DiscCopy) => void): Promise<string | null> {
  if (!inTauri) return "/home/you/Discs/game.iso";
  if (onAndroid) {
    // The system's picker; the image is copied into the app's storage as it is picked.
    const off = copying ? await onDiscCopy(copying) : null;
    try {
      return await call<string | null>("pick_disc");
    } finally {
      off?.();
    }
  }
  const picked = await open({
    directory: false,
    multiple: false,
    title,
    filters: [{ name: "Disc image", extensions: ["iso", "ISO", "chd", "CHD"] }],
  });
  return typeof picked === "string" ? picked : null;
}

/** `disc-copy` events: a picked image's copy (Android) or a .chd's unpacking. */
export async function onDiscCopy(handler: (progress: DiscCopy) => void): Promise<UnlistenFn> {
  if (!inTauri) return () => undefined;
  return listen<DiscCopy>("disc-copy", (e) => {
    handler(e.payload);
  });
}

/** `update-progress` events: the Android update's download. */
export async function onUpdateProgress(
  handler: (progress: { downloaded: number; total: number }) => void,
): Promise<UnlistenFn> {
  if (!inTauri) return () => undefined;
  return listen<{ downloaded: number; total: number }>("update-progress", (e) => {
    handler(e.payload);
  });
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
