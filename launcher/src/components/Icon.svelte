<script lang="ts" module>
  // Line icons on a 24-unit grid, drawn with the current colour.
  const PATHS = {
    play: "M7 4.5v15l12-7.5z",
    build: "M14.5 5.5a4 4 0 0 0-5.4 5.1L4 15.7 8.3 20l5.1-5.1a4 4 0 0 0 5.1-5.4l-2.6 2.6-2.4-.6-.6-2.4z",
    check: "M5 12.5l4.5 4.5L19 7.5",
    setup: "M12 3v4M12 17v4M3 12h4M17 12h4M5.6 5.6l2.8 2.8M15.6 15.6l2.8 2.8M5.6 18.4l2.8-2.8M15.6 8.4l2.8-2.8",
    edit: "M4 20h4L19 9l-4-4L4 16zM13.5 6.5l4 4",
    disc: "M12 3a9 9 0 1 0 0 18 9 9 0 0 0 0-18zM12 9.5a2.5 2.5 0 1 0 0 5 2.5 2.5 0 0 0 0-5z",
    folder:
      "M3 6.5A1.5 1.5 0 0 1 4.5 5H9l2 2.5h8.5A1.5 1.5 0 0 1 21 9v8.5a1.5 1.5 0 0 1-1.5 1.5h-15A1.5 1.5 0 0 1 3 17.5z",
    gear: "M12 8.5a3.5 3.5 0 1 0 0 7 3.5 3.5 0 0 0 0-7zM19 12l2-1.2-1.6-3.6-2.3.4-1.4-1.4.4-2.3L12.5 2.3 11.3 4.3h-.6L9.5 2.3 5.9 3.9l.4 2.3-1.4 1.4-2.3-.4L1 10.8 3 12l-2 1.2 1.6 3.6 2.3-.4 1.4 1.4-.4 2.3 3.6 1.6 1.2-2h.6l1.2 2 3.6-1.6-.4-2.3 1.4-1.4 2.3.4 1.6-3.6z",
    terminal: "M4 5h16v14H4zM7.5 9.5l3 2.5-3 2.5M12.5 15h4",
    book: "M5 4.5h5.5A2.5 2.5 0 0 1 13 7v13a2 2 0 0 0-2-2H5zM19 4.5h-3.5A2.5 2.5 0 0 0 13 7v13a2 2 0 0 1 2-2h4z",
    external: "M14 4h6v6M20 4l-9 9M18 14v5H5V6h5",
    x: "M6 6l12 12M18 6L6 18",
    refresh: "M20 11a8 8 0 0 0-14.7-4.3L4 8.5M4 4v4.5h4.5M4 13a8 8 0 0 0 14.7 4.3l1.3-1.8M20 20v-4.5h-4.5",
    library: "M4 4h4v16H4zM10 4h4v16h-4zM15.5 5.2l3.8-1 4 15.4-3.8 1z",
    arrow: "M14 6l-6 6 6 6",
    next: "M10 6l6 6-6 6",
    reset: "M4.5 12a7.5 7.5 0 1 0 2.2-5.3L4.5 9M4.5 4.5V9H9",
    up: "M6 15l6-6 6 6",
    down: "M6 9l6 6 6-6",
    warn: "M12 4l9 16H3zM12 10v4.5M12 17.2v.3",
    info: "M12 3a9 9 0 1 0 0 18 9 9 0 0 0 0-18zM12 11v6M12 7.6v.3",
    save: "M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2zM17 21v-8H7v8M7 3v5h8",
    bolt: "M7 3h10v4H7zm2 4v14h6V7zM9 11h6M9 15h6M9 19h6",
    globe:
      "M12 2a10 10 0 1 0 10 10A10 10 0 0 0 12 2zm0 0a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10zm-8 10h16",
    more: "M5 11a1 1 0 1 0 0 2 1 1 0 0 0 0-2zM12 11a1 1 0 1 0 0 2 1 1 0 0 0 0-2zM19 11a1 1 0 1 0 0 2 1 1 0 0 0 0-2z",
    trash: "M4 7h16M9 7V4.5h6V7M6.5 7l1 12.5h9l1-12.5M10 11v5M14 11v5",
  } as const;
  export type IconName = keyof typeof PATHS;
</script>

<script lang="ts">
  let { name, size = 18 }: { name: IconName; size?: number } = $props();
</script>

<svg
  width={size}
  height={size}
  viewBox="0 0 24 24"
  fill={name === "play" ? "currentColor" : "none"}
  stroke="currentColor"
  stroke-width="1.8"
  stroke-linecap="round"
  stroke-linejoin="round"
  aria-hidden="true"
>
  <path d={PATHS[name]} />
</svg>
