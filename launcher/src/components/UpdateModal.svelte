<script lang="ts">
  import { app } from "$lib/app.svelte";
  import { api, onAndroid } from "$lib/api";
  import { installUpdate } from "$lib/updates";
  import Icon from "./Icon.svelte";

  let installing = $state(false);
  let statusText = $state("");
  let percent = $state(0);
  let indeterminate = $state(false);
  let error = $state<string | null>(null);

  async function handleInstall() {
    if (installing) return;
    installing = true;
    error = null;
    statusText = "Downloading update...";
    percent = 0;
    indeterminate = true;

    try {
      await installUpdate((progress) => {
        percent = progress.percent;
        indeterminate = progress.stage === "downloading" && progress.totalBytes === 0;
        if (progress.stage === "downloading") {
          if (progress.totalBytes > 0) {
            statusText = `Downloading update (${progress.percent}%)...`;
          } else {
            const mb = (progress.downloadedBytes / (1024 * 1024)).toFixed(1);
            statusText = `Downloading update (${mb} MB)...`;
          }
        } else if (progress.stage === "installing") {
          statusText = "Installing update...";
        } else if (onAndroid) {
          // Android's installer asks the player to confirm, then restarts the launcher.
          statusText = "Confirm the update in Android's installer.";
        } else {
          statusText = "Update installed! Restarting launcher...";
        }
      });
      if (onAndroid) installing = false;
    } catch (e) {
      installing = false;
      indeterminate = false;
      error = e instanceof Error ? e.message : String(e);
      statusText = "";
    }
  }

  function handleClose() {
    if (!installing) {
      app.showUpdateModal = false;
    }
  }

  function handleKeydown(e: KeyboardEvent) {
    if (e.key === "Escape") {
      handleClose();
    }
  }
</script>

<svelte:window onkeydown={handleKeydown} />

{#if app.showUpdateModal && app.update}
  {@const update = app.update}
  <div class="overlay" role="presentation">
    <button class="backdrop" onclick={handleClose} tabindex="-1" aria-label="Close modal"></button>
    <div class="modal rise" role="dialog" aria-modal="true" aria-labelledby="update-title">
      <div class="header">
        <div class="title-row">
          <Icon name="bolt" size={22} />
          <h2 id="update-title">Update Available</h2>
        </div>
        {#if !installing}
          <button class="close-btn" onclick={handleClose} aria-label="Close dialog">✕</button>
        {/if}
      </div>

      <div class="body">
        <div class="version-banner">
          <span class="tag">{update.tag}</span>
          {#if update.publishedAt}
            <span class="date">{new Date(update.publishedAt).toLocaleDateString()}</span>
          {/if}
        </div>

        {#if update.body}
          <div class="notes-box">
            <h4>Release Notes</h4>
            <pre class="notes">{update.body}</pre>
          </div>
        {/if}

        {#if installing}
          <div class="progress-section">
            <div class="progress-bar-bg">
              <div class="progress-fill" class:indeterminate style={indeterminate ? "" : `width: ${percent}%`}></div>
            </div>
            <div class="status-row">
              <span class="status-msg">{statusText}</span>
              {#if !indeterminate}
                <span class="percent-msg">{percent}%</span>
              {/if}
            </div>
          </div>
        {:else if statusText}
          <div class="status-row"><span class="status-msg">{statusText}</span></div>
        {/if}

        {#if error}
          <div class="error-msg">
            <Icon name="warn" size={16} />
            <span>Update failed: {error}</span>
          </div>
        {/if}
      </div>

      <div class="actions">
        {#if !installing}
          <button class="btn-cancel" onclick={handleClose}>Later</button>
          {#if update.canInstallDirectly}
            <button class="btn-primary" onclick={handleInstall}>{onAndroid ? "Update" : "Update & Restart"}</button>
          {:else}
            <button class="btn-primary" onclick={() => void api.openUrl(update.url)}> Download Update </button>
          {/if}
        {:else}
          <span class="installing-hint">Please wait while the update is applied...</span>
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
    background: rgba(10, 10, 16, 0.75);
    backdrop-filter: blur(4px);
    cursor: default;
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
    background: rgba(255, 180, 67, 0.12);
    border: 1px solid rgba(255, 180, 67, 0.3);
    padding: 3px 10px;
    border-radius: 6px;
  }

  .version-banner .date {
    font-size: 13px;
    color: var(--dim);
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
    background: rgba(255, 138, 122, 0.12);
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
    font-style: italic;
  }
</style>
