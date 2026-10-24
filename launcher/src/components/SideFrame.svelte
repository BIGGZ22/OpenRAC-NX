<script lang="ts">
  // A tab with a list on the left (Games, Mods, Develop): the LIST, always there, and the open
  // page beside it, scrolling on its own. On Android the list is a row above the page instead: the
  // navigation rail is already on the left, and two columns there leave the page too narrow.
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

  /* The list stays put while the page beside it slides (app.css). */
  :global(html:not([data-platform="android"])) aside {
    view-transition-name: side;
  }

  @media (max-width: 1000px) {
    .frame {
      grid-template-columns: 76px minmax(0, 1fr);
    }
  }

  :global(html[data-platform="android"]) .frame {
    grid-template-columns: minmax(0, 1fr);
  }

  /* It scrolls sideways when the entries are wider than the page, by dragging (no bar on Android:
     app.css), and goes up with the page. */
  :global(html[data-platform="android"]) aside {
    position: static;
    height: auto;
    overflow: auto hidden;
  }
</style>
