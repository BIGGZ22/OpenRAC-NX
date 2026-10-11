<script lang="ts">
  // Where the OpenRAC folder and the programs are. With `setup`, the
  // first-run screen: the same fields, filled from detection.
  import PathField from "$components/PathField.svelte";
  import { api, type Check, type Config, type Detected } from "$lib/api";
  import { app, saveConfig } from "$lib/app.svelte";
  import { REPO_URL } from "$lib/links";

  let { setup = false }: { setup?: boolean } = $props();

  /** The saved settings; this page only shows once they are loaded. */
  function saved(): Config {
    if (!app.config) throw new Error("settings not loaded");
    return { ...app.config };
  }

  // Edits stay local until saved.
  let draft = $state<Config>(saved());
  let found = $state<Detected | null>(null);
  let saving = $state(false);

  $effect(() => {
    void api.detect().then((d) => {
      found = d;
      // First run: take what was found for anything not set yet.
      if (setup) {
        draft.root ??= d.roots[0]?.path ?? null;
        draft.python ??= d.pythons[0]?.path ?? null;
        draft.godot ??= d.godots[0]?.path ?? null;
        draft.docker ??= d.dockers[0]?.path ?? null;
      }
    });
  });

  const changed = $derived(JSON.stringify(draft) !== JSON.stringify(app.config));
  const tool = (t: "python" | "godot" | "docker") => (path: string) => api.checkTool(t, path);
  const root = (path: string): Promise<Check> => api.checkRoot(path);
  /** The install folder only has to be a place to write; the extractor makes what it needs inside. */
  const installFolder = (path: string): Promise<Check> =>
    Promise.resolve({ ok: true, version: null, message: `games are set up in ${path}/active/<game>/data` });

  async function save(finish: boolean) {
    saving = true;
    let customId: string | null = draft.discordClientId?.trim() ?? null;
    if (customId === "") customId = null;
    await saveConfig({
      ...draft,
      discordClientId: customId,
      setupComplete: draft.setupComplete || finish,
    });
    draft = saved();
    saving = false;
    if (finish) app.page = "library";
  }
</script>

<div class="page">
  {#if setup}
    <section class="welcome rise">
      <img src="/wrench.webp" alt="" width="72" height="72" />
      <div>
        <h1>Welcome to OpenRAC</h1>
        <p class="muted">
          Play the Ratchet &amp; Clank games natively and edit their levels, with your own discs. The launcher could not
          find everything it needs by itself: choose your OpenRAC folder below, and Python if none is shown.
        </p>
      </div>
    </section>
  {:else}
    <h1>Settings</h1>
  {/if}

  <section class="panel">
    <h2>OpenRAC</h2>
    <PathField
      label="OpenRAC folder"
      hint="Your checkout of OpenRAC: the folder with games/, tools/ and baserom/."
      folder
      bind:value={draft.root}
      candidates={found?.roots}
      check={root}
    />
    {#if !draft.root}
      <p class="dim note">
        No checkout yet? <button class="link" onclick={() => void api.openUrl(REPO_URL)}>Get OpenRAC</button> with git, then
        choose its folder here.
      </p>
    {/if}
    <PathField
      label="Install folder"
      hint="Where games are set up from your discs: the disc's files, and later the prepared assets and the built game. A few GB per game."
      folder
      bind:value={draft.installDir}
      check={installFolder}
    />
  </section>

  <section class="panel">
    <h2>Programs</h2>
    <PathField
      label="Godot"
      hint="Godot 4 (4.7 or newer), the game's 3D engine and level player. Optional."
      bind:value={draft.godot}
      candidates={found?.godots}
      check={tool("godot")}
    />
    <!-- A player is asked for Python only when none was found; contributors always see it. -->
    {#if draft.developer || setup || !app.config?.python}
      <PathField
        label="Python"
        hint="Python 3.10 or newer: OpenRAC's tools and the level editor run with it."
        bind:value={draft.python}
        candidates={found?.pythons}
        check={tool("python")}
      />
    {/if}
    {#if draft.developer}
      <PathField
        label="Docker"
        hint="The build container of Ratchet & Clank (PAL) and Deadlocked runs in it, on Linux and macOS. Optional."
        bind:value={draft.docker}
        candidates={found?.dockers}
        check={tool("docker")}
      />
    {/if}
  </section>

  <section class="panel">
    <h2>Appearance</h2>
    <label class="toggle-row">
      <input
        type="checkbox"
        checked={draft.layout !== "classic"}
        onchange={(e) => (draft.layout = e.currentTarget.checked ? "compact" : "classic")}
      />
      <div>
        <strong>Compact layout</strong>
        <p class="muted">The game, its versions and Play on one band. Off: the original layout.</p>
      </div>
    </label>
  </section>

  <section class="panel">
    <h2>Integrations</h2>
    <label class="toggle-row">
      <input type="checkbox" bind:checked={draft.discordRpc} />
      <div>
        <strong>Discord Rich Presence</strong>
        <p class="muted">Show in Discord when you are in the launcher or playing a game.</p>
      </div>
    </label>
    {#if draft.discordRpc}
      <div class="rpc-config">
        <label for="discord-client-id" class="field-title">Discord Application ID</label>
        <input
          id="discord-client-id"
          type="text"
          class="text-input mono"
          placeholder="e.g. 1348000000000000000"
          bind:value={draft.discordClientId}
        />
        <p class="muted hint">
          Leave empty for default. Create an app on
          <button class="link" onclick={() => void api.openUrl("https://discord.com/developers/applications")}>
            discord.com/developers/applications
          </button>
          named <em>OpenRAC</em> to show custom game name and assets on your profile.
        </p>
      </div>
    {/if}
  </section>

  {#if !setup}
    <section class="panel">
      <h2>Developer tools</h2>
      <label class="switch">
        <input type="checkbox" bind:checked={draft.developer} />
        <span>
          Show what contributors use: each game's build, checks, toolchains and decompilation progress. Off, the
          launcher shows a player's three steps: set up from your disc, play, edit levels.
        </span>
      </label>
    </section>
  {/if}

  <div class="row end">
    {#if app.info?.configFile}<span class="dim mono grow clip" title="Settings file">{app.info.configFile}</span>{/if}
    {#if setup}
      <button class="primary" disabled={saving || !draft.root} onclick={() => void save(true)}>Start</button>
    {:else}
      <button disabled={!changed || saving} onclick={() => (draft = saved())}>Undo</button>
      <button class="primary" disabled={!changed || saving} onclick={() => void save(false)}>Save</button>
    {/if}
  </div>
  {#if !setup && app.info}
    <p class="dim version">OpenRAC Launcher {app.info.version} · {app.info.platform}</p>
  {/if}
</div>

<style>
  .page {
    max-width: 820px;
    margin: 0 auto;
    padding: 28px 28px 60px;
    display: flex;
    flex-direction: column;
    gap: 18px;
  }

  .welcome {
    display: flex;
    gap: 20px;
    align-items: center;
    padding: 24px;
    border-radius: 30px;
    background: radial-gradient(80% 90% at 80% 0, rgb(213 138 0 / 0.28), transparent 70%), var(--panel);
    border: 1px solid var(--line);
  }

  .welcome h1 {
    font-size: 28px;
    margin-bottom: 6px;
  }

  .panel h2 {
    margin-bottom: 4px;
  }

  .note {
    font-size: 13px;
  }

  .switch {
    display: flex;
    gap: 12px;
    align-items: flex-start;
    font-size: 14px;
    color: var(--soft);
    cursor: pointer;
  }

  .switch input {
    margin-top: 3px;
    width: 18px;
    height: 18px;
    accent-color: var(--amber);
  }

  .link {
    border: 0;
    background: none;
    padding: 0;
    color: var(--amber);
    font-size: 13px;
  }

  .end {
    justify-content: flex-end;
  }

  .version {
    font-size: 12px;
    text-align: right;
  }

  .toggle-row {
    display: flex;
    align-items: center;
    gap: 14px;
    cursor: pointer;
    user-select: none;
    padding: 6px 0;
  }

  .toggle-row input[type="checkbox"] {
    width: 18px;
    height: 18px;
    accent-color: var(--amber);
    cursor: pointer;
  }

  .toggle-row strong {
    color: var(--text);
    font-size: 14px;
    display: block;
  }

  .toggle-row p {
    margin: 2px 0 0;
    font-size: 13px;
  }

  .rpc-config {
    margin-top: 14px;
    padding-top: 14px;
    border-top: 1px solid var(--border);
    display: flex;
    flex-direction: column;
    gap: 6px;
  }

  .field-title {
    font-size: 13px;
    font-weight: 600;
    color: var(--text);
  }

  .text-input {
    background: var(--bg-card);
    border: 1px solid var(--border);
    border-radius: 6px;
    color: var(--text);
    padding: 8px 12px;
    font-size: 13px;
    outline: none;
    width: 100%;
    max-width: 400px;
  }

  .text-input:focus {
    border-color: var(--amber);
  }

  .hint {
    font-size: 12px;
    margin: 2px 0 0;
  }
</style>
