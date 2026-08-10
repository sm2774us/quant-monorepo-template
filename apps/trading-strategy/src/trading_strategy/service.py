"""Service layer bridging Python callers to the zero-copy C++26 engine."""

from __future__ import annotations

import numpy as np

import core_engine

from trading_strategy.models import BucketResponse, TradeRequest

_ENGINE_VERSION = "0.2.0"


def engine_version() -> str:
    """Returns the linked core-engine extension module version string."""
    return _ENGINE_VERSION


def analyze(request: TradeRequest) -> BucketResponse:
    """Runs VWAP time-bucketing analysis via the native C++26 engine.

    Converts the incoming payload into contiguous, C-ordered float64 NumPy
    arrays so the nanobind extension can ingest them without copying, then
    invokes the native engine outside the GIL.

    Args:
        request: Validated trade payload to analyze.

    Returns:
        The resulting per-bucket VWAP curve and trade counts.

    Raises:
        ValueError: If array lengths are inconsistent.
    """
    sizes = request.resolved_sizes()
    timestamps = np.ascontiguousarray(request.timestamps, dtype=np.float64)
    prices = np.ascontiguousarray(request.prices, dtype=np.float64)
    sizes_arr = np.ascontiguousarray(sizes, dtype=np.float64)

    result = core_engine.calculate_time_buckets(
        timestamps, prices, sizes_arr, request.window_sec
    )

    return BucketResponse(
        bucket_count=result.bucket_count,
        vwap_curve=list(result.vwap_curve),
        trade_counts=list(result.trade_counts),
    )
