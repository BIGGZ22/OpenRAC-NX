<script lang="ts">
  // With developer tools on: the shell command behind a step, folded behind a small toggle, with
  // Copy. One line per command; a long line scrolls inside its box, never the page.
  import { toast } from "$lib/app.svelte";
  import { fold } from "$lib/motion";
  import { slide } from "svelte/transition";
  import Icon from "./Icon.svelte";

  let { lines }: { lines: string[] } = $props();

  let open = $state(false);
  let copied = $state(false);
  const text = $derived(lines.join("\n"));

  async function copy() {
    try {
      await navigator.clipboard.writeText(text);
      copied = true;
      setTimeout(() => (copied = false), 1500);
    } catch {
      toast("The command could not be copied: select it and copy it yourself.", "error");
    }
  }
</script>

<div class="command">
  <button class="ghost small toggle" aria-expanded={open} onclick={() => (open = !open)}>
    <span class="mono" aria-hidden="true">‹/›</span>{open ? "Hide" : "Show"}
    {lines.length === 1 ? "command" : "commands"}
  </button>
  {#if open}
    <div class="box" transition:slide={fold}>
      <pre>{text}</pre>
      <button class="small" onclick={() => void copy()}
        >{#if copied}<Icon name="check" size={14} />Copied{:else}Copy{/if}</button
      >
    </div>
  {/if}
</div>

<style>
  .command {
    display: flex;
    flex-direction: column;
    align-items: flex-start;
    gap: 6px;
    min-width: 0;
  }

  .toggle {
    padding: 2px 8px;
    margin-left: -8px;
    font-size: 0.75rem;
    color: var(--dim);
  }

  .toggle .mono {
    font-size: 0.75rem;
  }

  .box {
    align-self: stretch;
    display: flex;
    align-items: flex-start;
    gap: 10px;
    min-width: 0;
    padding: 8px 8px 8px 12px;
    background: var(--ink-deep);
    border: 1px solid var(--line);
    border-radius: var(--radius-sm);
  }

  pre {
    flex: 1;
    min-width: 0;
    margin: 0;
    padding: 3px 0;
    overflow-x: auto;
    font-family: var(--font-mono);
    font-size: 0.75rem;
    line-height: 1.6;
    color: var(--body);
  }

  .box button {
    flex: none;
  }
</style>
