"""Tests for the FastAPI TCA endpoints."""

from __future__ import annotations

from fastapi.testclient import TestClient

from trading_strategy.api import app

client = TestClient(app)


def test_health_returns_ok() -> None:
    response = client.get("/api/v1/health")
    assert response.status_code == 200
    body = response.json()
    assert body["status"] == "ok"
    assert body["engine_version"]


def test_analyze_returns_expected_buckets() -> None:
    payload = {
        "timestamps": [1.1, 1.5, 2.3, 2.9, 3.4],
        "prices": [100.1, 100.2, 100.1, 100.3, 100.4],
        "window_sec": 2.0,
    }
    response = client.post("/api/v1/analyze", json=payload)
    assert response.status_code == 200
    body = response.json()
    assert body["bucket_count"] == 2
    assert len(body["vwap_curve"]) == 2
    assert sum(body["trade_counts"]) == 5


def test_analyze_with_explicit_sizes_weights_correctly() -> None:
    payload = {
        "timestamps": [0.0, 0.5],
        "prices": [100.0, 200.0],
        "sizes": [3.0, 1.0],
        "window_sec": 2.0,
    }
    response = client.post("/api/v1/analyze", json=payload)
    body = response.json()
    assert body["bucket_count"] == 1
    assert body["vwap_curve"][0] == 125.0


def test_analyze_mismatched_lengths_returns_422() -> None:
    payload = {
        "timestamps": [1.0, 2.0],
        "prices": [1.0, 2.0],
        "sizes": [1.0],
        "window_sec": 1.0,
    }
    response = client.post("/api/v1/analyze", json=payload)
    assert response.status_code == 422


def test_analyze_empty_input_returns_zero_buckets() -> None:
    payload = {"timestamps": [], "prices": [], "window_sec": 1.0}
    response = client.post("/api/v1/analyze", json=payload)
    assert response.status_code == 200
    body = response.json()
    assert body["bucket_count"] == 0
    assert body["vwap_curve"] == []


def test_analyze_rejects_non_positive_window() -> None:
    payload = {"timestamps": [1.0], "prices": [1.0], "window_sec": 0.0}
    response = client.post("/api/v1/analyze", json=payload)
    assert response.status_code == 422
