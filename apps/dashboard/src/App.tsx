import { useCallback, useState } from "react";

import { AnalysisRequestError, runAnalysis, type AnalysisResult } from "./api";

const SAMPLE_TRADES = {
  timestamps: [1.1, 1.5, 2.3, 2.9, 3.4],
  prices: [100.1, 100.2, 100.1, 100.3, 100.4],
  windowSec: 2.0,
} as const;

export function App(): React.JSX.Element {
  const [result, setResult] = useState<AnalysisResult | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [isLoading, setIsLoading] = useState(false);

  const handleRunAnalysis = useCallback(async (): Promise<void> => {
    setIsLoading(true);
    setError(null);
    try {
      const analysis = await runAnalysis(SAMPLE_TRADES);
      setResult(analysis);
    } catch (err) {
      const message =
        err instanceof AnalysisRequestError
          ? err.message
          : "Unexpected error running analysis.";
      setError(message);
    } finally {
      setIsLoading(false);
    }
  }, []);

  return (
    <main style={{ fontFamily: "sans-serif", padding: "2rem" }}>
      <h1>Quantitative TCA Engine</h1>
      <button onClick={handleRunAnalysis} disabled={isLoading}>
        {isLoading ? "Running..." : "Execute 5-Trade / 2-Sec Window Analysis"}
      </button>

      {error !== null && (
        <p role="alert" style={{ color: "crimson" }}>
          {error}
        </p>
      )}

      {result !== null && (
        <section
          style={{ marginTop: "2rem", padding: "1rem", border: "1px solid #ccc" }}
        >
          <h2>Results (Zero-Copy C++26 Engine)</h2>
          <p>
            <strong>Bucket Count:</strong> {result.bucketCount}
          </p>
          <ul>
            {result.vwapCurve.map((vwap, idx) => (
              <li key={`bucket-${idx}`}>
                Bucket {idx + 1} VWAP: ${vwap.toFixed(2)} (
                {result.tradeCounts[idx] ?? 0} trades)
              </li>
            ))}
          </ul>
        </section>
      )}
    </main>
  );
}

export default App;
