// A mod's settings save as they change, without Enter or a Save button (ModPage.svelte). Each change
// waits a moment (a slider dragged, a word typed) and only its last value goes out; changes to
// several settings go out one after the other, in the order they settled, so the list the core
// answers with is never older than what the player sees. Until a value is saved the control shows
// it (held), and once it is, the core's list does. No Svelte here, so the timing can be tested.

export type SaveState = "idle" | "waiting" | "saving" | "saved" | "failed";

export interface SaverOptions<V> {
  /** Saves one setting's value; rejects with the reason it was refused. */
  save: (key: string, value: V) => Promise<void>;
  /** Called on every change of what is held or of the state, to draw it again. */
  onchange?: () => void;
  /** Called when a save is refused, with its key and reason. */
  onerror?: (key: string, reason: unknown) => void;
  /** Called when a key's last value is saved. */
  onsaved?: (key: string) => void;
}

export class Saver<V = unknown> {
  #opts: SaverOptions<V>;
  /** The value each setting shows until the core has it: waiting or being saved. */
  #held = new Map<string, V>();
  #timers = new Map<string, ReturnType<typeof setTimeout>>();
  /** The values waiting for their moment, by key. */
  #waiting = new Map<string, V>();
  #chain: Promise<void> = Promise.resolve();
  #inFlight = 0;
  #failed = false;
  #touched = false;

  constructor(opts: SaverOptions<V>) {
    this.#opts = opts;
  }

  /** What the saver is doing, for the page's "Saving… / Saved" line. */
  get state(): SaveState {
    if (this.#waiting.size) return "waiting";
    if (this.#inFlight) return "saving";
    if (this.#failed) return "failed";
    return this.#touched ? "saved" : "idle";
  }

  /** Whether a setting has a value not saved yet. */
  holds(key: string): boolean {
    return this.#held.has(key);
  }

  /** The value a setting shows: the one not saved yet, else FALLBACK (the core's). */
  value(key: string, fallback: V): V {
    return this.#held.has(key) ? (this.#held.get(key) as V) : fallback;
  }

  /** Every value not saved yet, by key. */
  held(): Record<string, V> {
    return Object.fromEntries(this.#held);
  }

  /** A setting changed: saved after DELAY ms of no further change to it (0: at once, in turn). */
  set(key: string, value: V, delay = 0): void {
    this.#held.set(key, value);
    this.#waiting.set(key, value);
    this.#failed = false;
    this.#touched = true;
    const timer = this.#timers.get(key);
    if (timer !== undefined) clearTimeout(timer);
    if (delay <= 0) {
      this.#timers.delete(key);
      this.#send(key);
    } else {
      this.#timers.set(
        key,
        setTimeout(() => {
          this.#timers.delete(key);
          this.#send(key);
        }, delay),
      );
    }
    this.#opts.onchange?.();
  }

  /** Sends every value still waiting now (leaving the page), and resolves once all are saved. */
  flush(): Promise<void> {
    for (const [key, timer] of this.#timers) {
      clearTimeout(timer);
      this.#timers.delete(key);
      this.#send(key);
    }
    return this.#chain;
  }

  /** Forgets what waits, unsaved (the mod was removed). */
  drop(): void {
    for (const timer of this.#timers.values()) clearTimeout(timer);
    this.#timers.clear();
    this.#waiting.clear();
    this.#held.clear();
    this.#opts.onchange?.();
  }

  #send(key: string): void {
    if (!this.#waiting.has(key)) return;
    const value = this.#waiting.get(key) as V;
    this.#waiting.delete(key);
    this.#inFlight += 1;
    this.#opts.onchange?.();
    this.#chain = this.#chain.then(async () => {
      try {
        await this.#opts.save(key, value);
        // Saved; shown from the core's list, unless the player has moved it on since.
        if (this.#held.get(key) === value && !this.#waiting.has(key)) {
          this.#held.delete(key);
          this.#opts.onsaved?.(key);
        }
      } catch (e) {
        this.#failed = true;
        if (this.#held.get(key) === value && !this.#waiting.has(key)) this.#held.delete(key);
        this.#opts.onerror?.(key, e);
      } finally {
        this.#inFlight -= 1;
        this.#opts.onchange?.();
      }
    });
  }
}
