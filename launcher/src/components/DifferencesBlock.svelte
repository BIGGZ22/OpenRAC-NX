<script lang="ts">
  // A game's differences from the original: what its PC port does otherwise than the PS2 game,
  // from port/game/<game>/differences.json (core/src/differences.rs). Grouped: the game fixes,
  // each with a switch bound to the port's own setting (the same one the game's Options menu
  // changes), then the guards that keep it from crashing, then what comes with being a PC program.
  // Each entry folds open to say what the original does and what the port does, and why. Where
  // in the code it is, and the settings file, are a contributor's details: with developer tools on.
  // `foldable` (Android): closed under its title and a line of what it holds, until opened.
  import FoldHead from "./FoldHead.svelte";
  import { api, type Differences, type DifferenceView, type VersionView } from "$lib/api";
  import { app, toast } from "$lib/app.svelte";
  import { differencesSummary, groupDifferences, splitVersion } from "$lib/differences";
  import { enter, fold } from "$lib/motion";
  import { isPlaying } from "$lib/player.svelte";
  import { fly, slide } from "svelte/transition";

  let { version, foldable = false }: { version: VersionView; foldable?: boolean } = $props();
  let unfolded = $state(false);

  const developer = $derived(app.config?.developer ?? false);
  const playing = $derived(isPlaying(version));

  let data = $state<Differences | null>(null);
  /** The entries folded open, by id. */
  let open = $state<Record<string, boolean>>({});
  /** The fix being written, by id. */
  let saving = $state<string | null>(null);

  /** The newest read wins: switching games while one is read drops the older answer. */
  let reading = 0;
  $effect(() => {
    const [game, disc] = splitVersion(version.key);
    const mine = ++reading;
    data = null;
    open = {};
    void api
      .differences(game, disc)
      .then((d) => {
        if (mine === reading) data = d;
      })
      .catch(() => undefined);
  });

  const groups = $derived(groupDifferences(data?.entries ?? []));

  async function setFix(entry: DifferenceView, on: boolean) {
    if (!data || playing) return; // the running game would save over it
    saving = entry.id;
    try {
      data = await api.setGameFix(data.game, data.version, entry.id, on);
    } catch (e) {
      toast(`${entry.name} could not be changed: ${String(e)}`, "error");
    } finally {
      saving = null;
    }
  }
</script>

{#if data && groups.length}
  <section class="panel differences" aria-labelledby="differences-title" in:fly|global={enter()}>
    {#if foldable}
      <FoldHead
        title="Differences from the original"
        id="differences-title"
        controls="differences-list"
        line={differencesSummary(data.entries)}
        bind:open={unfolded}
      />
    {:else}
      <div class="row top">
        <h2 id="differences-title" class="grow">Differences from the original</h2>
        {#if developer}<span class="chip mono" title="The game's settings file">{data.settingsFile}</span>{/if}
      </div>
    {/if}
    {#if !foldable || unfolded}
      <div id="differences-list" class="list" transition:slide={fold}>
        {#if foldable && developer}<span class="chip mono" title="The game's settings file">{data.settingsFile}</span
          >{/if}
        <p class="muted">What this PC version does otherwise than the PlayStation 2 game, and why.</p>

        {#each groups as group (group.kind)}
          <div class="group">
            <h3>{group.title}</h3>
            <p class="about">{group.about}</p>
            {#if group.kind === "fix" && playing}
              <p class="about warn">
                The game is running, so these are locked here: the game would save over a change made now. Change them
                in the game under Options, Game fixes, or here once it has closed.
              </p>
            {/if}
            <ul>
              {#each group.entries as entry (entry.id)}
                {@const id = `difference-${entry.id}`}
                <li>
                  <div class="head">
                    <button
                      class="fold grow"
                      class:open={open[entry.id]}
                      aria-expanded={!!open[entry.id]}
                      aria-controls={id}
                      onclick={() => (open[entry.id] = !open[entry.id])}>{entry.name}</button
                    >
                    {#if entry.kind === "fix"}
                      <span class="state" class:original={entry.on === false}>{entry.on ? "On" : "Original"}</span>
                      <input
                        type="checkbox"
                        class="switch"
                        role="switch"
                        aria-label={`${entry.name}: on is the port's fix, off plays as the original`}
                        checked={entry.on ?? true}
                        disabled={playing || saving === entry.id}
                        onchange={(ev) => void setFix(entry, ev.currentTarget.checked)}
                      />
                    {:else if entry.kind === "guard"}
                      <span class="state">Always on</span>
                    {/if}
                  </div>
                  {#if open[entry.id]}
                    <div {id} class="explain" transition:slide={fold}>
                      <p><strong>On a PlayStation 2</strong>{entry.original}</p>
                      <p><strong>In OpenRAC</strong>{entry.port}</p>
                      {#if developer && entry.refs.length}
                        <p class="refs mono">
                          {#if entry.setting}{entry.setting.key} ·
                          {/if}{entry.refs.join(" · ")}
                        </p>
                      {/if}
                    </div>
                  {/if}
                </li>
              {/each}
            </ul>
          </div>
        {/each}
      </div>
    {/if}
  </section>
{/if}

<style>
  .differences {
    position: relative;
    z-index: 1;
    display: flex;
    flex-direction: column;
    gap: 12px;
  }

  .top {
    flex-wrap: wrap;
  }

  .list {
    display: flex;
    flex-direction: column;
    gap: 12px;
  }

  /* A contributor's detail: a small mono chip. */
  .chip {
    max-width: 100%;
    overflow: hidden;
    text-overflow: ellipsis;
    font-size: 0.75rem;
    color: var(--soft);
    background: var(--panel-hi);
    border: 1px solid var(--line-strong);
    border-radius: 7px;
    padding: 2px 8px;
    white-space: nowrap;
  }

  .group {
    display: flex;
    flex-direction: column;
    gap: 4px;
    padding-top: 12px;
    border-top: 1px solid var(--line);
  }

  .about {
    font-size: 0.8125rem;
    color: var(--soft);
    max-width: 68ch;
  }

  .about.warn {
    color: var(--amber);
  }

  ul {
    margin: 4px 0 0;
    padding: 0;
    list-style: none;
  }

  /* A row an entry: its name, which folds the explanation open, and a fix's switch. */
  li + li {
    border-top: 1px solid var(--line);
  }

  .head {
    display: flex;
    align-items: center;
    gap: 12px;
    min-height: 48px;
  }

  .fold {
    justify-content: flex-start;
    gap: 8px;
    padding: 8px 0;
    border: 0;
    background: none;
    color: var(--text);
    font-weight: 600;
    text-align: left;
  }

  .fold::before {
    content: "";
    flex: none;
    border-left: 5px solid var(--lav);
    border-top: 4px solid transparent;
    border-bottom: 4px solid transparent;
    transition: transform var(--fast) var(--ease);
  }

  .fold.open::before {
    transform: rotate(90deg);
  }

  .fold:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
  }

  .state {
    flex: none;
    font-size: 0.8125rem;
    color: var(--soft);
  }

  .state.original {
    color: var(--amber);
  }

  .explain {
    display: flex;
    flex-direction: column;
    gap: 6px;
    padding: 0 0 12px 13px;
    font-size: 0.875rem;
    color: var(--body);
    max-width: 72ch;
  }

  .explain strong {
    display: block;
    font-size: 0.75rem;
    font-weight: 600;
    letter-spacing: 0.02em;
    color: var(--lav);
  }

  .refs {
    color: var(--dim);
    overflow-wrap: anywhere;
  }
</style>
