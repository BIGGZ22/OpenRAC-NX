<script lang="ts">
  // Android: the game a page is for (Mods, Develop), at its top right, in the game's typeface and
  // marked with the launcher's accent (the game's own, themes.ts). Android has no game list beside
  // those pages: a tap goes to Games to choose another.
  import { app } from "$lib/app.svelte";
  import Icon from "./Icon.svelte";

  let { title, what }: { title: string; what: string } = $props();
</script>

<button
  class="game"
  onclick={() => (app.page = "library")}
  title={`The ${what} of this game. Choose another in Games.`}
  aria-label={`${title}: choose another game in Games`}
  ><span class="mark" aria-hidden="true"></span><span class="name">{title}</span><Icon
    name="library"
    size={16}
  /></button
>

<style>
  .game {
    display: inline-flex;
    align-items: center;
    gap: 10px;
    max-width: 100%;
    padding: 8px 14px 8px 12px;
    border-radius: 999px;
    border: 1px solid color-mix(in srgb, var(--amber) 55%, transparent);
    background: color-mix(in srgb, var(--amber) 12%, var(--panel));
    color: var(--text);
    transition:
      background var(--medium) var(--ease),
      border-color var(--medium) var(--ease);
  }

  .mark {
    flex: none;
    width: 10px;
    height: 10px;
    border-radius: 50%;
    background: linear-gradient(var(--bar-top), var(--bar-bottom));
    box-shadow: 0 0 10px color-mix(in srgb, var(--amber) 70%, transparent);
  }

  .name {
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    font-family: var(--title-font);
    font-weight: var(--title-weight);
    color: var(--tx);
  }
</style>
