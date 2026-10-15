// Checks GitHub releases for launcher updates.
export interface UpdateInfo {
  version: string;
  tag: string;
  url: string;
  name: string;
  body: string;
  publishedAt: string;
}

interface GitHubReleaseResponse {
  tag_name?: string;
  html_url?: string;
  name?: string;
  body?: string;
  published_at?: string;
}

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

export async function checkGitHubRelease(currentVersion: string): Promise<UpdateInfo | null> {
  try {
    const res = await fetch("https://api.github.com/repos/OpenRAC/OpenRAC/releases/latest", {
      headers: { Accept: "application/vnd.github.v3+json" },
      signal: AbortSignal.timeout(4000),
    });
    if (!res.ok) return null;

    const data = (await res.json()) as GitHubReleaseResponse;
    const tag = data.tag_name ?? "";
    const cleanVersion = tag.replace(/^[^0-9]*/, "");

    if (isNewerVersion(currentVersion, cleanVersion)) {
      return {
        version: cleanVersion,
        tag,
        url: data.html_url ?? "https://github.com/OpenRAC/OpenRAC/releases",
        name: data.name ?? tag,
        body: data.body ?? "",
        publishedAt: data.published_at ?? "",
      };
    }
    return null;
  } catch {
    return null;
  }
}
