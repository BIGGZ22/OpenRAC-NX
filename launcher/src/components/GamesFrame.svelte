<script lang="ts">
  // The Games and Mods tabs: the game list on the left (GameList, its props passed through: none
  // for the Games tab; Mods gives its own selection and states) and the open game beside it. On
  // Android the Games tab's game has no list: its tiles (Library.svelte) are how to switch.
  import { onAndroid, type GameView } from "$lib/api";
  import type { EntryState } from "$lib/labels";
  import type { Snippet } from "svelte";
  import GameList from "./GameList.svelte";
  import SideFrame from "./SideFrame.svelte";

  let {
    children,
    selected,
    onselect,
    state,
    label = "Games",
  }: {
    children: Snippet;
    selected?: string | null;
    onselect?: (game: GameView) => void;
    state?: (game: GameView) => EntryState | null;
    label?: string;
  } = $props();
</script>

{#if onAndroid && !onselect}
  {@render children()}
{:else}
  <SideFrame>
    {#snippet list()}<GameList {selected} {onselect} {state} {label} />{/snippet}
    {@render children()}
  </SideFrame>
{/if}
