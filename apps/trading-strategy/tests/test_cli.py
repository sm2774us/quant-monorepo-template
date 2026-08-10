"""Tests for the Typer CLI commands."""

from __future__ import annotations

import json

from typer.testing import CliRunner

from trading_strategy.cli import app

runner = CliRunner()


def test_analyze_inline_prints_json_result() -> None:
    result = runner.invoke(
        app,
        [
            "analyze-inline",
            "--timestamps",
            "1.1,1.5,2.3,2.9,3.4",
            "--prices",
            "100.1,100.2,100.1,100.3,100.4",
            "--window-sec",
            "2.0",
        ],
    )
    assert result.exit_code == 0
    payload = json.loads(result.stdout)
    assert payload["bucket_count"] == 2


def test_analyze_inline_with_sizes() -> None:
    result = runner.invoke(
        app,
        [
            "analyze-inline",
            "--timestamps",
            "0.0,0.5",
            "--prices",
            "100.0,200.0",
            "--sizes",
            "3.0,1.0",
            "--window-sec",
            "2.0",
        ],
    )
    assert result.exit_code == 0
    payload = json.loads(result.stdout)
    assert payload["vwap_curve"][0] == 125.0


def test_analyze_file_reads_json_payload(tmp_path) -> None:
    trades_file = tmp_path / "trades.json"
    trades_file.write_text(
        json.dumps(
            {
                "timestamps": [1.0, 1.5],
                "prices": [100.0, 200.0],
                "window_sec": 2.0,
            }
        ),
        encoding="utf-8",
    )
    result = runner.invoke(app, ["analyze-file", str(trades_file)])
    assert result.exit_code == 0
    payload = json.loads(result.stdout)
    assert payload["bucket_count"] == 1


def test_help_lists_commands() -> None:
    result = runner.invoke(app, ["--help"])
    assert result.exit_code == 0
    assert "serve" in result.stdout
    assert "analyze-file" in result.stdout
