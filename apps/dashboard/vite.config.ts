/// <reference types="vitest/config" />
import react from "@vitejs/plugin-react";
import { defineConfig } from "vite";

export default defineConfig({
  plugins: [react()],
  server: {
    proxy: {
      "/api": "http://127.0.0.1:8000",
    },
  },
  test: {
    environment: "jsdom",
    setupFiles: ["./src/setupTests.ts"],
    coverage: {
      provider: "v8",
      reporter: ["text", "lcov"],
      // Use an explicit include allowlist rather than relying solely on
      // excludes: an exclude-only list previously failed to keep the
      // production `dist/` bundle (built by `npm run build`, which CI
      // runs before `npm run test`) out of the coverage scan, dragging
      // aggregate coverage down to ~69% by treating built, minified
      // output as uncovered "source". Restricting to src/**.{ts,tsx} is
      // immune to that class of bug regardless of what else exists on
      // disk at test time.
      include: ["src/**/*.ts", "src/**/*.tsx"],
      exclude: ["src/main.tsx", "src/**/*.test.ts", "src/**/*.test.tsx"],
      thresholds: {
        lines: 100,
        functions: 100,
        branches: 100,
        statements: 100,
      },
    },
  },
});
