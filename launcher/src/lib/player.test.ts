import { beforeEach, describe, expect, it } from "vitest";
import { jobs } from "./app.svelte";
import { isPlaying } from "./player.svelte";
import type { Job } from "./queue";
import type { VersionView } from "./api";

const version = { key: "rac1/pal", game: "rac1" } as VersionView;

function job(actionId: string): Job {
  return {
    key: 1,
    scope: { kind: "version", key: "rac1/pal" },
    actionId,
    title: actionId,
    state: "running",
    id: 1,
    command: "cmd",
    lines: [],
    code: null,
    queuedAt: 0,
    startedAt: 0,
    endedAt: null,
  };
}

describe("isPlaying", () => {
  beforeEach(() => {
    jobs.list.length = 0;
  });

  it("is true while the game runs", () => {
    jobs.list.push(job("play"));
    expect(isPlaying(version)).toBe(true);
  });

  it("is true while the game with the player's mods runs: the same program, the same card", () => {
    jobs.list.push(job("play-edited"));
    expect(isPlaying(version)).toBe(true);
  });

  it("is false while another job of the version runs", () => {
    jobs.list.push(job("port-build"));
    expect(isPlaying(version)).toBe(false);
  });

  it("is false when the job has ended", () => {
    const ended = job("play-edited");
    ended.state = "succeeded";
    jobs.list.push(ended);
    expect(isPlaying(version)).toBe(false);
  });
});
