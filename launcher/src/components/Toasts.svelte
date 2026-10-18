<script lang="ts">
  import { app } from "$lib/app.svelte";
  import Icon from "./Icon.svelte";
</script>

<div class="toasts" role="status" aria-live="polite">
  {#each app.toasts as t (t.id)}
    <div class="toast rise" class:error={t.kind === "error"}>
      <Icon name={t.kind === "error" ? "warn" : "info"} size={16} />
      <span>{t.text}</span>
    </div>
  {/each}
</div>

<style>
  .toasts {
    position: fixed;
    right: 20px;
    bottom: calc(20px + var(--nav-bottom));
    display: flex;
    flex-direction: column;
    gap: 8px;
    z-index: 50;
    max-width: 420px;
  }

  .toast {
    display: flex;
    gap: 10px;
    align-items: flex-start;
    background: var(--panel-hi);
    border: 1px solid var(--line-strong);
    border-left: 3px solid var(--amber);
    border-radius: var(--radius-sm);
    padding: 10px 14px;
    font-size: 14px;
    box-shadow: var(--shadow-sm);
  }

  /* Android: centred over the bottom of the page, as its snackbars are. */
  :global(html[data-platform="android"]) .toasts {
    right: 16px;
    left: calc(var(--nav-left) + 16px);
    bottom: calc(16px + var(--nav-bottom));
    align-items: center;
    max-width: none;
    pointer-events: none;
  }

  :global(html[data-platform="android"]) .toast {
    max-width: 560px;
    border-left-width: 1px;
    border-radius: 12px;
  }

  .toast.error {
    border-left-color: var(--err);
  }

  .toast :global(svg) {
    flex: none;
    margin-top: 2px;
  }
</style>
