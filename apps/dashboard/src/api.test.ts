import { describe, expect, it, vi } from "vitest";

import { AnalysisRequestError, runAnalysis } from "./api";

describe("runAnalysis", () => {
  it("maps a successful response to camelCase result", async () => {
    const mockFetch = vi.fn().mockResolvedValue({
      ok: true,
      json: async () => ({
        bucket_count: 2,
        vwap_curve: [100.15, 100.35],
        trade_counts: [2, 3],
      }),
    });

    const result = await runAnalysis(
      { timestamps: [1, 2], prices: [100, 101], windowSec: 2 },
      mockFetch as unknown as typeof fetch,
    );

    expect(result.bucketCount).toBe(2);
    expect(result.vwapCurve).toEqual([100.15, 100.35]);
    expect(result.tradeCounts).toEqual([2, 3]);
    expect(mockFetch).toHaveBeenCalledWith(
      "/api/v1/analyze",
      expect.objectContaining({ method: "POST" }),
    );
  });

  it("throws AnalysisRequestError on a non-ok response", async () => {
    const mockFetch = vi.fn().mockResolvedValue({
      ok: false,
      status: 422,
      text: async () => "window_sec must be positive",
    });

    await expect(
      runAnalysis(
        { timestamps: [], prices: [], windowSec: -1 },
        mockFetch as unknown as typeof fetch,
      ),
    ).rejects.toBeInstanceOf(AnalysisRequestError);
  });

  it("propagates the HTTP status code on failure", async () => {
    const mockFetch = vi.fn().mockResolvedValue({
      ok: false,
      status: 500,
      text: async () => "internal error",
    });

    try {
      await runAnalysis(
        { timestamps: [1], prices: [1], windowSec: 1 },
        mockFetch as unknown as typeof fetch,
      );
      expect.fail("expected runAnalysis to throw");
    } catch (err) {
      expect(err).toBeInstanceOf(AnalysisRequestError);
      expect((err as AnalysisRequestError).status).toBe(500);
    }
  });
});
