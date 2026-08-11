import { cleanup, fireEvent, render, screen, waitFor } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";

import { App } from "./App";

afterEach(() => {
  cleanup();
  vi.restoreAllMocks();
});

describe("App", () => {
  it("renders the heading and run button", () => {
    render(<App />);
    expect(
      screen.getByRole("heading", { name: /quantitative tca engine/i }),
    ).toBeDefined();
    expect(
      screen.getByRole("button", { name: /execute 5-trade/i }),
    ).toBeDefined();
  });

  it("displays results after a successful analysis", async () => {
    vi.stubGlobal(
      "fetch",
      vi.fn().mockResolvedValue({
        ok: true,
        json: async () => ({
          bucket_count: 2,
          vwap_curve: [100.15, 100.35],
          trade_counts: [3, 2],
        }),
      }),
    );

    render(<App />);
    fireEvent.click(screen.getByRole("button", { name: /execute 5-trade/i }));

    await waitFor(() =>
      expect(screen.getByText(/bucket count:/i)).toBeDefined(),
    );
    expect(screen.getByTestId("bucket-count").textContent).toBe("2");
  });

  it("displays an error message when the request fails", async () => {
    vi.stubGlobal(
      "fetch",
      vi.fn().mockResolvedValue({
        ok: false,
        status: 422,
        text: async () => "invalid payload",
      }),
    );

    render(<App />);
    fireEvent.click(screen.getByRole("button", { name: /execute 5-trade/i }));

    await waitFor(() => expect(screen.getByRole("alert")).toBeDefined());
  });

  it("falls back to a generic message for a non-AnalysisRequestError failure", async () => {
    vi.stubGlobal(
      "fetch",
      vi.fn().mockRejectedValue(new TypeError("Failed to fetch")),
    );

    render(<App />);
    fireEvent.click(screen.getByRole("button", { name: /execute 5-trade/i }));

    await waitFor(() =>
      expect(screen.getByText(/unexpected error running analysis/i)).toBeDefined(),
    );
  });

  it("defaults a missing per-bucket trade count to zero", async () => {
    vi.stubGlobal(
      "fetch",
      vi.fn().mockResolvedValue({
        ok: true,
        json: async () => ({
          bucket_count: 2,
          vwap_curve: [100.15, 100.35],
          // Deliberately shorter than vwap_curve to exercise the `?? 0`
          // fallback required by noUncheckedIndexedAccess.
          trade_counts: [3],
        }),
      }),
    );

    render(<App />);
    fireEvent.click(screen.getByRole("button", { name: /execute 5-trade/i }));

    await waitFor(() => expect(screen.getByText(/\(0 trades\)/)).toBeDefined());
  });
});
