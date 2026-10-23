<script lang="ts">
  // Develop's console: what ran, what runs, what waits, and each job's output, as a contributor
  // reads it. The list filters by state; a finished check or build can be run again.
  import type { ActionView } from "$lib/api";
  import { app, cancel, clearFinished, clock, jobs, run, toast } from "$lib/app.svelte";
  import { elapsed } from "$lib/format";
  import type { Job, JobState } from "$lib/queue";
  import Icon from "./Icon.svelte";

  const STATES: Record<JobState, { text: string; tone: string }> = {
    queued: { text: "Queued", tone: "info" },
    running: { text: "Running", tone: "warn" },
    succeeded: { text: "Done", tone: "ok" },
    failed: { text: "Failed", tone: "err" },
    cancelled: { text: "Cancelled", tone: "info" },
  };

  type Filter = "all" | "running" | "failed" | "done";
  const FILTERS: { id: Filter; label: string; states: JobState[] }[] = [
    { id: "all", label: "All", states: ["queued", "running", "succeeded", "failed", "cancelled"] },
    { id: "running", label: "Running", states: ["queued", "running"] },
    { id: "failed", label: "Failed", states: ["failed"] },
    { id: "done", label: "Done", states: ["succeeded"] },
  ];
  let filter = $state<Filter>("all");
  const shown = $derived.by(() => {
    const states = FILTERS.find((f) => f.id === filter)?.states ?? [];
    return [...jobs.list].reverse().filter((j) => states.includes(j.state));
  });
  const count = (f: (typeof FILTERS)[number]) => jobs.list.filter((j) => f.states.includes(j.state)).length;

  const selected = $derived(jobs.list.find((j) => j.key === jobs.selected) ?? jobs.list.at(-1) ?? null);
  function duration(job: Job): string {
    if (!job.startedAt) return "";
    return elapsed((job.endedAt ?? clock.now) - job.startedAt);
  }

  /** Which version or the checkout a job was for, in a few words. */
  function where(job: Job): string {
    if (job.scope.kind === "repository") return job.actionId.startsWith("install:") ? "Set-up" : "Checkout";
    return job.scope.key;
  }

  /** The action behind a finished job, when running it again needs nothing more: a set-up, build
   *  or check (an edit or a play can need a level or a file, and set-ups from a disc an image). */
  function again(job: Job): ActionView | null {
    if (job.state === "queued" || job.state === "running") return null;
    const scope = job.scope;
    const list =
      scope.kind === "repository"
        ? app.library?.repository
        : app.library?.games.flatMap((g) => g.versions).find((v) => v.key === scope.key)?.actions;
    const action = list?.find((a) => a.id === job.actionId && a.thisPlatform) ?? null;
    return action && action.runnable && !action.detached && ["setup", "build", "check"].includes(action.kind)
      ? action
      : null;
  }

  let output = $state<HTMLElement | null>(null);
  let follow = $state(true);
  // Keep the newest line in view, unless the reader has scrolled up.
  $effect(() => {
    const lines = selected?.lines.length ?? 0;
    if (lines && follow && output) output.scrollTop = output.scrollHeight;
  });

  async function copy(job: Job) {
    try {
      await navigator.clipboard.writeText([`$ ${job.command}`, ...job.lines.map((l) => l.line)].join("\n"));
    } catch {
      toast("The output could not be copied: the browser would not give it to the clipboard.", "error");
    }
  }
</script>

<div class="console">
  <aside class="panel list">
    <div class="row">
      <h2 class="grow">Jobs</h2>
      <button class="ghost small" onclick={clearFinished}>Clear finished</button>
    </div>
    <div class="filters" role="radiogroup" aria-label="Show">
      {#each FILTERS as f (f.id)}
        <button role="radio" aria-checked={filter === f.id} class:on={filter === f.id} onclick={() => (filter = f.id)}
          >{f.label}{#if f.id !== "all" && count(f) > 0}<span class="n">{count(f)}</span>{/if}</button
        >
      {/each}
    </div>
    {#if !jobs.list.length}
      <p class="muted">Nothing has run yet. Run an action from the checkout or a version; its output shows here.</p>
    {:else if !shown.length}
      <p class="muted">No job is {FILTERS.find((f) => f.id === filter)?.label.toLowerCase()}.</p>
    {/if}
    {#each shown as job (job.key)}
      <button class="job" class:on={selected?.key === job.key} onclick={() => (jobs.selected = job.key)}>
        <span class="row">
          {#if job.state === "running"}<span class="spinner"></span>{/if}
          <strong class="grow clip">{job.title}</strong>
        </span>
        <span class="row">
          <span class={`pill ${STATES[job.state].tone}`}><span class="dot"></span>{STATES[job.state].text}</span>
          <span class="dim small clip grow mono">{where(job)}</span>
          <span class="dim small">{duration(job)}</span>
        </span>
      </button>
    {/each}
  </aside>

  <section class="panel out">
    {#if selected}
      {@const action = again(selected)}
      <div class="row top">
        <div class="grow">
          <h2 class="clip">{selected.title}</h2>
          <p class="mono dim clip" title={selected.command}>{selected.command || "not started"}</p>
        </div>
        <span class="dim time">{duration(selected)}</span>
        {#if selected.code !== null}<span class="dim">exit {selected.code}</span>{/if}
        <button class="small" onclick={() => void copy(selected)}>Copy</button>
        {#if action}
          <button class="small" onclick={() => void run(selected.scope, action)}
            ><Icon name="refresh" size={14} />Run again</button
          >
        {/if}
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
      <p class="muted">Choose a job to see what it printed.</p>
    {/if}
  </section>
</div>

<style>
  /* As tall as the window leaves below the header (60px, as SideFrame's list) and the page's
     padding, whatever the page around it: the page scrolls as a whole, so a height of 100% would
     grow with the output. The job list and the output scroll inside it. */
  .console {
    height: calc(100dvh - 60px - var(--page-top, 24px) - var(--page-bottom, 48px));
    min-height: 360px;
    display: grid;
    grid-template-columns: minmax(200px, 300px) minmax(0, 1fr);
    gap: 16px;
  }

  .list {
    display: flex;
    flex-direction: column;
    gap: 8px;
    overflow: auto;
    min-height: 0;
    padding: 16px;
  }

  .filters {
    display: flex;
    flex-wrap: wrap;
    gap: 4px;
    margin-bottom: 4px;
  }

  .filters button {
    font-size: 0.75rem;
    padding: 3px 10px;
    background: transparent;
    border-color: var(--line);
    color: var(--soft);
  }

  .filters button.on {
    background: var(--panel-hi);
    border-color: var(--line-strong);
    color: var(--amber);
  }

  .n {
    font-size: 0.75rem;
    color: var(--dim);
  }

  .job {
    flex-direction: column;
    align-items: stretch;
    gap: 6px;
    text-align: left;
    border-radius: var(--radius-sm);
    background: transparent;
    border-color: var(--line);
    padding: 9px 11px;
    font-weight: 400;
    flex: none;
  }

  .job.on {
    background: var(--panel-hi);
    border-color: var(--brand);
  }

  .small {
    font-size: 0.75rem;
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
    min-width: 0;
    padding: 16px 18px;
  }

  .top {
    flex-wrap: wrap;
  }

  pre {
    flex: 1;
    min-height: 200px;
    margin: 0;
    overflow: auto;
    background: var(--ink-deep);
    border: 1px solid var(--line);
    border-radius: var(--radius-sm);
    padding: 12px 14px;
    font-family: var(--font-mono);
    font-size: 0.7812rem;
    line-height: 1.5;
    color: var(--text);
    white-space: pre-wrap;
    word-break: break-word;
  }

  /* Many tools log to stderr as a matter of course (unittest, make), so it is
     set apart, not alarming; a failure shows in the job's state. */
  .err {
    color: var(--gold);
  }

  @media (max-width: 1000px) {
    .console {
      grid-template-columns: minmax(0, 1fr);
      grid-template-rows: auto minmax(320px, 1fr);
      height: auto;
    }

    .list {
      max-height: 260px;
    }
  }
</style>
