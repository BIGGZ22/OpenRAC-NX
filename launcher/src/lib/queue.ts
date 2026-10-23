// The job queue's rules, without Svelte or Tauri so they can be tested:
// jobs run one at a time in the order they were asked for (two builds never
// share the build container or a game's folder), and the Rust side's events
// are matched to jobs by id, including events that arrive before the call
// that started the job has returned.
import type { JobEvent, Scope } from "./api";

export type JobState = "queued" | "running" | "succeeded" | "failed" | "cancelled";

export interface Line {
  stream: "stdout" | "stderr";
  line: string;
}

export interface Job {
  /** The launcher's own number for the job. */
  key: number;
  scope: Scope;
  actionId: string;
  title: string;
  state: JobState;
  /** The Rust side's job id, once started. */
  id: number | null;
  command: string;
  lines: Line[];
  code: number | null;
  queuedAt: number;
  startedAt: number | null;
  endedAt: number | null;
  /** The folders it wrote, from its exit event: what "Open folder" opens (api.openOutput). */
  outputs: string[];
  /** What it was asked with (a level, a file), as one string: the same action with other inputs
   *  is another job. Empty when it was asked with nothing. */
  inputs: string;
}

/** Actions that are one thing to the player: a second of any of them while one is queued or
 *  running is the same request again (the game, and the game with the player's mods, are the
 *  same program writing the same memory card). */
export type ActionGroups = readonly (readonly string[])[];

const sameScope = (a: Scope, b: Scope) =>
  a.kind === b.kind && (a.kind === "repository" || (b.kind === "version" && a.key === b.key));

/** Older lines are dropped past this many, so a chatty build cannot exhaust memory. */
export const MAX_LINES = 5000;

export class Queue {
  /**
   * The jobs, oldest first. The app passes a Svelte $state array, so every
   * change below must go through it (push, splice, its elements), never
   * replace it or hold on to an object from before it was added.
   */
  jobs: Job[];
  private nextKey = 0;
  /** Events for ids no job has yet (the start call has not returned). */
  private early = new Map<number, JobEvent[]>();
  /** The jobs that ended, waiting to be reported (see takeEnded). */
  private ended: Job[] = [];

  constructor(jobs: Job[] = []) {
    this.jobs = jobs;
  }

  add(scope: Scope, actionId: string, title: string, now = Date.now(), inputs = ""): Job {
    const job: Job = {
      key: ++this.nextKey,
      scope,
      actionId,
      title,
      state: "queued",
      id: null,
      command: "",
      lines: [],
      code: null,
      queuedAt: now,
      startedAt: null,
      endedAt: null,
      outputs: [],
      inputs,
    };
    this.jobs.push(job);
    // The array's own element: a reactive proxy when the array is $state.
    return this.jobs.at(-1) ?? job;
  }

  /** The queued or running job that asking for ACTION_ID in SCOPE with INPUTS would repeat: the
   *  same action with the same inputs, or any action of its group. A second click on a button
   *  finds the first one's job here and starts nothing. */
  live(scope: Scope, actionId: string, inputs = "", groups: ActionGroups = []): Job | undefined {
    const group = groups.find((g) => g.includes(actionId));
    return this.jobs.find(
      (j) =>
        (j.state === "queued" || j.state === "running") &&
        sameScope(j.scope, scope) &&
        (group ? group.includes(j.actionId) : j.actionId === actionId && j.inputs === inputs),
    );
  }

  running(): Job | undefined {
    return this.jobs.find((j) => j.state === "running");
  }

  /** The job to start now: the oldest queued one, if nothing runs. */
  next(): Job | undefined {
    if (this.running()) return undefined;
    return this.jobs.find((j) => j.state === "queued");
  }

  /** Jobs not finished: running and queued. */
  pending(): number {
    return this.jobs.filter((j) => j.state === "queued" || j.state === "running").length;
  }

  started(job: Job, id: number, command: string, now = Date.now()) {
    job.state = "running";
    job.id = id;
    job.command = command;
    job.startedAt = now;
    for (const event of this.early.get(id) ?? []) this.apply(event, now);
    this.early.delete(id);
  }

  /** A detached start (Android's game, an activity of its own): there is no process to follow, so
   *  the job is done once it started; it must not hold the queue up nor offer a stop that cannot
   *  stop anything. */
  launched(job: Job, command: string, now = Date.now()) {
    job.state = "succeeded";
    job.command = command;
    job.startedAt = now;
    job.endedAt = now;
  }

  /** The start call failed: the job never ran. */
  refused(job: Job, error: string, now = Date.now()) {
    job.state = "failed";
    job.lines.push({ stream: "stderr", line: error });
    job.endedAt = now;
  }

  /** A queued job is dropped; a running one is the Rust side's to stop. */
  dequeue(job: Job, now = Date.now()) {
    if (job.state !== "queued") return;
    job.state = "cancelled";
    job.endedAt = now;
  }

  apply(event: JobEvent, now = Date.now()) {
    const job = this.jobs.find((j) => j.id === event.id);
    if (!job) {
      this.early.set(event.id, [...(this.early.get(event.id) ?? []), event]);
      return;
    }
    if (event.type === "output") {
      job.lines.push({ stream: event.stream, line: event.line });
      if (job.lines.length > MAX_LINES) job.lines.splice(0, job.lines.length - MAX_LINES);
      return;
    }
    job.code = event.code;
    job.outputs = event.outputs;
    job.endedAt = now;
    job.state = event.cancelled ? "cancelled" : event.code === 0 ? "succeeded" : "failed";
    this.ended.push(job);
  }

  /** Forgets finished jobs. */
  clear() {
    for (let i = this.jobs.length - 1; i >= 0; i--) {
      const state = this.jobs[i]?.state;
      if (state !== "queued" && state !== "running") this.jobs.splice(i, 1);
    }
    this.ended = this.ended.filter((job) => this.jobs.includes(job));
  }

  /**
   * The job that ended, if one has and the app has not taken it yet.
   *
   * A job's end is reported here, and only here, so that it is acted on once
   * whether its exit event came before the start call returned (the event was
   * kept until the id was known) or after it.
   */
  takeEnded(): Job | undefined {
    return this.ended.shift();
  }
}
