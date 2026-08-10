/** Typed client for the Systematic TCA backend API. */

export interface TradePayload {
  readonly timestamps: readonly number[];
  readonly prices: readonly number[];
  readonly sizes?: readonly number[];
  readonly windowSec: number;
}

export interface AnalysisResult {
  readonly bucketCount: number;
  readonly vwapCurve: readonly number[];
  readonly tradeCounts: readonly number[];
}

interface AnalysisResultDto {
  readonly bucket_count: number;
  readonly vwap_curve: readonly number[];
  readonly trade_counts: readonly number[];
}

/** Thrown when the backend rejects an analysis request. */
export class AnalysisRequestError extends Error {
  constructor(
    message: string,
    readonly status: number,
  ) {
    super(message);
    this.name = "AnalysisRequestError";
  }
}

/**
 * Submits a batch of trades to the backend for VWAP time-bucketing.
 *
 * @param payload - Trade timestamps, prices, optional sizes, and window.
 * @param fetchImpl - Injectable fetch implementation, for testability.
 * @returns The bucketed VWAP curve and per-bucket trade counts.
 */
export async function runAnalysis(
  payload: TradePayload,
  fetchImpl: typeof fetch = fetch,
): Promise<AnalysisResult> {
  const response = await fetchImpl("/api/v1/analyze", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      timestamps: payload.timestamps,
      prices: payload.prices,
      sizes: payload.sizes,
      window_sec: payload.windowSec,
    }),
  });

  if (!response.ok) {
    const detail = await response.text();
    throw new AnalysisRequestError(
      `Analysis request failed: ${detail}`,
      response.status,
    );
  }

  const dto = (await response.json()) as AnalysisResultDto;
  return {
    bucketCount: dto.bucket_count,
    vwapCurve: dto.vwap_curve,
    tradeCounts: dto.trade_counts,
  };
}
