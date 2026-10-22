<script lang="ts">
  // A message on the Mods tab that may name a folder (a refusal, an error, a job's line): said with
  // each path cut to its last part, so a screenshot shows no folder and no user name, and the
  // message as it came under a folded Details (lib/paths.ts).
  import { MOD_PAGE } from "$lib/modsText";
  import { hidePaths } from "$lib/paths";

  let {
    text,
    tone = "plain",
    role,
  }: { text: string; tone?: "plain" | "ok" | "err" | "warn" | "dim"; role?: "status" | "alert" } = $props();

  const shown = $derived(hidePaths(text));
</script>

<div class={`reason ${tone}`} {role}>
  <p>{shown.text}</p>
  {#if shown.hidden}
    <details class="more">
      <summary>{MOD_PAGE.details}</summary>
      <code class="full">{text}</code>
    </details>
  {/if}
</div>

<style>
  .reason {
    display: flex;
    flex-wrap: wrap;
    align-items: baseline;
    gap: 4px 14px;
    font-size: 0.8125rem;
  }

  p {
    margin: 0;
    flex: 1 1 auto;
    min-width: 0;
  }

  .ok p {
    color: var(--ok);
  }

  .err p {
    color: var(--err);
  }

  .warn p {
    color: var(--amber);
  }

  .dim p {
    color: var(--dim);
  }

  details {
    flex: 0 1 auto;
  }

  details[open] {
    flex-basis: 100%;
  }

  summary {
    font-size: 0.75rem;
    font-weight: 600;
    color: var(--soft);
    gap: 10px;
    width: max-content;
  }

  .full {
    display: block;
    margin-top: 6px;
    padding: 8px 10px;
    border-radius: var(--radius-sm);
    background: var(--ink-deep);
    color: var(--body);
    font-size: 0.75rem;
    overflow-wrap: anywhere;
    white-space: pre-wrap;
  }
</style>
