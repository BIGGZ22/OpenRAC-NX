<script lang="ts">
  // Android: the GPU driver the game loads (docs/port/ANDROID.md, "GPU drivers"). The system's,
  // or a package the player adds: the zip Android emulators take (Mesa's Turnip, Qualcomm's own
  // drivers), with a meta.json. Packages load on Adreno only (libadrenotools); on Mali the
  // system's driver is the one there is.
  import { api, type GpuDrivers, type Renderer } from "$lib/api";
  import { guard, toast } from "$lib/app.svelte";

  let info = $state<GpuDrivers | null>(null);
  let busy = $state(false);

  async function load() {
    info = (await guard(api.gpuDrivers())) ?? null;
  }

  $effect(() => {
    void load();
  });

  async function select(id: string | null) {
    busy = true;
    await guard(api.gpuDriverSelect(id));
    await load();
    busy = false;
  }

  async function renderer(choice: Renderer) {
    busy = true;
    await guard(api.gpuRendererSelect(choice));
    await load();
    busy = false;
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
      checked={info?.renderer !== "vulkan"}
      disabled={busy || !info}
      onchange={() => void renderer("gles")}
    />
    <span><strong>OpenGL ES 3.2</strong><small>Default</small></span>
  </label>
  <label class="option">
    <input
      type="radio"
      name="renderer"
      checked={info?.renderer === "vulkan"}
      disabled={busy || !info}
      onchange={() => void renderer("vulkan")}
    />
    <span>
      <strong>Vulkan</strong>
      <small>Experimental. Needs Vulkan 1.3; the game falls back to OpenGL ES without it.</small>
    </span>
  </label>
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
        checked={!info.selected}
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
          checked={info.selected === driver.id}
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
