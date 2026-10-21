// Adding what the player chose or dropped on the Mods tab: a native mod goes to the game's mods
// (core/src/plugins.rs, plugins_install), a data pack to its overrides packs (editor/overrides.py,
// the version's overrides-add action). The player never has to know which is which.
import { api, type VersionView } from "./api";
import { jobs, runChain } from "./app.svelte";

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
  for (const path of paths) {
    const name = nameOf(path);
    let installed;
    try {
      installed = await api.pluginsInstall(v.game, v.name, path);
    } catch (e) {
      return { ...result, text: `${name}: ${typeof e === "string" ? e : String(e)}`, failed: true };
    }
    if (installed.kind === "plugin") {
      result.plugin = true;
      result.text = installed.replaced
        ? `Updated ${installed.name} to ${installed.version}; its settings are kept.`
        : installed.enabled
          ? `Added ${installed.name}. It is on from the game's next start.`
          : `Added ${installed.name}. It is off: its row says why.`;
      continue;
    }
    if (!adder?.runnable) {
      return { ...result, text: `${name} is a texture and model pack, which cannot be added here yet.`, failed: true };
    }
    const ok = await runChain({ kind: "version", key: v.key }, [adder], { path });
    if (!ok) {
      const why = lastLine().replace(/^overrides: /, "");
      return { ...result, text: `${name}: ${why.length > 0 ? why : "it could not be added"}`, failed: true };
    }
    result.pack = true;
    result.text = `Added ${name}.`;
  }
  return result;
}
