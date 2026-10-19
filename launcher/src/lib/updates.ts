// In-app and GitHub release updater for OpenRAC Launcher. On a desktop it is Tauri's updater; on
// Android the launcher's own (src-tauri/src/android.rs): the release's APK, checked against the
// same key, handed to Android's installer.
import {
  api,
  inTauri,
  onAndroid,
  onUpdateProgress,
  relaunchApp,
  type AndroidRelease,
  type UpdateChannel,
  type UpdateSource,
} from "./api";
import { Update, type DownloadEvent } from "@tauri-apps/plugin-updater";

export interface UpdateInfo {
  version: string;
  tag: string;
  url: string;
  name: string;
  body: string;
  publishedAt: string;
  canInstallDirectly: boolean;
}

export type UpdateProgressCallback = (progress: {
  stage: "downloading" | "installing" | "ready";
  percent: number;
  downloadedBytes: number;
  totalBytes: number;
}) => void;

let activeTauriUpdate: Update | null = null;
let activeAndroidRelease: AndroidRelease | null = null;

/** core/src/update.rs's is_newer: a pre-release is older than its release, and of two
 *  pre-releases of one version the one whose last number is larger is newer (the builds of a
 *  branch, `0.8.0-dev.10` after `0.8.0-dev.9`). */
export function isNewerVersion(current: string, latest: string): boolean {
  const parse = (v: string): [number[], number | null] => {
    const text = v.replace(/^[^0-9]*/, "");
    const cut = text.search(/[-+]/);
    const core = cut < 0 ? text : text.slice(0, cut);
    const rest = cut < 0 ? "" : text.slice(cut + 1);
    const pre = rest ? parseInt(rest.split(".").pop() ?? "", 10) || 0 : null;
    const parts = core.split(".").map((part) => parseInt(part, 10) || 0);
    return [[parts[0] ?? 0, parts[1] ?? 0, parts[2] ?? 0], pre];
  };
  const [c, cPre] = parse(current);
  const [l, lPre] = parse(latest);
  for (let i = 0; i < 3; i++) {
    if (l[i] !== c[i]) return (l[i] ?? 0) > (c[i] ?? 0);
  }
  if (cPre === null) return false;
  return lPre === null || lPre > cPre;
}

/** The releases of the repository that built this launcher (OpenRAC's when it is not known). */
const DEFAULT_SOURCE: UpdateSource = {
  repository: "OpenRAC/OpenRAC",
  channel: null,
  manifest: "https://github.com/OpenRAC/OpenRAC/releases/latest/download/latest.json",
  page: "https://github.com/OpenRAC/OpenRAC/releases",
};

interface GitHubReleaseResponse {
  tag_name?: string;
  html_url?: string;
  name?: string;
  body?: string;
  published_at?: string;
}

export async function checkGitHubRelease(
  currentVersion: string,
  source: UpdateSource = DEFAULT_SOURCE,
): Promise<UpdateInfo | null> {
  if (!/^[\w.-]+\/[\w.-]+$/.test(source.repository)) return null;
  const release = source.channel ? `tags/${encodeURIComponent(source.channel)}` : "latest";
  const where = `github.com/${source.repository} · ${source.channel ?? "latest release"}`;
  let res: Response;
  try {
    res = await fetch(`https://api.github.com/repos/${source.repository}/releases/${release}`, {
      headers: { Accept: "application/vnd.github.v3+json" },
      signal: AbortSignal.timeout(4000),
    });
  } catch {
    throw new Error(`GitHub could not be reached (${where})`);
  }
  // No release where this launcher looks is not "the latest": a fork's builds publish none until
  // it has the update key (TAURI_SIGNING_PRIVATE_KEY), and the player is told so.
  if (res.status === 404) throw new Error(`no release has been published there yet (${where})`);
  if (!res.ok) throw new Error(`GitHub answered ${res.status} (${where})`);
  try {
    const data = (await res.json()) as GitHubReleaseResponse;
    const tag = data.tag_name ?? "";
    // A branch's rolling release keeps its tag: its version is in its title, "OpenRAC Launcher X (branch)".
    const cleanVersion = source.channel
      ? ((data.name ?? "").replace(/^[^0-9]*/, "").split(" ")[0] ?? "")
      : tag.replace(/^[^0-9]*/, "");

    if (isNewerVersion(currentVersion, cleanVersion)) {
      return {
        version: cleanVersion,
        tag,
        url: data.html_url ?? source.page,
        name: data.name ?? tag,
        body: data.body ?? "",
        publishedAt: data.published_at ?? "",
        canInstallDirectly: false,
      };
    }
    return null;
  } catch {
    throw new Error(`the release's details could not be read (${where})`);
  }
}

/**
 * Checks for updates using the official Tauri updater plugin first.
 * Falls back to checking GitHub Releases API if running in browser or if updater is unavailable.
 * Null when this launcher is the newest; throws when no release could be found to compare with.
 */
export async function checkForUpdate(
  currentVersion: string,
  source: UpdateSource = DEFAULT_SOURCE,
  channel: UpdateChannel = source.channel ? "dev" : "stable",
): Promise<UpdateInfo | null> {
  if (!inTauri) {
    // The browser preview's own answer (?update, ?latest, ?offline), when it has one.
    const { mockUpdate } = await import("./mock");
    const answer = mockUpdate(currentVersion);
    if (answer instanceof Error) throw answer;
    if (answer !== undefined) return answer;
  }
  if (inTauri && onAndroid) {
    // The chosen channel's manifest in the repository that built it (a fork's) is the one answer:
    // GitHub's latest release may be another channel's, which this launcher cannot install from.
    activeAndroidRelease = null;
    const release = await api.updateCheck(channel);
    activeAndroidRelease = release;
    if (!release) return null;
    return {
      version: release.version,
      tag: `v${release.version}`,
      url: source.page,
      name: `OpenRAC Launcher v${release.version}`,
      body: release.notes,
      publishedAt: release.date,
      canInstallDirectly: true,
    };
  } else if (inTauri) {
    activeTauriUpdate = null;
    try {
      // Asked at the chosen channel's manifest (src-tauri/src/lib.rs, launcher_update_check), not
      // only the one it was built with.
      const found = await api.launcherUpdateCheck(channel);
      if (found) {
        const update = new Update(found);
        activeTauriUpdate = update;
        return {
          version: update.version,
          tag: `v${update.version}`,
          url: source.page,
          name: `OpenRAC Launcher v${update.version}`,
          body: update.body ?? "",
          publishedAt: update.date ?? "",
          canInstallDirectly: true,
        };
      }
    } catch (e) {
      console.warn("Tauri updater check failed, falling back to GitHub release check:", e);
    }
  }

  return checkGitHubRelease(currentVersion, source);
}

/**
 * Downloads and installs the update in-place, then restarts the application.
 */
export async function installUpdate(onProgress?: UpdateProgressCallback): Promise<void> {
  if (!inTauri) {
    const { mockInstall } = await import("./mock");
    await mockInstall(onProgress);
    return;
  }
  if (onAndroid && activeAndroidRelease) {
    await installAndroidUpdate(activeAndroidRelease, onProgress);
    return;
  }
  if (!activeTauriUpdate) {
    throw new Error("No downloaded update available to install");
  }

  let totalBytes = 0;
  let downloadedBytes = 0;

  await activeTauriUpdate.downloadAndInstall((event: DownloadEvent) => {
    if (event.event === "Started") {
      totalBytes = event.data.contentLength ?? 0;
      onProgress?.({
        stage: "downloading",
        percent: 0,
        downloadedBytes: 0,
        totalBytes,
      });
    } else if (event.event === "Progress") {
      downloadedBytes += event.data.chunkLength;
      const percent = totalBytes > 0 ? Math.min(100, Math.round((downloadedBytes / totalBytes) * 100)) : 0;
      onProgress?.({
        stage: "downloading",
        percent,
        downloadedBytes,
        totalBytes,
      });
    } else {
      onProgress?.({
        stage: "installing",
        percent: 100,
        downloadedBytes,
        totalBytes,
      });
    }
  });

  onProgress?.({
    stage: "ready",
    percent: 100,
    downloadedBytes,
    totalBytes,
  });

  await relaunchApp();
}

/** Android: the APK is downloaded and checked, then Android's installer asks the player to confirm
 *  and restarts the launcher itself; "ready" here means that dialog is up. */
async function installAndroidUpdate(release: AndroidRelease, onProgress?: UpdateProgressCallback): Promise<void> {
  const off = await onUpdateProgress(({ downloaded, total }) => {
    const percent = total > 0 ? Math.min(100, Math.round((downloaded / total) * 100)) : 0;
    onProgress?.({ stage: "downloading", percent, downloadedBytes: downloaded, totalBytes: total });
  });
  try {
    await api.updateInstall(release.url, release.signature);
  } finally {
    off();
  }
  onProgress?.({ stage: "ready", percent: 100, downloadedBytes: 0, totalBytes: 0 });
}
