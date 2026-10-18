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

<!-- One panel: what the game draws with, and on Vulkan the driver it loads. The drivers show only
     with Vulkan chosen, the one renderer that uses them. -->
<section class="panel">
  <h2>Graphics</h2>
  <!-- What the game draws with from its next start (docs/port/ANDROID.md, "A Vulkan renderer"). -->
  <div class="setting">
    <span class="text">
      <strong>Renderer</strong>
      <small>
        {rendererPick === "vulkan" ? "Experimental: needs Vulkan 1.3, else OpenGL ES." : "OpenGL ES 3.2, the default."}
        From the game's next start.
      </small>
    </span>
    <div class="segmented" role="radiogroup" aria-label="Renderer">
      <button
        role="radio"
        aria-checked={rendererPick === "gles"}
        disabled={busy || !info}
        onclick={() => void renderer("gles")}>OpenGL ES</button
      >
      <button
        role="radio"
        aria-checked={rendererPick === "vulkan"}
        disabled={busy || !info}
        onclick={() => void renderer("vulkan")}>Vulkan</button
      >
    </div>
  </div>
  {#if app.game?.running}
    <p class="dim note">
      The running game keeps what it started with.
      <button class="link" onclick={() => void quitGame()}>Quit the game</button>
    </p>
  {/if}

  {#if rendererPick === "vulkan"}
    <div class="drivers">
      <h3>GPU driver</h3>
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
        <label class="setting">
          <input
            type="radio"
            name="gpu-driver"
            checked={driverPick === ""}
            disabled={busy}
            onchange={() => void select(null)}
          />
          <span class="text"><strong>System driver</strong><small>Default</small></span>
        </label>
        {#each info.drivers as driver (driver.id)}
          <div class="setting">
            <input
              id="gpu-{driver.id}"
              type="radio"
              name="gpu-driver"
              checked={driverPick === driver.id}
              disabled={busy || !adreno}
              onchange={() => void select(driver.id)}
            />
            <label for="gpu-{driver.id}" class="text">
              <strong>{driver.name} {driver.version}</strong>
              <small>{[driver.vendor, driver.driverVersion].filter(Boolean).join(" · ")}</small>
            </label>
            <button class="link" disabled={busy} onclick={() => void remove(driver.id, driver.name)}>Remove</button>
          </div>
        {/each}
        {#if adreno}
          <p><button class="small" disabled={busy} onclick={() => void add()}>Add driver (.zip)</button></p>
        {:else}
          <p class="dim note">Custom drivers require an Adreno GPU.</p>
        {/if}
      {/if}
    </div>
  {/if}
</section>

<style>
  .setting input[type="radio"] {
    flex: none;
    width: 18px;
    height: 18px;
    margin: 0;
    accent-color: var(--amber);
  }

  .drivers {
    margin-top: 6px;
    padding-top: 12px;
    border-top: 1px solid var(--line);
  }

  .drivers h3 {
    margin: 0;
    font-size: 14px;
  }

  .device,
  .note {
    font-size: 13px;
  }

  .link {
    border: 0;
    background: none;
    padding: 0;
    color: var(--amber);
    font-size: 13px;
  }
</style>
