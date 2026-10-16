<script lang="ts">
  // The website's progress bar: the game's colours come from the --bd, --tx
  // and --tx2 custom properties of the element around it (themes.ts).
  // Functional C, when given, follows the matched part as a striped segment: code that behaves
  // like the original without matching its bytes, which ByteMatch shows apart and never counts.
  let { percent, label, functional = null }: { percent: number; label: string; functional?: number | null } = $props();
  const matched = $derived(Math.min(100, Math.max(0, percent)));
  const extra = $derived(functional === null ? 0 : Math.min(100 - matched, Math.max(0, functional)));
</script>

<div
  class="bar"
  role="progressbar"
  aria-label={functional === null ? label : `${label}; functional C ${Math.round(extra * 10) / 10}% more`}
  aria-valuenow={Math.round(percent * 10) / 10}
  aria-valuemin={0}
  aria-valuemax={100}
>
  <i style:width={`${matched}%`} title={`${Math.round(matched * 100) / 100}% matched`}></i>
  {#if extra > 0}
    <b
      class="functional"
      class:alone={matched === 0}
      style:width={`${extra}%`}
      title={`${Math.round(extra * 100) / 100}% functional C (not counted as matched)`}
    ></b>
  {/if}
</div>
