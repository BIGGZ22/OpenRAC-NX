// What the job lists say about a job, the same in Activity (pages/Tasks.svelte) and Develop's
// console (components/DevConsole.svelte): how long it ran, and its output as one text to copy.
// Plain functions of a job, without Svelte, so they can be tested; the clock is the caller's.
import { elapsed } from "./format";
import type { Job } from "./queue";

/** How long a job ran, or has been running at `now` (ms): "2 min 05 s". Empty before it starts. */
export function jobDuration(job: Pick<Job, "startedAt" | "endedAt">, now: number): string {
  if (!job.startedAt) return "";
  return elapsed((job.endedAt ?? now) - job.startedAt);
}

/** A job's command and everything it printed, one line each, as Copy puts it on the clipboard. */
export function jobTranscript(job: Pick<Job, "command" | "lines">): string {
  return [`$ ${job.command}`, ...job.lines.map((l) => l.line)].join("\n");
}
