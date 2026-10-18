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
