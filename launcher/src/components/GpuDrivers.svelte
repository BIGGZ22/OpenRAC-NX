<script lang="ts">
  // Android: the GPU driver the game loads (docs/port/ANDROID.md, "GPU drivers"). The system's,
  // or a package the player adds: the zip Android emulators take (Mesa's Turnip, Qualcomm's own
  // drivers), with a meta.json. Packages load on Adreno only (libadrenotools); on Mali the
  // system's driver is the one there is.
  import { api, type GpuDrivers } from "$lib/api";
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

  async function add() {
    busy = true;
    const driver = await guard(api.gpuDriverImport());
    if (driver) toast(`${driver.name} ${driver.version} is added. Choose it to use it.`);
    await load();
    busy = false;
  }

  async function remove(id: string, name: string) {
    if (!confirm(`Remove the driver package ${name}?`)) return;
    busy = true;
    await guard(api.gpuDriverRemove(id));
    await load();
    busy = false;
  }

  const adreno = $derived(info?.device.gpu === "adreno");
</script>

<section class="panel">
  <h2>GPU driver</h2>
  {#if !info}
    <p class="dim">Looking…</p>
  {:else}
    <p class="dim note">
      {info.device.model}{info.device.soc ? `, ${info.device.soc}` : ""}, Android API {info.device.api}:
      {#if adreno}
        an Adreno GPU, which can use a driver package of your own (a Vulkan driver such as Mesa's Turnip).
      {:else if info.device.gpu === "mali"}
        a Mali GPU. Driver packages are for Adreno GPUs: the game uses the system's driver here.
      {:else}
        driver packages are for Adreno GPUs: the game uses the system's driver here.
      {/if}
    </p>
    <label class="option">
      <input
        type="radio"
        name="gpu-driver"
        checked={info.selected === null}
        disabled={busy}
        onchange={() => void select(null)}
      />
      <span>
        <strong>The system's driver</strong>
        <small>What the device came with, updated with the system.</small>
      </span>
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
          <small>
            {[driver.vendor, driver.driverVersion, driver.author].filter(Boolean).join(" · ")}
            {#if driver.description}<br />{driver.description}{/if}
            <br /><span class="mono">{driver.library}</span>
          </small>
        </label>
        <button class="link" disabled={busy} onclick={() => void remove(driver.id, driver.name)}>Remove</button>
      </div>
    {/each}
    <p class="dim note">
      The game draws with OpenGL ES for now: it loads the package and names it in its log, and the Vulkan renderer that
      draws on it comes next. If a package keeps the game from starting, choose the system's driver again.
    </p>
    <p>
      <button disabled={busy} onclick={() => void add()}>Add a driver package (.zip)</button>
    </p>
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
