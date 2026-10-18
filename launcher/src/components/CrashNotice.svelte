<script lang="ts">
  // The game stopped unexpectedly: what happened in plain words, one obvious next step (copy the
  // report, or save it as a .zip to attach) and where to send it. The port's own words (the
  // fault, the exit status, the last lines) wait under Details.
  import Icon from "$components/Icon.svelte";
  import { api } from "$lib/api";
  import { app, dismissStopped, guard, toast } from "$lib/app.svelte";
  import { DISCORD_URL, ISSUES_URL } from "$lib/links";
  import { place, stopText, type StopNotice } from "$lib/reports";

  let { notice, title }: { notice: StopNotice; title: string } = $props();

  const crash = $derived(notice.crash);
  /** The session log of the version played, for a stop that left no report. */
  const sessionLog = $derived(`openrac-${notice.versionKey.replace("/", "-")}.log`);
  let copied = $state(false);
  let saving = $state(false);
  let savedTo = $state<string | null>(null);

  async function copy() {
    const text = crash
      ? await guard(api.reportText(notice.game, crash.file))
      : stopText(notice, title, app.info?.version ?? null);
    if (text === undefined) return;
    try {
      await navigator.clipboard.writeText(text);
      copied = true;
      setTimeout(() => (copied = false), 4000);
    } catch {
      toast("The report could not be copied: use Save report instead.", "error");
    }
  }

  async function save() {
    saving = true;
    const where = await guard(api.reportSave(notice.game, crash?.file ?? sessionLog));
    saving = false;
    if (where) savedTo = where;
  }
</script>

<section class="notice rise" role="alert" aria-labelledby="stopped-title">
  <div class="head">
    <span class="mark" aria-hidden="true"><Icon name="warn" size={22} /></span>
    <h2 id="stopped-title" class="grow">The game stopped unexpectedly</h2>
    <button
      class="ghost small close"
      onclick={dismissStopped}
      title="Hide this until the game crashes again"
      aria-label="Dismiss: hide this until the game crashes again"
    >
      <Icon name="x" size={16} />Dismiss
    </button>
  </div>

  <p class="say">
    {title} closed by itself{crash ? ` while you were ${place(crash)}` : ""}. This is a problem in OpenRAC, not
    something you did. Your saves on the memory card are kept: play again from your last save.
  </p>
  <p class="say">
    You can help us fix it: copy the report and paste it in a message to us{crash ? "" : ", or save it as a file"}.
  </p>

  <div class="actions">
    <button onclick={() => void copy()}>
      <Icon name={copied ? "check" : "book"} size={16} />{copied ? "Copied" : "Copy report"}
    </button>
    <button onclick={() => void save()} disabled={saving}>
      {#if saving}<span class="spinner"></span>{:else}<Icon name="save" size={16} />{/if}Save report
    </button>
    <button
      class="ghost"
      onclick={() => void guard(api.openReportsFolder(notice.game, !!crash))}
      title="The folder the game's logs and crash reports are in"
    >
      <Icon name="folder" size={16} />Open folder
    </button>
    <span class="live" aria-live="polite">{copied ? "The report is copied: paste it in your message." : ""}</span>
  </div>
  {#if savedTo}
    <p class="saved">Saved as <span class="mono">{savedTo}</span>. Attach this file to your message.</p>
  {/if}

  <p class="send">
    Send it to us in a
    <button class="link" onclick={() => void api.openUrl(ISSUES_URL)}>GitHub issue</button>
    or on our
    <button class="link" onclick={() => void api.openUrl(DISCORD_URL)}>Discord</button>. Say what you were doing when it
    happened.
  </p>

  <details>
    <summary>Details</summary>
    <dl>
      {#if crash}
        <dt>What the game reported</dt>
        <dd class="mono">{crash.reason ?? "unknown"}</dd>
        <dt>Report</dt>
        <dd class="mono">{crash.file}</dd>
      {:else}
        <dt>What happened</dt>
        <dd>
          {notice.code === null
            ? "The system stopped the game."
            : `The game ended with exit code ${notice.code}, without a crash report.`}
        </dd>
      {/if}
      <dt>When</dt>
      <dd>{new Date(notice.at).toLocaleString()}</dd>
    </dl>
    {#if !crash && notice.lines.length}
      <pre class="mono">{notice.lines.slice(-15).join("\n")}</pre>
    {/if}
  </details>
</section>

<style>
  .notice {
    position: relative;
    z-index: 1;
    display: flex;
    flex-direction: column;
    gap: 12px;
    padding: 20px 22px;
    border-radius: 16px;
    background: var(--panel);
    border: 1px solid color-mix(in srgb, var(--err) 55%, transparent);
    box-shadow: inset 4px 0 0 var(--err);
  }

  .head {
    display: flex;
    align-items: center;
    gap: 10px;
  }

  .mark {
    display: flex;
    color: var(--err);
  }

  h2 {
    margin: 0;
    font-size: 19px;
  }

  .say {
    margin: 0;
    color: var(--text);
    font-size: 15px;
    line-height: 1.5;
    max-width: 68ch;
  }

  .actions {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 10px;
    margin-top: 4px;
  }

  .live {
    font-size: 13px;
    color: var(--ok);
  }

  .saved,
  .send {
    margin: 0;
    font-size: 14px;
    color: var(--body);
  }

  .saved .mono {
    color: var(--text);
    overflow-wrap: anywhere;
  }

  .link {
    display: inline;
    padding: 0;
    border: 0;
    background: none;
    color: var(--amber);
    font: inherit;
    text-decoration: underline;
    cursor: pointer;
  }

  details {
    border-top: 1px solid var(--line);
    padding-top: 10px;
    font-size: 14px;
    color: var(--soft);
  }

  summary {
    cursor: pointer;
    color: var(--lav);
    width: fit-content;
  }

  summary:focus-visible {
    outline: 2px solid var(--amber);
    outline-offset: 2px;
    border-radius: 4px;
  }

  dl {
    display: grid;
    grid-template-columns: max-content minmax(0, 1fr);
    gap: 6px 14px;
    margin: 10px 0 0;
  }

  dt {
    color: var(--dim);
  }

  dd {
    margin: 0;
    color: var(--body);
    overflow-wrap: anywhere;
  }

  pre {
    margin: 10px 0 0;
    padding: 10px 12px;
    max-height: 220px;
    overflow: auto;
    border-radius: 10px;
    background: var(--ink-deep);
    color: var(--body);
    font-size: 12px;
    white-space: pre-wrap;
  }
</style>
