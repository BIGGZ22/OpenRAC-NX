import { describe, expect, it } from "vitest";
import { MAX_LINES, Queue } from "./queue";

const repo = { kind: "repository" } as const;

describe("queue", () => {
  it("runs one job at a time, oldest first", () => {
    const q = new Queue();
    const a = q.add(repo, "discs", "Identify discs");
    const b = q.add(repo, "setup", "Place inputs");
    expect(q.next()).toBe(a);
    q.started(a, 7, "python3 tools/openrac.py discs");
    expect(q.next()).toBeUndefined();
    expect(q.pending()).toBe(2);
    q.apply({ type: "output", id: 7, stream: "stdout", line: "missing: ..." });
    q.apply({ type: "exit", id: 7, code: 0, cancelled: false });
    expect(a.state).toBe("succeeded");
    expect(a.lines).toEqual([{ stream: "stdout", line: "missing: ..." }]);
    expect(q.next()).toBe(b);
  });

  it("keeps events that arrive before the job's id is known", () => {
    const q = new Queue();
    const a = q.add(repo, "discs", "Identify discs");
    q.apply({ type: "output", id: 3, stream: "stderr", line: "early" });
    q.apply({ type: "exit", id: 3, code: 2, cancelled: false });
    q.started(a, 3, "cmd");
    expect(a.lines).toEqual([{ stream: "stderr", line: "early" }]);
    expect(a.state).toBe("failed");
    expect(a.code).toBe(2);
  });

  it("records refusals, cancellations and dequeues", () => {
    const q = new Queue();
    const a = q.add(repo, "a", "A");
    const b = q.add(repo, "b", "B");
    const c = q.add(repo, "c", "C");
    q.refused(a, "A: needs Python (Settings)");
    expect(a.state).toBe("failed");
    q.started(b, 1, "cmd");
    q.apply({ type: "exit", id: 1, code: null, cancelled: true });
    expect(b.state).toBe("cancelled");
    q.dequeue(c);
    expect(c.state).toBe("cancelled");
    expect(q.next()).toBeUndefined();
    q.clear();
    expect(q.jobs).toEqual([]);
  });

  it("ends a detached launch at once, so that it holds nothing up", () => {
    const q = new Queue();
    const play = q.add(repo, "play", "Play");
    const next = q.add(repo, "b", "B");
    q.launched(play, "openrac-rac1-pal");
    expect(play.state).toBe("succeeded");
    expect(q.running()).toBeUndefined();
    expect(q.next()).toBe(next);
  });

  it("drops the oldest lines of a chatty job", () => {
    const q = new Queue();
    const a = q.add(repo, "a", "A");
    q.started(a, 1, "cmd");
    for (let i = 0; i < MAX_LINES + 10; i++) q.apply({ type: "output", id: 1, stream: "stdout", line: String(i) });
    expect(a.lines.length).toBe(MAX_LINES);
    expect(a.lines[0]?.line).toBe("10");
  });
});
