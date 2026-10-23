// Copying text for the player. Every Copy button (Activity, Develop's console, a step's command,
// the crash notice, the reports list) writes to the clipboard the same way and, when the browser
// refuses, says so in an error toast in its own words: what the player can do instead.
import { toast } from "./app.svelte";

/**
 * Puts `text` on the clipboard. True when it is there; false when the browser refused (no
 * clipboard, no permission), after `refused` has been shown as an error toast. A caller that
 * shows "Copied" does so only on true.
 */
export async function copyText(text: string, refused: string): Promise<boolean> {
  try {
    await navigator.clipboard.writeText(text);
    return true;
  } catch {
    toast(refused, "error");
    return false;
  }
}
