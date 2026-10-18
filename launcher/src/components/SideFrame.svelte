<script lang="ts">
  // A tab with a list on the left (Games, Mods, Develop): the LIST, always there, and the open
  // page beside it, scrolling on its own.
  import type { Snippet } from "svelte";

  let { list, children }: { list: Snippet; children: Snippet } = $props();
</script>

<div class="frame">
  <aside>{@render list()}</aside>
  <div class="content">{@render children()}</div>
</div>

<style>
  .frame {
    display: grid;
    grid-template-columns: 248px minmax(0, 1fr);
    min-height: 100%;
  }

  aside {
    position: sticky;
    top: 0;
    align-self: start;
    height: calc(100vh - 60px);
    overflow-y: auto;
  }

  .content {
    min-width: 0;
  }

  @media (max-width: 1000px) {
    .frame {
      grid-template-columns: 76px minmax(0, 1fr);
    }
  }
</style>
