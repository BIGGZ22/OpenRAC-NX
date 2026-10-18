<script lang="ts">
  // Activity: what the launcher did for the player's games, in words. Each job is a row: what it
  // does or did, how long it took, Stop while it runs; what it printed folds under "Show output",
  // to copy and send us when something failed. With developer tools on this is Develop's console.
  import DevConsole from "$components/DevConsole.svelte";
  import Icon from "$components/Icon.svelte";
  import { app, cancel, clearFinished, clock, jobs } from "$lib/app.svelte";
  import { elapsed } from "$lib/format";
  import { isGame, phaseLabel, readProgress } from "$lib/progress";
  import type { Job } from "$lib/queue";

  const developer = $derived(app.config?.developer ?? false);
  const list = $derived([...jobs.list].reverse());
  const finished = $derived(jobs.list.some((j) => j.state !== "queued" && j.state !== "running"));
  const live = (j: Job) => j.state === "queued" || j.state === "running";
  /** Which jobs' output is unfolded; a failed one opens by itself. */
  const opened = $state<Record<number, boolean>>({});
  /** The most lines of output written out on the page. */
  const SHOWN = 400;

  function duration(job: Job): string {
    if (!job.startedAt) return "";
    return elapsed((job.endedAt ?? clock.now) - job.startedAt);
  }

  /** What a finished job did, in the player's words. */
  function did(job: Job): string {
    if (job.actionId.startsWith("install:")) return "Set up from your disc.";
    switch (job.actionId) {
      case "port-build":
        return "Built the game for this computer.";
      case "port-levels":
        return "Read the levels from your disc.";
      case "port-levels-check":
        return "Checked the levels: the game can draw every one.";
      case "play":
      case "play-edited":
        return "Played.";
      default:
        return "Done.";
    }
  }

  /** One line under the job's name: the step it is on, or how it ended. */
  function line(job: Job): string {
    if (live(job)) {
      const progress = readProgress(job.actionId, job.lines);
      const how =
        progress.percent !== null ? ` · ${Math.round(progress.percent)} %` : progress.note ? ` · ${progress.note}` : "";
      return `${phaseLabel(job, clock.now)}${how}`;
    }
    if (job.state === "succeeded") return did(job);
    if (job.state === "cancelled") return "Stopped before it finished.";
    if (isGame(job)) return "The game stopped unexpectedly. Its page has a report you can send us.";
    return "It did not finish. Copy what it printed and send it to us on GitHub or Discord.";
  }

  async function copy(job: Job) {
    await navigator.clipboard.writeText([`$ ${job.command}`, ...job.lines.map((l) => l.line)].join("\n"));
  }
</script>

{#if developer}
  <div class="page console"><DevConsole /></div>
{:else}
  <div class="page">
    <div class="row">
      <h1 class="grow">Activity</h1>
      {#if finished}<button class="ghost small" onclick={clearFinished}>Clear finished</button>{/if}
    </div>
    <p class="muted lead">What the launcher did for your games: setting them up, building them, playing them.</p>

    {#if !jobs.list.length}
      <section class="panel empty">
        <p class="muted">Nothing has run yet. When the launcher sets a game up or builds it, what it did shows here.</p>
      </section>
    {/if}

    <ul>
      {#each list as job (job.key)}
        {@const progress = live(job) && !isGame(job) ? readProgress(job.actionId, job.lines) : null}
        <li class="panel job" class:failed={job.state === "failed"}>
          <div class="row">
            <span
              class="mark"
              class:ok={job.state === "succeeded"}
              class:err={job.state === "failed"}
              class:warn={job.state === "running"}
            >
              {#if job.state === "running"}<span class="spinner"></span>
              {:else if job.state === "succeeded"}<Icon name="check" size={16} />
              {:else if job.state === "failed"}<Icon name="warn" size={16} />
              {:else if job.state === "cancelled"}<Icon name="x" size={16} />
              {:else}<span class="dot"></span>{/if}
            </span>
            <div class="grow">
              <strong class="clip">{job.title}</strong>
              <p class="line">{line(job)}</p>
            </div>
            <span class="dim time">{duration(job)}</span>
            {#if live(job)}
              <button class="small" onclick={() => void cancel(job)}>
                <Icon name="x" size={14} />{job.state === "queued" ? "Don't run" : "Stop"}
              </button>
            {/if}
          </div>
          {#if progress && job.state === "running"}
            <div class="track" role="progressbar" aria-label={job.title} aria-valuenow={progress.percent ?? undefined}>
              {#if progress.percent !== null}<i style:width={`${progress.percent}%`}></i>{:else}<i class="moving"
                ></i>{/if}
            </div>
          {/if}
          {#if job.lines.length || job.command}
            <details
              open={opened[job.key] ?? job.state === "failed"}
              ontoggle={(e) => (opened[job.key] = e.currentTarget.open)}
            >
              <summary>Show output</summary>
              <div class="row tools">
                <span class="dim mono clip grow" title={job.command}>{job.command}</span>
                <button class="small" onclick={() => void copy(job)}>Copy</button>
              </div>
              {#if opened[job.key] ?? job.state === "failed"}
                <!-- Only while open, and the last lines only: a long build prints thousands. -->
                {#if job.lines.length > SHOWN}<p class="dim cut">
                    The first {job.lines.length - SHOWN} lines are left out here; Copy takes them all.
                  </p>{/if}
                <pre>{job.lines
                    .slice(-SHOWN)
                    .map((l) => l.line)
                    .join("\n") || "Nothing has been printed yet."}</pre>
              {/if}
            </details>
          {/if}
        </li>
      {/each}
    </ul>
  </div>
{/if}

<style>
  .cut {
    margin: 6px 0 0;
    font-size: 12px;
  }

  .page {
    max-width: 900px;
    margin: 0 auto;
    padding: 28px 28px 60px;
    display: flex;
    flex-direction: column;
    gap: 14px;
  }

  .page.console {
    max-width: 1400px;
    height: 100%;
    padding: 22px 28px;
  }

  h1 {
    margin: 0;
  }

  .lead {
    margin: -6px 0 6px;
  }

  ul {
    margin: 0;
    padding: 0;
    list-style: none;
    display: flex;
    flex-direction: column;
    gap: 10px;
  }

  .job {
    padding: 14px 18px;
    display: flex;
    flex-direction: column;
    gap: 10px;
  }

  .job.failed {
    border-color: color-mix(in srgb, var(--err) 45%, transparent);
  }

  .mark {
    width: 28px;
    height: 28px;
    flex: none;
    display: grid;
    place-items: center;
    border-radius: 50%;
    border: 1px solid var(--line-strong);
    color: var(--soft);
  }

  .mark .dot {
    width: 7px;
    height: 7px;
    border-radius: 50%;
    background: currentColor;
  }

  .ok {
    color: var(--ok);
  }

  .err {
    color: var(--err);
  }

  .warn {
    color: var(--warn);
  }

  strong {
    display: block;
    color: var(--text);
  }

  .line {
    margin: 0;
    font-size: 13px;
    color: var(--soft);
  }

  .time {
    font-size: 13px;
    font-variant-numeric: tabular-nums;
    white-space: nowrap;
  }

  .track {
    height: 4px;
    border-radius: 999px;
    background: var(--ink-deep);
    overflow: hidden;
  }

  .track i {
    display: block;
    height: 100%;
    border-radius: 999px;
    background: var(--amber);
    transition: width 0.4s;
  }

  .track i.moving {
    width: 30%;
    animation: slide 1.4s ease-in-out infinite;
  }

  @keyframes slide {
    from {
      transform: translateX(-100%);
    }
    to {
      transform: translateX(340%);
    }
  }

  summary {
    cursor: pointer;
    font-size: 13px;
    font-weight: 600;
    color: var(--lav);
  }

  .tools {
    margin: 8px 0;
  }

  .tools .mono {
    font-size: 12px;
  }

  pre {
    margin: 0;
    max-height: 280px;
    overflow: auto;
    background: var(--ink-deep);
    border: 1px solid var(--line);
    border-radius: var(--radius-sm);
    padding: 10px 12px;
    font-family: var(--font-mono);
    font-size: 12px;
    line-height: 1.5;
    color: var(--body);
    white-space: pre-wrap;
    word-break: break-word;
  }
</style>
