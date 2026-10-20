<script lang="ts">
  // Help, beside a game's saves: a recent crash first, open, with the two ways to send us its
  // report (copy it, save it as a .zip) and Dismiss; then the logs and crash reports folded under
  // one line, the newest first, in a player's words ("Crashed on Batalia", "Played 23 minutes on
  // Kerwan"), each with Copy, Save and Delete (and all at once), after a plain question; then
  // Detailed logs. File names are a contributor's detail: shown with developer tools on.
  import Icon from "$components/Icon.svelte";
  import { api, onAndroid, type ReportEntry } from "$lib/api";
  import { app, countCrashes, dismissCrashes, guard, saveConfig, toast } from "$lib/app.svelte";
  import { DISCORD_URL, ISSUES_URL } from "$lib/links";
  import { headline, when } from "$lib/reports";

  // PROGRAM: only that version's logs and reports (a developer's chosen disc, DiscSwitch); none: all.
  let { game, program = null }: { game: string; program?: string | null } = $props();

  const developer = $derived(app.config?.developer ?? false);
  /** Rows shown before "Show all". */
  const FIRST = 6;

  let entries = $state<ReportEntry[] | null>(null);
  let failed = $state<string | null>(null);
  let all = $state(false);
  let copied = $state<string | null>(null);
  let saving = $state<string | null>(null);
  let savedTo = $state<string | null>(null);

  /** What is being asked about: a row's file, "all", or nothing. */
  let asking = $state<string | null>(null);
  let deleting = $state(false);
  /** Something was deleted here: an empty list then says so, not "nothing yet". */
  let cleared = $state(false);

  const crashes = $derived(entries?.filter((e) => e.kind === "crash").length ?? 0);
  /** The crash to tell the player about: the newest, while it is recent and not dismissed (the tile's mark). */
  const latest = $derived(
    (app.crashes[game]?.recent ?? 0) > 0 ? (entries?.find((e) => e.kind === "crash") ?? null) : null,
  );
  /** The logs' fold; the game's menu opens it ("Logs and crash reports"). */
  let open = $state(false);
  const shown = $derived(entries ? (all ? entries : entries.slice(0, FIRST)) : []);

  /** The list read in flight: only the newest answer is kept, so that a list read before a
   *  delete does not put the deleted log back. */
  let reading = 0;

  async function load(only: string | null) {
    const mine = ++reading;
    try {
      const list = await api.reportsList(game);
      if (mine !== reading) return;
      entries = only ? list.filter((e) => !e.program || e.program === only) : list;
      failed = null;
    } catch (e) {
      if (mine !== reading) return;
      entries = [];
      failed = String(e);
    }
  }

  $effect(() => {
    // Again whenever the game ends (app.reportsChanged) or another game or disc is shown.
    if (app.reportsChanged >= 0 && game) void load(program);
  });

  async function copy(e: ReportEntry) {
    const text = await guard(api.reportText(game, e.file));
    if (text === undefined) return;
    try {
      await navigator.clipboard.writeText(text);
      copied = e.file;
      setTimeout(() => (copied = copied === e.file ? null : copied), 4000);
    } catch {
      toast("The report could not be copied: use Save instead.", "error");
    }
  }

  async function save(e: ReportEntry) {
    saving = e.file;
    const where = await guard(api.reportSave(game, e.file));
    saving = null;
    if (where) savedTo = where;
  }

  const noun = (e: ReportEntry) => (e.kind === "crash" ? "crash report" : "log");

  async function remove(e: ReportEntry) {
    deleting = true;
    const done = await guard(api.reportDelete(game, e.file));
    deleting = false;
    asking = null;
    if (done === undefined) return;
    cleared = true;
    savedTo = null;
    app.reportsChanged++; // the list reads them again
    void countCrashes();
  }

  async function removeAll() {
    deleting = true;
    // Listing one disc's (PROGRAM): only those go, the other disc's stay.
    const count = program
      ? await guard(Promise.all((entries ?? []).map((e) => api.reportDelete(game, e.file))))
      : await guard(api.reportsDeleteAll(game));
    deleting = false;
    asking = null;
    if (count === undefined) return;
    cleared = true;
    savedTo = null;
    app.reportsChanged++;
    void countCrashes();
  }

  async function setDetailed(on: boolean) {
    if (app.config) await saveConfig({ ...app.config, detailedGameLog: on });
  }
</script>

<section class="panel reports" id="logs" tabindex="-1" aria-labelledby="logs-title">
  <div class="row top">
    <h2 id="logs-title" class="grow">Help</h2>
    {#if latest}<span class="badge">Crashed recently</span>{/if}
  </div>

  {#if latest}
    {@const e = latest}
    <div class="latest" role="group" aria-label="The last crash">
      <p class="head">
        <Icon name="warn" size={16} />
        <strong>{headline(e)}</strong>
        <span class="when">{when(e.time)}</span>
      </p>
      <p class="say">
        Your saves are kept. To help us fix it, send us the report in a
        <button class="link" onclick={() => void api.openUrl(ISSUES_URL)}>GitHub issue</button>
        or on
        <button class="link" onclick={() => void api.openUrl(DISCORD_URL)}>Discord</button>, and say what you were
        doing.
      </p>
      <span class="acts">
        <button class="small" onclick={() => void copy(e)}>
          <Icon name={copied === e.file ? "check" : "book"} size={14} />{copied === e.file ? "Copied" : "Copy report"}
        </button>
        <button class="small" onclick={() => void save(e)} disabled={saving === e.file}>
          {#if saving === e.file}<span class="spinner"></span>{:else}<Icon name="save" size={14} />{/if}Save report
        </button>
        <button
          class="ghost small"
          onclick={() => void dismissCrashes(game)}
          title="Hide this until the game crashes again">Dismiss</button
        >
      </span>
    </div>
  {/if}

  <details id="logs-list" bind:open>
    <summary>
      Logs and crash reports{entries
        ? entries.length
          ? ` (${entries.length})`
          : ": none yet"
        : ""}{#if crashes > 0 && !latest}<span class="dim count">· {crashes} {crashes === 1 ? "crash" : "crashes"}</span
        >{/if}
    </summary>
    <div class="inside">
      <p class="intro muted">
        The game keeps a log each time you play, and writes a report when it crashes. If something goes wrong, copy or
        save one and send it to us.
      </p>
      <div class="row tools">
        {#if !onAndroid}
          <!-- Android opens no app's own folders: Save puts a report where the player chooses. -->
          <button class="ghost small" onclick={() => void guard(api.openReportsFolder(game, false))}>
            <Icon name="folder" size={14} />Open folder
          </button>
        {/if}
        {#if entries && entries.length > 0}
          <button class="ghost small danger" onclick={() => (asking = asking === "all" ? null : "all")}>
            <Icon name="trash" size={14} />Delete all
          </button>
        {/if}
      </div>

      {#if asking === "all" && entries}
        <div class="ask" role="alertdialog" aria-label="Delete all logs and crash reports">
          <p>
            Delete all {entries.length} logs and crash reports of this game? They can't be undone. Your saves are not touched.
          </p>
          <span class="choices">
            <button class="danger-solid small" onclick={() => void removeAll()} disabled={deleting}>Delete all</button>
            <button class="ghost small" onclick={() => (asking = null)}>Keep them</button>
          </span>
        </div>
      {/if}

      {#if entries === null}
        <p class="muted row"><span class="spinner"></span>Reading the logs…</p>
      {:else if entries.length === 0}
        <p class="empty">
          {#if cleared}
            All cleared. A new log is kept here each time you play, and if the game ever crashes, its report too.
          {:else}
            Nothing here yet. After you play, each session is listed here, and if the game ever crashes, its report too,
            ready to send to us.
          {/if}
        </p>
        {#if failed && developer}<p class="dim mono">{failed}</p>{/if}
      {:else}
        <ul>
          {#each shown as e (e.file)}
            <li class:crash={e.kind === "crash"}>
              <span class="kind" aria-hidden="true">
                <Icon name={e.kind === "crash" ? "warn" : "book"} size={16} />
              </span>
              <span class="what">
                <strong>{headline(e)}</strong>
                <span class="when">{when(e.time)}</span>
                {#if developer}<span class="file mono dim">{e.file}</span>{/if}
              </span>
              <span class="acts">
                <button
                  class="ghost small"
                  onclick={() => void copy(e)}
                  aria-label={`Copy the report: ${headline(e)}, ${when(e.time)}`}
                  title="Copy"
                >
                  <Icon name={copied === e.file ? "check" : "book"} size={14} />
                </button>
                <button
                  class="ghost small"
                  onclick={() => void save(e)}
                  disabled={saving === e.file}
                  aria-label={`Save the report as a file: ${headline(e)}, ${when(e.time)}`}
                  title="Save as a file"
                >
                  {#if saving === e.file}<span class="spinner"></span>{:else}<Icon name="save" size={14} />{/if}
                </button>
                <button
                  class="ghost small danger"
                  onclick={() => (asking = asking === e.file ? null : e.file)}
                  aria-expanded={asking === e.file}
                  aria-label={`Delete the ${noun(e)}: ${headline(e)}, ${when(e.time)}`}
                  title="Delete"
                >
                  <Icon name="trash" size={14} />
                </button>
              </span>
              {#if asking === e.file}
                <div class="ask" role="alertdialog" aria-label={`Delete this ${noun(e)}?`}>
                  <p>Delete this {noun(e)}? It can't be undone.</p>
                  <span class="choices">
                    <button class="danger-solid small" onclick={() => void remove(e)} disabled={deleting}>Delete</button
                    >
                    <button class="ghost small" onclick={() => (asking = null)}>Keep it</button>
                  </span>
                </div>
              {/if}
            </li>
          {/each}
        </ul>
        {#if entries.length > FIRST}
          <button class="ghost small more" onclick={() => (all = !all)} aria-expanded={all}>
            {all ? "Show fewer" : `Show all ${entries.length}`}
          </button>
        {/if}
      {/if}
    </div>
  </details>

  <span class="live" aria-live="polite">
    {copied ? "Copied: paste it in your message." : ""}{savedTo ? ` Saved as ${savedTo}: attach this file.` : ""}
  </span>

  <label class="option">
    <input
      type="checkbox"
      checked={app.config?.detailedGameLog ?? false}
      onchange={(ev) => void setDetailed(ev.currentTarget.checked)}
    />
    <span>
      <strong>Detailed logs</strong>
      <small>Turn this on when we ask you to: the game writes more about what it does.</small>
    </span>
  </label>
</section>

<style>
  .reports {
    position: relative;
    z-index: 1;
    display: flex;
    flex-direction: column;
    gap: 12px;
    scroll-margin-top: 80px;
  }

  h2 {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 10px;
  }

  .reports:focus {
    outline: none;
  }

  .badge {
    font-family: var(--font-sans);
    font-size: 0.75rem;
    font-weight: 600;
    letter-spacing: 0;
    padding: 3px 10px;
    border-radius: 999px;
    color: var(--err);
    border: 1px solid color-mix(in srgb, var(--err) 45%, transparent);
    background: color-mix(in srgb, var(--err) 10%, transparent);
  }

  /* The last crash: first in Help, its mark in the crash colour, with its two ways to send the
     report; a rule under it, not a box in the panel's box. */
  .latest {
    display: flex;
    flex-direction: column;
    gap: 8px;
    padding: 2px 0 14px;
    border-bottom: 1px solid var(--line);
  }

  .latest .head {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 4px 8px;
    margin: 0;
    color: var(--err);
  }

  .latest .head strong {
    color: var(--text);
  }

  .say {
    margin: 0;
    font-size: 0.8125rem;
    color: var(--body);
  }

  .acts {
    display: flex;
    flex-wrap: wrap;
    gap: 6px;
  }

  details > summary {
    cursor: pointer;
    color: var(--lav);
    font-weight: 600;
    font-size: 0.875rem;
    padding: 2px 0;
  }

  summary .count {
    margin-left: 6px;
    font-weight: 400;
  }

  details[open] .inside {
    animation: unfold 0.22s cubic-bezier(0.2, 0.8, 0.2, 1) both;
  }

  @keyframes unfold {
    from {
      opacity: 0;
      transform: translateY(-6px);
    }
  }

  details .inside {
    display: flex;
    flex-direction: column;
    gap: 10px;
    padding: 10px 0 0 14px;
  }

  .tools {
    gap: 4px;
  }

  .intro,
  .empty {
    margin: 0;
    font-size: 0.875rem;
    line-height: 1.5;
    max-width: 72ch;
  }

  .empty {
    color: var(--soft);
    padding: 14px 16px;
    border: 1px dashed var(--line-strong);
    border-radius: 12px;
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

  ul {
    margin: 0;
    padding: 0;
    list-style: none;
  }

  li {
    display: grid;
    grid-template-columns: 22px minmax(0, 1fr) auto;
    align-items: center;
    gap: 12px;
    padding: 10px 2px;
    border-top: 1px solid var(--line);
  }

  .kind {
    display: flex;
    color: var(--dim);
  }

  li.crash .kind {
    color: var(--err);
  }

  .what {
    display: flex;
    flex-wrap: wrap;
    align-items: baseline;
    gap: 2px 12px;
    min-width: 0;
  }

  .what strong {
    color: var(--text);
    font-weight: 600;
  }

  .when {
    color: var(--soft);
    font-size: 0.8125rem;
  }

  .file {
    flex-basis: 100%;
    font-size: 0.75rem;
    overflow-wrap: anywhere;
  }

  li .acts {
    gap: 2px;
  }

  button.danger {
    color: var(--err);
  }

  button.danger:hover:not(:disabled) {
    border-color: color-mix(in srgb, var(--err) 45%, transparent);
  }

  button.danger-solid {
    color: var(--ink-deep);
    background: var(--err);
    border-color: transparent;
    font-weight: 600;
  }

  button.danger-solid:hover:not(:disabled) {
    filter: brightness(1.08);
  }

  /* The question before anything is deleted: in the row it is about, under its text. */
  .ask {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 8px 16px;
    padding: 10px 14px;
    border-radius: 12px;
    background: color-mix(in srgb, var(--err) 8%, var(--panel-hi));
    border: 1px solid color-mix(in srgb, var(--err) 40%, transparent);
  }

  li > .ask {
    grid-column: 2 / -1;
  }

  .ask p {
    margin: 0;
    flex: 1 1 260px;
    font-size: 0.875rem;
    color: var(--text);
  }

  .choices {
    display: flex;
    gap: 6px;
  }

  .more {
    align-self: flex-start;
  }

  .live {
    font-size: 0.8125rem;
    color: var(--ok);
  }

  .live:empty {
    display: none;
  }

  .option {
    display: flex;
    gap: 10px;
    align-items: flex-start;
    padding-top: 12px;
    border-top: 1px solid var(--line);
    font-size: 0.875rem;
    cursor: pointer;
  }

  .option input {
    margin-top: 3px;
  }

  .option span {
    display: flex;
    flex-direction: column;
    gap: 2px;
  }

  .option small {
    color: var(--soft);
    font-size: 0.8125rem;
  }
</style>
