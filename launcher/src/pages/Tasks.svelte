<script lang="ts">
  // The job queue: what ran, what runs, what waits, and each job's output.
  import Icon from "$components/Icon.svelte";
  import { app, cancel, clearFinished, clock, jobs } from "$lib/app.svelte";
  import { elapsed } from "$lib/format";
  import type { Job, JobState } from "$lib/queue";

  const STATES: Record<JobState, { text: string; tone: string }> = {
    queued: { text: "Queued", tone: "info" },
    running: { text: "Running", tone: "warn" },
    succeeded: { text: "Done", tone: "ok" },
    failed: { text: "Failed", tone: "err" },
    cancelled: { text: "Cancelled", tone: "info" },
  };

  const selected = $derived(jobs.list.find((j) => j.key === jobs.selected) ?? jobs.list.at(-1) ?? null);
  function duration(job: Job): string {
    if (!job.startedAt) return "";
    return elapsed((job.endedAt ?? clock.now) - job.startedAt);
  }

  let output = $state<HTMLElement | null>(null);
  let follow = $state(true);
  // Keep the newest line in view, unless the reader has scrolled up.
  $effect(() => {
    const lines = selected?.lines.length ?? 0;
    if (lines && follow && output) output.scrollTop = output.scrollHeight;
  });

  async function copy(job: Job) {
    await navigator.clipboard.writeText([`$ ${job.command}`, ...job.lines.map((l) => l.line)].join("\n"));
  }
</script>

<div class="page">
  <aside class="panel list">
    <div class="row">
      <h2 class="grow">{app.config?.developer ? "Tasks" : "Activity"}</h2>
      <button class="ghost small" onclick={clearFinished}>Clear finished</button>
    </div>
    {#if !jobs.list.length}
      <p class="muted">Nothing has run yet. When the launcher sets a game up or builds it, what it did shows here.</p>
    {/if}
    {#each [...jobs.list].reverse() as job (job.key)}
      <button class="job" class:on={selected?.key === job.key} onclick={() => (jobs.selected = job.key)}>
        <span class="row">
          {#if job.state === "running"}<span class="spinner"></span>{/if}
          <strong class="grow clip">{job.title}</strong>
        </span>
        <span class="row">
          <span class={`pill ${STATES[job.state].tone}`}><span class="dot"></span>{STATES[job.state].text}</span>
          <span class="dim small">{duration(job)}</span>
        </span>
      </button>
    {/each}
  </aside>

  <section class="panel out">
    {#if selected}
      <div class="row">
        <div class="grow">
          <h2 class="clip">{selected.title}</h2>
          <p class="mono dim clip" title={selected.command}>{selected.command || "not started"}</p>
        </div>
        <span class="dim time">{duration(selected)}</span>
        {#if selected.code !== null && app.config?.developer}<span class="dim">exit {selected.code}</span>{/if}
        <button class="small" onclick={() => void copy(selected)}>Copy</button>
        {#if selected.state === "running" || selected.state === "queued"}
          <button class="small" onclick={() => void cancel(selected)}><Icon name="x" size={14} />Cancel</button>
        {/if}
      </div>
      <pre
        bind:this={output}
        onscroll={(e) => {
          const el = e.currentTarget;
          follow = el.scrollTop + el.clientHeight >= el.scrollHeight - 8;
        }}>{#each selected.lines as l, i (i)}<span class:err={l.stream === "stderr"}
            >{l.line}
</span>{/each}</pre>
    {:else}
      <p class="muted">Choose an entry to see what it printed.</p>
    {/if}
  </section>
</div>

<style>
  .page {
    height: 100%;
    display: grid;
    grid-template-columns: minmax(220px, 320px) minmax(0, 1fr);
    gap: 18px;
    padding: 22px 28px;
    max-width: 1400px;
    margin: 0 auto;
  }

  .list {
    display: flex;
    flex-direction: column;
    gap: 8px;
    overflow: auto;
  }

  .job {
    flex-direction: column;
    align-items: stretch;
    gap: 6px;
    text-align: left;
    border-radius: var(--radius-sm);
    background: transparent;
    border-color: var(--line);
    padding: 10px 12px;
    font-weight: 400;
  }

  .job.on {
    background: var(--panel-hi);
    border-color: var(--brand);
  }

  .small {
    font-size: 12px;
  }

  .time {
    font-variant-numeric: tabular-nums;
    white-space: nowrap;
  }

  .out {
    display: flex;
    flex-direction: column;
    gap: 12px;
    min-height: 0;
  }

  pre {
    flex: 1;
    margin: 0;
    overflow: auto;
    background: var(--ink-deep);
    border: 1px solid var(--line);
    border-radius: var(--radius-sm);
    padding: 12px 14px;
    font-family: var(--font-mono);
    font-size: 12.5px;
    line-height: 1.5;
    color: #e3e9ff;
    white-space: pre-wrap;
    word-break: break-word;
  }

  /* Many tools log to stderr as a matter of course (unittest, make), so it is
     set apart, not alarming; a failure shows in the job's state. */
  .err {
    color: var(--gold);
  }
</style>
