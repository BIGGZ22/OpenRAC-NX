<script lang="ts">
  // A number that counts up to VALUE when it first shows and eases to a new one, as the site's
  // progress figures do; FORMAT writes it (pct for "92,32 %"). No motion with reduced motion on.
  import { reduced } from "$lib/motion";
  import { cubicOut } from "svelte/easing";
  import { Tween } from "svelte/motion";

  let { value, format }: { value: number; format: (n: number) => string } = $props();

  const shown = new Tween(0, { duration: reduced ? 0 : 900, easing: cubicOut });
  // Only a new figure moves it: the library is read again on every focus of the window, and a
  // tween to the same figure would redraw it for 900 ms each time.
  let last: number | undefined;
  $effect(() => {
    if (value === last) return;
    last = value;
    shown.target = value;
  });
</script>

<span class="count">{format(shown.current)}</span>

<style>
  .count {
    font-variant-numeric: tabular-nums;
  }
</style>
