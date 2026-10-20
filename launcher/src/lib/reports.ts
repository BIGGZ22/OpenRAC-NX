// How the Play page talks about a game's logs and crash reports (core/src/reports.rs): in a
// player's words, with the port's own words kept for the Details. No Svelte or Tauri here, so it
// can be tested.
import type { ReportEntry } from "./api";
import type { Line } from "./queue";

/** The game stopped while being played: the notice on the Play page. */
export interface StopNotice {
  /** The game (`rac1`) and the version played (`rac1/pal`). */
  game: string;
  versionKey: string;
  /** Its exit status; null when the system stopped it (a signal). */
  code: number | null;
  /** When it stopped, milliseconds since 1970. */
  at: number;
  /** The crash report it wrote, when it wrote one. */
  crash: ReportEntry | null;
  /** The game's last lines of output, for the Details when there is no report. */
  lines: string[];
  /** The game was played again since, and ended well: the notice is a line now. */
  playedSince?: boolean;
}

/** The ids of the actions that start the game itself. */
export const PLAY_ACTIONS = ["play", "play-edited"];

/** The planet or place, in a sentence: "on Batalia", "in the menus". */
export function place(e: Pick<ReportEntry, "planet" | "level">): string {
  if (e.planet) return `on ${e.planet}`;
  if (e.level !== null) return `in level ${e.level}`;
  return "in the menus";
}

/** "23 minutes", "1 hour 14 minutes", "less than a minute". */
export function duration(seconds: number): string {
  if (seconds < 60) return "less than a minute";
  const minutes = Math.round(seconds / 60);
  const h = Math.floor(minutes / 60);
  const m = minutes % 60;
  const part = (n: number, word: string) => `${n} ${word}${n === 1 ? "" : "s"}`;
  if (h === 0) return part(m, "minute");
  return m === 0 ? part(h, "hour") : `${part(h, "hour")} ${part(m, "minute")}`;
}

/** What a row says happened: "Crashed on Batalia", "Played 23 minutes on Kerwan". */
export function headline(e: ReportEntry): string {
  if (e.kind === "crash") return `Crashed ${place(e)}`;
  const time = e.duration !== null ? `Played ${duration(e.duration)}` : "Played";
  if (e.crashed) return `${time}, then crashed ${place(e)}`;
  return e.level !== null || e.planet ? `${time} ${place(e)}` : time;
}

/** "Today at 20:08", "Yesterday at 18:00", "14 October at 10:00" (the player's own language). */
export function when(seconds: number, now = Date.now(), locale?: string): string {
  const d = new Date(seconds * 1000);
  // "5:01 p.m.", not "05:01 p.m.": the hour as the player writes it.
  const time = d.toLocaleTimeString(locale, { hour: "numeric", minute: "2-digit" });
  const day = (x: Date) => new Date(x.getFullYear(), x.getMonth(), x.getDate()).getTime();
  const days = Math.round((day(new Date(now)) - day(d)) / 86400000);
  if (days === 0) return `Today at ${time}`;
  if (days === 1) return `Yesterday at ${time}`;
  const sameYear = d.getFullYear() === new Date(now).getFullYear();
  const date = d.toLocaleDateString(locale, {
    day: "numeric",
    month: "long",
    ...(sameYear ? {} : { year: "numeric" }),
  });
  return `${date} at ${time}`;
}

/** The short text to copy when the game stopped without writing a report (it ended early). */
export function stopText(n: StopNotice, title: string, launcherVersion: string | null): string {
  const how = n.code === null ? "it was stopped by the system" : `it ended with exit code ${n.code}`;
  const tail = n.lines.slice(-25).join("\n");
  return [
    `**OpenRAC: ${title} stopped unexpectedly**`,
    "",
    `- What happened: ${how}, without a crash report`,
    `- Launcher: ${launcherVersion ?? "?"}`,
    "",
    "The game's last lines:",
    "```",
    tail || "(none)",
    "```",
  ].join("\n");
}

/** The game's last output lines, newest last, with the home folder already ~ where the game wrote it. */
export function lastLines(lines: Line[], count = 40): string[] {
  return lines.slice(-count).map((l) => l.line);
}

/** The crashes the player should still hear about: newer than SINCE (seconds since 1970) and than
 *  the newest one they dismissed (SEEN, 0 for none). */
export function unseenRecent<T extends Pick<ReportEntry, "time">>(crashes: T[], seen: number, since: number): T[] {
  return crashes.filter((e) => e.time >= since && e.time > seen);
}
