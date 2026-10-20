// An action (use:tablist) for a row of tabs, as a tab list is used from the keyboard: Tab reaches
// the open tab only (the others have tabindex -1), and the arrows, Home and End move to another
// and open it. A tab that cannot be opened now (aria-disabled) is passed over.
export function tablist(node: HTMLElement): { destroy: () => void } {
  const key = (e: KeyboardEvent) => {
    const tabs = [...node.querySelectorAll<HTMLElement>('[role="tab"]')].filter(
      (t) => t.getAttribute("aria-disabled") !== "true",
    );
    const at = tabs.indexOf(document.activeElement as HTMLElement);
    if (at < 0 || !tabs.length) return;
    const n = tabs.length;
    const to =
      e.key === "ArrowRight"
        ? (at + 1) % n
        : e.key === "ArrowLeft"
          ? (at - 1 + n) % n
          : e.key === "Home"
            ? 0
            : e.key === "End"
              ? n - 1
              : null;
    if (to === null) return;
    e.preventDefault();
    tabs[to]?.focus();
    tabs[to]?.click();
  };
  node.addEventListener("keydown", key);
  return {
    destroy() {
      node.removeEventListener("keydown", key);
    },
  };
}
