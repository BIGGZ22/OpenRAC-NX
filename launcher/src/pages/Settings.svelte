<script lang="ts">
  // Three groups: where things are (folders), the programs the launcher uses,
  // and how the launcher itself behaves. Edits stay local until saved; the bar
  // that saves them stays in view at the bottom of the window. With `setup`,
  // the first-run screen: the folders and programs, filled from detection.
  import GpuDrivers from "$components/GpuDrivers.svelte";
  import PathField from "$components/PathField.svelte";
  import { api, errorText, onAndroid, type Check, type Config, type Detected } from "$lib/api";
  import { app, lookForUpdate, saveConfig } from "$lib/app.svelte";
  import { DISCORD_URL, REPO_URL, SITE_URL } from "$lib/links";

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
  /** "Check now": what the last look for a new launcher found. */
  let looked = $state<"idle" | "looking" | "found" | "none" | "failed">("idle");
  let lookError = $state("");

  async function checkNow() {
    looked = "looking";
    try {
      looked = (await lookForUpdate()) ? "found" : "none";
    } catch (e) {
      lookError = errorText(e);
      looked = "failed";
    }
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

  const changed = $derived(JSON.stringify(draft) !== JSON.stringify(app.config));
  const tool = (t: "python" | "godot" | "docker") => (path: string) => api.checkTool(t, path);
  const root = (path: string): Promise<Check> => api.checkRoot(path);
  /** The install folder only has to be a place to write; the extractor makes what it needs inside. */
  const installFolder = (path: string): Promise<Check> =>
    Promise.resolve({ ok: true, version: null, message: `games are set up in ${path}/active/<game>/data` });

  /** The games folder is looked through when it is saved; what it holds shows on the Games page. */
  const gamesFolder = (path: string): Promise<Check> =>
    Promise.resolve({ ok: true, version: null, message: `disc images (.iso) are looked for in ${path}` });

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
          Play the Ratchet &amp; Clank games natively, with your own discs. Choose your OpenRAC folder to start.
        </p>
      </div>
    </section>
  {:else}
    <h1>Settings</h1>
  {/if}

  <section class="panel">
    <h2>Folders</h2>
    <!-- An installed launcher carries its own copy: the folder is a contributor's choice there. -->
    {#if !app.info?.standalone || draft.developer}
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
    <!-- Android's picker gives no folders: the app keeps its own (src-tauri/src/android.rs). -->
    {#if onAndroid && !draft.developer}
      <p class="dim note">
        The disc images you choose are copied into the app, and your games are set up in its storage: there are no
        folders to choose here.
      </p>
    {:else}
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
    {/if}
  </section>

  {#if onAndroid && !setup}
    <GpuDrivers />
  {/if}

  {#if !app.info?.standalone || draft.developer}
    <section class="panel">
      <h2>Programs</h2>
      <PathField
        label="Godot"
        hint="Godot 4.7 or newer, for the level editor. Optional."
        bind:value={draft.godot}
        candidates={found?.godots}
        detail={draft.developer}
        check={tool("godot")}
      />
      <!-- A player is asked for Python only when none was found; contributors always see it. -->
      {#if draft.developer || setup || !app.config?.python}
        <PathField
          label="Python"
          hint="Python 3.10 or newer: OpenRAC's tools run with it."
          bind:value={draft.python}
          candidates={found?.pythons}
          detail={draft.developer}
          check={tool("python")}
        />
      {/if}
      {#if draft.developer}
        <PathField
          label="Docker"
          hint="The build container of Ratchet & Clank (PAL) and Deadlocked runs in it, on Linux and macOS. Optional."
          bind:value={draft.docker}
          candidates={found?.dockers}
          detail={draft.developer}
          check={tool("docker")}
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
            <button class="link" disabled={looked === "looking"} onclick={() => void checkNow()}>Check now</button>
            {#if looked === "looking"}· Looking…{:else if looked === "found" || app.update}· {app.update?.name ??
                "A new version"} is out.
              <button class="link" onclick={() => (app.showUpdateModal = true)}>Update</button
              >{:else if looked === "none"}· You have the latest.{:else if looked === "failed"}· Could not check: {lookError}.{/if}
          </small>
        </span>
      </label>
      {#if app.info?.updates}
        {@const source = app.info.updates}
        <div class="field">
          <label for="update-source">Update source</label>
          <input
            id="update-source"
            type="text"
            class="mono"
            readonly
            aria-readonly="true"
            title="Set by the build: the repository that built this launcher"
            value={`github.com/${source.repository} · ${source.channel ?? "latest release"}`}
          />
          <p class="dim note">
            Set when this launcher was built, to the repository that built it, and not changeable here.
            {source.channel ? "This build of a branch updates from that branch's newest build." : ""}
            <button class="link" onclick={() => void api.openUrl(source.page)}>See the releases</button>
          </p>
        </div>
      {/if}
      <label class="option">
        <input type="checkbox" bind:checked={draft.detailedGameLog} />
        <span>
          <strong>Detailed game logs</strong>
          <small>
            Turn this on when we ask you to: the game writes more about what it does. Each game's logs and crash reports
            are on its own page, under its save files.
          </small>
        </span>
      </label>
      <label class="option">
        <input type="checkbox" bind:checked={draft.developer} />
        <span>
          <strong>Developer tools</strong>
          <small>
            For contributors: each game's builds, checks and decompilation progress, disc serials, and the Tasks page.
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
      {#if draft.developer && draft.discordRpc}
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
            To show your own name and art, create an app at
            <button class="link" onclick={() => void api.openUrl("https://discord.com/developers/applications")}>
              discord.com/developers/applications
            </button>.
          </p>
        </div>
      {/if}
    </section>
  {/if}

  {#if setup}
    <div class="row end">
      <button class="primary" disabled={saving || !draft.root} onclick={() => void save(true)}>Start</button>
    </div>
  {:else}
    <p class="dim about">
      OpenRAC Launcher {app.info?.version ?? ""}{#if draft.developer && app.info}
        · {app.info.platform}{#if app.info.configFile}
          · <span class="mono">{app.info.configFile}</span>{/if}{/if}
      <!-- The header's links, which Android's navigation has no room for. -->
      {#if onAndroid}
        · <button class="link" onclick={() => void api.openUrl(SITE_URL)}>openrac.dev</button>
        · <button class="link" onclick={() => void api.openUrl(DISCORD_URL)}>Discord</button>
      {/if}
    </p>
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
    /* Room at the bottom for the bar that saves. */
    padding: var(--page-top) var(--page-x) calc(var(--page-bottom) + 56px);
    display: flex;
    flex-direction: column;
    gap: var(--page-gap);
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

  .field input[readonly] {
    color: var(--soft);
    cursor: default;
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
    /* The middle of the page, beside or above Android's navigation (app.css). */
    left: calc(50% + var(--nav-left) / 2);
    bottom: calc(22px + var(--nav-bottom));
    transform: translateX(-50%);
    z-index: 25;
    width: min(764px, calc(100% - var(--nav-left) - 2 * var(--page-x)));
    display: flex;
    align-items: center;
    gap: 10px;
    padding: 12px 14px 12px 20px;
    background: var(--panel-hi);
    border: 1px solid var(--line-strong);
    border-radius: 999px;
    box-shadow: var(--shadow);
    font-size: 14px;
    animation: bar-in var(--medium) var(--ease) both;
  }

  @keyframes bar-in {
    from {
      opacity: 0;
      transform: translate(-50%, 24px);
    }
  }
</style>
