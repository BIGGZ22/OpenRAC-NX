<script lang="ts">
  // A newer launcher: what it brings, then one button that downloads it, installs it and starts it
  // again, with the download explained as it goes (how much of how much, what it is doing). When
  // it stops, it says why, that the launcher is unchanged, and what to try. A copy that cannot
  // update itself (one from GitHub's page, the browser preview's ?download) is sent to the release.
  import { app } from "$lib/app.svelte";
  import { api } from "$lib/api";
  import { size } from "$lib/format";
  import { installUpdate } from "$lib/updates";
  import Icon from "./Icon.svelte";

  let stage = $state<"idle" | "downloading" | "installing" | "ready" | "failed">("idle");
  let percent = $state(0);
  let done = $state(0);
  let total = $state(0);
  let error = $state<string | null>(null);
  const busy = $derived(stage === "downloading" || stage === "installing" || stage === "ready");

  /** What the update is doing, in words. */
  const doing = $derived.by(() => {
    if (stage === "downloading") {
      return total > 0 ? `Downloading: ${size(done)} of ${size(total)}` : `Downloading: ${size(done)}`;
    }
    if (stage === "installing") return "Installing…";
    if (stage === "ready") return "Installed. The launcher starts again now.";
    return "";
  });

  async function install() {
    if (busy) return;
    error = null;
    stage = "downloading";
    percent = 0;
    try {
      await installUpdate((p) => {
        stage = p.stage;
        percent = p.percent;
        done = p.downloadedBytes;
        total = p.totalBytes;
      });
    } catch (e) {
      stage = "failed";
      error = e instanceof Error ? e.message : String(e);
    }
  }

  function close() {
    if (!busy) app.showUpdateModal = false;
  }

  function onkeydown(e: KeyboardEvent) {
    if (e.key === "Escape") close();
  }
</script>

<svelte:window {onkeydown} />

{#if app.showUpdateModal && app.update}
  {@const update = app.update}
  <div class="overlay" role="presentation">
    <button class="backdrop" onclick={close} tabindex="-1" aria-label="Close"></button>
    <div class="modal rise" role="dialog" aria-modal="true" aria-labelledby="update-title">
      <div class="header">
        <div class="title-row">
          <Icon name="refresh" size={20} />
          <h2 id="update-title">A new launcher is ready</h2>
        </div>
        {#if !busy}
          <button class="close-btn" onclick={close} aria-label="Close"><Icon name="x" size={16} /></button>
        {/if}
      </div>

      <div class="body">
        <div class="version-banner">
          <span class="from">{app.info?.version ?? "this version"}</span>
          <span class="arrow" aria-hidden="true">→</span>
          <span class="tag">{update.version}</span>
          {#if update.publishedAt}
            <span class="date">released {new Date(update.publishedAt).toLocaleDateString()}</span>
          {/if}
        </div>

        {#if update.body}
          <div class="notes-box">
            <h4>What's new</h4>
            <pre class="notes">{update.body}</pre>
          </div>
        {/if}

        <p class="kept">Your games, saves, mods and settings are kept.</p>

        {#if busy}
          <div class="progress-section" aria-live="polite">
            <div
              class="progress-bar-bg"
              role="progressbar"
              aria-label="Update"
              aria-valuenow={stage === "downloading" ? percent : 100}
            >
              <div
                class="progress-fill"
                class:indeterminate={stage === "downloading" && total === 0}
                style:width={`${stage === "downloading" ? percent : 100}%`}
              ></div>
            </div>
            <div class="status-row">
              <span class="status-msg">{doing}</span>
              {#if stage === "downloading" && total > 0}<span class="percent-msg">{percent} %</span>{/if}
            </div>
          </div>
        {/if}

        {#if stage === "failed"}
          <div class="error-msg" role="alert">
            <Icon name="warn" size={16} />
            <span>The update stopped: {error}. Your launcher is unchanged; try again, or download it from GitHub.</span>
          </div>
        {/if}
      </div>

      <div class="actions">
        {#if busy}
          <span class="installing-hint">Leave the launcher open until it starts again.</span>
        {:else}
          {#if !update.canInstallDirectly}
            <span class="installing-hint grow">This copy cannot update itself: get the new one from GitHub.</span>
          {/if}
          <button class="btn-cancel" onclick={close}>Later</button>
          {#if stage === "failed" || !update.canInstallDirectly}
            <button
              class={update.canInstallDirectly ? "btn-cancel" : "btn-primary"}
              onclick={() => void api.openUrl(update.url)}>Download from GitHub</button
            >
          {/if}
          {#if update.canInstallDirectly}
            <button class="btn-primary" onclick={() => void install()}
              >{stage === "failed" ? "Try again" : "Update and restart"}</button
            >
          {/if}
        {/if}
      </div>
    </div>
  </div>
{/if}

<style>
  .overlay {
    position: fixed;
    inset: 0;
    z-index: 100;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 24px;
  }

  .backdrop {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    border: none;
    border-radius: 0;
    padding: 0;
    background: rgb(10 10 16 / 0.75);
    backdrop-filter: blur(4px);
    cursor: default;
    animation: dim 0.2s ease-out both;
  }

  @keyframes dim {
    from {
      opacity: 0;
    }
  }

  .modal {
    position: relative;
    z-index: 1;
    width: 100%;
    max-width: 560px;
    background: var(--panel);
    border: 1px solid var(--line-strong);
    border-radius: var(--radius);
    box-shadow: var(--shadow);
    display: flex;
    flex-direction: column;
    overflow: hidden;
  }

  .header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 18px 24px;
    border-bottom: 1px solid var(--line);
    background: var(--panel-hi);
  }

  .title-row {
    display: flex;
    align-items: center;
    gap: 10px;
    color: var(--amber);
  }

  .title-row h2 {
    margin: 0;
    font-size: 18px;
    font-family: var(--font-head);
    font-weight: 600;
    color: var(--text);
  }

  .close-btn {
    background: transparent;
    border: none;
    color: var(--dim);
    font-size: 18px;
    cursor: pointer;
    padding: 4px 8px;
    border-radius: 4px;
    transition:
      color 0.15s,
      background 0.15s;
  }

  .close-btn:hover {
    color: var(--text);
    background: var(--line);
  }

  .body {
    padding: 20px 24px;
    display: flex;
    flex-direction: column;
    gap: 16px;
    max-height: 400px;
    overflow-y: auto;
  }

  .version-banner {
    display: flex;
    align-items: center;
    gap: 12px;
  }

  .version-banner .tag {
    font-family: var(--font-mono);
    font-size: 14px;
    font-weight: 600;
    color: var(--amber);
    background: color-mix(in srgb, var(--amber) 12%, transparent);
    border: 1px solid color-mix(in srgb, var(--amber) 30%, transparent);
    padding: 3px 10px;
    border-radius: 6px;
  }

  .version-banner .date {
    font-size: 13px;
    color: var(--dim);
  }

  .version-banner .from {
    font-family: var(--font-mono);
    font-size: 14px;
    color: var(--soft);
  }

  .version-banner .arrow {
    color: var(--dim);
  }

  .kept {
    margin: 0;
    font-size: 13px;
    color: var(--soft);
  }

  .grow {
    flex: 1;
  }

  .notes-box {
    display: flex;
    flex-direction: column;
    gap: 6px;
  }

  .notes-box h4 {
    margin: 0;
    font-size: 13px;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    color: var(--soft);
  }

  .notes {
    margin: 0;
    padding: 12px;
    background: var(--ink-deep);
    border: 1px solid var(--line);
    border-radius: var(--radius-sm);
    font-family: var(--font-sans);
    font-size: 13px;
    line-height: 1.5;
    color: var(--body);
    white-space: pre-wrap;
    max-height: 180px;
    overflow-y: auto;
  }

  .progress-section {
    display: flex;
    flex-direction: column;
    gap: 8px;
    margin-top: 8px;
  }

  .progress-bar-bg {
    width: 100%;
    height: 8px;
    background: var(--ink-deep);
    border-radius: 4px;
    overflow: hidden;
    border: 1px solid var(--line);
  }

  .progress-fill {
    height: 100%;
    background: linear-gradient(90deg, var(--bar-top), var(--amber));
    border-radius: 4px;
    transition: width 0.2s ease-out;
  }

  .progress-fill.indeterminate {
    width: 40%;
    animation: indeterminate-slide 1.4s ease-in-out infinite alternate;
  }

  @keyframes indeterminate-slide {
    from {
      margin-left: 0;
    }
    to {
      margin-left: 60%;
    }
  }

  .status-row {
    display: flex;
    justify-content: space-between;
    font-size: 13px;
    color: var(--soft);
  }

  .percent-msg {
    font-family: var(--font-mono);
    color: var(--amber);
    font-weight: 600;
  }

  .error-msg {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 10px 14px;
    background: color-mix(in srgb, var(--err) 12%, transparent);
    border: 1px solid var(--err);
    border-radius: var(--radius-sm);
    color: var(--err);
    font-size: 13px;
  }

  .actions {
    display: flex;
    align-items: center;
    justify-content: flex-end;
    gap: 12px;
    padding: 16px 24px;
    border-top: 1px solid var(--line);
    background: var(--panel-hi);
  }

  .btn-cancel {
    background: transparent;
    border: 1px solid var(--line);
    color: var(--soft);
    padding: 8px 18px;
    border-radius: var(--radius-sm);
    font-size: 14px;
    cursor: pointer;
    transition:
      background 0.15s,
      color 0.15s;
  }

  .btn-cancel:hover {
    background: var(--line);
    color: var(--text);
  }

  .btn-primary {
    background: linear-gradient(180deg, var(--bar-top), var(--bar-bottom));
    border: none;
    color: var(--bar-ink-deep);
    font-weight: 600;
    font-size: 14px;
    padding: 8px 20px;
    border-radius: var(--radius-sm);
    cursor: pointer;
    box-shadow: 0 4px 12px rgba(229, 160, 19, 0.25);
    transition:
      transform 0.1s,
      filter 0.15s;
  }

  .btn-primary:hover {
    filter: brightness(1.1);
  }

  .btn-primary:active {
    transform: translateY(1px);
  }

  .installing-hint {
    font-size: 13px;
    color: var(--dim);
    margin-right: auto;
  }
</style>
