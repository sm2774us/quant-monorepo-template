"""Request/response schemas for the Systematic TCA service."""

from __future__ import annotations

from pydantic import BaseModel, Field, field_validator


class TradeRequest(BaseModel):
    """A batch of raw trade prints to bucket for TCA analysis."""

    timestamps: list[float] = Field(..., min_length=0)
    prices: list[float] = Field(..., min_length=0)
    sizes: list[float] | None = Field(
        default=None,
        description="Per-trade executed size. Defaults to unit size "
        "(1.0) per trade when omitted.",
    )
    window_sec: float = Field(..., gt=0.0)

    @field_validator("prices")
    @classmethod
    def _validate_prices_length(
        cls, value: list[float], info
    ) -> list[float]:
        timestamps = info.data.get("timestamps")
        if timestamps is not None and len(value) != len(timestamps):
            raise ValueError("prices and timestamps must be the same length")
        return value

    def resolved_sizes(self) -> list[float]:
        """Returns explicit sizes, or unit weights if none were supplied."""
        if self.sizes is None:
            return [1.0] * len(self.timestamps)
        if len(self.sizes) != len(self.timestamps):
            raise ValueError("sizes and timestamps must be the same length")
        return self.sizes


class BucketResponse(BaseModel):
    """VWAP time-bucketing result returned by the core engine."""

    bucket_count: int
    vwap_curve: list[float]
    trade_counts: list[int]


class HealthResponse(BaseModel):
    """Liveness/readiness probe payload."""

    status: str = "ok"
    engine_version: str
