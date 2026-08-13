"""FastAPI application for the Systematic TCA service."""

from __future__ import annotations

from fastapi import FastAPI, HTTPException

from trading_strategy.models import BucketResponse, HealthResponse, TradeRequest
from trading_strategy.service import analyze, engine_version

app = FastAPI(
    title="Systematic TCA API",
    description="Zero-copy C++26 / Python 3.13 transaction-cost-analysis "
    "service for VWAP time-bucketing.",
    version="0.1.3",
)


@app.get("/api/v1/health", response_model=HealthResponse)
async def health() -> HealthResponse:
    """Liveness/readiness probe used by orchestrators and load balancers."""
    return HealthResponse(engine_version=engine_version())


@app.post("/api/v1/analyze", response_model=BucketResponse)
async def analyze_trades(request: TradeRequest) -> BucketResponse:
    """Buckets a batch of trades into fixed-width windows and returns VWAP.

    Args:
        request: Trade timestamps, prices, optional sizes, and window size.

    Returns:
        The per-bucket VWAP curve and trade counts.
    """
    try:
        return analyze(request)
    except ValueError as exc:
        raise HTTPException(status_code=422, detail=str(exc)) from exc
