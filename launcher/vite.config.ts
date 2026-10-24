/// <reference types="vitest/config" />
import { svelte } from "@sveltejs/vite-plugin-svelte";
import { defineConfig } from "vite";

// Tauri expects a fixed dev port (tauri.conf.json's devUrl) and serves dist/ in release builds.
export default defineConfig({
  plugins: [svelte()],
  resolve: {
    // The same aliases as tsconfig.json's "paths".
    alias: { $lib: "/src/lib", $components: "/src/components" },
  },
  clearScreen: false,
  server: {
    port: 1430,
    strictPort: true,
    // The browser preview (src/lib/mock.ts) reads games/*/game.json and
    // progress/summary.json from the checkout around the launcher.
    fs: { allow: [".."] },
    // Cargo's folders: Tauri watches its own sources, and target/ changes during every build.
    watch: { ignored: ["**/src-tauri/**", "**/core/**", "**/target/**"] },
  },
  envPrefix: ["VITE_", "TAURI_ENV_"],
  build: { target: "es2023", outDir: "dist", emptyOutDir: true },
  // app.css read as text by the tests (palettes.test.ts, stylesheets.test.ts): Vitest otherwise
  // hands every stylesheet to them empty.
  test: { include: ["src/**/*.test.ts"], environment: "node", css: { include: [/app\.css/] } },
});
