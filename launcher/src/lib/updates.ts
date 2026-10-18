// In-app and GitHub release updater for OpenRAC Launcher.
import { inTauri, relaunchApp } from "./api";
import type { Update, DownloadEvent } from "@tauri-apps/plugin-updater";

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

export function isNewerVersion(current: string, latest: string): boolean {
  const parse = (v: string) =>
    v
      .replace(/^[^0-9]*/, "")
      .split(".")
      .map((part) => parseInt(part, 10) || 0);

  const [cMaj = 0, cMin = 0, cPatch = 0] = parse(current);
  const [lMaj = 0, lMin = 0, lPatch = 0] = parse(latest);

  if (lMaj !== cMaj) return lMaj > cMaj;
  if (lMin !== cMin) return lMin > cMin;
  return lPatch > cPatch;
}

interface GitHubReleaseResponse {
  tag_name?: string;
  html_url?: string;
  name?: string;
  body?: string;
  published_at?: string;
}

/** The newest release on GitHub when it is newer than CURRENT, else null; throws when GitHub could
 *  not be asked (no connection, a refusal), so "you have the latest" is never said by mistake. */
export async function checkGitHubRelease(currentVersion: string): Promise<UpdateInfo | null> {
  let res: Response;
  try {
    res = await fetch("https://api.github.com/repos/OpenRAC/OpenRAC/releases/latest", {
      headers: { Accept: "application/vnd.github.v3+json" },
      signal: AbortSignal.timeout(4000),
    });
  } catch {
    throw new Error("no connection to GitHub");
  }
  if (!res.ok) throw new Error(`GitHub answered ${res.status}`);

  const data = (await res.json()) as GitHubReleaseResponse;
  const tag = data.tag_name ?? "";
  const cleanVersion = tag.replace(/^[^0-9]*/, "");
  if (!isNewerVersion(currentVersion, cleanVersion)) return null;
  return {
    version: cleanVersion,
    tag,
    url: data.html_url ?? "https://github.com/OpenRAC/OpenRAC/releases",
    name: data.name ?? tag,
    body: data.body ?? "",
    publishedAt: data.published_at ?? "",
    canInstallDirectly: false,
  };
}

/**
 * Checks for updates using the official Tauri updater plugin first.
 * Falls back to checking GitHub Releases API if running in browser or if updater is unavailable.
 */
export async function checkForUpdate(currentVersion: string): Promise<UpdateInfo | null> {
  if (!inTauri) {
    // The browser preview's own answer (?update, ?latest, ?offline), when it has one.
    const { mockUpdate } = await import("./mock");
    const answer = mockUpdate(currentVersion);
    if (answer instanceof Error) throw answer;
    if (answer !== undefined) return answer;
  }
  if (inTauri) {
    try {
      const { check } = await import("@tauri-apps/plugin-updater");
      const update = await check();
      if (update) {
        activeTauriUpdate = update;
        return {
          version: update.version,
          tag: `v${update.version}`,
          url: "https://github.com/OpenRAC/OpenRAC/releases",
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

  return checkGitHubRelease(currentVersion);
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
