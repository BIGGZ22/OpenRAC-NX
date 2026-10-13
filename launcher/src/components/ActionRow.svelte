<script lang="ts">
  // One action from launcher/actions.json as a row: its name, a line on what it
  // does, why it cannot run if it cannot, and its button. The rest (the whole
  // description, what is left to check, the document) opens under it.
  import { api, type ActionView, type Scope } from "$lib/api";
  import { guard, jobs, run } from "$lib/app.svelte";
  import { STATE } from "$lib/labels";
  import Icon from "./Icon.svelte";

  let { action, scope }: { action: ActionView; scope: Scope } = $props();

  let open = $state(false);
  const planned = $derived(action.state === "planned");
  const sameScope = (s: Scope) =>
    s.kind === scope.kind && (s.kind === "repository" || (scope.kind === "version" && s.key === scope.key));
  const busy = $derived(
    jobs.list.some(
      (j) => j.actionId === action.id && sameScope(j.scope) && (j.state === "queued" || j.state === "running"),
    ),
  );
  const blocked = $derived(planned ? "" : action.blockers.map((b) => b.replace(/^needs /, "")).join(", "));
</script>

<li class:planned class:open>
  <button class="what" aria-expanded={open} onclick={() => (open = !open)}>
    <span class="name">
      {action.label}
      {#if action.done && !planned}<span class="done" title="Done"><Icon name="check" size={14} /></span>{/if}
    </span>
    {#if action.description}<span class="desc" class:clip={!open}>{action.description}</span>{/if}
    {#if blocked}<span class="blocked"><Icon name="warn" size={14} />Needs {blocked}</span>{/if}
  </button>
  {#if planned}
    <span class="soon">Not connected yet</span>
  {:else}
    <button class="run" disabled={!action.runnable || busy} onclick={() => void run(scope, action)}>
      {#if busy}<span class="spinner"></span>{/if}{busy ? "Running…" : action.detached ? "Launch" : "Run"}
    </button>
  {/if}
  {#if open}
    <div class="more">
      {#if action.state === "unverified"}<p>{STATE.unverified.hint}</p>{/if}
      {#if action.todo && action.state !== "connected"}<p><strong>Left to do.</strong> {action.todo}</p>{/if}
      {#if action.docs}
        {@const docs = action.docs}
        <button class="ghost small" onclick={() => void guard(api.openPath(docs))}>
          <Icon name="book" size={14} />{docs}
        </button>
      {/if}
    </div>
  {/if}
</li>

<style>
  li {
    display: grid;
    grid-template-columns: minmax(0, 1fr) auto;
    align-items: center;
    gap: 4px 16px;
    padding: 10px 2px;
    border-top: 1px solid var(--line);
  }

  /* The row's words are its own button: they open what is under it. */
  .what {
    display: flex;
    flex-direction: column;
    align-items: stretch;
    gap: 2px;
    border: 0;
    border-radius: var(--radius-sm);
    background: none;
    padding: 0;
    text-align: left;
    font-weight: 400;
  }

  .name {
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 15px;
    font-weight: 600;
    color: var(--text);
  }

  .what:hover .name {
    color: var(--gold);
  }

  .done {
    display: inline-flex;
    color: var(--ok);
  }

  .desc {
    font-size: 13px;
    color: var(--soft);
  }

  .blocked {
    display: flex;
    align-items: center;
    gap: 6px;
    font-size: 13px;
    color: var(--amber);
  }

  .run {
    min-width: 84px;
    justify-content: center;
  }

  .soon {
    font-size: 13px;
    color: var(--dim);
  }

  .planned .name {
    color: var(--soft);
  }

  .more {
    grid-column: 1 / -1;
    display: flex;
    flex-direction: column;
    align-items: flex-start;
    gap: 8px;
    padding: 4px 0 6px;
    font-size: 13px;
    color: var(--dim);
  }

  .more strong {
    color: var(--soft);
  }

  .more .ghost {
    margin-left: -12px;
    font-family: var(--font-mono);
    font-weight: 400;
  }
</style>
