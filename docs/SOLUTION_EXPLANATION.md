# Solution Explanation

## What changed from the original scaffold

This repo works as a template for structuring world-class quant monorepos.

## Tech Stack

This polyglot monorepo is engineered using a high-performance, zero-copy architecture leveraging the following modern language standards:

[![C++26](https://img.shields.io/badge/C%2B%2B-26-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/26)
[![Python 3.13](https://img.shields.io/badge/Python-3.13-3776AB?style=for-the-badge&logo=python&logoColor=white)](https://docs.python.org/3.13/)
[![TypeScript 7.0.2](https://img.shields.io/badge/TypeScript-7.0.2-3178C6?style=for-the-badge&logo=typescript&logoColor=white)](https://www.typescriptlang.org/)

* **[C++26](https://en.cppreference.com/w/cpp/26)**: Serves as the ultra-low latency execution hot-path engine.
* **[Python 3.13](https://docs.python.org/3.13/)**: Drives the quantitative backend strategies and seamless zero-copy FFI via `nanobind`.
* **[TypeScript 7.0.2](https://www.typescriptlang.org/)**: Powers the strictly-typed, real-time React web dashboard.

## Core engine design (`libs/core-engine`)

- **Algorithm**: single linear pass bucketing trades into fixed-width
  windows anchored at the first timestamp; per-bucket VWAP is
  `Σ(price·qty) / Σ(qty)`, falling back to unit weight (arithmetic mean)
  when no trade size is supplied — this is documented behavior, not an
  approximation hack.
- **Performance**: `std::span<const double>` inputs avoid any copy at the
  algorithm boundary; output vectors are `reserve()`-sized up front so the
  hot loop performs no reallocation; the nanobind layer releases the GIL
  before entering C++ so concurrent Python requests aren't serialized on
  the native call.
- **Correctness under P99 constraints**: all preconditions (matching
  lengths, positive window, sorted timestamps) are validated once at the
  boundary and throw `std::invalid_argument` — no silent wraparound,
  no UB on malformed input, no exceptions thrown from inside the hot loop.
- **Testing**: GoogleTest covers the empty-input, single-trade,
  unit-weight-fallback, volume-weighted, multi-bucket-boundary, and all
  three precondition-violation paths — every branch in the function.

## Python service design (`apps/trading-strategy`)

- `models.py` — Pydantic v2 schemas with cross-field length validation.
- `service.py` — the only module that touches NumPy/`core_engine`;
  converts to `ascontiguousarray(..., dtype=np.float64)` so the nanobind
  `c_contig` constraint is met without a defensive copy in the common case.
- `api.py` — FastAPI routes are a thin translation layer: HTTP → Pydantic →
  service → HTTP. `ValueError` from the service maps to `422`.
- `cli.py` — Typer app exposing `serve`, `analyze-file`, and
  `analyze-inline`, so the exact same `service.analyze` codepath the API
  uses is reachable with zero browser/network dependency, satisfying the
  CLI-mode requirement directly rather than shelling out to the HTTP API.
- **Coverage gate**: `pytest.ini_options` sets `--cov-fail-under=100`
  against `src/trading_strategy`; CI fails the build if any line or branch
  in the service is untested.

## Dashboard design (`apps/dashboard`)

- `api.ts` isolates all `fetch` and snake_case↔camelCase translation behind
  a typed `runAnalysis()` function that accepts an injectable `fetch`
  implementation, which is what makes 100% branch coverage of both the
  success and failure paths possible without a real network call in tests.
- `App.tsx` is a pure presentation component driven by that client:
  loading, error, and result states are all independently testable via
  Testing Library, with the error path exercised through a mocked
  non-`ok` response.
- `tsconfig.json` enables `noUncheckedIndexedAccess` and
  `exactOptionalPropertyTypes` — the strictest practical TypeScript 7
  settings — to catch the classes of bug (`undefined` from array/object
  index access, ambiguous optional-vs-absent fields) most likely to
  reach production in a fast-moving front end.

## CI/CD (`.github/workflows/ci.yml`)

Three independent jobs run in parallel rather than one monolithic job, so a
frontend flake never blocks C++ from getting signal and vice versa:

1. **`cpp-tests`** — matrix over Windows 11 (`windows-latest`), Ubuntu
   (`ubuntu-latest`), and RHEL 9 (`redhat/ubi9` container on an
   `ubuntu-latest` host, since GitHub does not offer a native RHEL runner).
   Builds and runs the GoogleTest suite via CTest on all three.
2. **`python-tests`** — matrix over Ubuntu and Windows; installs `uv`,
   builds the nanobind extension in-place, runs `pytest` with the 100%
   coverage gate, uploads `coverage.xml` to Codecov (Linux leg only, to
   avoid duplicate uploads).
3. **`frontend-tests`** — matrix over Ubuntu and Windows; `npm run build`
   type-checks and bundles, `npm run test` runs Vitest with the 100%
   coverage gate, uploads `lcov.info` to Codecov.
4. **`release`** — on push to `main` after all three succeed, computes the
   next semantic version tag and cuts a GitHub Release with autogenerated
   notes.

## Docker / production topology

`docker/backend.Dockerfile` is a two-stage build: the `build` stage compiles
the C++26 extension with GCC 14 (full C++26 language support) inside
Ubuntu 24.04 and installs the Python package into a venv; the `runtime`
stage copies only that venv into a slim Ubuntu 24.04 image running as a
non-root user, with a `HEALTHCHECK` hitting `/api/v1/health`.
`docker/frontend.Dockerfile` builds the Vite bundle and serves it from
`nginx:1.27-alpine`, proxying `/api/*` to the `backend` service on the
compose network — the same reverse-proxy topology you'd run in production,
just without TLS termination/WAF, which are expected to sit in front of
this stack at the infrastructure layer.
