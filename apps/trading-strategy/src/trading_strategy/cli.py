"""Typer-based CLI for the Systematic TCA service (no browser required)."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Annotated

import typer
import uvicorn

from trading_strategy.models import TradeRequest
from trading_strategy.service import analyze

app = typer.Typer(
    name="trading-strategy",
    help="Systematic TCA engine CLI: serve the API or run one-off analyses.",
    no_args_is_help=True,
)


@app.command()
def serve(
    host: Annotated[str, typer.Option(help="Bind address.")] = "0.0.0.0",
    port: Annotated[int, typer.Option(help="Bind port.")] = 8000,
    reload: Annotated[
        bool, typer.Option(help="Enable autoreload for local development.")
    ] = False,
) -> None:
    """Starts the FastAPI service with uvicorn."""
    uvicorn.run(
        "trading_strategy.api:app", host=host, port=port, reload=reload
    )


@app.command()
def analyze_file(
    trades_json: Annotated[
        Path,
        typer.Argument(
            help="Path to a JSON file with timestamps, prices, "
            "optional sizes, and window_sec.",
            exists=True,
            readable=True,
        ),
    ],
) -> None:
    """Runs VWAP time-bucketing analysis on trades loaded from a JSON file."""
    payload = json.loads(trades_json.read_text(encoding="utf-8"))
    request = TradeRequest.model_validate(payload)
    result = analyze(request)
    typer.echo(result.model_dump_json(indent=2))


@app.command()
def analyze_inline(
    timestamps: Annotated[
        str, typer.Option(help="Comma-separated timestamps, e.g. '1.0,1.5,2.3'.")
    ],
    prices: Annotated[
        str, typer.Option(help="Comma-separated prices, e.g. '100.1,100.2'.")
    ],
    window_sec: Annotated[float, typer.Option(help="Bucket window in seconds.")],
    sizes: Annotated[
        str | None,
        typer.Option(help="Comma-separated trade sizes; defaults to unit size."),
    ] = None,
) -> None:
    """Runs VWAP time-bucketing analysis on inline comma-separated values."""
    request = TradeRequest(
        timestamps=[float(x) for x in timestamps.split(",") if x],
        prices=[float(x) for x in prices.split(",") if x],
        sizes=[float(x) for x in sizes.split(",") if x] if sizes else None,
        window_sec=window_sec,
    )
    result = analyze(request)
    typer.echo(result.model_dump_json(indent=2))


def main() -> None:
    """Entry point registered as the `platformopshub`-style console script."""
    app()


if __name__ == "__main__":
    main()
