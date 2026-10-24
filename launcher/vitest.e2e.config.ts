// The Android launcher's pages in a browser (e2e/): `yarn test:android`. Apart from `yarn test`,
// which needs no browser.
import { defineConfig } from "vitest/config";

export default defineConfig({
  test: { include: ["e2e/**/*.test.ts"], environment: "node", testTimeout: 20_000 },
});
