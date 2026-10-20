// An action (use:portal) that moves its element to the end of <body> while it is shown: a dialog
// then stacks above the header and everything else, wherever it is written (a panel with its own
// z-index, which the game page's saves and Help have, would otherwise keep it under the header).
// The game menu does the same with the browser's popover top layer (Menu.svelte).
export function portal(node: HTMLElement): { destroy: () => void } {
  document.body.appendChild(node);
  return {
    destroy() {
      node.remove();
    },
  };
}

/**
 * A modal dialog's overlay: moved under <body> like `portal`, the page behind made inert (so Tab
 * stays in the dialog and a screen reader reads only it), focus on the dialog, and focus back
 * where it was when it closes.
 */
export function modal(node: HTMLElement): { destroy: () => void } {
  const back = document.activeElement instanceof HTMLElement ? document.activeElement : null;
  document.body.appendChild(node);
  const page = document.getElementById("app");
  page?.setAttribute("inert", "");
  node.querySelector<HTMLElement>('[role="dialog"]')?.focus();
  return {
    destroy() {
      node.remove();
      page?.removeAttribute("inert");
      back?.focus();
    },
  };
}
