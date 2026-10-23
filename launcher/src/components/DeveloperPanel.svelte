<script lang="ts">
  // A version in Develop: how far its decompilation is, its actions grouped by area, its files and
  // the facts about it. Develop's left list chooses the version; playing stays on the game's page,
  // and nothing chosen here changes what Play starts.
  import {
    api,
    onAndroid,
    type ActionArea,
    type ActionKind,
    type DiscState,
    type GameView,
    type VersionView,
  } from "$lib/api";
  import { app, guard } from "$lib/app.svelte";
  import { grouped, matchedLine, pct, size } from "$lib/format";
  import { DISC, INPUT, regionLabel } from "$lib/labels";
  import { themeStyle } from "$lib/themes";
  import ActionRow from "./ActionRow.svelte";
  import CountUp from "./CountUp.svelte";
  import Icon from "./Icon.svelte";
  import ProgressBar from "./ProgressBar.svelte";

  let { game, version: v }: { game: GameView; version: VersionView } = $props();

  const ORDER: ActionKind[] = ["setup", "build", "check", "play", "edit"];
  const scope = $derived({ kind: "version" as const, key: v.key });
  const here = $derived(
    v.actions.filter((a) => a.thisPlatform).toSorted((a, b) => ORDER.indexOf(a.kind) - ORDER.indexOf(b.kind)),
  );
  // Android runs none of the others (no shell, Python or compiler there): they are not listed.
  const elsewhere = $derived(onAndroid ? [] : v.actions.filter((a) => !a.thisPlatform && a.area !== "port"));
  const editor = $derived(here.filter((a) => a.area === "editor"));
  // Editing itself is under Mods; here, the editor's checks and what is not connected.
  const canEdit = $derived(editor.some((a) => a.kind === "edit" && a.state !== "planned"));

  /** The actions by area, in this order; an area with nothing to show is left out. */
  const AREAS: { area: ActionArea; title: string; blurb: string }[] = [
    {
      area: "decomp",
      title: "Decompilation",
      blurb: "The original PlayStation 2 program, rebuilt from the C and compared with your disc.",
    },
    {
      area: "port",
      title: "Native port",
      blurb: onAndroid
        ? "The game this launcher carries, built for this device."
        : "The game built for this computer, and its levels.",
    },
    { area: "editor", title: "Level editor", blurb: "" },
  ];
  const groups = $derived(
    AREAS.map((g) => ({
      ...g,
      actions:
        g.area === "editor"
          ? editor.filter((a) => a.kind === "check" || a.state === "planned")
          : here.filter((a) => a.area === g.area),
    })).filter((g) => g.actions.length || (g.area === "editor" && editor.length)),
  );

  /** The disc image's state in a word or two; the whole message is its tooltip. */
  const DISC_SHORT: Record<DiscState, string> = {
    found: "found",
    mismatch: "wrong size",
    missing: "missing",
    unknown: "not needed",
  };
  const plays = $derived(game.install?.extracted?.version === v.key);
</script>

<div class="version" style={themeStyle(v.game)}>
  <section class="panel">
    <div class="head">
      <div class="grow">
        <h2>{v.title}</h2>
        <div class="chips">
          <span class="chip mono">{v.key}</span>
          <span class="chip mono">{v.serial}</span>
          <span class="chip">{regionLabel(v.name)}</span>
          <span class={`pill ${DISC[v.status.disc.state].tone}`}
            ><span class="dot"></span>{DISC[v.status.disc.state].text}</span
          >
          {#if plays}<span class="pill ok"><span class="dot"></span>Set up from this disc</span>{/if}
        </div>
      </div>
      {#if v.progress}<span class="percent"><CountUp value={v.progress.percent} format={pct} /></span>{/if}
    </div>
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
    {:else}
      <p class="count">No progress report for this version yet.</p>
    {/if}
  </section>

  <section class="panel">
    {#each groups as group (group.area)}
      <div class="group">
        <div class="row">
          <h3 class="grow">{group.title}</h3>
          {#if group.area === "editor" && canEdit}
            <button class="small" onclick={() => (app.page = "mods")}><Icon name="edit" size={14} />Open Mods</button>
          {/if}
        </div>
        {#if group.area === "editor"}
          <p class="blurb">
            {canEdit ? "Extracting, editing and packing levels is under Mods." : "Not connected for this version yet."}
          </p>
        {:else if group.blurb}
          <p class="blurb">{group.blurb}</p>
        {/if}
        {#if group.actions.length}
          <ul>
            {#each group.actions as action (action.id)}<ActionRow {action} {scope} />{/each}
          </ul>
        {/if}
      </div>
    {:else}
      <p class="muted">No actions for this version on this computer.</p>
    {/each}
    {#if elsewhere.length}
      <details>
        <summary>{elsewhere.length} more for other platforms</summary>
        <ul>
          {#each elsewhere as action (action.id)}<ActionRow {action} {scope} />{/each}
        </ul>
      </details>
    {/if}
  </section>

  <!-- A checkout's files: Android has none of them, and cannot show a folder of the launcher's. -->
  {#if !onAndroid}
    <section class="panel">
      <div class="row">
        <h3 class="grow">Files</h3>
        <button class="ghost small" onclick={() => void guard(api.openPath(v.dir))}
          ><Icon name="folder" size={14} />Open folder</button
        >
      </div>
      <dl class="files">
        <dt>Disc image</dt>
        <dd class="mono clip" title={v.disc?.file ?? ""}>
          {v.disc?.file ?? "–"}{#if v.disc?.size}<span class="dim after">· {size(v.disc.size)}</span>{/if}
        </dd>
        <dd class={`mark ${DISC[v.status.disc.state].tone}`} title={v.status.disc.message}>
          {v.status.disc.state === "found" ? "✓" : "!"}
          {DISC_SHORT[v.status.disc.state]}
        </dd>
        {#each v.status.inputs as input (input.path)}
          <dt>{INPUT[input.from] ?? input.from}</dt>
          <dd class="mono clip" title={input.path}>{input.path}</dd>
          <dd class={`mark ${input.present ? "ok" : "warn"}`} title={input.message}>
            {input.present ? "✓" : "!"}
            {input.message}
          </dd>
        {/each}
        <dt>Folder</dt>
        <dd class="clip">
          <button class="link mono" onclick={() => void guard(api.openPath(v.dir))}>{v.dir}</button>
        </dd>
        <dd></dd>
      </dl>
    </section>
  {/if}

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
</div>

<style>
  /* What follows a file name on its line, a space after it. */
  .after {
    margin-left: 0.35em;
  }

  .version {
    display: flex;
    flex-direction: column;
    gap: 16px;
  }

  .head {
    display: flex;
    align-items: flex-end;
    gap: 16px;
    margin-bottom: 14px;
  }

  h2 {
    margin: 0 0 8px;
  }

  .chips {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 6px;
  }

  .chip {
    display: inline-flex;
    align-items: center;
    background: var(--panel-hi);
    border: 1px solid var(--line-strong);
    border-radius: 7px;
    padding: 1px 8px;
    font-size: 0.75rem;
    color: var(--soft);
    white-space: nowrap;
  }

  .percent {
    font-family: var(--font-head);
    font-size: 1.75rem;
    font-weight: 700;
    line-height: 1;
    font-variant-numeric: tabular-nums;
    color: var(--gold);
    white-space: nowrap;
  }

  .count {
    margin: 8px 0 0;
    font-size: 0.8125rem;
    color: var(--dim);
    font-variant-numeric: tabular-nums;
  }

  .count.functional {
    opacity: 0.8;
  }

  .group + .group {
    margin-top: 18px;
  }

  h3 {
    margin: 0;
    font-family: var(--font-head);
    font-size: 0.75rem;
    font-weight: 600;
    letter-spacing: 0.14em;
    text-transform: uppercase;
    color: var(--dim);
  }

  .blurb {
    margin: 2px 0 8px;
    font-size: 0.8125rem;
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

  summary {
    cursor: pointer;
    font-weight: 600;
    color: var(--soft);
  }

  details:not(.about) {
    margin-top: 14px;
    padding-top: 10px;
    border-top: 1px solid var(--line);
  }

  details:not(.about) ul {
    margin-top: 8px;
  }

  /* Files: what, where, and whether it is there, in three columns. */
  .files {
    display: grid;
    grid-template-columns: 100px minmax(0, 1fr) auto;
    gap: 7px 14px;
    margin: 10px 0 0;
    font-size: 0.8125rem;
  }

  .files dd {
    margin: 0;
    min-width: 0;
  }

  .mark {
    white-space: nowrap;
    font-size: 0.7812rem;
  }

  dt {
    color: var(--dim);
  }

  .about dl {
    display: grid;
    grid-template-columns: 110px minmax(0, 1fr);
    gap: 6px 16px;
    margin: 12px 0 14px;
    font-size: 0.875rem;
  }

  .about dd {
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

  .err {
    color: var(--err);
  }

  .info {
    color: var(--info);
  }

  .link {
    border: 0;
    border-radius: 4px;
    background: none;
    padding: 0;
    color: var(--amber);
    font-weight: 600;
    text-align: left;
    max-width: 100%;
    overflow: hidden;
    text-overflow: ellipsis;
  }

  .link.mono {
    font-weight: 400;
  }
</style>
