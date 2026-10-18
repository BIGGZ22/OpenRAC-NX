<script lang="ts">
  // Android: the GPU driver the game loads (docs/port/ANDROID.md, "GPU drivers"). The system's,
  // or a package the player adds: the zip Android emulators take (Mesa's Turnip, Qualcomm's own
  // drivers), with a meta.json. Packages load on Adreno only (libadrenotools); on Mali the
  // system's driver is the one there is.
  import { api, type GpuDrivers, type Renderer } from "$lib/api";
  import { app, gameWindow, guard, readGame, toast } from "$lib/app.svelte";

  let info = $state<GpuDrivers | null>(null);
  let busy = $state(false);
  // The radios show the choice, then what is saved (load): one that failed to save goes back
  // ("" is the system's driver).
  let rendererPick = $state<Renderer>("gles");
  let driverPick = $state("");

  async function load() {
    info = (await guard(api.gpuDrivers())) ?? null;
    rendererPick = info?.renderer === "vulkan" ? "vulkan" : "gles";
    driverPick = info?.selected ?? "";
  }

  $effect(() => {
    void load();
  });

  async function select(id: string | null) {
    driverPick = id ?? "";
    busy = true;
    await guard(api.gpuDriverSelect(id));
    await load();
    busy = false;
  }

  async function renderer(choice: Renderer) {
    rendererPick = choice;
    busy = true;
    const saved = await guard(api.gpuRendererSelect(choice).then(() => true));
    await load();
    await readGame();
    busy = false;
    if (!saved) return;
    // A running game keeps what it started with: "Return to game" brings that one back.
    const name = choice === "vulkan" ? "Vulkan" : "OpenGL ES";
    toast(
      app.game?.running
        ? `Saved. The game that is running keeps its renderer: quit it to start it with ${name}.`
        : `Saved. The game starts with ${name}.`,
    );
  }

  async function quitGame() {
    if (!confirm("Quit the game? What is not saved in it is lost.")) return;
    await gameWindow(true);
  }

  async function add() {
    busy = true;
    const driver = await guard(api.gpuDriverImport());
    if (driver) toast(`Added ${driver.name} ${driver.version}.`);
    await load();
    busy = false;
  }

  async function remove(id: string, name: string) {
    if (!confirm(`Remove ${name}?`)) return;
    busy = true;
    await guard(api.gpuDriverRemove(id));
    await load();
    busy = false;
  }

  const adreno = $derived(info?.device.gpu === "adreno");
</script>

<section class="panel">
  <h2>Renderer</h2>
  <!-- What the game draws with from its next start (docs/port/ANDROID.md, "A Vulkan renderer"). -->
  <label class="option">
    <input
      type="radio"
      name="renderer"
      checked={rendererPick === "gles"}
      disabled={busy || !info}
      onchange={() => void renderer("gles")}
    />
    <span><strong>OpenGL ES 3.2</strong><small>Default</small></span>
  </label>
  <label class="option">
    <input
      type="radio"
      name="renderer"
      checked={rendererPick === "vulkan"}
      disabled={busy || !info}
      onchange={() => void renderer("vulkan")}
    />
    <span>
      <strong>Vulkan</strong>
      <small>Experimental. Needs Vulkan 1.3; the game falls back to OpenGL ES without it.</small>
    </span>
  </label>
  <p class="dim note">
    Used from the game's next start; its menu (the Back button) says what it draws with now.
    {#if app.game?.running}
      The game is running with what it started with.
      <button class="link" onclick={() => void quitGame()}>Quit the game</button>
    {/if}
  </p>
</section>

<section class="panel">
  <h2>GPU driver</h2>
  {#if !info}
    <p class="dim">Looking…</p>
  {:else}
    <p class="dim device">
      {[
        info.device.model,
        info.device.soc,
        info.device.gpu === "other" ? "" : info.device.gpu === "adreno" ? "Adreno" : "Mali",
      ]
        .filter(Boolean)
        .join(" · ")}
    </p>
    <label class="option">
      <input
        type="radio"
        name="gpu-driver"
        checked={driverPick === ""}
        disabled={busy}
        onchange={() => void select(null)}
      />
      <span><strong>System driver</strong><small>Default</small></span>
    </label>
    {#each info.drivers as driver (driver.id)}
      <div class="option">
        <input
          id="gpu-{driver.id}"
          type="radio"
          name="gpu-driver"
          checked={driverPick === driver.id}
          disabled={busy || !adreno}
          onchange={() => void select(driver.id)}
        />
        <label for="gpu-{driver.id}" class="grow">
          <strong>{driver.name} {driver.version}</strong>
          <small>{[driver.vendor, driver.driverVersion].filter(Boolean).join(" · ")}</small>
        </label>
        <button class="link" disabled={busy} onclick={() => void remove(driver.id, driver.name)}>Remove</button>
      </div>
    {/each}
    {#if adreno}
      <p class="dim note">Custom drivers are used by the Vulkan renderer.</p>
      <p><button disabled={busy} onclick={() => void add()}>Add driver (.zip)</button></p>
    {:else}
      <p class="dim note">Custom drivers require an Adreno GPU.</p>
    {/if}
  {/if}
</section>

<style>
  .option {
    display: flex;
    align-items: flex-start;
    gap: 14px;
    padding: 12px 0;
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

  .grow {
    flex: 1;
  }

  .link {
    border: 0;
    background: none;
    padding: 0;
    color: var(--amber);
    font-size: 13px;
  }
</style>
