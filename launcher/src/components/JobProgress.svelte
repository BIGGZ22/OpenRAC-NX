<script lang="ts">
  // What the job under the Play button is doing: its step in plain words, a slim bar (a percentage
  // when the output says how far along it is, an animated bar when it does not) and, behind
  // "Details", the last lines it printed. A strip of fixed layout across the hero card: nothing
  // here changes the width of the button above it, and no line of output can push it wider.
  import type { Job } from "$lib/queue";
  import { isGame, phaseLabel, readProgress, tailLines } from "$lib/progress";
  import Icon from "./Icon.svelte";

  let { job, now }: { job: Job; now: number } = $props();

  let open = $state(false);
  let box = $state<HTMLPreElement | null>(null);

  const phase = $derived(phaseLabel(job, now));
  const progress = $derived(readProgress(job.actionId, job.lines));
  const queued = $derived(job.state === "queued");
  /** The game itself has nothing to count: a bar would only wait for the end of the session. */
  const bar = $derived(!isGame(job));
  const text = $derived(tailLines(job.lines).join("\n"));
  const percent = $derived(progress.percent === null ? null : Math.round(progress.percent));

  // The newest line is in view while the box is open.
  $effect(() => {
    if (open && box && text.length >= 0) box.scrollTop = box.scrollHeight;
  });
</script>

<div class="job" aria-live="off">
  <div class="top">
    <span class="phase" role="status">{phase}{queued ? "…" : ""}</span>
    {#if percent !== null}
      <span class="pct">{percent} %</span>
    {:else if progress.note}
      <span class="pct">{progress.note}</span>
    {/if}
    <button class="ghost small toggle" onclick={() => (open = !open)} aria-expanded={open} aria-controls="job-details">
      Details<span class="caret" class:open aria-hidden="true"><Icon name="arrow" size={13} /></span>
    </button>
  </div>

  {#if bar}
    <div
      class="track"
      role="progressbar"
      aria-label={phase}
      aria-valuemin={0}
      aria-valuemax={100}
      aria-valuenow={percent ?? undefined}
    >
      {#if queued}
        <i style:width="0%"></i>
      {:else if percent !== null}
        <i style:width={`${progress.percent}%`}></i>
      {:else}
        <i class="moving"></i>
      {/if}
    </div>
  {/if}

  {#if open}
    <pre id="job-details" class="mono" bind:this={box} aria-label="The last lines the task printed">{text ||
        "Nothing has been printed yet."}</pre>
  {/if}
</div>

<style>
  .job {
    box-sizing: border-box;
    align-self: stretch;
    min-width: 0;
    display: flex;
    flex-direction: column;
    gap: 8px;
    padding: 12px 16px;
    border-radius: 16px;
    background: rgb(18 18 26 / 0.72);
    border: 1px solid var(--line);
    backdrop-filter: blur(6px);
  }

  .top {
    display: flex;
    align-items: center;
    gap: 12px;
    min-width: 0;
  }

  .phase {
    flex: 1 1 auto;
    min-width: 0;
    font-size: 0.875rem;
    font-weight: 600;
    color: var(--text);
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }

  .pct {
    flex: none;
    font-size: 0.8125rem;
    font-variant-numeric: tabular-nums;
    color: var(--soft);
  }

  .toggle {
    flex: none;
  }

  .caret {
    display: inline-flex;
    transform: rotate(-90deg);
    transition: transform 0.15s;
  }

  .caret.open {
    transform: rotate(90deg);
  }

  .track {
    position: relative;
    height: 6px;
    overflow: hidden;
    border-radius: 3px;
    background: rgb(255 255 255 / 0.12);
  }

  .track > i {
    position: absolute;
    inset: 0 auto 0 0;
    border-radius: 3px;
    background: linear-gradient(to right, var(--tx2, var(--brand)), var(--tx, var(--amber)));
    transition: width 0.5s ease-out;
  }

  .track > i.moving {
    width: 35%;
    animation: slide 1.5s ease-in-out infinite;
  }

  @keyframes slide {
    from {
      transform: translateX(-100%);
    }
    to {
      transform: translateX(290%);
    }
  }

  pre {
    box-sizing: border-box;
    margin: 0;
    padding: 8px 12px;
    /* About eight lines, then it scrolls. */
    max-height: calc(8 * 1.45em + 16px);
    overflow: auto;
    border-radius: 10px;
    background: var(--ink-deep);
    color: var(--body);
    font-size: 0.75rem;
    line-height: 1.45;
    white-space: pre-wrap;
    overflow-wrap: anywhere;
  }
</style>
