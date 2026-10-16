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
import type {
  ActionView,
  BuildInfo,
  InstallState,
  AppInfo,
  Check,
  Config,
  Detected,
  GameView,
  JobEvent,
  Library,
  PackView,
  Platform,
  Progress,
  Scope,
  Started,
  VersionStatus,
  VersionView,
} from "./api";

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
    };
    note?: string;
  }
>;

const manifests = import.meta.glob<RawGame>("../../../games/*/game.json", { eager: true, import: "default" });
const actions = actionsFile as { repository: RawAction[]; versions: Record<string, RawAction[] | undefined> };
const summary = summaryFile as RawSummary;

// ---- made-up state -------------------------------------------------------------------

const platform: Platform = "linux";

let config: Config = {
  root: "/home/you/OpenRAC",
  python: "/usr/bin/python3",
  godot: null,
  docker: "/usr/bin/docker",
  // ?developer shows the contributor's pages.
  developer: new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("developer"),
  // ?setup opens the first-run screen.
  setupComplete: !new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("setup"),
  installDir: "/home/you/.local/share/dev.openrac.launcher",
  // ?discs: a games folder is chosen.
  discsDir: new URLSearchParams(typeof location === "undefined" ? "" : location.search).has("discs")
    ? "/home/you/Games/PS2"
    : null,
  lastVersion: null,
  discordRpc: true,
  layout: "compact",
  discordClientId: null,
};

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

function installState(game: string): InstallState {
  return {
    dir: `${config.installDir ?? ""}/active/${game}/data`,
    extracted: installs[game] ?? null,
    decompiled: false,
    compiled: false,
  };
}

/** Which versions pretend to have their disc found and their inputs placed. */
const discs: Record<string, "found" | "missing" | "mismatch"> = {
  "rac1/pal": "found",
  "rac1/ntsc": "found",
  "rac3/ntsc": "mismatch",
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
        artifact: true,
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
    // The mock has no files: nothing is built or made.
    done: false,
  };
}

function progress(key: string): Progress | null {
  const row = summary[key];
  if (!row) return null;
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
  if (action.detached) return { id: 0, title: action.label, command, detached: true };

  const lines = [`$ ${command}`, "(browser preview: nothing really runs)", "working…", ...lastLines(id, args)];
  return fakeJob(action.label, command, lines, () => undefined);
}

/** A job that prints LINES one by one, then succeeds and calls DONE. */
function fakeJob(title: string, command: string, lines: string[], done: () => void): Started {
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
      emit({ type: "exit", id: job, code: 0, cancelled: false });
    }, 450),
  );
  return { id: job, title, command, detached: false };
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
      });
    case "get_config":
      return delay({ ...config });
    case "save_config":
      config = { ...(args.config as Config) };
      return delay({ ...config });
    case "detect":
      return delay<Detected>({
        roots: [{ path: "/home/you/OpenRAC", source: "next to the launcher" }],
        pythons: [{ path: "/usr/bin/python3", source: "PATH" }],
        godots: [],
        dockers: [{ path: "/usr/bin/docker", source: "PATH" }],
      });
    case "check_root":
      return delay<Check>({ ok: true, version: null, message: "4 games, 5 versions" });
    case "check_tool":
      return delay<Check>({ ok: true, version: "Python 3.12.4", message: "Python 3.12.4" });
    case "library":
      return delay(library());
    case "install_game":
      return delay(install(args.game as string, args.image as string));
    case "scan_discs":
      // ?discs: the games folder holds the first game's PAL disc.
      return delay(
        config.discsDir && new URLSearchParams(location.search).has("discs")
          ? [
              {
                path: `${config.discsDir}/Ratchet & Clank (Europe).iso`,
                serial: "SCES_509.16",
                game: "rac1",
                version: "rac1/pal",
                title: "Ratchet & Clank",
                region: "PAL (Europe)",
              },
            ]
          : [],
      );
    case "uninstall_game":
      installs[args.game as string] = undefined;
      return delay(null);
    case "sync_progress_from_web":
    case "apply_progress_json":
      return delay(library());
    case "run_action":
      return delay(start(args.scope as Scope, args.id as string, args));
    case "cancel_job": {
      const id = args.id as number;
      clearInterval(timers.get(id));
      timers.delete(id);
      emit({ type: "exit", id, code: null, cancelled: true });
      return delay(null);
    }
    case "open_path":
    case "open_url":
      console.info(`[preview] ${command}`, args);
      return delay(null);
    case "set_discord_status":
      return delay(null);
    case "overrides_list":
      return delay([...mockPacks]);
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
      return delay([...mockPacks]);
    }
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
            createdMillis: Date.now() - 86400000,
            totalSize: 364544,
          },
        ],
      });
    }
    case "backup_saves": {
      const serial = typeof args.serial === "string" ? args.serial : "SCES_509.16";
      const note = typeof args.note === "string" ? args.note : "manual";
      return delay({
        name: `.backup_${Math.floor(Date.now() / 1000)}_${note}`,
        path: `/home/you/.local/share/openrac/memcard/${serial}/.backup_${Math.floor(Date.now() / 1000)}_${note}`,
        createdMillis: Date.now(),
        totalSize: 364544,
      });
    }
    case "restore_backup":
    case "open_saves_folder":
      return delay(null);
    default:
      throw new Error(`the preview has no ${command}: add it to src/lib/mock.ts`);
  }
}
