// ESLint flat config: typed TypeScript rules for .ts and .svelte files, and
// Svelte's own rules. Formatting is Prettier's job.
import js from "@eslint/js";
import { defineConfig } from "eslint/config";
import svelte from "eslint-plugin-svelte";
import globals from "globals";
import ts from "typescript-eslint";
import svelteConfig from "./svelte.config.js";

export default defineConfig(
  {
    ignores: [
      "dist/",
      "src/lib/generated/",
      "src-tauri/",
      "core/",
      "target/",
      "node_modules/",
      ".svelte-check/",
      ".pnp.*",
      ".yarn/",
    ],
  },
  js.configs.recommended,
  ts.configs.strictTypeChecked,
  ts.configs.stylisticTypeChecked,
  svelte.configs.recommended,
  {
    languageOptions: {
      globals: { ...globals.browser },
      parserOptions: {
        projectService: { allowDefaultProject: ["*.config.js"] },
        tsconfigRootDir: import.meta.dirname,
        extraFileExtensions: [".svelte"],
      },
    },
    rules: {
      // Errors from Tauri commands arrive as strings; numbers in messages are fine.
      "@typescript-eslint/restrict-template-expressions": ["error", { allowNumber: true }],
    },
  },
  {
    files: ["**/*.svelte", "**/*.svelte.ts"],
    languageOptions: { parserOptions: { parser: ts.parser, svelteConfig } },
  },
  {
    files: ["**/*.svelte"],
    rules: {
      // Typed linting cannot see other components' prop types inside .svelte
      // files and reports their callbacks as `any`; svelte-check checks them.
      "@typescript-eslint/no-unsafe-argument": "off",
      "@typescript-eslint/no-unsafe-assignment": "off",
      "@typescript-eslint/no-unsafe-call": "off",
      "@typescript-eslint/no-unsafe-member-access": "off",
      "@typescript-eslint/no-unsafe-return": "off",
    },
  },
  { files: ["*.config.js"], extends: [ts.configs.disableTypeChecked] },
);
