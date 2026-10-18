<script lang="ts">
  // One action from launcher/actions.json as a row of Develop: its name, a line on what it does,
  // a pill for where it stands (its last job, else what it needs) and one button. The rest (the
  // whole description, what is left to check, the document) opens under it.
  import { api, type ActionView, type Scope } from "$lib/api";
  import { clock, guard, jobs, openDevelop, run } from "$lib/app.svelte";
  import { elapsed } from "$lib/format";
  import { STATE } from "$lib/labels";
  import Icon from "./Icon.svelte";

  let { action, scope }: { action: ActionView; scope: Scope } = $props();

  let open = $state(false);
  const planned = $derived(action.state === "planned");
  const sameScope = (s: Scope) =>
    s.kind === scope.kind && (s.kind === "repository" || (scope.kind === "version" && s.key === scope.key));
  /** This action's newest job in this scope, if it ran since the launcher started. */
  const last = $derived(jobs.list.findLast((j) => j.actionId === action.id && sameScope(j.scope)) ?? null);
  const busy = $derived(!!last && (last.state === "queued" || last.state === "running"));
  const blocked = $derived(planned ? "" : action.blockers.map((b) => b.replace(/^needs /, "")).join(", "));

  /** Where the action stands, as a pill: its last job first, then what it needs, then its artifact. */
  const status = $derived.by<{ text: string; tone: string }>(() => {
    if (last?.state === "queued") return { text: "Queued", tone: "info" };
    if (last?.state === "running") {
      const time = last.startedAt ? ` · ${elapsed(clock.now - last.startedAt)}` : "";
      return { text: `Running${time}`, tone: "warn" };
    }
    if (planned) return { text: "Not connected", tone: "info" };
    if (last?.state === "succeeded") return { text: action.kind === "check" ? "Passed" : "Done", tone: "ok" };
    if (last?.state === "failed") return { text: "Failed", tone: "err" };
    if (blocked) return { text: `Needs ${blocked}`, tone: "warn" };
    if (action.done) return { text: "Done", tone: "ok" };
    if (last?.state === "cancelled") return { text: "Cancelled", tone: "info" };
    return { text: "Not run", tone: "idle" };
  });

  function output() {
    if (last) jobs.selected = last.key;
    openDevelop("console");
  }
</script>

<li class:planned class:open>
  <button class="what" aria-expanded={open} onclick={() => (open = !open)}>
    <span class="name">{action.label}</span>
    {#if action.description}<span class="desc" class:clip={!open}>{action.description}</span>{/if}
  </button>
  <span class={`pill ${status.tone}`} title={status.text}
    ><span class="dot"></span><span class="clip">{status.text}</span></span
  >
  {#if planned}
    <span></span>
  {:else if busy || (last && last.state !== "cancelled" && !action.runnable)}
    <button class="small" onclick={output}>Output</button>
  {:else}
    <button class="small run" disabled={!action.runnable} onclick={() => void run(scope, action)}>
      {action.detached ? "Launch" : "Run"}
    </button>
  {/if}
  {#if open}
    <div class="more">
      {#if blocked}<p class="blocked"><Icon name="warn" size={14} />Needs {blocked}</p>{/if}
      {#if action.state === "unverified"}<p>{STATE.unverified.hint}</p>{/if}
      {#if action.todo && action.state !== "connected"}<p><strong>Left to do.</strong> {action.todo}</p>{/if}
      <div class="row links">
        {#if last}
          <button class="ghost small" onclick={output}><Icon name="terminal" size={14} />Last output</button>
        {/if}
        {#if action.docs}
          {@const docs = action.docs}
          <button class="ghost small mono" onclick={() => void guard(api.openPath(docs))}>
            <Icon name="book" size={14} />{docs}
          </button>
        {/if}
      </div>
    </div>
  {/if}
</li>

<style>
  li {
    display: grid;
    grid-template-columns: minmax(0, 1fr) auto 84px;
    align-items: center;
    gap: 4px 14px;
    padding: 9px 2px;
    border-top: 1px solid var(--line);
  }

  /* The row's words are its own button: they open what is under it. */
  .what {
    display: flex;
    flex-direction: column;
    align-items: stretch;
    gap: 1px;
    min-width: 0;
    border: 0;
    border-radius: var(--radius-sm);
    background: none;
    padding: 0;
    text-align: left;
    font-weight: 400;
  }

  .name {
    font-size: 14px;
    font-weight: 600;
    color: var(--text);
  }

  .what:hover .name {
    color: var(--gold);
  }

  .desc {
    font-size: 12.5px;
    color: var(--dim);
  }

  /* A long "Needs …" is cut in the pill and said whole under the row. */
  .pill {
    max-width: 190px;
  }

  .pill.idle {
    color: var(--dim);
  }

  li > button {
    justify-content: center;
  }

  .planned .name {
    color: var(--soft);
  }

  .more {
    grid-column: 1 / -1;
    display: flex;
    flex-direction: column;
    align-items: flex-start;
    gap: 6px;
    padding: 2px 0 4px;
    font-size: 13px;
    color: var(--dim);
  }

  .more p {
    margin: 0;
  }

  .more strong {
    color: var(--soft);
  }

  .blocked {
    display: flex;
    align-items: center;
    gap: 6px;
    color: var(--amber);
  }

  .links {
    flex-wrap: wrap;
    gap: 4px;
    margin-left: -12px;
  }

  .links .mono {
    font-weight: 400;
  }
</style>
