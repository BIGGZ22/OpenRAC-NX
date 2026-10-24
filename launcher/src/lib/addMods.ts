// Adding what the player chose or dropped on the Mods tab: a native mod goes to the game's mods
// (core/src/plugins.rs, plugins_install), a data pack to its overrides packs (editor/overrides.py,
// the version's overrides-add action). The player never has to know which is which.
import { api, type VersionView } from "./api";
import { jobs, runChain } from "./app.svelte";
import { added, withName } from "./modsText";

export interface Added {
  /** What happened, in a sentence. */
  text: string;
  failed: boolean;
  /** A native mod was added (the mods' list is read again). */
  plugin: boolean;
  /** A pack was added (the packs' list is read again). */
  pack: boolean;
}

/** The last line the add printed. */
function lastLine(): string {
  const job = jobs.list.filter((j) => j.scope.kind === "version" && j.actionId === "overrides-add").at(-1);
  return job?.lines.map((l) => l.line).findLast((l) => l.trim() !== "") ?? "";
}

const nameOf = (path: string) => path.split(/[\\/]/).at(-1) ?? path;

/** Adds each path in turn, stopping at the first that fails; what the last one came to. */
export async function addMods(v: VersionView, paths: string[]): Promise<Added> {
  const result: Added = { text: "", failed: false, plugin: false, pack: false };
  const adder = v.actions.find((a) => a.id === "overrides-add" && a.thisPlatform) ?? null;
  // A folder of several mods (a repository's checkout) is each of its mods.
  const each: string[] = [];
  for (const path of paths) {
    try {
      each.push(...(await api.pluginsModFolders(path)));
    } catch (e) {
      return { ...result, text: withName(nameOf(path), typeof e === "string" ? e : String(e)), failed: true };
    }
  }
  let count = 0;
  for (const path of each) {
    const name = nameOf(path);
    let installed;
    try {
      installed = await api.pluginsInstall(v.game, v.name, path);
    } catch (e) {
      return { ...result, text: withName(name, typeof e === "string" ? e : String(e)), failed: true };
    }
    if (installed.kind === "plugin") {
      result.plugin = true;
      count++;
      result.text = installed.replaced
        ? added.replaced(installed.name, installed.version)
        : installed.enabled
          ? added.on(installed.name)
          : added.off(installed.name);
      continue;
    }
    if (!adder?.runnable) {
      return { ...result, text: added.packHereNot(name), failed: true };
    }
    const ok = await runChain({ kind: "version", key: v.key }, [adder], { path });
    if (!ok) {
      const why = lastLine().replace(/^overrides: /, "");
      return { ...result, text: added.packFailed(name, why), failed: true };
    }
    result.pack = true;
    result.text = added.pack(name);
  }
  if (count > 1) result.text = added.several(count);
  return result;
}
