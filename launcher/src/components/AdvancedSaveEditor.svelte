<script lang="ts">
  // The save editor's advanced tab (Settings, "Advanced save editing"): every chunk of the save as
  // bytes. Fields whose meaning is decoded (core/src/save_edit.rs, chunk_name) get a control in a
  // player's words; flags and chunks that are not decoded are marked "raw" or "not decoded". Every
  // chunk can also be edited as hex, at its fixed size. Saving sends the changed chunks as raw
  // patches: no rules are applied for you here (giving an item does not stock the vendor, moving
  // the planet does not unlock it), which is the point for debugging. The checksums are written
  // fresh by the Rust side.
  import type { SaveView } from "$lib/api";
  import { bytesToHex, chunkKey, hexToBytes, readI32, writeI32 } from "$lib/saveEdit";

  let {
    view,
    bytes = $bindable({}),
    oninvalid,
  }: {
    view: SaveView;
    bytes?: Record<string, number[]>;
    /** How many hex boxes hold something that is not valid bytes of the chunk's size. */
    oninvalid: (count: number) => void;
  } = $props();

  const g = (id: number) => chunkKey(null, id);
  const lv = (level: number, id: number) => chunkKey(level, id);

  // ---- names ----------------------------------------------------------------------
  const extraItems: Record<number, string> = { 1: "Clank", 8: "Wrench" };
  function itemName(id: number): string {
    return view.items.find((i) => i.id === id)?.name ?? extraItems[id] ?? `Item ${String(id)} (not decoded)`;
  }
  const itemIds = Array.from({ length: 37 }, (_, i) => i);
  function levelName(id: number): string {
    const l = view.levels.find((x) => x.id === id);
    return l ? `${l.name} · ${l.location}` : `Level ${String(id)} (unused)`;
  }
  const levelIds = Array.from({ length: 20 }, (_, i) => i);
  function goldTotal(level: number): number {
    return view.fields.planets.find((p) => p.id === level)?.goldBolts.length ?? 0;
  }
  const knownFlags: Record<number, string> = {
    4: "Premium Nanotech",
    5: "Ultra Nanotech",
    8: "Clank met on Veldin",
  };

  // ---- reading and writing bytes ----------------------------------------------------
  function i32(key: string, i = 0): number {
    return readI32(bytes[key] ?? [], i);
  }
  function setI32(key: string, i: number, text: string) {
    const n = Number(text);
    const b = bytes[key];
    if (!b || text.trim() === "" || !Number.isInteger(n) || n < -2147483648 || n > 2147483647) return;
    writeI32(b, i, n);
  }
  function u8(key: string, i = 0): number {
    return bytes[key]?.[i] ?? 0;
  }
  function setU8(key: string, i: number, v: number) {
    const b = bytes[key];
    if (b && i < b.length) b[i] = v & 0xff;
  }

  /** Whole numbers of the global section, by chunk: label and what it holds. */
  const numbers: [number, string][] = [
    [1, "Bolts"],
    [19, "Max health (4, 5 Premium, 8 Ultra)"],
    [2, "Times completed (challenge mode)"],
    [31, "Game beaten (0 or 1)"],
    [3, "Time played (1/60 s)"],
    [1003, "Play time (ticks)"],
    [1004, "Hits taken"],
    [1005, "Deaths"],
    [21, "Last item in hand (item number)"],
    [22, "Wrench in hand (0 or 1)"],
    [23, "Thruster-Pack on the back (0 or 1)"],
    [25, "Camera left/right (1 normal, 0 reversed)"],
    [26, "Camera up/down (1 normal, 0 reversed)"],
    [27, "Camera speed (0 slow to 2 fast)"],
    [34, "Stereo (1) or mono (0)"],
    [35, "Music volume (0 to 1024)"],
    [36, "Effects volume (0 to 1024)"],
  ];
  const bytesOne: [number, string][] = [
    [28, "HelpDesk voice (0 or 1)"],
    [29, "HelpDesk text (0 or 1)"],
    [33, "Subtitles (0 or 1)"],
  ];
  const equippedSlots = ["Hand", "Feet", "Head", "Back", "Unused 5", "Unused 6", "Unused 7"];

  // ---- the raw view ---------------------------------------------------------------
  let rawSection = $state("g");
  let drafts = $state<Record<string, string>>({});
  const rawChunks = $derived(
    view.chunks.filter((c) => (rawSection === "g" ? c.section === null : c.section === Number(rawSection))),
  );

  function draftError(key: string, size: number): string | null {
    const d = drafts[key];
    if (d === undefined) return null;
    const b = hexToBytes(d);
    if (typeof b === "string") return b;
    if (b.length !== size) return `${String(b.length)} bytes; this part is ${String(size)}`;
    return null;
  }

  $effect(() => {
    oninvalid(view.chunks.filter((c) => draftError(chunkKey(c.section, c.id), c.size) !== null).length);
  });

  function onHex(key: string, size: number, text: string) {
    drafts[key] = text;
    const b = hexToBytes(text);
    if (typeof b !== "string" && b.length === size) bytes[key] = b;
  }

  function onHexDone(key: string, size: number) {
    if (draftError(key, size) === null) drafts = Object.fromEntries(Object.entries(drafts).filter(([k]) => k !== key));
  }
</script>

<div class="adv">
  <p class="warn-line">
    Every part of the save, for debugging. Nothing here is checked against the story: a wrong value can leave the game
    stuck or the save unloadable. Your memory card is backed up before saving, and Undo puts it back.
  </p>

  <details open>
    <summary>Game</summary>
    <div class="grid">
      <label class="cell">
        <span>Planet (where the save loads)</span>
        <select
          value={i32(g(0))}
          onchange={(e) => {
            setI32(g(0), 0, e.currentTarget.value);
          }}
        >
          {#each levelIds.slice(0, 19) as id (id)}
            <option value={id}
              >{String(id)} · {levelName(id)}{view.levels.find((l) => l.id === id)?.canLand ? "" : " (story)"}</option
            >
          {/each}
        </select>
      </label>
      {#each numbers as [id, label] (id)}
        <label class="cell">
          <span>{label}</span>
          <input
            type="number"
            step="1"
            value={i32(g(id))}
            onchange={(e) => {
              setI32(g(id), 0, e.currentTarget.value);
            }}
          />
        </label>
      {/each}
      {#each bytesOne as [id, label] (id)}
        <label class="cell">
          <span>{label}</span>
          <input
            type="number"
            min="0"
            max="255"
            step="1"
            value={u8(g(id))}
            onchange={(e) => {
              setU8(g(id), 0, Number(e.currentTarget.value));
            }}
          />
        </label>
      {/each}
    </div>
  </details>

  <details>
    <summary>Items</summary>
    <table>
      <thead>
        <tr><th>#</th><th>Item</th><th>Owned</th><th>Ever had</th><th>Gold</th><th>Ammo</th></tr>
      </thead>
      <tbody>
        {#each itemIds as id (id)}
          <tr>
            <td class="dim">{id}</td>
            <td class:unknown={itemName(id).includes("not decoded")}>{itemName(id)}</td>
            <td
              ><input
                type="checkbox"
                aria-label={`${itemName(id)} owned`}
                checked={u8(g(10), id) !== 0}
                onchange={(e) => {
                  setU8(g(10), id, e.currentTarget.checked ? 1 : 0);
                }}
              /></td
            >
            <td
              ><input
                type="checkbox"
                aria-label={`${itemName(id)} ever had`}
                checked={u8(g(11), id) !== 0}
                onchange={(e) => {
                  setU8(g(11), id, e.currentTarget.checked ? 1 : 0);
                }}
              /></td
            >
            <td
              ><input
                type="checkbox"
                aria-label={`${itemName(id)} gold`}
                checked={u8(g(30), id) !== 0}
                onchange={(e) => {
                  setU8(g(30), id, e.currentTarget.checked ? 1 : 0);
                }}
              /></td
            >
            <td
              ><input
                type="number"
                step="1"
                aria-label={`${itemName(id)} ammo`}
                value={i32(g(9), id)}
                onchange={(e) => {
                  setI32(g(9), id, e.currentTarget.value);
                }}
              /></td
            >
          </tr>
        {/each}
      </tbody>
    </table>
    <p class="note">Gold flags 37 to 39 and the ammo statistics (bought, picked up, used) are in the raw view.</p>
  </details>

  <details>
    <summary>Quick select, equipped and vendor stock</summary>
    <h4>Quick select</h4>
    <div class="grid">
      {#each Array.from({ length: 8 }, (_, i) => i) as i (i)}
        <label class="cell">
          <span>Slot {i + 1}</span>
          <select
            value={i32(g(13), i)}
            onchange={(e) => {
              setI32(g(13), i, e.currentTarget.value);
            }}
          >
            <option value={0}>Empty</option>
            {#each itemIds.slice(1) as id (id)}<option value={id}>{itemName(id)}</option>{/each}
          </select>
        </label>
      {/each}
    </div>
    <h4>Equipped</h4>
    <div class="grid">
      {#each equippedSlots as slotName, i (i)}
        <label class="cell">
          <span>{slotName}{i >= 4 ? " (raw)" : ""}</span>
          <select
            value={i32(g(32), i)}
            onchange={(e) => {
              setI32(g(32), i, e.currentTarget.value);
            }}
          >
            <option value={0}>Nothing</option>
            {#each itemIds.slice(1) as id (id)}<option value={id}>{itemName(id)}</option>{/each}
          </select>
        </label>
      {/each}
    </div>
    <h4>Vendor stock</h4>
    <div class="grid">
      {#each Array.from({ length: 12 }, (_, i) => i) as i (i)}
        {@const b = u8(g(12), i)}
        <div class="cell">
          <span>Place {i + 1}</span>
          <div class="row tight">
            <select
              aria-label={`Vendor place ${String(i + 1)}`}
              value={b === 0xff ? -1 : b & 0x3f}
              onchange={(e) => {
                const v = Number(e.currentTarget.value);
                setU8(g(12), i, v < 0 ? 0xff : (b === 0xff ? 0 : b & 0xc0) | v);
              }}
            >
              <option value={-1}>Empty</option>
              {#each itemIds as id (id)}<option value={id}>{itemName(id)}</option>{/each}
            </select>
            <label class="tiny"
              ><input
                type="checkbox"
                disabled={b === 0xff}
                checked={b !== 0xff && (b & 0x40) !== 0}
                onchange={(e) => {
                  setU8(g(12), i, e.currentTarget.checked ? b | 0x40 : b & ~0x40);
                }}
              />Owned</label
            >
          </div>
        </div>
      {/each}
    </div>
  </details>

  <details>
    <summary>Skill points and flags</summary>
    <h4>Skill points <span class="dim">(31 and 32 are not used by the game)</span></h4>
    <div class="toggles">
      {#each Array.from({ length: 32 }, (_, i) => i) as i (i)}
        <button
          class="t"
          class:on={u8(g(8), i) !== 0}
          class:unused={i >= 30}
          aria-pressed={u8(g(8), i) !== 0}
          aria-label={`Skill point ${String(i + 1)}`}
          onclick={() => {
            setU8(g(8), i, u8(g(8), i) ? 0 : 1);
          }}>{i + 1}</button
        >
      {/each}
    </div>
    <h4>Story flags <span class="dim">(raw: only 4, 5 and 8 are decoded)</span></h4>
    <div class="toggles">
      {#each Array.from({ length: 128 }, (_, i) => i) as i (i)}
        <button
          class="t"
          class:on={u8(g(5), i) !== 0}
          class:known={i in knownFlags}
          aria-pressed={u8(g(5), i) !== 0}
          title={knownFlags[i] ?? "Not decoded"}
          aria-label={`Story flag ${String(i)}${knownFlags[i] ? `: ${knownFlags[i]}` : ""}`}
          onclick={() => {
            setU8(g(5), i, u8(g(5), i) ? 0 : 1);
          }}>{i}</button
        >
      {/each}
    </div>
    <p class="note">
      {#each Object.entries(knownFlags) as [i, name] (i)}<span class="legend">{i}: {name}</span>{/each}
    </p>
    {#each [[7, "Cheats on"], [37, "Cheats ever used"]] as [id, label] (id)}
      <h4>{label} <span class="dim">(raw: which cheat each is is not decoded)</span></h4>
      <div class="toggles">
        {#each Array.from({ length: 12 }, (_, i) => i) as i (i)}
          <button
            class="t"
            class:on={u8(g(Number(id)), i) !== 0}
            aria-pressed={u8(g(Number(id)), i) !== 0}
            aria-label={`${String(label)} ${String(i)}`}
            onclick={() => {
              setU8(g(Number(id)), i, u8(g(Number(id)), i) ? 0 : 1);
            }}>{i}</button
          >
        {/each}
      </div>
    {/each}
  </details>

  <details>
    <summary>Planets</summary>
    <h4>Galaxy map order <span class="dim">(as unlocked; unused places hold 0)</span></h4>
    <div class="grid">
      {#each levelIds as i (i)}
        <label class="cell">
          <span>Place {i + 1}</span>
          <select
            value={i32(g(20), i)}
            onchange={(e) => {
              setI32(g(20), i, e.currentTarget.value);
            }}
          >
            {#each levelIds as id (id)}<option value={id}>{String(id)} · {levelName(id)}</option>{/each}
          </select>
        </label>
      {/each}
    </div>
    {#each levelIds as l (l)}
      <details class="planet">
        <summary>{String(l)} · {levelName(l)}</summary>
        <div class="grid">
          <label class="cell check">
            <input
              type="checkbox"
              checked={u8(g(14), l) !== 0}
              onchange={(e) => {
                setU8(g(14), l, e.currentTarget.checked ? 1 : 0);
              }}
            />
            <span>Unlocked for the ship</span>
          </label>
          <label class="cell">
            <span>Visited</span>
            <select
              value={u8(lv(l, 3001))}
              onchange={(e) => {
                setU8(lv(l, 3001), 0, Number(e.currentTarget.value));
              }}
            >
              <option value={0}>0 · Never</option>
              <option value={1}>1 · Visited</option>
              <option value={2}>2 · Left (completed)</option>
              {#if u8(lv(l, 3001)) > 2}<option value={u8(lv(l, 3001))}>{u8(lv(l, 3001))} · raw</option>{/if}
            </select>
          </label>
          {#each [[4000, "Bolts collected here"], [4001, "Hits taken here"], [4002, "Deaths here"]] as [id, label] (id)}
            <label class="cell">
              <span>{label}</span>
              <input
                type="number"
                step="1"
                value={i32(lv(l, Number(id)))}
                onchange={(e) => {
                  setI32(lv(l, Number(id)), 0, e.currentTarget.value);
                }}
              />
            </label>
          {/each}
        </div>
        <h4>Gold bolts <span class="dim">({goldTotal(l)} on this planet; the others are not used)</span></h4>
        <div class="toggles">
          {#each [0, 1, 2, 3] as k (k)}
            <button
              class="t"
              class:on={u8(lv(l, 3003), k) !== 0}
              class:unused={k >= goldTotal(l)}
              aria-pressed={u8(lv(l, 3003), k) !== 0}
              aria-label={`Gold bolt ${String(k + 1)}`}
              onclick={() => {
                setU8(lv(l, 3003), k, u8(lv(l, 3003), k) ? 0 : 1);
              }}>{k + 1}</button
            >
          {/each}
        </div>
        <h4>Missions <span class="dim">(on: done, 0xff; what each is is not decoded)</span></h4>
        <div class="toggles">
          {#each Array.from({ length: 16 }, (_, i) => i) as k (k)}
            <button
              class="t"
              class:on={u8(lv(l, 3004), k) === 0xff}
              aria-pressed={u8(lv(l, 3004), k) === 0xff}
              aria-label={`Mission ${String(k)}`}
              onclick={() => {
                setU8(lv(l, 3004), k, u8(lv(l, 3004), k) === 0xff ? 0 : 0xff);
              }}>{k}</button
            >
          {/each}
        </div>
        <p class="note">
          Map, enemies and crates destroyed, bolts from crates, arrivals and Metal Detector spots: raw view.
        </p>
      </details>
    {/each}
  </details>

  <details>
    <summary>Raw view: every part of the file</summary>
    <div class="row tight">
      <label for="raw-section">Section</label>
      <select id="raw-section" bind:value={rawSection}>
        <option value="g">Global</option>
        {#each levelIds as l (l)}<option value={String(l)}>{String(l)} · {levelName(l)}</option>{/each}
      </select>
    </div>
    <p class="note">
      Each part keeps its size; the checksums are written fresh when you save. "Raw" parts are not decoded.
    </p>
    {#each rawChunks as c (chunkKey(c.section, c.id))}
      {@const key = chunkKey(c.section, c.id)}
      {@const err = draftError(key, c.size)}
      <details class="chunk">
        <summary>
          <span class="mono">{c.id}</span>
          <span class="grow">{c.name}</span>
          <span class="dim">{c.size} bytes</span>
          <span class="tag" class:raw={!c.known}>{c.known ? "decoded" : "raw"}</span>
        </summary>
        <textarea
          class="mono"
          spellcheck="false"
          aria-label={`Chunk ${String(c.id)} bytes`}
          aria-invalid={!!err}
          rows={Math.min(12, Math.max(1, Math.ceil(c.size / 24)))}
          value={drafts[key] ?? bytesToHex(bytes[key] ?? [])}
          oninput={(e) => {
            onHex(key, c.size, e.currentTarget.value);
          }}
          onblur={() => {
            onHexDone(key, c.size);
          }}></textarea>
        {#if err}<p class="err-text">{err}</p>{/if}
      </details>
    {/each}
  </details>
</div>

<style>
  .adv {
    display: flex;
    flex-direction: column;
  }

  .warn-line {
    margin: 14px 0 0;
    font-size: 0.8125rem;
    color: var(--amber);
  }

  details {
    border-top: 1px solid var(--line);
    padding: 6px 2px;
  }

  .adv > details:last-child {
    border-bottom: 1px solid var(--line);
  }

  summary {
    cursor: pointer;
    font-family: var(--font-head);
    font-size: 0.875rem;
    color: var(--gold);
    padding: 4px 0;
  }

  details.planet summary,
  details.chunk summary {
    font-family: var(--font-sans);
    font-size: 0.8125rem;
    color: var(--text);
  }

  details.chunk summary {
    display: flex;
    gap: 10px;
    align-items: center;
  }

  h4 {
    margin: 12px 0 6px;
    font-size: 0.8125rem;
    color: var(--lav);
  }

  .grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
    gap: 8px 14px;
    margin-top: 6px;
  }

  .cell {
    display: flex;
    flex-direction: column;
    gap: 4px;
    font-size: 0.75rem;
    color: var(--soft);
  }

  .cell.check {
    flex-direction: row;
    align-items: center;
    gap: 8px;
  }

  .row.tight {
    gap: 8px;
    align-items: center;
    font-size: 0.8125rem;
  }

  .tiny {
    display: inline-flex;
    gap: 5px;
    align-items: center;
    font-size: 0.75rem;
  }

  input[type="number"],
  select,
  textarea {
    font: inherit;
    font-size: 0.8125rem;
    background: var(--ink-deep);
    color: var(--text);
    border: 1px solid var(--line-strong);
    border-radius: var(--radius-sm);
    padding: 5px 8px;
    min-width: 0;
    max-width: 100%;
  }

  textarea {
    width: 100%;
    font-family: var(--font-mono);
    font-size: 0.75rem;
    resize: vertical;
  }

  textarea[aria-invalid="true"] {
    border-color: var(--err);
  }

  input[type="checkbox"] {
    accent-color: var(--amber);
  }

  table {
    width: 100%;
    border-collapse: collapse;
    font-size: 0.8125rem;
  }

  th {
    text-align: left;
    font-weight: 500;
    color: var(--soft);
    padding: 4px;
  }

  td {
    border-top: 1px solid var(--line);
    padding: 3px 4px;
  }

  td input[type="number"] {
    width: 90px;
  }

  .unknown {
    color: var(--dim);
    font-style: italic;
  }

  .toggles {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(34px, 1fr));
    gap: 5px;
  }

  .t {
    padding: 0;
    height: 26px;
    justify-content: center;
    font-size: 0.75rem;
    font-variant-numeric: tabular-nums;
    color: var(--dim);
  }

  .t.known {
    border-color: var(--soft);
    color: var(--soft);
  }

  .t.unused {
    border-style: dashed;
  }

  .t.on {
    color: var(--bar-ink-deep);
    background: var(--amber);
    border-color: var(--amber);
  }

  .note {
    margin: 6px 0 0;
    font-size: 0.75rem;
    color: var(--dim);
  }

  .legend {
    margin-right: 14px;
  }

  .tag {
    font-size: 0.75rem;
    border: 1px solid var(--ok);
    color: var(--ok);
    border-radius: 999px;
    padding: 0 8px;
  }

  .tag.raw {
    border-color: var(--amber);
    color: var(--amber);
  }

  .err-text {
    margin: 4px 0 0;
    font-size: 0.75rem;
    color: var(--err);
  }
</style>
