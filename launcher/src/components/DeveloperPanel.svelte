<script lang="ts">
  // What a contributor needs under a game's page, with developer tools on: the
  // decompilation of the chosen version, the level editor's checks, and the
  // files and facts of the version. Playing is the page's top buttons only:
  // the version chosen here never changes what Play starts.
  import { api, type ActionKind, type GameView, type VersionView } from "$lib/api";
  import { app, guard, openVersion } from "$lib/app.svelte";
  import { grouped, matchedLine, pct, size } from "$lib/format";
  import { DISC, INPUT, regionLabel } from "$lib/labels";
  import ActionRow from "./ActionRow.svelte";
  import Icon from "./Icon.svelte";
  import ProgressBar from "./ProgressBar.svelte";

  let { game, version: v }: { game: GameView; version: VersionView } = $props();

  const ORDER: ActionKind[] = ["setup", "build", "check", "play", "edit"];
  const scope = $derived({ kind: "version" as const, key: v.key });
  const here = $derived(
    v.actions.filter((a) => a.thisPlatform).toSorted((a, b) => ORDER.indexOf(a.kind) - ORDER.indexOf(b.kind)),
  );
  const elsewhere = $derived(v.actions.filter((a) => !a.thisPlatform && a.area !== "port"));
  const decomp = $derived(here.filter((a) => a.area === "decomp"));
  const editor = $derived(here.filter((a) => a.area === "editor"));
  // Editing itself is under Mods; here, the editor's checks and what is not connected.
  const editorRows = $derived(editor.filter((a) => a.kind === "check" || a.state === "planned"));
  const canEdit = $derived(editor.some((a) => a.kind === "edit" && a.state !== "planned"));
</script>

{#if game.versions.length > 1}
  <!-- The game is one game; its versions (each disc's build) are a developer's choice only, for the
       decompilation's progress and files below. Play always starts the version the disc gives. -->
  <section class="panel versions">
    <h2>Version</h2>
    <div class="choices" role="radiogroup" aria-label="Version">
      {#each game.versions as other (other.key)}
        <button
          role="radio"
          aria-checked={other.key === v.key}
          class:on={other.key === v.key}
          onclick={() => {
            openVersion(other.key);
          }}>{regionLabel(other.name)} · <span class="mono">{other.serial}</span></button
        >
      {/each}
    </div>
  </section>
{/if}

<section class="panel">
  <div class="row">
    <h2 class="grow">Decompilation</h2>
    {#if v.progress}<span class="percent">{pct(v.progress.percent)}</span>{/if}
  </div>
  <p class="blurb">The original PlayStation 2 program, rebuilt from the C and compared with your disc.</p>
  {#if v.progress}
    <ProgressBar
      percent={v.progress.percent}
      functional={v.progress.functional?.percent ?? null}
      label={`${v.title} code matched`}
    />
    <p class="count">
      {matchedLine(v.progress.matchedCode, v.progress.totalCode)}
      {#if v.progress.matchedFunctions !== null && v.progress.totalFunctions !== null}
        · {grouped(v.progress.matchedFunctions)} of {grouped(v.progress.totalFunctions)} functions
      {/if}
      {#if v.progress.date}· {v.progress.date}{/if}
    </p>
    {#if v.progress.functional}
      <!-- Not progress: C that behaves like retail without matching its bytes (ByteMatch's line under the Code bar). -->
      <p class="count functional">
        Functional C: {pct(v.progress.functional.percent)} of the code · {grouped(v.progress.functional.functions)} functions,
        not counted as matched
      </p>
    {/if}
  {/if}
  <ul>
    {#each decomp as action (action.id)}<ActionRow {action} {scope} />{/each}
  </ul>
  {#if elsewhere.length}
    <details>
      <summary>{elsewhere.length} more for other platforms</summary>
      <ul>
        {#each elsewhere as action (action.id)}<ActionRow {action} {scope} />{/each}
      </ul>
    </details>
  {/if}
</section>

{#if editor.length}
  <section class="panel">
    <div class="row">
      <h2 class="grow">Level editor</h2>
      {#if canEdit}<button onclick={() => (app.page = "mods")}><Icon name="edit" size={16} />Open Mods</button>{/if}
    </div>
    <p class="blurb">
      {canEdit ? "Extracting, editing and packing levels is under Mods." : "Not connected for this version yet."}
    </p>
    {#if editorRows.length}
      <ul>
        {#each editorRows as action (action.id)}<ActionRow {action} {scope} />{/each}
      </ul>
    {/if}
  </section>
{/if}

<section class="panel">
  <div class="row">
    <h2 class="grow">Files</h2>
    <span class={`pill ${DISC[v.status.disc.state].tone}`}
      ><span class="dot"></span>{DISC[v.status.disc.state].text}</span
    >
  </div>
  <dl>
    <dt>Disc image</dt>
    <dd>
      <span class="mono">{v.disc?.file ?? "–"}</span>
      {#if v.disc?.size}<span class="dim">· {size(v.disc.size)}</span>{/if}
      <span class="muted">· {v.status.disc.message}</span>
    </dd>
    {#each v.status.inputs as input (input.path)}
      <dt>{INPUT[input.from] ?? input.from}</dt>
      <dd>
        <span class="mono">{input.path}</span>
        <span class={input.present ? "ok" : "warn"}>· {input.message}</span>
      </dd>
    {/each}
    <dt>Folder</dt>
    <dd>
      <button class="link mono" onclick={() => void guard(api.openPath(v.dir))}>{v.dir}</button>
    </dd>
  </dl>
</section>

<details class="panel about">
  <summary>About this version</summary>
  <dl>
    <dt>Version</dt>
    <dd>{v.region} · <span class="mono">{v.serial}</span>{game.year ? ` · ${game.year}` : ""}</dd>
    {#if v.target}<dt>Target</dt>
      <dd>{v.target}</dd>{/if}
    {#if v.buildHost}<dt>Builds on</dt>
      <dd>{v.buildHost}</dd>{/if}
    {#if v.source}
      {@const repo = v.source.repo}
      <dt>From</dt>
      <dd>
        <button class="link" onclick={() => void guard(api.openUrl(repo))}>{v.source.name}</button>
        {#if v.source.commit}<span class="mono dim">@{v.source.commit.slice(0, 7)}</span>{/if}
        {#if v.source.date}<span class="dim">· {v.source.date}</span>{/if}
      </dd>
      {#if v.source.license}<dt>License</dt>
        <dd>{v.source.license}</dd>{/if}
    {/if}
    {#if v.progress?.note}<dt>Counted</dt>
      <dd>{v.progress.note}</dd>{/if}
    {#if v.setup}<dt>By hand</dt>
      <dd class="muted">{v.setup}</dd>{/if}
  </dl>
  {#if v.readme}
    {@const readme = v.readme}
    <button onclick={() => void guard(api.openPath(readme))}><Icon name="book" size={16} />README</button>
  {/if}
</details>

<style>
  .count.functional {
    margin-top: -4px;
    opacity: 0.8;
  }

  /* Which version (each disc's build) the panel shows: one control, the chosen half filled. */
  .versions .choices {
    display: inline-flex;
    padding: 3px;
    border-radius: 999px;
    border: 1px solid var(--bd);
  }

  .versions button {
    border: 0;
    background: none;
    font-size: 13px;
    color: var(--tx2);
    padding: 6px 16px;
    border-radius: 999px;
  }

  .versions button:hover {
    color: var(--tx1, #fff);
  }

  .versions button.on {
    background: var(--bd);
    color: #fff;
  }

  .blurb {
    margin: 2px 0 12px;
    font-size: 14px;
    color: var(--soft);
  }

  ul {
    margin: 0;
    padding: 0;
    list-style: none;
  }

  ul > :global(li:last-child) {
    padding-bottom: 0;
  }

  .percent {
    font-family: var(--font-head);
    font-size: 20px;
    font-weight: 700;
    font-variant-numeric: tabular-nums;
    color: var(--gold);
  }

  .count {
    margin: 8px 0 14px;
    font-size: 13px;
    color: var(--dim);
  }

  dl {
    display: grid;
    grid-template-columns: 110px minmax(0, 1fr);
    gap: 6px 16px;
    margin: 12px 0 0;
    font-size: 14px;
  }

  dt {
    color: var(--dim);
  }

  dd {
    margin: 0;
    min-width: 0;
    overflow-wrap: anywhere;
  }

  .ok {
    color: var(--ok);
  }

  .warn {
    color: var(--warn);
  }

  summary {
    cursor: pointer;
    font-weight: 600;
    color: var(--soft);
  }

  details:not(.about) {
    margin-top: 12px;
    padding-top: 10px;
    border-top: 1px solid var(--line);
  }

  details:not(.about) ul {
    margin-top: 8px;
  }

  .about dl {
    margin-bottom: 14px;
  }

  .link {
    border: 0;
    border-radius: 4px;
    background: none;
    padding: 0;
    color: var(--amber);
    font-weight: 600;
    text-align: left;
  }

  .link.mono {
    font-weight: 400;
  }
</style>
