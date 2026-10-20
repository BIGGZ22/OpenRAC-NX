<script lang="ts">
  // The game stopped unexpectedly: what happened in plain words, one obvious next step (copy the
  // report) and where to send it (Discord). Saving it as a file, the folder, GitHub and the port's
  // own words (the fault, the exit status, the last lines) wait under More. Once the game has run
  // to its end since, the notice is a line.
  import Icon from "$components/Icon.svelte";
  import { api, onAndroid } from "$lib/api";
  import { app, dismissStopped, guard, toast } from "$lib/app.svelte";
  import { DISCORD_URL, ISSUES_URL } from "$lib/links";
  import { place, stopText, type StopNotice } from "$lib/reports";
  import { untrack } from "svelte";

  let { notice, title }: { notice: StopNotice; title: string } = $props();

  const crash = $derived(notice.crash);
  /** The session log of the version played, for a stop that left no report. */
  const sessionLog = $derived(`openrac-${notice.versionKey.replace("/", "-")}.log`);
  let copied = $state(false);
  let saving = $state(false);
  let savedTo = $state<string | null>(null);
  /** Stopped moments ago: announced as it arrives, not each time the page is opened again. */
  const fresh = untrack(() => Date.now() - notice.at < 10_000);
  const when = $derived(new Date(notice.at).toLocaleString([], { dateStyle: "medium", timeStyle: "short" }));

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
      toast("The report could not be copied: save it as a file instead (under More).", "error");
    }
  }

  async function save() {
    saving = true;
    const where = await guard(api.reportSave(notice.game, crash?.file ?? sessionLog));
    saving = false;
    if (where) savedTo = where;
  }
</script>

{#if notice.playedSince}
  <!-- Played to the end since: a line, the report still a click away. -->
  <section class="notice short" aria-labelledby="stopped-title">
    <span class="mark" aria-hidden="true"><Icon name="warn" size={18} /></span>
    <p id="stopped-title" class="grow">
      {title} stopped unexpectedly {when}. It has run fine since; the report can still help us.
    </p>
    <button class="small" onclick={() => void copy()}>
      <Icon name={copied ? "check" : "book"} size={14} />{copied ? "Copied" : "Copy report"}
    </button>
    <button class="ghost small" onclick={dismissStopped} aria-label="Dismiss the crash notice">
      <Icon name="x" size={14} />Dismiss
    </button>
    <span class="live" aria-live="polite">{copied ? "The report is copied." : ""}</span>
  </section>
{:else}
  <!-- An alert only as it arrives: coming back to the page does not announce it again. -->
  <section class="notice rise" role={fresh ? "alert" : undefined} aria-labelledby="stopped-title">
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
      something you did, and your saves are kept. Copy the report and paste it in a message to us on Discord: it helps
      us fix it.
    </p>

    <div class="actions">
      <button onclick={() => void copy()}>
        <Icon name={copied ? "check" : "book"} size={16} />{copied ? "Copied" : "Copy report"}
      </button>
      <button class="ghost" onclick={() => void api.openUrl(DISCORD_URL)}>
        <Icon name="external" size={16} />Open Discord
      </button>
      <span class="live" aria-live="polite">{copied ? "The report is copied: paste it in your message." : ""}</span>
    </div>

    <details>
      <summary>More</summary>
      <div class="actions more">
        <button class="small" onclick={() => void save()} disabled={saving}>
          {#if saving}<span class="spinner"></span>{:else}<Icon name="save" size={14} />{/if}Save report as a file
        </button>
        {#if !onAndroid}
          <button
            class="small ghost"
            onclick={() => void guard(api.openReportsFolder(notice.game, !!crash))}
            title="The folder the game's logs and crash reports are in"
          >
            <Icon name="folder" size={14} />Open folder
          </button>
        {/if}
        <button class="small ghost" onclick={() => void api.openUrl(ISSUES_URL)}>
          <Icon name="external" size={14} />Open a GitHub issue
        </button>
      </div>
      {#if savedTo}
        <p class="saved">Saved as <span class="mono">{savedTo}</span>. Attach this file to your message.</p>
      {/if}
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
{/if}

<style>
  .notice {
    position: relative;
    z-index: 1;
    display: flex;
    flex-direction: column;
    gap: 12px;
    padding: 20px 22px;
    border-radius: 16px;
    border: 1px solid color-mix(in srgb, var(--err) 55%, transparent);
    background: linear-gradient(color-mix(in srgb, var(--err) 7%, transparent), transparent 60%), var(--panel);
  }

  .short {
    flex-direction: row;
    flex-wrap: wrap;
    align-items: center;
    gap: 10px;
    padding: 12px 16px;
    border-color: var(--line-strong);
    background: var(--panel);
  }

  .short p {
    margin: 0;
    min-width: 16ch;
    font-size: 0.875rem;
    color: var(--body);
  }

  .more {
    margin: 10px 0 0;
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
    font-size: 1.1875rem;
  }

  .say {
    margin: 0;
    color: var(--text);
    font-size: 0.9375rem;
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
    font-size: 0.8125rem;
    color: var(--ok);
  }

  .saved {
    margin: 0;
    font-size: 0.875rem;
    color: var(--body);
  }

  .saved .mono {
    color: var(--text);
    overflow-wrap: anywhere;
  }

  details {
    border-top: 1px solid var(--line);
    padding-top: 10px;
    font-size: 0.875rem;
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
    font-size: 0.75rem;
    white-space: pre-wrap;
  }
</style>
