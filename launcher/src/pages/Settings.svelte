<script lang="ts">
  // Three groups: where things are (folders), the programs the launcher uses,
  // and how the launcher itself behaves. Edits stay local until saved; the bar
  // that saves them stays in view at the bottom of the window. With `setup`,
  // the first-run screen: the folders and programs, filled from detection.
  import PathField from "$components/PathField.svelte";
  import { api, type Check, type Config, type Detected } from "$lib/api";
  import { app, lookForUpdate, saveConfig } from "$lib/app.svelte";
  import { REPO_URL } from "$lib/links";

  let { setup = false }: { setup?: boolean } = $props();

  /** The saved settings; this page only shows once they are loaded. */
  function saved(): Config {
    if (!app.config) throw new Error("settings not loaded");
    return { ...app.config };
  }

  /** What this page edits. The rest of the config (the last game opened, the crash reports seen)
   *  changes elsewhere while the page is open, so it is neither compared nor saved from here. */
  const EDITED = [
    "root",
    "python",
    "godot",
    "docker",
    "discsDir",
    "installDir",
    "layout",
    "discordRpc",
    "checkUpdates",
    "detailedGameLog",
    "saveEditorAdvanced",
    "developer",
    "discordClientId",
  ] as const satisfies (keyof Config)[];
  type Edited = Pick<Config, (typeof EDITED)[number]>;

  function edited(config: Config): Edited {
    return Object.fromEntries(EDITED.map((k) => [k, config[k]])) as Edited;
  }

  // Edits stay local until saved.
  let draft = $state<Config>(saved());
  let found = $state<Detected | null>(null);
  let saving = $state(false);
  /** "Check now": what the last look for a new launcher found. */
  let looked = $state<"idle" | "looking" | "found" | "latest" | "failed">("idle");

  async function checkNow() {
    looked = "looking";
    looked = await lookForUpdate();
  }

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

  const changed = $derived(!!app.config && JSON.stringify(edited(draft)) !== JSON.stringify(edited(app.config)));
  /** The OpenRAC folder is the first thing on the first run; after it, a player sees it only while
   *  it is missing (an installed launcher carries its own copy), and a contributor in Developer. */
  const rootHere = $derived(setup || (!app.info?.standalone && !draft.root));
  const tool = (t: "python" | "godot" | "docker") => (path: string) => api.checkTool(t, path);
  const root = (path: string): Promise<Check> => api.checkRoot(path);
  /** The install folder only has to be a place to write; the extractor makes what it needs inside. */
  const installFolder = (path: string): Promise<Check> =>
    Promise.resolve({ ok: true, version: null, message: `Games are set up in ${path}/active/<game>/data` });

  /** The games folder is looked through when it is saved; what it holds shows on the Games page. */
  const gamesFolder = (path: string): Promise<Check> =>
    Promise.resolve({ ok: true, version: null, message: `Disc images (.iso) are looked for in ${path}` });

  async function save(finish: boolean) {
    saving = true;
    let customId: string | null = draft.discordClientId?.trim() ?? null;
    if (customId === "") customId = null;
    await saveConfig({
      ...saved(),
      ...edited(draft),
      discordClientId: customId,
      setupComplete: (app.config?.setupComplete ?? false) || finish,
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
          Play the Ratchet &amp; Clank games natively, with your own discs. Choose your OpenRAC folder to start.
        </p>
      </div>
    </section>
  {:else}
    <h1>Settings</h1>
  {/if}

  <!-- Most used first; what a contributor needs is in Developer, last (it opens with the toggle). -->
  <section class="panel">
    <h2>Your games</h2>
    {#if rootHere}
      <PathField
        label="OpenRAC folder"
        hint="Your copy of OpenRAC: the folder with games/ and tools/ in it."
        folder
        bind:value={draft.root}
        candidates={found?.roots}
        detail={draft.developer}
        check={root}
      />
      {#if !draft.root}
        <p class="dim note">
          No copy yet? <button class="link" onclick={() => void api.openUrl(REPO_URL)}>Get OpenRAC</button>, then choose
          its folder here.
        </p>
      {/if}
    {/if}
    <PathField
      label="Games folder"
      hint="The folder with your PlayStation 2 disc images (.iso). OpenRAC finds its games in it."
      folder
      bind:value={draft.discsDir}
      detail={draft.developer}
      check={gamesFolder}
    />
    <PathField
      label="Install folder"
      hint="Where your games are set up from your discs. A few GB per game."
      folder
      bind:value={draft.installDir}
      detail={draft.developer}
      check={installFolder}
    />
  </section>

  {#if !app.info?.standalone}
    <section class="panel">
      <h2>Programs</h2>
      <PathField
        label="Godot"
        hint="Godot 4.7 or newer, for the level editor (Mods). Optional."
        bind:value={draft.godot}
        candidates={found?.godots}
        detail={draft.developer}
        check={tool("godot")}
      />
      <!-- A player is asked for Python only when none was found; contributors find it in Developer. -->
      {#if setup || (!draft.developer && !app.config?.python)}
        <PathField
          label="Python"
          hint="Python 3.10 or newer: OpenRAC's tools run with it."
          bind:value={draft.python}
          candidates={found?.pythons}
          detail={draft.developer}
          check={tool("python")}
        />
      {/if}
    </section>
  {/if}

  {#if !setup}
    <section class="panel">
      <h2>Launcher</h2>
      <label class="option">
        <input
          type="checkbox"
          checked={draft.layout !== "classic"}
          onchange={(e) => (draft.layout = e.currentTarget.checked ? "compact" : "classic")}
        />
        <span>
          <strong>Compact layout</strong>
          <small>A game's name and its Play button side by side. Off: stacked and centred.</small>
        </span>
      </label>
      <label class="option">
        <input type="checkbox" bind:checked={draft.discordRpc} />
        <span>
          <strong>Show in Discord</strong>
          <small>Your Discord status says when you are in the launcher or playing.</small>
        </span>
      </label>
      <label class="option">
        <input type="checkbox" bind:checked={draft.checkUpdates} />
        <span>
          <strong>Check for updates</strong>
          <small>
            Look for a new launcher when it starts. Off, it only looks when you ask. An update is never installed
            without your click.
            <!-- In the option's label: the click must not also switch the option. -->
            <button
              class="link"
              disabled={looked === "looking"}
              onclick={(e) => {
                e.preventDefault();
                void checkNow();
              }}>Check now</button
            >
            {#if looked === "looking"}<span class="said">Looking…</span>
            {:else if looked === "found"}<span class="said ok"
                >{app.update?.name ?? "A new version"} is out.
                <button class="link" onclick={() => (app.showUpdateModal = true)}>See what's new</button></span
              >
            {:else if looked === "latest"}<span class="said ok">You have the latest ({app.info?.version ?? ""}).</span>
            {:else if looked === "failed"}<span class="said warn"
                >Could not check: there is no connection to GitHub. Try again later.</span
              >{/if}
          </small>
        </span>
      </label>
    </section>

    <section class="panel">
      <h2>Troubleshooting</h2>
      <label class="option">
        <input type="checkbox" bind:checked={draft.detailedGameLog} />
        <span>
          <strong>Detailed game logs</strong>
          <small>
            Turn this on when we ask you to: the game writes more about what it does. Each game's logs and crash reports
            are under Help, on its page.
          </small>
        </span>
      </label>
      <label class="option">
        <input type="checkbox" bind:checked={draft.saveEditorAdvanced} />
        <span>
          <strong>Advanced save editing</strong>
          <small>
            For debugging: the save editor shows every field of a save, the story planets, and each part of the file as
            raw bytes. A wrong value can break a save; your memory card is backed up before every change.
          </small>
        </span>
      </label>
    </section>

    <section class="panel">
      <h2>Developer</h2>
      <label class="option">
        <input type="checkbox" bind:checked={draft.developer} />
        <span>
          <strong>Developer tools</strong>
          <small>
            For contributors: the Develop tab (each game's builds, checks and decompilation, the console), disc serials
            and file names, and the commands behind Mods.
          </small>
        </span>
      </label>
      {#if draft.developer}
        <div class="dev">
          {#if !rootHere}
            <PathField
              label="OpenRAC folder"
              hint="Your copy of OpenRAC: the folder with games/ and tools/ in it."
              folder
              bind:value={draft.root}
              candidates={found?.roots}
              detail={draft.developer}
              check={root}
            />
          {/if}
          <PathField
            label="Python"
            hint="Python 3.10 or newer: OpenRAC's tools run with it."
            bind:value={draft.python}
            candidates={found?.pythons}
            detail={draft.developer}
            check={tool("python")}
          />
          <PathField
            label="Docker"
            hint="The build container of Ratchet & Clank (PAL) and Deadlocked runs in it, on Linux and macOS. Optional."
            bind:value={draft.docker}
            candidates={found?.dockers}
            detail={draft.developer}
            check={tool("docker")}
          />
          {#if draft.discordRpc}
            <div class="field">
              <label for="discord-client-id">Discord application ID</label>
              <input
                id="discord-client-id"
                type="text"
                class="mono"
                placeholder="Empty: OpenRAC's own"
                bind:value={draft.discordClientId}
              />
              <p class="dim note">
                For Show in Discord: to show your own name and art, create an app at
                <button class="link" onclick={() => void api.openUrl("https://discord.com/developers/applications")}>
                  discord.com/developers/applications
                </button>.
              </p>
            </div>
          {/if}
          {#if app.info}
            <dl class="facts">
              <dt>System</dt>
              <dd>{app.info.platform}</dd>
              {#if app.info.configFile}<dt>Settings file</dt>
                <dd class="mono">{app.info.configFile}</dd>{/if}
            </dl>
          {/if}
        </div>
      {/if}
    </section>
  {/if}

  {#if setup}
    <div class="row end">
      <button class="primary" disabled={saving || !draft.root} onclick={() => void save(true)}>Start</button>
    </div>
  {:else}
    <p class="dim about">OpenRAC Launcher {app.info?.version ?? ""}</p>
  {/if}
</div>

{#if !setup && changed}
  <div class="save-bar" role="status">
    <span class="grow">You have unsaved changes.</span>
    <button disabled={saving} onclick={() => (draft = saved())}>Discard</button>
    <button class="primary" disabled={saving} onclick={() => void save(false)}>Save</button>
  </div>
{/if}

<style>
  .page {
    max-width: 820px;
    margin: 0 auto;
    padding: 28px 28px 110px;
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

  .panel {
    display: flex;
    flex-direction: column;
  }

  .panel h2 {
    margin-bottom: 4px;
  }

  .note {
    font-size: 13px;
  }

  /* What Check now found, on the line under the option. */
  .said {
    display: block;
    margin-top: 4px;
  }

  .said.ok {
    color: var(--ok);
  }

  .said.warn {
    color: var(--amber);
  }

  /* What developer tools add: under the toggle, set in from it. */
  .dev {
    display: flex;
    flex-direction: column;
    margin-top: 6px;
    padding-left: 28px;
    border-left: 1px solid var(--line);
  }

  .facts {
    display: grid;
    grid-template-columns: max-content minmax(0, 1fr);
    gap: 4px 14px;
    margin: 14px 0 4px;
    font-size: 13px;
  }

  .facts dt {
    color: var(--dim);
  }

  .facts dd {
    margin: 0;
    overflow-wrap: anywhere;
  }

  /* A checkbox with its name and one line saying what it does. */
  .option {
    display: flex;
    align-items: flex-start;
    gap: 14px;
    padding: 12px 0;
    cursor: pointer;
  }

  .option + .option {
    border-top: 1px solid var(--line);
  }

  .option input {
    flex: none;
    width: 18px;
    height: 18px;
    margin: 2px 0 0;
    accent-color: var(--amber);
    cursor: pointer;
  }

  .option strong {
    display: block;
    font-size: 14px;
    font-weight: 600;
  }

  .option small {
    display: block;
    margin-top: 2px;
    font-size: 13px;
    color: var(--soft);
  }

  .field {
    display: flex;
    flex-direction: column;
    gap: 6px;
    padding: 12px 0 2px 32px;
    border-top: 1px solid var(--line);
  }

  .field label {
    font-size: 13px;
    font-weight: 600;
  }

  .field input {
    max-width: 400px;
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

  .about {
    font-size: 12px;
  }

  /* Stays in view while there is something to save. */
  .save-bar {
    position: fixed;
    left: 50%;
    bottom: 22px;
    transform: translateX(-50%);
    z-index: 25;
    width: min(764px, calc(100% - 56px));
    display: flex;
    align-items: center;
    gap: 10px;
    padding: 12px 14px 12px 20px;
    background: var(--panel-hi);
    border: 1px solid var(--line-strong);
    border-radius: 999px;
    box-shadow: var(--shadow);
    font-size: 14px;
  }
</style>
