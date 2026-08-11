# QUANT_PROJECT — Systematic TCA Engine Monorepo

A simple, self-contained, production-grade polyglot monorepo template
(**C++26 + Python 3.13 + TypeScript 7.0.2**) implementing a real-time
transaction-cost-analysis (TCA) VWAP time-bucketing service, structured the
way a top-tier quant firm would ship it: a zero-copy native compute core, a
thin async API/CLI layer, and a typed dashboard — each independently built,
tested to 100% coverage, and CI-verified on Windows 11, Ubuntu, and RHEL.

## Tech Stack

This polyglot monorepo template is engineered using a high-performance, zero-copy architecture leveraging the following modern language standards:

[![C++26](https://img.shields.io/badge/C%2B%2B-26-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/26)
[![Python 3.13](https://img.shields.io/badge/Python-3.13-3776AB?style=for-the-badge&logo=python&logoColor=white)](https://docs.python.org/3.13/)
[![TypeScript 7.0.2](https://img.shields.io/badge/TypeScript-7.0.2-3178C6?style=for-the-badge&logo=typescript&logoColor=white)](https://www.typescriptlang.org/)

* **[C++26](https://en.cppreference.com/w/cpp/26)**: Serves as the ultra-low latency execution hot-path engine.
* **[Python 3.13](https://docs.python.org/3.13/)**: Drives the quantitative backend strategies and seamless zero-copy FFI via `nanobind`.
* **[TypeScript 7.0.2](https://www.typescriptlang.org/)**: Powers the strictly-typed, real-time React web dashboard.

## Architecture

```
libs/core-engine/     C++26 VWAP bucketing algorithm + nanobind Python binding + GoogleTest suite
apps/trading-strategy/ Python 3.13 FastAPI service + Typer CLI + pytest suite (100% coverage gate)
apps/dashboard/        TypeScript 7.0.2 + React 18 dashboard + Vitest suite (100% coverage gate)
docker/                Production Dockerfiles (backend, nginx frontend) + nginx.conf
.github/workflows/     CI matrix: Windows 11, Ubuntu latest, RHEL 9 (UBI9 container)
```

The core algorithm (`core_engine::CalculateTimeBuckets`) partitions
timestamp-sorted trades into fixed-width windows and computes true
volume-weighted average price per window in a single O(n) pass with the
Python GIL released, using `std::span` for zero-copy, allocation-free input
handling.

## Local Development (no Docker, no cloud)

```bash
# 1. Backend (compiles the C++26 nanobind extension automatically)
cd apps/trading-strategy
pip install uv
uv venv && source .venv/bin/activate    # .venv\Scripts\activate on Windows
uv pip install -e ../../libs/core-engine
uv pip install -e .[dev]
uv run trading-strategy serve            # http://localhost:8000

# CLI mode (no browser required)
uv run trading-strategy analyze-inline \
  --timestamps "1.1,1.5,2.3,2.9,3.4" \
  --prices "100.1,100.2,100.1,100.3,100.4" \
  --window-sec 2.0

# 2. Frontend
cd apps/dashboard
npm install
npm run dev                              # http://localhost:5173
```

## Testing & Coverage

```bash
# C++26 (GoogleTest)
cd libs/core-engine
cmake -S . -B build -G Ninja -DCORE_ENGINE_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure

# Python 3.13 (pytest, 100% coverage gate via --cov-fail-under=100)
cd apps/trading-strategy
uv run pytest

# TypeScript 7.0.2 (Vitest, 100% coverage gate)
cd apps/dashboard
npm run test
```

Coverage reports (`coverage.xml`, `coverage/lcov.info`) upload to
[Codecov](https://codecov.io) via `.github/workflows/ci.yml`; add a
`CODECOV_TOKEN` repository secret to enable publishing.

## Calling the API with curl

```bash
curl -X GET http://localhost:8000/api/v1/health

curl -X POST http://localhost:8000/api/v1/analyze \
  -H "Content-Type: application/json" \
  -d '{
        "timestamps": [1.1, 1.5, 2.3, 2.9, 3.4],
        "prices": [100.1, 100.2, 100.1, 100.3, 100.4],
        "window_sec": 2.0
      }'
```

## Calling the API with Postman

1. New request → `POST http://localhost:8000/api/v1/analyze`.
2. Body → raw → JSON, paste the same payload as above.
3. Headers → `Content-Type: application/json` (Postman sets this
   automatically when Body type is JSON).
4. Send — response contains `bucket_count`, `vwap_curve`, `trade_counts`.
5. Interactive OpenAPI docs are also available at
   `http://localhost:8000/docs` for one-click "Try it out" testing.

## Docker (production-like, nginx-fronted)

```bash
docker compose up --build
# Dashboard: http://localhost:8080  (nginx proxies /api/* to the backend)
```

## Using This Repo as a Template

Starting a new project from this monorepo? See
[`docs/TEMPLATE_USAGE_GUIDE.md`](docs/TEMPLATE_USAGE_GUIDE.md) for the
buy-in rationale, bootstrap steps, and what to harden before production.

## CI/CD

`.github/workflows/ci.yml` runs three parallel jobs — `cpp-tests` (Windows
11 / Ubuntu / RHEL 9 via UBI9 container), `python-tests`, and
`frontend-tests` — followed by a `release` job on merge to `main` that:

1. Computes the next semantic version from the latest `vX.Y.Z` tag.
2. Generates a categorized changelog from merged PRs since that tag.
3. Prepends it to `CHANGELOG.md` and commits that change to `main`.
4. Tags the resulting changelog commit (not a stale earlier one).
5. Builds a `git archive` source tarball containing the updated
   `CHANGELOG.md`, with a SHA-256 checksum.
6. Publishes a GitHub Release with the changelog as its body and the
   tarball + checksum as downloadable assets.

Set the optional `RELEASE_PAT` repository secret (a token with `contents:
write` and the ability to bypass/satisfy branch protection on `main`) if
`main` is protected; otherwise the job falls back to the default
`GITHUB_TOKEN`. See `docs/SOLUTION_EXPLANATION.md` for full design
rationale.
