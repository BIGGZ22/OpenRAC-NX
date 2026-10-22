// Players share screenshots of their Mods tab, so it never shows a folder in plain view: a mod's
// install folder, the data folder, plugins.json or the home folder (with the user's name in it).
// A message that names one says the file's or folder's own name instead, and the whole message
// waits under a folded Details (Reason.svelte); a folder opens with Open folder, unseen.

/** An absolute path: /a/b…, ~/a…, C:\a… or \\server\share… (not a link's //host/…, not 1/2). */
const PATH =
  /(?<![\w:/.~\\])(?:~\/|\/(?=[^\s/]+\/)|[A-Za-z]:[\\/]|\\\\)(?:[^\s/\\:;,'"()<>`]+[\\/])*[^\s/\\:;,'"()<>`]*/g;

/** A path's last part ("plugins.json", "keyboard-mouse"), or "a folder" for a root. */
const lastPart = (path: string): string => {
  const parts = path.split(/[\\/]+/).filter((p) => p && !/^[A-Za-z]:$/.test(p) && p !== "~");
  return parts.at(-1) ?? "a folder";
};

/** TEXT with every absolute path in it cut to its last part, and whether there was one. */
export function hidePaths(text: string): { text: string; hidden: boolean } {
  let hidden = false;
  const out = text.replace(PATH, (path) => {
    hidden = true;
    const dots = /[.]+$/.exec(path)?.[0] ?? "";
    return lastPart(path.slice(0, path.length - dots.length)) + dots;
  });
  return { text: out, hidden };
}

/** Whether TEXT names a folder or file by its full path. */
export const hasPath = (text: string): boolean => hidePaths(text).hidden;
