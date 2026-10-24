<script lang="ts">
  // Three groups: where things are (folders), the programs the launcher uses,
  // and how the launcher itself behaves. Edits stay local until saved; the bar
  // that saves them stays in view at the bottom of the window. With `setup`,
  // the first-run screen: one question (the games folder), and what was found by itself, folded.
  import GpuDrivers from "$components/GpuDrivers.svelte";
  import Logo from "$components/Logo.svelte";
  import PathField from "$components/PathField.svelte";
  import {
    api,
    errorText,
    onAndroid,
    updateBranchOf,
    updateChannelOf,
    updateSourceOf,
    type Check,
    type Config,
    type Detected,
    type UpdateChannel,
  } from "$lib/api";
  import { app, guardShown, lookForUpdate, saveConfig } from "$lib/app.svelte";
  import { DISCORD_URL, PYTHON_URL, REPO_URL, SITE_URL } from "$lib/links";
  import { PURIST } from "$lib/modsText";
  import { applyPalette, PALETTES } from "$lib/palettes";
  import { devBranches } from "$lib/updates";
  import { tablist } from "$lib/tabs";

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
    "theme",
    "discordRpc",
    "purist",
    "checkUpdates",
    "updateChannel",
    "updateBranch",
    "detailedGameLog",
    "renderer",
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
  let looked = $state<"idle" | "looking" | "found" | "none" | "failed">("idle");
  let lookError = $state("");

  /** The OpenRAC folder is the first thing on the first run; after it, a player sees it only while
   *  it is missing (an installed launcher carries its own copy), and a contributor in Developer. */
  const rootHere = $derived(!app.info?.standalone && !draft.root);
  /** The game's Vulkan renderer, on the desktops (on macOS through the MoltenVK its bundle carries);
   *  Android chooses its own in Graphics (GpuDrivers). */
  const desktopRenderer = $derived(
    app.info?.platform === "windows" || app.info?.platform === "linux" || app.info?.platform === "macos",
  );
  /** Advanced opens by itself when something in it is on. */
  let advancedOpen = $state(
    saved().detailedGameLog || saved().developer || saved().saveEditorAdvanced || saved().renderer === "vulkan",
  );

  /** The update channel as this page has it, saved or not. */
  const channel = $derived<UpdateChannel>(app.info ? updateChannelOf(app.info, draft) : "stable");
  /** Where the launcher looks for updates as this page has it: the channel, and a fork's branch. */
  const source = $derived(app.info ? updateSourceOf(app.info, draft) : null);
  /** The branch the development builds come from, as the release's tag names it. */
  const devBranch = $derived(source?.channel?.replace(/^launcher-dev-/, "") ?? "master");
  /** The branch the fork's development builds follow when none is named: the one it was built from. */
  const builtBranch = $derived(app.info?.updateChannels.dev.channel?.replace(/^launcher-dev-/, "") ?? "master");
  /** A fork's branches with development builds, offered beside the field (asked once, when it shows). */
  let branches = $state<string[]>([]);
  $effect(() => {
    const dev = app.info?.updateChannels.dev;
    if (channel === "dev" && dev?.fork && !branches.length) void devBranches(dev).then((b) => (branches = b));
  });

  async function checkNow() {
    looked = "looking";
    try {
      // The channel and branch chosen here, before they are saved too.
      looked = (await lookForUpdate(draft)) ? "found" : "none";
    } catch (e) {
      lookError = errorText(e);
      looked = "failed";
    }
  }

  /** First run (shown only when the OpenRAC folder or Python was not found: firstRun in
   *  app.svelte.ts): what was not found is asked for beside the games folder, and stays there once
   *  chosen, so the field does not move away under the pointer. */
  let askRoot = $state(false);
  let askPython = $state(false);
  /** First run: what "Found for you" holds, in a line, so it can stay folded. */
  const foundSummary = $derived.by(() => {
    if (!found) return "Looking…";
    const names = [
      !askRoot && draft.root && "OpenRAC",
      !app.info?.standalone && !askPython && draft.python && "Python",
      !app.info?.standalone && draft.godot && "Godot",
    ].filter(Boolean);
    return names.length ? `${names.join(", ")}. Change them here if they are wrong.` : "Nothing yet.";
  });

  $effect(() => {
    void api.detect().then((d) => {
      found = d;
      // First run: take what was found for anything not set yet.
      if (setup) {
        askRoot = !draft.root && !d.roots.length;
        askPython = !app.info?.standalone && !draft.python && !d.pythons.length;
        draft.root ??= d.roots[0]?.path ?? null;
        draft.python ??= d.pythons[0]?.path ?? null;
        draft.godot ??= d.godots[0]?.path ?? null;
        draft.docker ??= d.dockers[0]?.path ?? null;
      }
    });
  });

  // A theme picked shows at once; left unsaved, the saved one comes back with the page's end.
  $effect(() => {
    applyPalette(draft.theme);
    return () => {
      applyPalette(app.config?.theme);
    };
  });

  const changed = $derived(!!app.config && JSON.stringify(edited(draft)) !== JSON.stringify(edited(app.config)));

  /** Android saves a change by itself, a moment after it is made (a field being typed in is
   *  saved once the typing stops), with no bar to press: a switch flipped is a setting changed.
   *  A change that could not be written is not tried again until it changes again. */
  let unwritten = $state<string | null>(null);
  $effect(() => {
    if (!onAndroid || setup || !changed || saving) return;
    const now = JSON.stringify(edited(draft));
    if (now === unwritten) return;
    const timer = setTimeout(() => {
      void save(false).then(() => {
        const written = !!app.config && JSON.stringify(edited(app.config)) === now;
        unwritten = written ? null : now;
      });
    }, 600);
    return () => {
      clearTimeout(timer);
    };
  });
  const tool = (t: "python" | "godot" | "docker") => (path: string) => api.checkTool(t, path);
  const root = (path: string): Promise<Check> => api.checkRoot(path);
  /** The install folder only has to be a place to write; the extractor makes what it needs inside,
   *  and the games their saves and settings (core saves.rs). */
  const installFolder = (path: string): Promise<Check> =>
    Promise.resolve({
      ok: true,
      version: null,
      message: `Games in ${path}/active, saves in ${path}/memcard, settings in ${path}/settings`,
    });

  /** The games folder is looked through when it is saved; what it holds shows on the Games page. */
  const gamesFolder = (path: string): Promise<Check> =>
    Promise.resolve({ ok: true, version: null, message: `Disc images (.iso) are looked for in ${path}` });

  async function save(finish: boolean) {
    saving = true;
    let customId: string | null = draft.discordClientId?.trim() ?? null;
    if (customId === "") customId = null;
    const before = app.info ? updateSourceOf(app.info, app.config).manifest : null;
    const written = await saveConfig({
      ...saved(),
      ...edited(draft),
      discordClientId: customId,
      updateBranch: updateBranchOf(draft),
      setupComplete: (app.config?.setupComplete ?? false) || finish,
    });
    if (!written) {
      // The settings were not written (the toast says why): what was typed stays, and the
      // first run does not leave for a page with nothing in it.
      saving = false;
      return;
    }
    draft = saved();
    // Another channel or branch: what was found on the last one is not what this one offers.
    if (app.info && before !== updateSourceOf(app.info, app.config).manifest) {
      app.update = null;
      looked = "idle";
      if (app.config?.checkUpdates) lookForUpdate().catch(() => undefined);
    }
    saving = false;
    if (finish) app.page = "library";
  }
</script>

<div class="page">
  {#if setup}
    <section class="welcome rise">
      <Logo mark height={72} label="" />
      <div>
        <h1>Welcome to OpenRAC</h1>
        <p class="muted">Play the Ratchet &amp; Clank games natively, with your own discs.</p>
      </div>
    </section>

    <!-- One question; what the launcher found by itself waits under "Found for you". -->
    <section class="panel">
      {#if !onAndroid}
        <h2>Where are your PS2 disc images?</h2>
        <PathField
          label="Games folder"
          hint="The folder with your disc images (.iso). You can also choose it later, on the Games page."
          folder
          bind:value={draft.discsDir}
          check={gamesFolder}
        />
      {/if}
      {#if askRoot}
        <PathField
          label="OpenRAC folder"
          hint="Your copy of OpenRAC: the folder with games/ and tools/ in it. It was not found by itself."
          folder
          bind:value={draft.root}
          candidates={found?.roots}
          check={root}
        />
        {#if !draft.root}
          <p class="dim note">
            No copy yet? <button class="link" onclick={() => void api.openUrl(REPO_URL)}>Get OpenRAC</button>, then
            choose its folder here.
          </p>
        {/if}
      {/if}
      {#if askPython}
        <PathField
          label="Python"
          hint="Python 3.10 or newer: OpenRAC's tools run with it. It was not found by itself."
          bind:value={draft.python}
          check={tool("python")}
        />
        {#if !draft.python}
          <p class="dim note">
            No Python yet? <button class="link" onclick={() => void api.openUrl(PYTHON_URL)}>Get Python</button>, then
            choose it here.
          </p>
        {/if}
      {/if}
    </section>

    <section class="panel">
      <details class="more">
        <summary>
          <h2>Found for you</h2>
          <span class="dim note">{foundSummary}</span>
        </summary>
        {#if !askRoot}
          <PathField
            label="OpenRAC folder"
            hint="Your copy of OpenRAC: the folder with games/ and tools/ in it."
            folder
            bind:value={draft.root}
            candidates={found?.roots}
            check={root}
          />
        {/if}
        {#if !onAndroid}
          <PathField
            label="Install folder"
            hint="Everything OpenRAC keeps for you: your games set up from your discs (a few GB each), your saves and your settings."
            folder
            bind:value={draft.installDir}
            check={installFolder}
          />
        {/if}
        {#if !app.info?.standalone}
          {#if !askPython}
            <PathField
              label="Python"
              hint="Python 3.10 or newer: OpenRAC's tools run with it."
              bind:value={draft.python}
              candidates={found?.pythons}
              check={tool("python")}
            />
          {/if}
          <PathField
            label="Godot"
            hint="Godot 4.7 or newer, for the level editor (Mods). Optional."
            bind:value={draft.godot}
            candidates={found?.godots}
            check={tool("godot")}
          />
        {/if}
      </details>
    </section>

    <div class="row end start">
      {#if !draft.root && found}
        <p class="dim note" id="start-why">Choose your OpenRAC folder to start.</p>
      {/if}
      <button
        class="primary"
        disabled={saving || !draft.root}
        aria-describedby={!draft.root && found ? "start-why" : undefined}
        onclick={() => void save(true)}>{saving ? "Starting…" : "Start"}</button
      >
    </div>
  {:else}
    <h1>Settings</h1>

    <!-- Android has no folders to choose: the app keeps its files in one place, which the Files app
         shows (FilesProvider.kt, "Your files" below), and its picker copies discs there. -->
    {#if !onAndroid}
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
              No copy yet? <button class="link" onclick={() => void api.openUrl(REPO_URL)}>Get OpenRAC</button>, then
              choose its folder here.
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
          hint="Everything OpenRAC keeps for you: your games set up from your discs (a few GB each), your saves and your settings."
          folder
          bind:value={draft.installDir}
          detail={draft.developer}
          check={installFolder}
        />
      </section>
    {:else}
      <section class="panel">
        <h2>Your files</h2>
        <div class="setting">
          <span class="text">
            <strong>In the Files app, under OpenRAC</strong>
            <small>
              Your saves, settings, games and disc images. Uninstalling asks whether to keep the app's data: keep it,
              and all of it is back when you install OpenRAC again.
            </small>
          </span>
          <button onclick={() => void guardShown(api.showFiles())}>Open Files</button>
        </div>
      </section>
      <GpuDrivers />
    {/if}

    <section class="panel">
      <h2>General</h2>
      <div class="setting theme">
        <span class="text">
          <strong id="theme-label">Theme</strong>
          <small>The launcher's colours. Its accent is the game's.</small>
        </span>
        <div class="themes" role="radiogroup" aria-labelledby="theme-label" use:tablist>
          {#each PALETTES as p (p.id)}
            <button
              role="radio"
              aria-checked={draft.theme === p.id}
              tabindex={draft.theme === p.id ? 0 : -1}
              class:on={draft.theme === p.id}
              onclick={() => (draft.theme = p.id)}
              ><span
                class="swatch"
                style={`--s-ink: ${p.swatch[0]}; --s-panel: ${p.swatch[1]}; --s-text: ${p.swatch[2]}`}
                aria-hidden="true"><i></i></span
              >{p.name}</button
            >
          {/each}
        </div>
      </div>
      <!-- Android has neither: its games are tiles, and no Discord app on the device reads a
           desktop's rich presence. -->
      {#if !onAndroid}
        <label class="setting">
          <span class="text">
            <strong>Compact layout</strong>
            <small>A game's name and its Play button side by side.</small>
          </span>
          <input
            type="checkbox"
            role="switch"
            class="switch"
            checked={draft.layout !== "classic"}
            onchange={(e) => (draft.layout = e.currentTarget.checked ? "compact" : "classic")}
          />
        </label>
        <label class="setting">
          <span class="text">
            <strong>Show in Discord</strong>
            <small>Your status says when you are playing.</small>
          </span>
          <input type="checkbox" role="switch" class="switch" bind:checked={draft.discordRpc} />
        </label>
      {/if}
      <!-- Android too: its script mods load there (docs/port/MODS.md), native ones do not. -->
      <label class="setting">
        <span class="text">
          <strong>{PURIST.title}</strong>
          <small>{PURIST.settingsLine}</small>
        </span>
        <input type="checkbox" role="switch" class="switch" bind:checked={draft.purist} />
      </label>
    </section>

    <section class="panel">
      <h2>Updates</h2>
      <label class="setting">
        <span class="text">
          <strong>Look for updates at start</strong>
          <small>Nothing is installed without your click.</small>
        </span>
        <input type="checkbox" role="switch" class="switch" bind:checked={draft.checkUpdates} />
      </label>
      {#if app.info?.updateChannels}
        <div class="setting">
          <span class="text">
            <strong>Channel</strong>
            <small>
              {#if channel === "dev"}
                The newest build of <span class="mono">{devBranch}</span>, tried less.
              {:else}
                The released versions.
              {/if}
            </small>
          </span>
          <div class="segmented" role="radiogroup" aria-label="Update channel">
            <button role="radio" aria-checked={channel === "stable"} onclick={() => (draft.updateChannel = "stable")}
              >Stable</button
            >
            <button role="radio" aria-checked={channel === "dev"} onclick={() => (draft.updateChannel = "dev")}
              >Development</button
            >
          </div>
        </div>
        <!-- A fork's launcher may follow any branch of the fork that CI builds (launcher.yml publishes
             each one's rolling launcher-dev-<branch> release); OpenRAC's own follows master. -->
        {#if channel === "dev" && app.info.updateChannels.dev.fork}
          <div class="field">
            <label for="update-branch">Branch</label>
            <input
              id="update-branch"
              type="text"
              class="mono"
              list="update-branches"
              autocomplete="off"
              spellcheck="false"
              placeholder={`Empty: ${builtBranch}`}
              bind:value={draft.updateBranch}
            />
            <datalist id="update-branches">
              {#each branches as branch (branch)}<option value={branch}></option>{/each}
            </datalist>
            <p class="dim note">
              The development builds of this branch of <span class="mono">{app.info.updateChannels.dev.repository}</span
              >. A build installs only when it is newer than the one you have.
            </p>
          </div>
        {/if}
      {/if}
      <p class="dim note status">
        <button class="link" disabled={looked === "looking"} onclick={() => void checkNow()}>Check now</button>
        {#if looked === "looking"}· Looking…{:else if looked === "found" || app.update}· {app.update?.name ??
            "A new version"} is out.
          <button class="link" onclick={() => (app.showUpdateModal = true)}>Update</button>{:else if looked === "none"}·
          You have the latest ({app.info?.version ?? ""}).{:else if looked === "failed"}· Could not check: {lookError}.{/if}
        {#if source}
          · <button class="link" onclick={() => void api.openUrl(source.page)}>Releases</button>
          {#if draft.developer}
            · <span class="mono">github.com/{source.repository} · {source.channel ?? "latest release"}</span>
          {/if}
        {/if}
      </p>
      {#if channel === "stable" && app.info?.updates.channel}
        <p class="dim note">This development build stays until the next release replaces it.</p>
      {/if}
    </section>

    {#if !app.info?.standalone && !onAndroid}
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
        {#if !draft.developer && !app.config?.python}
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

    <section class="panel">
      <details class="more" bind:open={advancedOpen}>
        <summary><h2>Advanced</h2></summary>
        <label class="setting">
          <span class="text">
            <strong>Detailed game logs</strong>
            <small>Turn on when we ask you to. Logs and crash reports are under Help, on each game's page.</small>
          </span>
          <input type="checkbox" role="switch" class="switch" bind:checked={draft.detailedGameLog} />
        </label>
        {#if desktopRenderer}
          <!-- What the game draws with (OPENRAC_GL_API, core/src/actions.rs); without Vulkan it
               falls back to OpenGL by itself (port/platform/window.cpp). -->
          <div class="setting">
            <span class="text">
              <strong>Renderer</strong>
              <small>
                {draft.renderer === "vulkan"
                  ? "Experimental: needs Vulkan 1.3, else the game draws with OpenGL."
                  : "OpenGL 4.1, the default."}
                From the game's next start: a running game keeps its renderer.
              </small>
            </span>
            <div class="segmented" role="radiogroup" aria-label="Renderer">
              <button
                role="radio"
                aria-checked={draft.renderer !== "vulkan"}
                onclick={() => (draft.renderer = "opengl")}>OpenGL</button
              >
              <button
                role="radio"
                aria-checked={draft.renderer === "vulkan"}
                onclick={() => (draft.renderer = "vulkan")}>Vulkan</button
              >
            </div>
          </div>
        {/if}
        <label class="setting">
          <span class="text">
            <strong>Advanced save editing</strong>
            <small>Every field of a save, raw bytes too. Your memory card is backed up first.</small>
          </span>
          <input type="checkbox" role="switch" class="switch" bind:checked={draft.saveEditorAdvanced} />
        </label>
        <label class="setting">
          <span class="text">
            <strong>Developer tools</strong>
            <small>The Develop tab (builds, checks, decompilation, the console), disc serials and file names.</small>
          </span>
          <input type="checkbox" role="switch" class="switch" bind:checked={draft.developer} />
        </label>
        {#if draft.developer}
          <!-- What developer tools add: under the switch, set in from it. -->
          <div class="dev">
            {#if !rootHere && !onAndroid}
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
            {#if !onAndroid}
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
        {#if draft.developer && draft.discordRpc && !onAndroid}
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
      </details>
    </section>

    <p class="dim about">
      OpenRAC Launcher {app.info?.version ?? ""}
      <!-- The header's links, which Android's navigation has no room for. -->
      {#if onAndroid}
        · <button class="link" onclick={() => void api.openUrl(SITE_URL)}>openrac.dev</button>
        · <button class="link" onclick={() => void api.openUrl(DISCORD_URL)}>Discord</button>
      {/if}
    </p>
  {/if}
</div>

{#if !setup && changed && !onAndroid}
  <div class="save-bar" role="status">
    <span class="grow">You have unsaved changes.</span>
    <button disabled={saving} onclick={() => (draft = saved())}>Discard</button>
    <button class="primary" disabled={saving} onclick={() => void save(false)}>Save</button>
  </div>
{/if}

<style>
  .page {
    max-width: var(--page-max, 820px);
    margin: 0 auto;
    /* Room at the bottom for the bar that saves. */
    padding: var(--page-top, 28px) var(--page-x, 28px) calc(var(--page-bottom, 54px) + 56px);
    display: flex;
    flex-direction: column;
    gap: var(--page-gap, 18px);
  }

  .welcome {
    display: flex;
    gap: 20px;
    align-items: center;
    padding: 24px;
    border-radius: 30px;
    background:
      radial-gradient(80% 90% at 80% 0, color-mix(in srgb, var(--brand) 28%, transparent), transparent 70%),
      var(--panel);
    border: 1px solid var(--line);
  }

  /* The logo's mark in the accent, beside the welcome that names it. */
  .welcome :global(.logo) {
    color: var(--amber);
  }

  .welcome h1 {
    font-size: 1.75rem;
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
    font-size: 0.8125rem;
  }

  /* The themes, each a small picture of its page (a panel and text on it) and its name: in a row
     under the setting's name, as five do not fit beside a line of text. */
  .setting.theme {
    flex-wrap: wrap;
    row-gap: 10px;
  }

  .setting.theme > .text {
    flex-basis: 100%;
  }

  .themes {
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
  }

  .themes button {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    padding: 6px 12px 6px 6px;
    border-radius: 999px;
    border: 1px solid var(--line-strong);
    background: var(--panel);
    color: var(--soft);
    font-size: 0.8125rem;
    font-weight: 600;
    transition:
      border-color var(--fast) var(--ease),
      color var(--fast) var(--ease);
  }

  .themes button.on {
    border-color: var(--amber);
    color: var(--text);
    box-shadow: 0 0 0 1px var(--amber);
  }

  .swatch {
    position: relative;
    width: 26px;
    height: 26px;
    border-radius: 50%;
    background: linear-gradient(135deg, var(--s-ink) 50%, var(--s-panel) 50%);
    box-shadow: inset 0 0 0 1px rgb(255 255 255 / 0.18);
  }

  .swatch i {
    position: absolute;
    inset: 9px;
    border-radius: 50%;
    background: var(--s-text);
  }

  /* "Check now", what it found and where it looks, under the update settings. */
  .status {
    padding-top: 10px;
    border-top: 1px solid var(--line);
  }

  .status + .note {
    margin-top: 4px;
  }

  .dev {
    display: flex;
    flex-direction: column;
    margin: 4px 0 6px;
    padding-left: 16px;
    border-left: 1px solid var(--line);
  }

  .facts {
    display: grid;
    grid-template-columns: max-content minmax(0, 1fr);
    gap: 4px 14px;
    margin: 12px 0 4px;
    font-size: 0.8125rem;
  }

  .facts dt {
    color: var(--dim);
  }

  .facts dd {
    margin: 0;
    overflow-wrap: anywhere;
  }

  details.more > summary h2 {
    margin: 0;
  }

  details.more[open] > summary {
    margin-bottom: 6px;
  }

  .field {
    display: flex;
    flex-direction: column;
    gap: 6px;
    padding: 12px 0 2px;
    border-top: 1px solid var(--line);
  }

  .field label {
    font-size: 0.8125rem;
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
    font-size: 0.8125rem;
  }

  .end {
    justify-content: flex-end;
  }

  /* First run: why Start waits, beside it. */
  .start {
    align-items: center;
    gap: 14px;
  }

  .start p {
    margin: 0;
  }

  /* First run: what was found, in a line beside its heading. */
  details.more > summary .note {
    min-width: 0;
  }

  .about {
    font-size: 0.75rem;
  }

  /* Stays in view while there is something to save. */
  .save-bar {
    position: fixed;
    /* The middle of the page, beside or above Android's navigation (app.css). */
    left: calc(50% + var(--nav-left) / 2);
    bottom: calc(22px + var(--nav-bottom));
    transform: translateX(-50%);
    z-index: 25;
    width: min(764px, calc(100% - var(--nav-left) - 2 * var(--page-x, 28px)));
    display: flex;
    align-items: center;
    gap: 10px;
    padding: 12px 14px 12px 20px;
    background: var(--panel-hi);
    border: 1px solid var(--line-strong);
    border-radius: 999px;
    box-shadow: var(--shadow);
    font-size: 0.875rem;
    animation: bar-in var(--medium) var(--ease) both;
  }

  @keyframes bar-in {
    from {
      opacity: 0;
      transform: translate(-50%, 24px);
    }
  }
</style>
