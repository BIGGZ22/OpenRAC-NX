// The browser preview's stand-in for the Rust side (`yarn dev` without
// Tauri). It reads the same files the desktop app reads (games/*/game.json,
// progress/summary.json, launcher/actions.json, bundled at build time), so
// the preview shows the real games and actions; the disc and tool states
// are made up, and jobs print a few lines and succeed.
//
// Add a case here for every new command in api.ts.
import actionsFile from "../../actions.json";
import summaryFile from "../../../progress/summary.json";
import packageFile from "../../package.json";
import { onAndroid } from "./api";
import { mockEnhanced } from "./mockEnhanced";
import type { ActionView } from "./generated/ActionView";
import type { BuildInfo } from "./generated/BuildInfo";
import type { Check } from "./generated/Check";
import type { Config } from "./generated/Config";
import type { Detected } from "./generated/Detected";
import type { DifferenceView } from "./generated/DifferenceView";
import type { Differences } from "./generated/Differences";
import type { GameView } from "./generated/GameView";
import type { InstallState } from "./generated/InstallState";
import type { JobEvent } from "./generated/JobEvent";
import type { Library } from "./generated/Library";
import type { ModSources } from "./generated/ModSources";
import type { PackView } from "./generated/PackView";
import type { Platform } from "./generated/Platform";
import type { PluginStatus } from "./generated/PluginStatus";
import type { PluginInstalled } from "./generated/PluginInstalled";
import type { RemoteInstalled } from "./generated/RemoteInstalled";
import type { PluginView } from "./generated/PluginView";
import type { Progress } from "./generated/Progress";
import type { RemotePreview } from "./generated/RemotePreview";
import type { ReportEntry } from "./generated/ReportEntry";
import type { SaveChange } from "./generated/SaveChange";
import type { SaveFields } from "./generated/SaveFields";
import type { SaveView } from "./generated/SaveView";
import type { Scope } from "./generated/Scope";
import type { Started } from "./generated/Started";
import type { VersionStatus } from "./generated/VersionStatus";
import type { VersionView } from "./generated/VersionView";
import type { WriteResult } from "./generated/WriteResult";
import type { AppInfo, DiscScan, GpuDriver, GpuDrivers } from "./api";
import type { UpdateInfo, UpdateProgressCallback } from "./updates";

// ---- the files, as the Rust side parses them --------------------------------------

interface RawAction {
  id: string;
  label: string;
  description?: string;
  kind: ActionView["kind"];
  area?: ActionView["area"];
  state?: ActionView["state"];
  program?: string;
  args?: string[];
  platforms?: Platform[];
  requires?: string[];
  artifact?: string;
  detached?: boolean;
  docs?: string;
  todo?: string;
}

interface RawVersion {
  region?: string;
  serial?: string;
  disc?: { file: string; size?: number; sha1?: string };
  boot?: { file: string; size?: number; sha1?: string };
  inputs?: { from: string; path: string; target?: string }[];
  target?: string;
  setup?: string;
  build?: { host?: string; readme?: string };
  source?: { name: string; repo: string; license?: string; commit?: string; date?: string };
}

interface RawGame {
  id: string;
  title: string;
  year?: number;
  titles?: Record<string, string>;
  versions: Record<string, RawVersion>;
}

type RawSummary = Record<
  string,
  {
    progress: {
      matched_code: number;
      total_code: number;
      matched_functions: number | null;
      total_functions: number | null;
      fuzzy_percent: number | null;
      percent: number;
      date: string | null;
    } | null;
    note?: string;
  }
>;

const manifests = import.meta.glob<RawGame>("../../../games/*/game.json", { eager: true, import: "default" });

/** port/game/<game>/differences.json, as core/src/differences.rs reads it. */
interface RawDifferences {
  game: string;
  differences: (Omit<DifferenceView, "on" | "refs" | "setting"> & {
    discs: string[];
    refs?: string[];
    setting?: { key: string; default: boolean };
  })[];
}
const differenceLists = import.meta.glob<RawDifferences>("../../../port/game/*/differences.json", {
  eager: true,
  import: "default",
});
/** The preview's settings files: `rac1/pal` -> key -> on. */
const fixValues = new Map<string, Map<string, boolean>>();

function mockDifferences(game: string, version: string): Differences {
  const list = Object.values(differenceLists).find((l) => l.game === game);
  const values = fixValues.get(`${game}/${version}`) ?? new Map<string, boolean>();
  return {
    game,
    version,
    settingsFile: `~/.local/share/openrac/settings/${game}-${version}.ini`,
    entries: (list?.differences ?? [])
      .filter((d) => d.discs.includes(version))
      .map((d) => ({
        id: d.id,
        kind: d.kind,
        name: d.name,
        original: d.original,
        port: d.port,
        refs: d.refs ?? [],
        setting: d.setting ?? null,
        on: d.setting ? (values.get(d.setting.key) ?? d.setting.default) : null,
      })),
  };
}
const actions = actionsFile as { repository: RawAction[]; versions: Record<string, RawAction[] | undefined> };
const summary = summaryFile as RawSummary;

// ---- made-up state -------------------------------------------------------------------

const platform: Platform = onAndroid ? "android" : "linux";

/** ?android: the Android launcher's GPU driver packages. */
const turnip: GpuDriver = {
  id: "Turnip-25.2.0",
  name: "Turnip",
  description: "Mesa's open-source Vulkan driver for Adreno",
  author: "Mesa",
  version: "25.2.0",
  vendor: "Mesa",
  driverVersion: "Vulkan 1.4",
  minApi: 28,
  library: "vulkan.ad07XX.so",
};
let gpu: GpuDrivers = {
  drivers: [turnip],
  selected: null,
  renderer: "gles",
  device: { model: "Google Pixel (preview)", api: 35, soc: "Qualcomm SM8650", gpu: "adreno" },
};

// The launcher's update channels (core/src/update.rs): a release's build, and master's development builds.
const STABLE = {
  repository: "OpenRAC/OpenRAC",
  channel: null,
  manifest: "https://github.com/OpenRAC/OpenRAC/releases/latest/download/latest.json",
  page: "https://github.com/OpenRAC/OpenRAC/releases",
};
const DEV = {
  repository: "OpenRAC/OpenRAC",
  channel: "launcher-dev-master",
  manifest: "https://github.com/OpenRAC/OpenRAC/releases/download/launcher-dev-master/latest.json",
  page: "https://github.com/OpenRAC/OpenRAC/releases/tag/launcher-dev-master",
};

let config: Config = {
  // ?setup: a first start where no OpenRAC folder was found, so the welcome screen asks for one.
  root: new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("setup")
    ? null
    : "/home/you/OpenRAC",
  python: "/usr/bin/python3",
  // ?godot: Godot is chosen, so the Mods tab's level editor can open a level.
  godot: new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("godot")
    ? "/usr/bin/godot"
    : null,
  docker: "/usr/bin/docker",
  // ?developer shows the contributor's pages.
  developer: new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("developer"),
  // ?advanced switches the advanced save editor on.
  saveEditorAdvanced: new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("advanced"),
  // ?setup opens the first-run screen.
  setupComplete: !new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("setup"),
  installDir: "/home/you/.local/share/dev.openrac.launcher",
  // ?discs: a games folder is chosen.
  discsDir: new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("discs")
    ? "/home/you/Games/PS2"
    : null,
  lastVersion: null,
  discordRpc: true,
  checkUpdates: true,
  updateChannel: null,
  layout: "compact",
  discordClientId: null,
  detailedGameLog: false,
  // ?vulkan: the desktop's Vulkan renderer is chosen (Settings, Advanced).
  renderer: new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("vulkan")
    ? "vulkan"
    : "opengl",
  // ?purist: Purist mode is on.
  purist: new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("purist"),
  crashesSeen: {},
  discChoice: {},
};

const query = new URLSearchParams(typeof location === "undefined" ? "" : location.search);

/** The preview's logs and crash reports: ?crash makes Play stop with a crash; ?nologs shows none. */
const now = Math.floor(Date.now() / 1000);
const mockCrash: ReportEntry = {
  kind: "crash",
  file: "crashes/crash-2026-10-17_20-08-29.txt",
  program: "openrac-rac1-pal",
  time: now - 60,
  duration: null,
  reason: "access (SIGSEGV) at game address 0x0F000000, which is not mapped",
  level: 6,
  planet: "Batalia",
  crashed: true,
  session: "2026-10-17 20:01:09 -0600",
  size: 9120,
};
let mockReports: ReportEntry[] = query.has("nologs")
  ? []
  : [
      mockCrash,
      {
        ...mockCrash,
        kind: "session",
        file: "openrac-rac1-pal.log",
        time: now - 23 * 60 - 60,
        duration: 23 * 60,
        reason: null,
        size: 48211,
      },
      {
        kind: "session",
        file: "openrac-rac1-pal.1.log",
        program: "openrac-rac1-pal",
        time: now - 26 * 3600,
        duration: 74 * 60,
        reason: null,
        level: 3,
        planet: "Kerwan",
        crashed: false,
        session: "2026-10-16 18:00:00 -0600",
        size: 120400,
      },
      {
        kind: "session",
        file: "openrac-rac1-pal.2.log",
        program: "openrac-rac1-pal",
        time: now - 3 * 86400,
        duration: 40,
        reason: null,
        level: null,
        planet: null,
        crashed: false,
        session: "2026-10-14 10:00:00 -0600",
        size: 8200,
      },
    ];

/** Which games pretend to be set up from the player's disc, and as which version. */
const fresh = new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("fresh");
// ?fresh: nothing is set up yet, as on a player's first start.
const installs: Record<string, BuildInfo | undefined> = fresh
  ? {}
  : {
      rac1: {
        serial: "SCES_509.16",
        version: "rac1/pal",
        elfSha1: "79956931bd62fafd8d20fa2eae796dbaf2e15e83",
        files: 3,
        image: "Ratchet & Clank.iso",
      },
    };

/** ?built (and ?crash): the game is built and its levels are ready, so Play is the main button. */
const built = query.has("built") || query.has("crash");

/** The steps a built game and ready levels mean are done: the ones that made them. The rest
 *  (the editor's projects, the exports, the packs) are not, whatever the mock is asked to
 *  pretend. */
const BUILT_DONE = new Set(["play", "port-build", "port-rebuild", "port-levels", "port-levels-check"]);

function installState(game: string): InstallState {
  return {
    dir: `${config.installDir ?? ""}/active/${game}/data`,
    extracted: installs[game] ?? null,
    decompiled: false,
    compiled: built && !!installs[game],
  };
}

/** Which versions pretend to have their disc found and their inputs placed. */
const discs: Record<string, "found" | "missing" | "mismatch"> = {
  "rac1/pal": "found",
  "rac1/ntsc": "found",
  "rac3/ntsc": "mismatch",
  // ?early: Going Commando's disc is found, for the Mods tab's early access.
  ...(query.has("early") ? { "rac2/ntsc": "found" as const } : {}),
};
const placed = new Set(["rac1/pal"]);
const toolchains = false;

function status(key: string, v: RawVersion): VersionStatus {
  const disc = discs[key] ?? "missing";
  const file = v.disc?.file ?? null;
  const messages = {
    found: "found (size matches; Identify discs checks the checksums)",
    mismatch: `baserom/${file ?? "?"} is not the expected size: another version or a bad dump?`,
    missing: `put your own image in baserom/ (as ${file ?? "?"})`,
  };
  const inputs = (v.inputs ?? []).map((i) => ({
    from: i.from,
    path: i.path,
    present: placed.has(key),
    message: placed.has(key) ? "in place" : "not placed yet: run Place inputs",
  }));
  const boot = inputs.find((i) => i.from === "boot");
  return {
    disc: {
      state: disc,
      path: disc === "found" && file ? `${config.root ?? ""}/baserom/${file}` : null,
      expected: file,
      message: messages[disc],
    },
    boot: boot ? { path: boot.path, present: boot.present, message: boot.message } : null,
    inputs,
    inputsReady: inputs.every((i) => i.present),
  };
}

function view(action: RawAction, st: VersionStatus | null): ActionView {
  const state = action.state ?? "planned";
  const thisPlatform = !action.platforms?.length || action.platforms.includes(platform);
  const blockers: string[] = [];
  if (state === "planned") {
    blockers.push("not connected to the launcher yet");
  } else {
    if (!thisPlatform) blockers.push(`runs on ${(action.platforms ?? []).join(" and ")} only`);
    for (const need of action.requires ?? []) {
      const missing: Record<string, boolean> = {
        disc: st?.disc.state !== "found",
        inputs: !st?.inputsReady,
        toolchains: !toolchains,
        python: !config.python,
        godot: !config.godot,
        docker: !config.docker,
        artifact: !built,
      };
      const words: Record<string, string> = {
        disc: "your disc image (baserom/)",
        inputs: "the inputs Place inputs puts in the game",
        toolchains: "the compilers in toolchains/",
        python: "Python (Settings)",
        godot: "Godot (Settings)",
        docker: "Docker",
        artifact: `${action.artifact ?? "its artifact"}, which an earlier step makes`,
      };
      if (missing[need]) blockers.push(`needs ${words[need] ?? need}`);
    }
  }
  return {
    id: action.id,
    label: action.label,
    description: action.description ?? "",
    kind: action.kind,
    area: action.area ?? "decomp",
    state,
    detached: action.detached ?? false,
    docs: action.docs ?? null,
    todo: action.todo ?? null,
    thisPlatform,
    runnable: blockers.length === 0,
    blockers,
    // The mock has no files: nothing is built or made, except what ?built says is (the game
    // built and its levels ready). Only the steps that made those are done; the rest are not,
    // or a page would say an action that never ran is done.
    done: built && BUILT_DONE.has(action.id),
  };
}

function progress(key: string): Progress | null {
  const row = summary[key];
  // A version with no report yet (progress null) has no figures, as in catalog.rs.
  if (!row?.progress) return null;
  const p = row.progress;
  return {
    matchedCode: p.matched_code,
    totalCode: p.total_code,
    matchedFunctions: p.matched_functions,
    totalFunctions: p.total_functions,
    fuzzyPercent: p.fuzzy_percent,
    percent: p.percent,
    date: p.date,
    note: row.note ?? null,
    functional: null,
  };
}

function library(): Library {
  const games: GameView[] = Object.entries(manifests)
    .sort(([a], [b]) => a.localeCompare(b))
    .map(([, g]) => {
      const versions: VersionView[] = Object.entries(g.versions).map(([name, v]) => {
        const key = `${g.id}/${name}`;
        const st = status(key, v);
        return {
          key,
          game: g.id,
          name,
          title: g.titles?.[name] ?? g.title,
          region: v.region ?? "",
          serial: v.serial ?? "",
          dir: `games/${g.id}/${name}`,
          disc: v.disc ? { file: v.disc.file, size: v.disc.size ?? null, sha1: v.disc.sha1 ?? null } : null,
          boot: v.boot ? { file: v.boot.file, size: v.boot.size ?? null, sha1: v.boot.sha1 ?? null } : null,
          inputs: (v.inputs ?? []).map((i) => ({ from: i.from, path: i.path, target: i.target ?? null })),
          target: v.target ?? null,
          setup: v.setup ?? null,
          buildHost: v.build?.host ?? null,
          readme: v.build?.readme ?? null,
          source: v.source
            ? {
                name: v.source.name,
                repo: v.source.repo,
                license: v.source.license ?? null,
                commit: v.source.commit ?? null,
                date: v.source.date ?? null,
              }
            : null,
          progress: progress(key),
          status: st,
          actions: (actions.versions[key] ?? []).map((a) => view(a, st)),
          portOutdated: false,
        };
      });
      return { id: g.id, title: g.title, year: g.year ?? null, install: installState(g.id), versions };
    });
  return {
    root: config.root ?? "",
    platform,
    toolchains,
    repository: actions.repository.map((a) => view(a, null)),
    games,
    actionsError: null,
  };
}

// ---- jobs ----------------------------------------------------------------------------

type Listener = (event: JobEvent) => void;
const listeners = new Set<Listener>();
let nextJob = 0;
let gameRunning = typeof location !== "undefined" && new URLSearchParams(location.search).has("playing");
const timers = new Map<number, ReturnType<typeof setInterval>>();

export function mockListen(handler: Listener): () => void {
  listeners.add(handler);
  return () => listeners.delete(handler);
}

function emit(event: JobEvent) {
  for (const l of listeners) l(event);
}

function findAction(scope: Scope, id: string): RawAction | undefined {
  const list = scope.kind === "repository" ? actions.repository : (actions.versions[scope.key] ?? []);
  return list.find((a) => a.id === id);
}

/** What a few actions print at the end in the preview, so the pages that read it can be tried. */
function lastLines(id: string, args: Record<string, unknown>): string[] {
  const data = `${config.installDir ?? ""}/active/rac1/data`;
  const level = String(typeof args.level === "number" ? args.level : 0).padStart(2, "0");
  const path = typeof args.path === "string" ? args.path : "";
  const name =
    path
      .split(/[\\/]/)
      .at(-1)
      ?.replace(/\.zip$/i, "") ?? "pack";
  switch (id) {
    case "assets-export":
      return [
        `level ${level}: 215 models, 414 textures`,
        `Exported 215 models and 414 textures: ${data}/exports/level_${level}-${String(args.option)}`,
      ];
    case "assets-import":
      mockPacks.push({ ...samplePack(name), id: name.toLowerCase().replace(/[^a-z0-9]+/g, "-") });
      return [
        "texture moby_0000 (1024 x 1024) replaces the game's in 20 places (the same picture in 19 more)",
        "refused: level 00 texture tie_0004.png is 300 x 200, but the game's tie_0004 is 64 x 64 (1:1); keep that shape, for example 128 x 128, 256 x 256",
        `Imported 19 model file(s) and 80 texture file(s) into ${data}/overrides as ${name}; 1 refused (see above)`,
      ];
    case "overrides-add":
      mockPacks.push({ ...samplePack(name), id: name.toLowerCase().replace(/[^a-z0-9]+/g, "-") });
      return [`Added ${name} as ${name.toLowerCase()}, on, last in the order.`];
    case "overrides-stack":
      return [`Stacked ${mockPacks.filter((p) => p.enabled).length + 1} layer(s), 99 file(s): ${data}/mods/pal`];
    default:
      return ["\u2714 step one", "\u2714 step two", "done"];
  }
}

/** The native mods the preview pretends are installed: ?nomods shows none. */
/** Where the mock's mods came from (remote.rs's sources.json): the lock-strafe, from rac1-mods. */
const mockSources: ModSources = {
  format: 1,
  mods: query.has("nomods")
    ? {}
    : {
        "lock-strafe": {
          url: "https://github.com/OpenRAC/rac1-mods",
          path: "lock-strafe",
          version: "1.0.0",
          tag: "v1.0.0",
          commit: null,
          kind: "native",
          pin: false,
        },
      },
};

/** What rac1-mods holds, as remote.rs's preview gives it: the lock-strafe, and a mod for another game. */
function mockPreview(url: string): RemotePreview {
  return {
    url: "https://github.com/OpenRAC/rac1-mods",
    label: url.includes("rac1-mods") ? "OpenRAC/rac1-mods" : url.replace(/^https?:\/\/(www\.)?github\.com\//, ""),
    host: "github",
    owner: "OpenRAC",
    reference: "v1.1.0",
    commit: "7c11ca00e36c814b8e56d8d331df47852c3af271",
    note: null,
    mods: [
      {
        id: "lock-strafe",
        name: "Lock-strafe",
        version: "1.1.0",
        author: "OpenRAC contributors",
        description: "Strafe as in the later games: hold L2 or R2, or lock it on with a weapon in hand.",
        homepage: "https://github.com/OpenRAC/rac1-mods",
        path: "lock-strafe",
        kind: "native",
        games: { rac1: ["pal", "ntsc"] },
        api: "1.0",
        compatible: true,
        why: null,
        asset: "lock-strafe-1.1.0-macos-arm64.zip",
        tag: "v1.1.0",
        installed: mockSources.mods["lock-strafe"]?.version ?? null,
      },
      {
        id: "commando-hud",
        name: "Commando HUD",
        version: "0.2.0",
        author: "Someone",
        description: "Going Commando's HUD layout.",
        homepage: "",
        path: "commando-hud",
        kind: "native",
        games: { rac2: ["ntsc"] },
        api: "1.0",
        compatible: false,
        why: "It is not for rac1.",
        asset: null,
        tag: null,
        installed: null,
      },
      {
        id: "button-swap",
        name: "Button swap",
        version: "1.0.0",
        author: "OpenRAC contributors",
        description: "Swaps two buttons of the controller: Cross and Square, Circle and Triangle, or L1 and R1.",
        homepage: "https://github.com/OpenRAC/rac1-mods",
        path: "button-swap",
        kind: "script",
        games: { rac1: ["pal", "ntsc"] },
        api: "1.1",
        compatible: true,
        why: null,
        asset: null,
        tag: null,
        installed: null,
      },
    ],
  };
}

let mockPlugins: PluginView[] = query.has("nomods")
  ? []
  : [
      {
        id: "keyboard-mouse",
        name: "Keyboard and mouse",
        version: "1.0.0",
        author: "OpenRAC contributors",
        description:
          "Plays Ratchet & Clank with a keyboard and mouse: WASD moves, the mouse aims the camera, left click fires, holding right click opens the Quick Select and the mouse picks its slot, Space jumps, Escape pauses, the number keys and the wheel change weapons. Every action can be bound to any key, mouse button or wheel direction, two binds each, with Left-handed and Arrow keys presets. A gamepad keeps working beside it.",
        homepage: "https://github.com/OpenRAC/rac1-mods/tree/master/keyboard-mouse",
        license: "GPL-3.0-or-later",
        enabled: true,
        state: "ready",
        reason: "",
        blocked: "",
        presets: [
          {
            id: "left_handed",
            name: "Left-handed",
            values: {
              move_forward: "KeyI",
              move_back: "KeyK",
              move_left: "KeyJ",
              move_right: "KeyL",
              walk: "AltRight",
              wrench: "KeyH,MouseMiddle",
              crouch: "ControlRight,KeyN",
              strafe: "ShiftRight",
              first_person: "KeyU,MouseBack",
              map: "KeyM,Backslash",
              camera_reset: "KeyO",
              pause: "Escape,KeyP",
            },
          },
          {
            id: "arrows",
            name: "Arrow keys",
            values: {
              move_forward: "ArrowUp",
              move_back: "ArrowDown",
              move_left: "ArrowLeft",
              move_right: "ArrowRight",
              walk: "AltRight",
              crouch: "ControlRight,KeyC",
              strafe: "ShiftRight",
              menu_up: "",
              menu_down: "",
              menu_left: "",
              menu_right: "",
            },
          },
        ],
        settings: [
          {
            key: "mouse_sensitivity",
            name: "Mouse sensitivity",
            type: "float",
            default: 1.0,
            min: 0.1,
            max: 10.0,
            step: 0.1,
            description: "How far the camera turns for the mouse's motion.",
            value: 1.0,
          },
          {
            key: "mouse_acceleration",
            name: "Mouse acceleration",
            type: "float",
            default: 0.0,
            min: 0.0,
            max: 2.0,
            step: 0.1,
            description: "0 for none; more turns a fast flick further than a slow drag.",
            value: 0.0,
          },
          {
            key: "invert_y",
            name: "Invert the mouse's up and down",
            type: "bool",
            default: false,
            value: false,
          },
          {
            key: "camera_tilt",
            name: "Camera tilt",
            type: "choice",
            default: "hold",
            choices: [
              {
                value: "hold",
                name: "Stays",
              },
              {
                value: "return",
                name: "Levels out",
              },
            ],
            description: "Whether the camera keeps the tilt the mouse gives it, or levels out when the mouse rests.",
            value: "hold",
          },
          {
            key: "capture_cursor",
            name: "Capture the cursor in play",
            type: "bool",
            default: true,
            description: "Hides the cursor and keeps it in the window while you play; menus and pausing let it go.",
            value: true,
          },
          {
            key: "walk_speed",
            name: "Walk speed",
            type: "float",
            default: 0.5,
            min: 0.2,
            max: 0.9,
            step: 0.05,
            description: "How far the walk key pushes the stick.",
            value: 0.5,
          },
          {
            key: "stick_ramp_ms",
            name: "Stick ramp (ms)",
            type: "int",
            default: 60,
            min: 0,
            max: 300,
            description: "How long the movement keys take to push the stick all the way: a tap nudges, a hold runs.",
            value: 60,
          },
          {
            key: "quick_select_dead_zone",
            name: "Quick Select dead zone",
            type: "int",
            default: 30,
            min: 5,
            max: 200,
            description: "How far, in pixels, the mouse moves before the Quick Select points at a slot.",
            value: 30,
          },
          {
            key: "move_forward",
            name: "Move forward",
            type: "keybind",
            default: "KeyW",
            group: "play",
            description: "Pushes the left stick forward.",
            value: "KeyW",
          },
          {
            key: "move_back",
            name: "Move back",
            type: "keybind",
            default: "KeyS",
            group: "play",
            value: "KeyS",
          },
          {
            key: "move_left",
            name: "Move left",
            type: "keybind",
            default: "KeyA",
            group: "play",
            value: "KeyA",
          },
          {
            key: "move_right",
            name: "Move right",
            type: "keybind",
            default: "KeyD",
            group: "play",
            value: "KeyD",
          },
          {
            key: "walk",
            name: "Walk (hold)",
            type: "keybind",
            default: "AltLeft",
            group: "play",
            description: "Moves at the walk speed below while held.",
            value: "AltLeft",
          },
          {
            key: "jump",
            name: "Jump",
            type: "keybind",
            default: "Space",
            group: "play",
            description: "Cross.",
            value: "Space",
          },
          {
            key: "fire",
            name: "Fire",
            type: "keybind",
            default: "MouseLeft",
            group: "play",
            description: "Circle: fires the weapon or gadget in hand.",
            value: "MouseLeft",
          },
          {
            key: "quick_select",
            name: "Quick Select (hold)",
            type: "keybind",
            default: "MouseRight",
            group: "play",
            description: "Triangle, held: move the mouse toward a slot and let go to take it.",
            value: "MouseRight",
          },
          {
            key: "wrench",
            name: "Wrench",
            type: "keybind",
            default: "KeyF,MouseMiddle",
            group: "play",
            description: "Square.",
            value: "KeyF,MouseMiddle",
          },
          {
            key: "crouch",
            name: "Crouch",
            type: "keybind",
            default: "ControlLeft,KeyC",
            group: "play",
            description: "R1.",
            value: "ControlLeft,KeyC",
          },
          {
            key: "strafe",
            name: "Strafe",
            type: "keybind",
            default: "ShiftLeft",
            group: "play",
            description: "L2 (the lock-strafe mod's Hold mode strafes with it).",
            value: "ShiftLeft",
          },
          {
            key: "first_person",
            name: "First-person view",
            type: "keybind",
            default: "KeyV,MouseBack",
            group: "play",
            description: "L1, held.",
            value: "KeyV,MouseBack",
          },
          {
            key: "map",
            name: "Map",
            type: "keybind",
            default: "Tab,KeyM",
            group: "play",
            description: "Select.",
            value: "Tab,KeyM",
          },
          {
            key: "camera_reset",
            name: "Camera behind Ratchet",
            type: "keybind",
            default: "KeyQ",
            group: "play",
            description: "Swings the camera round behind him and levels it.",
            value: "KeyQ",
          },
          {
            key: "pause",
            name: "Pause",
            type: "keybind",
            default: "Escape,KeyP",
            group: "play",
            description: "Start: the pause menu.",
            value: "Escape,KeyP",
          },
          {
            key: "weapon_1",
            name: "Quick Select slot 1",
            type: "keybind",
            default: "Digit1",
            group: "play",
            description: "Takes the item in that slot of the Quick Select at once (slot 1 is the top, then clockwise).",
            value: "Digit1",
          },
          {
            key: "weapon_2",
            name: "Quick Select slot 2",
            type: "keybind",
            default: "Digit2",
            group: "play",
            value: "Digit2",
          },
          {
            key: "weapon_3",
            name: "Quick Select slot 3",
            type: "keybind",
            default: "Digit3",
            group: "play",
            value: "Digit3",
          },
          {
            key: "weapon_4",
            name: "Quick Select slot 4",
            type: "keybind",
            default: "Digit4",
            group: "play",
            value: "Digit4",
          },
          {
            key: "weapon_5",
            name: "Quick Select slot 5",
            type: "keybind",
            default: "Digit5",
            group: "play",
            value: "Digit5",
          },
          {
            key: "weapon_6",
            name: "Quick Select slot 6",
            type: "keybind",
            default: "Digit6",
            group: "play",
            value: "Digit6",
          },
          {
            key: "weapon_7",
            name: "Quick Select slot 7",
            type: "keybind",
            default: "Digit7",
            group: "play",
            value: "Digit7",
          },
          {
            key: "weapon_8",
            name: "Quick Select slot 8",
            type: "keybind",
            default: "Digit8",
            group: "play",
            value: "Digit8",
          },
          {
            key: "weapon_next",
            name: "Next weapon",
            type: "keybind",
            default: "WheelDown",
            group: "play",
            description: "The next Quick Select slot that holds something.",
            value: "WheelDown",
          },
          {
            key: "weapon_prev",
            name: "Previous weapon",
            type: "keybind",
            default: "WheelUp",
            group: "play",
            value: "WheelUp",
          },
          {
            key: "menu_confirm",
            name: "Menu: confirm",
            type: "keybind",
            default: "Enter,NumpadEnter",
            group: "menu",
            description: "Cross, in menus.",
            value: "Enter,NumpadEnter",
          },
          {
            key: "menu_back",
            name: "Menu: back",
            type: "keybind",
            default: "Backspace",
            group: "menu",
            description: "Triangle, in menus.",
            value: "Backspace",
          },
          {
            key: "menu_up",
            name: "Menu: up",
            type: "keybind",
            default: "ArrowUp",
            group: "menu",
            description: "The d-pad, in menus.",
            value: "ArrowUp",
          },
          {
            key: "menu_down",
            name: "Menu: down",
            type: "keybind",
            default: "ArrowDown",
            group: "menu",
            value: "ArrowDown",
          },
          {
            key: "menu_left",
            name: "Menu: left",
            type: "keybind",
            default: "ArrowLeft",
            group: "menu",
            value: "ArrowLeft",
          },
          {
            key: "menu_right",
            name: "Menu: right",
            type: "keybind",
            default: "ArrowRight",
            group: "menu",
            value: "ArrowRight",
          },
        ],
        hooks: ["FollowCamera_ApplyOverrides"],
        sharedHooks: [],
        native: true,
        kind: "native",
        path: "/home/you/OpenRAC/active/rac1/data/plugins/keyboard-mouse",
      },
      {
        id: "lock-strafe",
        name: "Lock-strafe",
        version: "1.0.0",
        author: "OpenRAC contributors",
        description:
          "Strafe as in the later games: with a weapon that fires ahead the camera stays behind Ratchet, the right stick turns him and the left stick strafes.",
        homepage: "https://github.com/OpenRAC/rac1-mods",
        license: "GPL-3.0-or-later",
        enabled: true,
        state: "ready",
        reason: "",
        blocked: "",
        settings: [
          {
            key: "mode",
            name: "Mode",
            type: "choice",
            default: "lock",
            description: "Lock: always, with a weapon that fires ahead. Hold: while L2 or R2 is held.",
            choices: [
              { value: "lock", name: "Lock with weapons" },
              { value: "hold", name: "Hold L2 or R2" },
            ],
            value: "lock",
          },
          { key: "trace", name: "Log each pad report", type: "bool", default: false, value: false },
        ],
        hooks: ["Pad_ProcessReport", "Hero_TurnToHeading", "FollowCamera_ApplyOverrides"],
        sharedHooks: [],
        native: true,
        kind: "native",
        path: "/home/you/.local/share/dev.openrac.launcher/active/rac1/data/plugins/lock-strafe",
      },
      {
        id: "camera-tweaks",
        name: "Camera tweaks",
        version: "0.3.0",
        author: "someone",
        description: "A wider field of view and a slower follow camera.",
        homepage: "",
        license: "",
        enabled: true,
        state: "refused",
        reason: "Needs OpenRAC with mod interface 2.0; this one has 1.0.",
        blocked: "",
        settings: [{ key: "fov", name: "Field of view", type: "int", default: 70, min: 50, max: 110, value: 80 }],
        hooks: ["FollowCamera_ApplyOverrides"],
        sharedHooks: [{ hook: "FollowCamera_ApplyOverrides", with: ["Lock-strafe"] }],
        native: true,
        kind: "native",
        path: "/home/you/.local/share/dev.openrac.launcher/active/rac1/data/plugins/camera-tweaks",
      },
      {
        id: "button-swap",
        name: "Button swap",
        version: "1.0.0",
        author: "OpenRAC contributors",
        description: "Swaps two buttons of the controller.",
        homepage: "https://github.com/OpenRAC/rac1-mods",
        license: "GPL-3.0-or-later",
        enabled: false,
        state: "off",
        reason: "",
        blocked: "",
        settings: [
          {
            key: "pair",
            name: "Swap",
            type: "choice",
            default: "cross-square",
            choices: [
              { value: "cross-square", name: "Cross and Square" },
              { value: "circle-triangle", name: "Circle and Triangle" },
              { value: "l1-r1", name: "L1 and R1" },
            ],
            value: "cross-square",
          },
        ],
        hooks: [],
        sharedHooks: [],
        native: false,
        kind: "script",
        path: "/home/you/OpenRAC/active/rac1/data/plugins/button-swap",
      },
      // The item randomizer (OpenRAC/rac1-mods), its manifest's settings.
      {
        id: "item-randomizer",
        name: "Item randomizer",
        version: "1.0.0",
        author: "OpenRAC contributors",
        description:
          "Shuffles what the game gives out: the gadgets and weapons from NPCs and pickups, the vendor's weapons, the gold bolts and the infobots' coordinates, so each seed is a new route through the game. The placement follows what each place needs, so every seed can be finished. Share a seed's code and a friend plays the same game. A spoiler log of each seed is written to the mod's data folder. A save made with a seed belongs to that seed.",
        homepage: "https://github.com/OpenRAC/rac1-mods/tree/master/item-randomizer",
        license: "GPL-3.0-or-later",
        enabled: true,
        state: "ready",
        reason: "",
        blocked: "",
        settings: [
          {
            key: "seed",
            name: "Seed",
            type: "string",
            default: "",
            description:
              "A seed code to play a shared game (it carries its settings), or any word. Blank: a random seed, rolled once and kept for your next starts (the log and the spoiler log show its code).",
            maxLength: 32,
            value: "K7QX-2M9P",
          },
          {
            key: "difficulty",
            name: "Logic",
            type: "choice",
            default: "standard",
            choices: [
              {
                value: "relaxed",
                name: "Relaxed",
              },
              {
                value: "standard",
                name: "Standard",
              },
              {
                value: "expert",
                name: "Expert",
              },
            ],
            description:
              "Standard: a gadget is placed only where you can get it without it, counting what each place uses. Relaxed: the same, with the gadgets early in the game and never in a gold bolt or an expensive purchase. Expert: only what a place cannot be done without counts.",
            value: "standard",
          },
          {
            key: "shuffle_gadgets",
            name: "Shuffle gadgets",
            type: "bool",
            default: true,
            description: "The gadgets NPCs and pickups give: the Swingshot, the Heli-Pack, the Magneboots, ...",
            value: true,
          },
          {
            key: "shuffle_weapons",
            name: "Shuffle weapons",
            type: "bool",
            default: true,
            description: "The weapons NPCs and pickups give: the Suck Cannon, the Morph-o-Ray, the R.Y.N.O.",
            value: true,
          },
          {
            key: "shuffle_vendor",
            name: "Shuffle the vendor",
            type: "bool",
            default: true,
            description:
              "Each weapon the Gadgetron vendor sells is a slot that gives something else when bought, at the slot's price.",
            value: true,
          },
          {
            key: "shuffle_gold_bolts",
            name: "Shuffle gold bolts",
            type: "bool",
            default: true,
            description:
              "Gold bolts join the pool: a gold bolt's place may hold a gadget, and a gold bolt may be anywhere.",
            value: true,
          },
          {
            key: "shuffle_infobots",
            name: "Shuffle infobots",
            type: "bool",
            default: true,
            description: "The planets' coordinates the infobots give join the pool.",
            value: true,
          },
          {
            key: "keep_starting_gear",
            name: "Keep the starting gear",
            type: "bool",
            default: true,
            description:
              "Start with the Bomb Glove, as the game does. Off: what Veldin gives at the start is shuffled too.",
            value: true,
          },
          {
            key: "spoiler_log",
            name: "Spoiler log",
            type: "bool",
            default: true,
            description: "Write where everything is, and a way through, to the mod's data folder.",
            value: true,
          },
        ],
        hooks: ["rac1.items.give", "rac1.vendor.stock"],
        sharedHooks: [],
        native: true,
        kind: "native",
        path: "/home/you/.local/share/dev.openrac.launcher/active/rac1/data/plugins/item-randomizer",
      },
      // The difficulty mod (OpenRAC/rac1-mods), its manifest's settings and presets.
      {
        id: "difficulty",
        name: "Difficulty",
        version: "1.1.0",
        author: "OpenRAC contributors",
        description:
          "A slider for everything that makes Ratchet & Clank hard or easy: how many enemies there are, the damage Ratchet takes and deals, how fast the enemies and Ratchet move, weapon and ammo prices, ammo per shot and max ammo, bolt drops, max health and health pickups. Presets fill them: Easy, Retail, Hard, Brutal, and Chaos, which rolls new values for each level from a seed (and, if you like, a random event every few minutes). Every slider at 1.0 plays as the original game.",
        homepage: "https://github.com/OpenRAC/rac1-mods/tree/master/difficulty-mod",
        license: "GPL-3.0-or-later",
        enabled: true,
        state: "ready",
        reason: "",
        blocked: "",
        presets: [
          {
            id: "easy",
            name: "Easy",
            values: {
              enemy_count: 0.75,
              damage_taken: 0.5,
              damage_dealt: 1.5,
              enemy_speed: 0.85,
              player_speed: 1.1,
              weapon_cost: 0.75,
              ammo_cost: 0.5,
              ammo_use: 0.75,
              max_ammo: 1.5,
              bolts: 1.5,
              max_health: 2.0,
              health_pickups: 2.0,
              chaos: false,
            },
          },
          {
            id: "default",
            name: "Retail",
            values: {
              enemy_count: 1.0,
              damage_taken: 1.0,
              damage_dealt: 1.0,
              enemy_speed: 1.0,
              player_speed: 1.0,
              weapon_cost: 1.0,
              ammo_cost: 1.0,
              ammo_use: 1.0,
              max_ammo: 1.0,
              bolts: 1.0,
              max_health: 1.0,
              health_pickups: 1.0,
              chaos: false,
            },
          },
          {
            id: "hard",
            name: "Hard",
            values: {
              enemy_count: 1.5,
              damage_taken: 2.0,
              damage_dealt: 0.75,
              enemy_speed: 1.2,
              player_speed: 1.0,
              weapon_cost: 1.5,
              ammo_cost: 1.5,
              ammo_use: 1.25,
              max_ammo: 0.75,
              bolts: 0.75,
              max_health: 1.0,
              health_pickups: 0.75,
              chaos: false,
            },
          },
          {
            id: "brutal",
            name: "Brutal",
            values: {
              enemy_count: 2.0,
              damage_taken: 3.0,
              damage_dealt: 0.5,
              enemy_speed: 1.4,
              player_speed: 1.0,
              weapon_cost: 2.5,
              ammo_cost: 2.5,
              ammo_use: 1.5,
              max_ammo: 0.5,
              bolts: 0.5,
              max_health: 0.5,
              health_pickups: 0.5,
              chaos: false,
            },
          },
          {
            id: "chaos",
            name: "Chaos",
            values: {
              enemy_count: 1.0,
              damage_taken: 1.0,
              damage_dealt: 1.0,
              enemy_speed: 1.0,
              player_speed: 1.0,
              weapon_cost: 1.0,
              ammo_cost: 1.0,
              ammo_use: 1.0,
              max_ammo: 1.0,
              bolts: 1.0,
              max_health: 1.0,
              health_pickups: 1.0,
              chaos: true,
            },
          },
        ],
        settings: [
          {
            key: "enemy_count",
            name: "Enemy count",
            type: "float",
            default: 1.0,
            description:
              "Above 1: copies of the enemies beside them, as room in the level allows (bosses, waves, turrets and scripted enemies never). Below 1: that share of the free-roaming enemies left out. Counts from the next level loaded.",
            min: 0.25,
            max: 4.0,
            step: 0.25,
            value: 1.0,
          },
          {
            key: "damage_taken",
            name: "Damage to Ratchet",
            type: "float",
            default: 1.0,
            description: "Cells of health a hit takes, on average: 2 is two cells a hit, 0.5 a cell every other hit.",
            min: 0.25,
            max: 4.0,
            step: 0.25,
            value: 1.0,
          },
          {
            key: "damage_dealt",
            name: "Damage to enemies",
            type: "float",
            default: 1.0,
            description: "The damage of every hit on an enemy or a boss: your weapons, the wrench, explosions.",
            min: 0.25,
            max: 4.0,
            step: 0.25,
            value: 1.0,
          },
          {
            key: "enemy_speed",
            name: "Enemy speed",
            type: "float",
            default: 1.0,
            description: "How fast enemies and turrets move, turn, animate and fire (what their own clocks run on).",
            min: 0.5,
            max: 2.0,
            step: 0.05,
            value: 1.0,
          },
          {
            key: "player_speed",
            name: "Ratchet's speed",
            type: "float",
            default: 1.0,
            description: "Ratchet's walking and running speed.",
            min: 0.5,
            max: 2.0,
            step: 0.1,
            value: 1.0,
          },
          {
            key: "weapon_cost",
            name: "Weapon prices",
            type: "float",
            default: 1.0,
            description: "What the Gadgetron vendor and the NPCs ask for weapons and gadgets.",
            min: 0.1,
            max: 5.0,
            step: 0.05,
            value: 1.0,
          },
          {
            key: "ammo_cost",
            name: "Ammo prices",
            type: "float",
            default: 1.0,
            description: "The vendor's price for each round of ammo.",
            min: 0.1,
            max: 5.0,
            step: 0.1,
            value: 1.0,
          },
          {
            key: "ammo_use",
            name: "Ammo per shot",
            type: "float",
            default: 1.0,
            description: "Rounds each shot uses, on average.",
            min: 0.25,
            max: 4.0,
            step: 0.25,
            value: 1.0,
          },
          {
            key: "max_ammo",
            name: "Max ammo",
            type: "float",
            default: 1.0,
            description: "How much ammo each weapon holds.",
            min: 0.25,
            max: 4.0,
            step: 0.25,
            value: 1.0,
          },
          {
            key: "bolts",
            name: "Bolt drops",
            type: "float",
            default: 1.0,
            description: "Bolts from enemies and crates. 0: none.",
            min: 0.0,
            max: 5.0,
            step: 0.25,
            value: 1.0,
          },
          {
            key: "max_health",
            name: "Max health",
            type: "float",
            default: 1.0,
            description:
              "Cells of health: the HUD shows 1 to 5 or 8, so the nearest of those at or below (4 cells at 2.0 make 8; with Ultra Nanotech 8 stays the most).",
            min: 0.25,
            max: 2.0,
            step: 0.25,
            value: 1.0,
          },
          {
            key: "health_pickups",
            name: "Health pickups",
            type: "float",
            default: 1.0,
            description: "Cells each nanotech pickup gives, on average. 0: none.",
            min: 0.0,
            max: 4.0,
            step: 0.25,
            value: 1.0,
          },
          {
            key: "chaos",
            name: "Chaos",
            type: "bool",
            default: false,
            description:
              "Each level rolls its own value for every slider left at 1.0, from the seed (the Chaos preset). A slider moved off 1.0 holds on every level.",
            value: false,
          },
          {
            key: "seed",
            name: "Seed",
            type: "string",
            default: "",
            description:
              "For Chaos: the 8-character code from the log plays the same rolls again; any other word too. Blank: a new seed each start, shown in the log.",
            maxLength: 32,
            value: "",
          },
          {
            key: "chaos_events",
            name: "Chaos events",
            type: "bool",
            default: false,
            description:
              "Every few minutes, a random event for 45 seconds: Turbo Ratchet, slow motion, frenzy, glass cannon, bolt rain, ammo famine, ... (with any preset).",
            value: false,
          },
          {
            key: "chaos_minutes",
            name: "Minutes between events",
            type: "int",
            default: 3,
            description: "Of play, with Chaos events on.",
            min: 1,
            max: 30,
            value: 3,
          },
          {
            key: "trace",
            name: "Trace",
            type: "bool",
            default: false,
            description: "For bug reports: more lines in the game's log (each level's values, max health changes).",
            value: false,
          },
        ],
        hooks: [
          "rac1.player.take_damage",
          "rac1.combat.get_hit",
          "rac1.weapon.take_ammo",
          "rac1.moby.record_death",
          "rac1.level.load_gameplay",
          "rac1.player.approach_speed",
        ],
        sharedHooks: [],
        native: true,
        kind: "native",
        path: "/home/you/.local/share/dev.openrac.launcher/active/rac1/data/plugins/difficulty",
      },
    ];

/** What the preview's game says it did with the mods at its last start. */
function mockPluginStatus(): PluginStatus | null {
  if (mockPlugins.length === 0) return null;
  return {
    time: new Date().toISOString(),
    version: "pal",
    purist: config.purist,
    mods: mockPlugins
      .filter((p) => p.enabled)
      .map((p) => ({
        id: p.id,
        version: p.version,
        state: p.state === "ready" ? "loaded" : "refused",
        reason: p.reason,
        hooks: p.hooks,
      })),
    hookPoints: ["Pad_ProcessReport", "Hero_TurnToHeading", "FollowCamera_ApplyOverrides"],
  };
}

function samplePack(name: string): PackView {
  return {
    id: name,
    name,
    author: "you",
    description: "",
    enabled: true,
    levels: 19,
    textures: 80,
    models: 19,
    levelFiles: 0,
  };
}

/** The overrides packs the preview pretends are added. */
const mockPacks: PackView[] = [
  {
    ...samplePack("hd-textures"),
    name: "HD textures",
    author: "someone",
    description: "Every texture of Veldin, 4x.",
    levels: 1,
    textures: 414,
    models: 0,
  },
  { ...samplePack("green-ratchet"), name: "Green Ratchet", enabled: false },
];

function start(scope: Scope, id: string, args: Record<string, unknown> = {}): Started {
  const action = findAction(scope, id);
  if (!action) throw new Error(`no action ${id}`);
  const lib = library();
  const list =
    scope.kind === "repository"
      ? lib.repository
      : (lib.games.flatMap((g) => g.versions).find((v) => v.key === scope.key)?.actions ?? []);
  const v = list.find((a) => a.id === id);
  if (!v?.runnable) throw new Error(`${action.label}: ${v?.blockers.join("; ") ?? "cannot run"}`);
  const command = [action.program ?? "?", ...(action.args ?? [])].join(" ");
  if (action.detached) return { id: 0, title: action.label, command, detached: true, output: null };

  const lines = [`$ ${command}`, "(browser preview: nothing really runs)", "working…", ...lastLines(id, args)];
  // The build prints ninja's counter and long paths, as the real one does: to try the progress strip.
  if (id === "port-build") {
    const total = 60;
    for (let n = 1; n <= total; n++) {
      lines.splice(
        2 + n,
        0,
        `[${n}/${total}] Building C object CMakeFiles/openrac-rac1-pal.dir/__w/OpenRAC/OpenRAC/games/rac1/pal/src/level_${n}/func_001E99D8_generated_long_name.c.o`,
      );
    }
  }
  // ?fail: every job stops with an error, to see how a failure reads.
  if (query.has("fail")) {
    const failed = [...lines.slice(0, 3), "error: the step could not finish (browser preview)"];
    return fakeJob(action.label, command, failed, () => undefined, 1);
  }
  // ?crash: the game stops on a fault, as the port does (exit status 3, a report in crashes/).
  if (action.kind === "play" && query.has("crash")) {
    const crashed = [...lines, "[error] ==== the program stopped: fault ===="];
    return fakeJob(action.label, command, crashed, () => (mockCrash.time = Math.floor(Date.now() / 1000)), 3);
  }
  // The folder the editor's pack and the asset export write, for the Mods tab's "Open folder".
  const outputs =
    id === "editor-pack"
      ? [`/home/you/OpenRAC/assets/packed/${scope.kind === "version" ? scope.key : ""}`]
      : id === "assets-export"
        ? [
            `/home/you/exports/level_${String(Number(args.level ?? 0)).padStart(2, "0")}-${typeof args.option === "string" ? args.option : "mobys"}`,
          ]
        : [];
  return fakeJob(action.label, command, lines, () => undefined, 0, outputs);
}

/** A job that prints LINES one by one, then ends with CODE (0: it worked) and calls DONE. */
function fakeJob(
  title: string,
  command: string,
  lines: string[],
  done: () => void,
  code = 0,
  outputs: string[] = [],
): Started {
  const job = ++nextJob;
  let i = 0;
  timers.set(
    job,
    setInterval(() => {
      const line = lines[i++];
      if (line !== undefined) {
        emit({ type: "output", id: job, stream: "stdout", line });
        return;
      }
      clearInterval(timers.get(job));
      timers.delete(job);
      done();
      emit({ type: "exit", id: job, code, cancelled: false, outputs });
    }, 450),
  );
  return { id: job, title, command, detached: false, output: null };
}

function install(game: string, image: string): Started {
  const g = Object.values(manifests).find((m) => m.id === game);
  const [name, v] = Object.entries(g?.versions ?? {})[0] ?? [];
  if (!g || !name || !v) throw new Error(`no game ${game}`);
  const serial = v.serial ?? "";
  const command = `python3 tools/extractor.py ${image} --game ${game} --extract --validate`;
  const lines = [
    `${image}: 3 files on the disc, boot executable ${serial}`,
    "  SYSTEM.CNF: 58 bytes",
    `  ${serial}: ${v.boot?.size ?? 0} bytes`,
    `validated: ${g.title} (${v.region ?? ""}, ${serial}), boot executable SHA-1 matches games/${game}/game.json`,
    "  disc.iso: linked",
    "extracted (browser preview: nothing really runs)",
  ];
  return fakeJob(`Set up ${game} from your disc`, command, lines, () => {
    installs[game] = { serial, version: `${game}/${name}`, elfSha1: v.boot?.sha1 ?? "", files: 3, image };
  });
}

// ---- the save editor ---------------------------------------------------------------

/** The items and planets as core/src/save_edit.rs lists them. */
const mockItems: [number, string, "weapon" | "gadget", number, boolean][] = [
  [2, "Heli-Pack", "gadget", 0, false],
  [3, "Thruster-Pack", "gadget", 0, false],
  [4, "Hydro-Pack", "gadget", 0, false],
  [5, "Sonic Summoner", "gadget", 0, false],
  [6, "O2 Mask", "gadget", 0, false],
  [7, "Pilot's Helmet", "gadget", 0, false],
  [9, "Suck Cannon", "weapon", 0, true],
  [10, "Bomb Glove", "weapon", 40, true],
  [11, "Devastator", "weapon", 20, true],
  [12, "Swingshot", "gadget", 0, false],
  [13, "Visibomb Gun", "weapon", 20, false],
  [14, "Taunter", "gadget", 0, false],
  [15, "Blaster", "weapon", 200, true],
  [16, "Pyrocitor", "weapon", 240, true],
  [17, "Mine Glove", "weapon", 50, true],
  [18, "Walloper", "weapon", 0, false],
  [19, "Tesla Claw", "weapon", 240, true],
  [20, "Glove of Doom", "weapon", 10, true],
  [21, "Morph-o-Ray", "weapon", 0, true],
  [22, "Hydrodisplacer", "gadget", 0, false],
  [23, "R.Y.N.O.", "weapon", 50, false],
  [24, "Drone Device", "weapon", 10, false],
  [25, "Decoy Glove", "weapon", 20, true],
  [26, "Trespasser", "gadget", 0, false],
  [27, "Metal Detector", "gadget", 0, false],
  [28, "Magneboots", "gadget", 0, false],
  [29, "Grindboots", "gadget", 0, false],
  [30, "Hoverboard", "gadget", 0, false],
  [31, "Hologuise", "gadget", 0, false],
  [32, "PDA", "gadget", 0, false],
  [33, "Map-o-Matic", "gadget", 0, false],
  [34, "Bolt Grabber", "gadget", 0, false],
  [35, "Persuader", "gadget", 0, false],
];
const mockPlanets: [number, string, string, number][] = [
  [1, "Novalis", "Tobruk Crater", 3],
  [2, "Aridia", "Outpost X11", 4],
  [3, "Kerwan", "Metropolis", 3],
  [4, "Eudora", "Logging Site", 1],
  [5, "Rilgar", "Blackwater City", 2],
  [6, "Nebula G34", "Blarg Station", 2],
  [7, "Umbris", "Qwark's HQ", 2],
  [8, "Batalia", "Fort Krontos", 2],
  [9, "Gaspar", "Blarg Depot", 2],
  [10, "Orxon", "Kogor Refinery", 2],
  [11, "Pokitaru", "Jowai Resort", 1],
  [12, "Hoven", "Bomb Factory", 2],
  [13, "Oltanis Orbit", "Gemlik Base", 1],
  [14, "Oltanis", "Gorda City Ruins", 4],
  [15, "Quartu", "Robot Plant", 2],
  [16, "Kalebo III", "Gadgetron Site", 2],
  [17, "Veldin Orbit", "Drek's Fleet", 2],
  [18, "Veldin", "Kyzil Plateau", 3],
];
const savedFields: Record<string, SaveFields> = {};
const savedRaw: Record<string, Record<string, string>> = {};

/** The chunks as core/src/save_edit.rs lists them: id, size, name, decoded. */
const mockGlobalChunks: [number, number, string, boolean][] = [
  [0, 4, "Planet", true],
  [1, 4, "Bolts", true],
  [2, 4, "Times completed (challenge mode)", true],
  [3, 4, "Time played (1/60 s)", true],
  [4, 8, "When saved (clock)", true],
  [5, 128, "Story flags", false],
  [7, 12, "Cheats on", false],
  [8, 32, "Skill points", true],
  [9, 148, "Ammo by item", true],
  [10, 37, "Items owned", true],
  [11, 37, "Items ever had", true],
  [12, 12, "Vendor stock", true],
  [13, 32, "Quick select", true],
  [19, 4, "Max health", true],
  [14, 20, "Planets unlocked", true],
  [20, 80, "Galaxy map order", true],
  [15, 1936, "Map landmarks", false],
  [16, 1184, "Help messages seen", false],
  [17, 288, "Move help seen", false],
  [18, 296, "Gadget help seen", false],
  [21, 4, "Last item in hand", true],
  [22, 4, "Wrench in hand", true],
  [23, 4, "Thruster-Pack on the back", true],
  [24, 4, "Unused", false],
  [25, 4, "Camera left/right (1 normal)", true],
  [26, 4, "Camera up/down (1 normal)", true],
  [27, 4, "Camera speed (0 to 2)", true],
  [28, 1, "HelpDesk voice", true],
  [29, 1, "HelpDesk text", true],
  [30, 40, "Gold weapons", true],
  [31, 4, "Game beaten", true],
  [32, 28, "Equipped: hand, feet, head, back", true],
  [33, 1, "Subtitles", true],
  [34, 4, "Stereo (1) or mono", true],
  [35, 4, "Music volume (0 to 1024)", true],
  [36, 4, "Effects volume (0 to 1024)", true],
  [37, 12, "Cheats ever used", false],
  [1000, 148, "Ammo bought by item", true],
  [1001, 148, "Ammo picked up by item", true],
  [1002, 148, "Ammo used by item", true],
  [1003, 4, "Play time (ticks)", true],
  [1004, 4, "Hits taken", true],
  [1005, 4, "Deaths", true],
  [1008, 4, "Bolt rate: ticks", false],
  [1009, 4, "Bolt rate: bolts", false],
  [1010, 150, "Help log", false],
  [1011, 4, "Help log position", false],
];
const mockLevelChunks: [number, number, string, boolean][] = [
  [3001, 1, "Visited (0 never, 1 visited, 2 left)", true],
  [3002, 2048, "Map: the parts uncovered (packed)", false],
  [3003, 4, "Gold bolts found", true],
  [3004, 16, "Missions (0xff done)", true],
  [3005, 256, "Enemies and crates destroyed (bits)", false],
  [3006, 256, "Bolts from crates and enemies (pairs)", false],
  [3007, 8, "Arrivals: count, time, levels", false],
  [3008, 16, "Metal Detector spots", false],
  [4000, 4, "Bolts collected here", true],
  [4001, 4, "Hits taken here", true],
  [4002, 4, "Deaths here", true],
];
const levelNames: [string, string][] = [
  ["Veldin", "Kyzil Plateau"],
  ...mockPlanets.map(([, name, location]): [string, string] => [name, location]),
];

function word(n: number): string {
  return [0, 8, 16, 24].map((s) => ((n >>> s) & 0xff).toString(16).padStart(2, "0")).join(" ");
}

function mockChunks(serial: string, slot: number, f: SaveFields): SaveView["chunks"] {
  const raw = savedRaw[`${serial}/${String(slot)}`] ?? {};
  const zero = (n: number) => Array.from({ length: n }, () => "00").join(" ");
  const out: SaveView["chunks"] = [];
  for (const [id, size, name, known] of mockGlobalChunks) {
    let hex = zero(size);
    if (id === 0) hex = word(f.planet);
    if (id === 1) hex = word(f.bolts);
    if (id === 19) hex = word(f.nanotech === "ultra" ? 8 : f.nanotech === "premium" ? 5 : 4);
    out.push({ section: null, id, size, name, known, hex: raw[`g:${String(id)}`] ?? hex });
  }
  for (let l = 0; l < 20; l++) {
    for (const [id, size, name, known] of mockLevelChunks) {
      out.push({ section: l, id, size, name, known, hex: raw[`${String(l)}:${String(id)}`] ?? zero(size) });
    }
  }
  return out;
}

function mockSave(serial: string, slot: number): SaveView {
  const owned = new Set([2, 10, 12, 15, 16, 28]);
  const fields: SaveFields = savedFields[`${serial}/${String(slot)}`] ?? {
    planet: slot === 1 ? 3 : 0,
    bolts: slot === 1 ? 4890 : 1250,
    nanotech: "premium",
    items: mockItems.map(([id, , , max, gold]) => ({
      id,
      owned: owned.has(id),
      ammo: max > 0 ? (owned.has(id) ? Math.floor(max / 2) : 0) : null,
      gold: gold ? id === 15 : null,
    })),
    planets: mockPlanets.map(([id, , , n]) => ({
      id,
      unlocked: id <= 4,
      goldBolts: Array.from({ length: n }, (_, k) => id <= 3 && k === 0),
    })),
    skillPoints: Array.from({ length: 30 }, (_, k) => k % 7 === 0),
  };
  return {
    serial,
    slot,
    fingerprint: "preview",
    planetName: levelNames[fields.planet]?.[0] ?? null,
    savedAt: "2026-10-09 17:21:44",
    fields: structuredClone(fields),
    items: mockItems.map(([id, name, kind, max, gold]) => ({
      id,
      name,
      kind,
      maxAmmo: max > 0 ? max : null,
      hasGold: gold,
    })),
    planets: mockPlanets.map(([id, name, location]) => ({
      id,
      name,
      location,
      canUnlock: id <= 16,
      canLand: id <= 16,
    })),
    levels: levelNames.map(([name, location], id) => ({
      id,
      name,
      location,
      canUnlock: id >= 1 && id <= 16,
      canLand: id >= 1 && id <= 16,
    })),
    maxBolts: 9_999_999,
    chunks: mockChunks(serial, slot, fields),
    playedSeconds: slot === 1 ? 3 * 3600 + 12 * 60 : 47 * 60,
    // The names come from the player's disc; the preview has none, so ?skillnames shows sample ones.
    skillNames: new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("skillnames")
      ? Array.from({ length: 30 }, (_, k) => `Skill point name ${String(k + 1)}`)
      : [],
  };
}

// ---- the commands ------------------------------------------------------------------

const delay = <T>(value: T) =>
  new Promise<T>((resolve) =>
    setTimeout(() => {
      resolve(value);
    }, 120),
  );

export async function mockCall(command: string, args: Record<string, unknown> = {}): Promise<unknown> {
  switch (command) {
    case "app_info":
      return delay<AppInfo>({
        version: `${packageFile.version}-preview`,
        platform,
        configFile: "~/.config/dev.openrac.launcher/launcher.json",
        // ?standalone shows an installed launcher's pages: no checkout, nothing to find.
        standalone: new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("standalone"),
        updates: STABLE,
        updateChannels: { stable: STABLE, dev: DEV },
      });
    case "get_config":
      return delay<Config>({ ...config });
    case "save_config":
      config = { ...(args.config as Config) };
      return delay<Config>({ ...config });
    case "adopt_godot":
      return delay<Config>({ ...config });
    case "detect":
      return delay<Detected>({
        roots: query.has("setup") ? [] : [{ path: "/home/you/OpenRAC", source: "next to the launcher" }],
        pythons: [{ path: "/usr/bin/python3", source: "PATH" }],
        godots: [],
        dockers: [{ path: "/usr/bin/docker", source: "PATH" }],
      });
    case "check_root":
      return delay<Check>({
        ok: true,
        version: null,
        message: `${Object.keys(manifests).length} games, ${Object.values(manifests).flatMap((m) => Object.keys(m.versions)).length} versions`,
      });
    case "check_tool": {
      // What each program says about itself, as core/src/detect.rs reads it.
      const versions: Record<string, string> = {
        python: "Python 3.12.4",
        godot: "Godot 4.7.stable",
        docker: "Docker version 28.3.2",
      };
      const version = versions[String(args.tool)] ?? null;
      return delay<Check>({ ok: true, version, message: version ?? "found" });
    }
    case "library":
      // ?nolib: the OpenRAC folder cannot be read, as when none is set.
      if (query.has("nolib")) throw new Error("No OpenRAC folder is set: choose it in Settings.");
      return delay<Library>(library());
    case "install_game":
      return delay<Started>(install(args.game as string, args.image as string));
    case "prepare_disc":
      return delay((args.image as string).replace(/\.chd$/i, ".iso"));
    case "scan_discs": {
      // ?discs: the games folder holds the first game's PAL disc; ?discs=both, its NTSC-U disc too,
      // which with developer tools on offers the disc switch (core/src/disc_choice.rs).
      const asked = new URLSearchParams(location.search).get("discs");
      const disc = (version: string, serial: string, region: string, file: string) => ({
        path: `${config.discsDir}/${file}`,
        serial,
        game: "rac1",
        version,
        title: "Ratchet & Clank",
        region,
      });
      const found =
        config.discsDir && asked !== null
          ? [
              disc("rac1/pal", "SCES_509.16", "PAL (Europe)", "Ratchet & Clank (Europe).iso"),
              ...(asked === "both"
                ? [disc("rac1/ntsc", "SCUS_971.99", "NTSC-U (North America)", "Ratchet & Clank (USA).iso")]
                : []),
            ]
          : [];
      const versions = found.map((d) => d.version);
      const remembered = config.discChoice.rac1 ?? "";
      const choices =
        config.developer && versions.length > 1
          ? [
              {
                game: "rac1",
                versions,
                chosen: versions.includes(remembered) ? remembered : (versions[0] ?? remembered),
              },
            ]
          : [];
      return delay<DiscScan>({ found, choices });
    }
    case "choose_disc": {
      const version = args.version as string;
      const game = version.split("/")[0] ?? version;
      config = { ...config, discChoice: { ...config.discChoice, [game]: version } };
      return delay<Config>(config);
    }
    case "uninstall_game":
      installs[args.game as string] = undefined;
      return delay(null);
    case "sync_progress_from_web":
    case "apply_progress_json":
      return delay<Library>(library());
    case "run_action":
      return delay<Started>(start(args.scope as Scope, args.id as string, args));
    case "cancel_job": {
      const id = args.id as number;
      clearInterval(timers.get(id));
      timers.delete(id);
      emit({ type: "exit", id, code: null, cancelled: true, outputs: [] });
      return delay(null);
    }
    case "open_path":
    case "open_url":
      console.info(`[preview] ${command}`, args);
      return delay(null);
    case "set_discord_status":
      return delay(null);
    case "overrides_list":
      return delay<PackView[]>([...mockPacks]);
    case "overrides_set_enabled":
    case "overrides_move":
    case "overrides_remove": {
      const at = mockPacks.findIndex((p) => p.id === args.id);
      const pack = mockPacks[at];
      if (!pack) throw new Error(`no pack ${String(args.id)}`);
      mockPacks.splice(at, 1);
      if (command === "overrides_set_enabled") mockPacks.splice(at, 0, { ...pack, enabled: !!args.enabled });
      if (command === "overrides_move") {
        mockPacks.splice(Math.max(0, Math.min(mockPacks.length, at + (args.up ? -1 : 1))), 0, pack);
      }
      return delay<PackView[]>([...mockPacks]);
    }
    case "enhanced_view":
    case "enhanced_set_enabled":
    case "enhanced_set_setting": {
      if (args.game !== "rac1") return delay(null);
      if (command === "enhanced_set_enabled") mockEnhanced.mod = { ...mockEnhanced.mod, enabled: !!args.enabled };
      if (command === "enhanced_set_setting") {
        mockEnhanced.mod = {
          ...mockEnhanced.mod,
          settings: mockEnhanced.mod.settings.map((s) => (s.key === args.key ? { ...s, value: args.value } : s)),
        };
      }
      return delay({
        ...mockEnhanced,
        mod: { ...mockEnhanced.mod, state: mockEnhanced.mod.enabled ? "ready" : "off" },
      });
    }
    case "plugins_list":
      return delay<PluginView[]>(mockPlugins.map((p) => ({ ...p })));
    case "plugins_status":
      return delay<PluginStatus | null>(mockPluginStatus());
    case "plugins_set_enabled":
    case "plugins_move":
    case "plugins_set_setting":
    case "plugins_remove": {
      const at = mockPlugins.findIndex((p) => p.id === args.id);
      const mod = mockPlugins[at];
      if (!mod) throw new Error(`no mod ${String(args.id)}`);
      if (command === "plugins_set_enabled") {
        if (args.enabled && mod.state === "refused") throw new Error(mod.reason);
        mockPlugins[at] = { ...mod, enabled: !!args.enabled, state: args.enabled ? "ready" : "off" };
      } else if (command === "plugins_move") {
        mockPlugins.splice(at, 1);
        mockPlugins.splice(Math.max(0, Math.min(mockPlugins.length, at + (args.up ? -1 : 1))), 0, mod);
      } else if (command === "plugins_set_setting") {
        mockPlugins[at] = {
          ...mod,
          settings: mod.settings.map((s) => (s.key === args.key ? { ...s, value: args.value } : s)),
        };
      } else {
        mockPlugins = mockPlugins.filter((p) => p.id !== mod.id);
      }
      return delay<PluginView[]>(mockPlugins.map((p) => ({ ...p })));
    }
    case "plugins_preview": {
      const url = typeof args.url === "string" ? args.url : "";
      if (!/github\.com\/[^/]+\/[^/]+/.test(url)) {
        throw new Error(`${url} is not a repository's address: paste one that starts with https://`);
      }
      return delay<RemotePreview>(mockPreview(url));
    }
    case "plugins_install_remote": {
      const m = mockPreview(String(args.url)).mods.find((x) => x.id === args.id);
      if (!m) throw new Error(`no mod ${String(args.id)}`);
      if (!m.compatible) throw new Error(m.why ?? "it cannot be installed here");
      mockSources.mods[m.id] = {
        url: "https://github.com/OpenRAC/rac1-mods",
        path: m.path,
        version: m.version,
        tag: m.tag,
        commit: null,
        kind: m.kind,
        pin: mockSources.mods[m.id]?.pin ?? false,
      };
      return delay<RemoteInstalled>({
        kind: "plugin",
        id: m.id,
        name: m.name,
        version: m.version,
        replaced: true,
        enabled: true,
      });
    }
    case "plugins_check_updates":
      return delay({
        updates: Object.entries(mockSources.mods)
          .filter(([, s]) => !s.pin && s.version !== "1.1.0")
          .map(([id, s]) => ({ id, installed: s.version, available: "1.1.0", tag: "v1.1.0", commit: null })),
        errors: [],
      });
    case "plugins_update": {
      const s = mockSources.mods[String(args.id)];
      if (!s) throw new Error(`${String(args.id)} was not installed from a repository`);
      s.version = "1.1.0";
      return delay<RemoteInstalled>({
        kind: "plugin",
        id: String(args.id),
        name: "Lock-strafe",
        version: "1.1.0",
        replaced: true,
        enabled: true,
      });
    }
    case "plugins_sources":
      return delay<ModSources>(structuredClone(mockSources));
    case "plugins_set_pin": {
      const s = mockSources.mods[String(args.id)];
      if (!s) throw new Error(`${String(args.id)} was not installed from a repository`);
      s.pin = !!args.pin;
      return delay<ModSources>(structuredClone(mockSources));
    }
    case "plugins_install": {
      const path = typeof args.path === "string" ? args.path : "";
      // A name with "pack" in it stands for a data pack, which goes to the overrides.
      if (/pack/i.test(path)) return delay<PluginInstalled>({ kind: "pack" });
      const name =
        path
          .split(/[\\/]/)
          .at(-1)
          ?.replace(/\.zip$/i, "") ?? "mod";
      const id =
        name
          .toLowerCase()
          .replace(/[^a-z0-9]+/g, "-")
          .replace(/^-|-$/g, "") || "mod";
      const replaced = mockPlugins.some((p) => p.id === id);
      if (!replaced) {
        mockPlugins.push({
          ...mockPlugins[0],
          ...{ id, name, settings: [], hooks: [], sharedHooks: [], path: `/home/you/plugins/${id}` },
        } as PluginView);
      }
      return delay<PluginInstalled>({ kind: "plugin", id, name, version: "1.0.0", replaced, enabled: true });
    }
    case "open_output":
    case "open_level_in_godot":
    case "open_game_folder":
      console.info(`[preview] ${command}`, args);
      return delay(null);
    case "inspect_saves": {
      const serial = typeof args.serial === "string" ? args.serial : "SCES_509.16";
      return delay({
        serial,
        memcardDir: `/home/you/.local/share/openrac/memcard/${serial}`,
        exists: true,
        gameFolderName: "BESCES-50916RATCHET",
        title: "Ratchet & Clank",
        slots: [
          {
            slotIndex: 0,
            filename: "save0.bin",
            path: `/home/you/.local/share/openrac/memcard/${serial}/BESCES-50916RATCHET/save0.bin`,
            size: 59912,
            exists: true,
            isEmpty: false,
            editable: true,
            bolts: 1250,
            planetId: 0,
            planetName: "Veldin",
            timestamp: "2026-10-09 17:01:21",
            modifiedMillis: Date.now() - 3600000,
          },
          {
            slotIndex: 1,
            filename: "save1.bin",
            path: `/home/you/.local/share/openrac/memcard/${serial}/BESCES-50916RATCHET/save1.bin`,
            size: 59912,
            exists: true,
            isEmpty: false,
            editable: true,
            bolts: 4890,
            planetId: 3,
            planetName: "Kerwan",
            timestamp: "2026-10-09 17:21:44",
            modifiedMillis: Date.now() - 1800000,
          },
          {
            slotIndex: 2,
            filename: "save2.bin",
            path: `/home/you/.local/share/openrac/memcard/${serial}/BESCES-50916RATCHET/save2.bin`,
            size: 59912,
            exists: true,
            isEmpty: true,
            editable: false,
            bolts: null,
            planetId: null,
            planetName: null,
            timestamp: null,
            modifiedMillis: null,
          },
          {
            slotIndex: 3,
            filename: "save3.bin",
            path: `/home/you/.local/share/openrac/memcard/${serial}/BESCES-50916RATCHET/save3.bin`,
            size: 59912,
            exists: true,
            isEmpty: true,
            editable: false,
            bolts: null,
            planetId: null,
            planetName: null,
            timestamp: null,
            modifiedMillis: null,
          },
          {
            slotIndex: 4,
            filename: "save4.bin",
            path: `/home/you/.local/share/openrac/memcard/${serial}/BESCES-50916RATCHET/save4.bin`,
            size: 59912,
            exists: true,
            isEmpty: true,
            editable: false,
            bolts: null,
            planetId: null,
            planetName: null,
            timestamp: null,
            modifiedMillis: null,
          },
        ],
        backups: [
          {
            name: ".backup_1791560000_checkpoint",
            path: `/home/you/.local/share/openrac/memcard/${serial}/.backup_1791560000_checkpoint`,
            note: "checkpoint",
            createdMillis: Date.now() - 86400000,
            totalSize: 364544,
          },
        ],
      });
    }
    case "backup_saves": {
      const serial = typeof args.serial === "string" ? args.serial : "SCES_509.16";
      const note = typeof args.note === "string" && args.note.trim() ? args.note.trim() : null;
      const name = `.backup_${Math.floor(Date.now() / 1000)}${note ? `_${note.replace(/\s+/g, "_")}` : ""}`;
      return delay({
        name,
        path: `/home/you/.local/share/openrac/memcard/${serial}/${name}`,
        note,
        createdMillis: Date.now(),
        totalSize: 364544,
      });
    }
    case "restore_backup":
    case "open_saves_folder":
      return delay(null);
    case "reports_list":
      return delay<ReportEntry[]>(args.game === "rac1" ? mockReports : []);
    case "report_delete":
      mockReports = mockReports.filter((e) => e.file !== args.file);
      return delay(null);
    case "reports_delete_all": {
      const count = mockReports.length;
      mockReports = [];
      return delay(count);
    }
    case "latest_crash":
      return delay(
        query.has("crash") && mockReports.includes(mockCrash) && mockCrash.time >= Number(args.since ?? 0)
          ? mockCrash
          : null,
      );
    case "report_text":
      return delay(
        "**OpenRAC: the game stopped unexpectedly**\n\n- What happened: access (SIGSEGV) at game address 0x0F000000, which is not mapped\n- Where: Batalia (level 6)\n- OpenRAC: 0.7.1, commit ceffee00; launcher 0.7.1\n\n```\n4 Ratchet & Clank (PAL) 0x0000000104f1b3c0 func_0012DB18 + 68\n```\n",
      );
    case "report_save":
      return delay(
        `~/Downloads/OpenRAC-${String(args.file)
          .split("/")
          .pop()
          ?.replace(/\.(txt|log)$/, "")}.zip`,
      );
    case "open_reports_folder":
      console.info(`[preview] ${command}`, args);
      return delay(null);
    case "differences":
      return delay<Differences>(mockDifferences(String(args.game), String(args.version)));
    case "set_game_fix": {
      const now = mockDifferences(String(args.game), String(args.version));
      const fix = now.entries.find((e) => e.id === args.id && e.kind === "fix" && e.setting);
      if (!fix?.setting) throw new Error(`${String(args.id)} is not something that can be switched off`);
      const key = `${now.game}/${now.version}`;
      const values = fixValues.get(key) ?? new Map<string, boolean>();
      values.set(fix.setting.key, Boolean(args.on));
      fixValues.set(key, values);
      return delay<Differences>(mockDifferences(now.game, now.version));
    }
    case "read_save":
      return delay<SaveView>(
        mockSave(typeof args.serial === "string" ? args.serial : "SCES_509.16", Number(args.slot ?? 0)),
      );
    case "write_save": {
      const key = `${String(args.serial)}/${String(args.slot)}`;
      const change = args.change as SaveChange;
      if (change.kind === "fields") {
        if (change.fields.bolts > 9_999_999) throw new Error("bolts go up to 9999999");
        savedFields[key] = structuredClone(change.fields);
      } else {
        if (!config.saveEditorAdvanced) throw new Error("raw edits need the advanced save editor (Settings)");
        const raw = (savedRaw[key] ??= {});
        for (const c of change.chunks) raw[`${c.section === null ? "g" : String(c.section)}:${String(c.id)}`] = c.hex;
      }
      const view = mockSave(String(args.serial), Number(args.slot));
      const name = `.backup_${Math.floor(Date.now() / 1000)}_before_edit_slot${Number(args.slot) + 1}`;
      return delay<WriteResult>({
        backup: {
          name,
          path: `/home/you/.local/share/openrac/memcard/${String(args.serial)}/${name}`,
          note: `Before editing slot ${Number(args.slot) + 1}`,
          createdMillis: Date.now(),
          totalSize: 364544,
        },
        view,
      });
    }
    case "pick_disc":
      return delay("/data/user/0/dev.openrac.launcher/files/discs/game.iso");
    case "tools_ready":
      // The APK's copy of OpenRAC, unpacked by the time the preview asks.
      return delay(null);
    case "gpu_drivers":
      return delay(structuredClone(gpu));
    case "gpu_driver_import": {
      const driver = { ...turnip, id: `Turnip-25.3.0`, version: "25.3.0" };
      gpu = { ...gpu, drivers: [...gpu.drivers.filter((d) => d.id !== driver.id), driver] };
      return delay({ driver });
    }
    case "gpu_driver_select":
      gpu = { ...gpu, selected: (args.id as string | null) ?? null };
      return delay(null);
    case "gpu_renderer_select":
      gpu = { ...gpu, renderer: args.renderer as GpuDrivers["renderer"] };
      return delay(null);
    case "gpu_driver_remove":
      gpu = {
        ...gpu,
        drivers: gpu.drivers.filter((d) => d.id !== args.id),
        selected: gpu.selected === args.id ? null : gpu.selected,
      };
      return delay(null);
    case "background_status":
      return delay(null);
    // `?playing` in the preview: the Android game runs beside the launcher.
    case "game_status":
      return delay({ running: gameRunning, library: gameRunning ? "openrac-rac1-pal" : null });
    case "game_window":
      if (args.stop) gameRunning = false;
      return delay(null);
    case "update_check":
    case "launcher_update_check":
      return delay(null);
    case "update_install":
      return delay(null);
    default:
      throw new Error(`the preview has no ${command}: add it to src/lib/mock.ts`);
  }
}

// ---- launcher updates ----------------------------------------------------------------

/** The preview's answer to "is a newer launcher out?": ?update (one is, and installs itself;
 *  with ?download only from GitHub), ?latest (none), ?offline (GitHub cannot be reached).
 *  Undefined: no switch, so the real check runs. */
export function mockUpdate(current: string): UpdateInfo | null | Error | undefined {
  if (query.has("offline")) return new Error("no connection to GitHub");
  if (query.has("latest")) return null;
  if (!query.has("update")) return undefined;
  const [major = 0, minor = 0] = current
    .replace(/^[^0-9]*/, "")
    .split(".")
    .map((n) => parseInt(n, 10) || 0);
  const version = `${major}.${minor + 1}.0`;
  return {
    version,
    tag: `v${version}`,
    url: "https://github.com/OpenRAC/OpenRAC/releases",
    name: `OpenRAC Launcher v${version}`,
    body: [
      "- Every game has the same page; the list on the left switches between them.",
      "- Develop: the checkout, each version's decompilation and the console in one tab.",
      "- Ratchet & Clank: Size Matters joins the games.",
      "- Help beside your saves: a crash report to send in two clicks.",
    ].join("\n"),
    publishedAt: new Date(Date.now() - 86400000).toISOString(),
    canInstallDirectly: !query.has("download"),
  };
}

/** The preview's install: a 30 MB download in a few seconds, then installing; ?updatefail stops
 *  it part way, as a lost connection would. Nothing restarts in a browser. */
export async function mockInstall(onProgress?: UpdateProgressCallback): Promise<void> {
  const total = 31_457_280;
  for (let done = 0; done <= total; done += total / 20) {
    await new Promise((r) => setTimeout(r, 120));
    if (query.has("updatefail") && done > total * 0.4) throw new Error("the download stopped: the connection was lost");
    onProgress?.({
      stage: "downloading",
      percent: Math.round((done / total) * 100),
      downloadedBytes: done,
      totalBytes: total,
    });
  }
  onProgress?.({ stage: "installing", percent: 100, downloadedBytes: total, totalBytes: total });
  await new Promise((r) => setTimeout(r, 900));
  onProgress?.({ stage: "ready", percent: 100, downloadedBytes: total, totalBytes: total });
}
