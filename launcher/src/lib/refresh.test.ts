import { beforeEach, describe, expect, it, vi } from "vitest";
import type { Config, Library } from "./api";

const library = vi.fn();
const scanDiscs = vi.fn();

vi.mock("./api", async (importOriginal) => {
  const actual = await importOriginal<typeof import("./api")>();
  return { ...actual, api: { ...actual.api, library, scanDiscs } };
});

const { app, refresh } = await import("./app.svelte");

/** A library with a name, so that two of them differ. */
const withRoot = (root: string): Library => ({
  root,
  platform: "linux",
  toolchains: false,
  repository: [],
  games: [],
  actionsError: null,
});

/** A promise and the function that answers it. */
function deferred<T>() {
  let answer: (value: T) => void = () => {
    throw new Error("answered before it was asked for");
  };
  const promise = new Promise<T>((resolve) => {
    answer = resolve;
  });
  return { promise, answer };
}

describe("refresh", () => {
  beforeEach(() => {
    library.mockReset();
    scanDiscs.mockReset();
    scanDiscs.mockResolvedValue({ found: [], choices: [] });
    app.config = { root: "/somewhere" } as Config;
  });

  it("keeps the newest library when an older read comes back later", async () => {
    const first = deferred<Library>();
    const second = deferred<Library>();
    library.mockImplementationOnce(() => first.promise);
    library.mockImplementationOnce(() => second.promise);

    const reading = refresh();
    const readingToo = refresh();
    // The second read answers first, the first one after it: the older answer must not
    // put the older library back.
    second.answer(withRoot("/newer"));
    await readingToo;
    first.answer(withRoot("/older"));
    await reading;

    expect(app.library?.root).toBe("/newer");
  });
});
