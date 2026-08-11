# QUANT_PROJECT — Monorepo Template Adoption Playbook
### From `git clone` to a Production-Deployable Quant Service in One Sitting
#### Buy-In Rationale · Bootstrap Instructions · Extension Patterns · Anti-Patterns to Avoid

> **Delivery philosophy:** This template exists so the *hard, easy-to-get-wrong* 5% of a quant monorepo — zero-copy native/Python FFI, coverage-gated CI across three OSes, reproducible releases — is solved once, correctly, and never re-litigated per project. Everything below assumes you are starting a **new** repository from this one, not modifying this repository in place.

---
---

[↩️ Back to README.md](../README.md)

---
---

## ⏱️ Adoption Time Budget

```
PHASE                    TIME        WHAT YOU'RE DOING                      SKIP IF...
────────────────────    ─────────   ──────────────────────────────────    ─────────────────────────
0. Buy-In Review         10 min      Confirm this template fits your        You already know you want it
                                     problem shape (Section 1)
1. Bootstrap             15 min      Fork/template, rename, first green     N/A — always do this first
                                     CI run on a trivial change
2. Replace the Core      1–3 days    Swap the VWAP bucketing algorithm      Your first slice IS TCA
                                     for your actual quant logic
3. Extend the Contract   0.5–1 day   Add fields/endpoints/CLI commands      Single-endpoint service is enough
4. Harden for Your Team  1–2 days    Branch protection, secrets, on-call    Solo project / prototype
```

> **Priority rule:** Do not skip Phase 1. Teams that jump straight to "delete the sample code and write mine" without first proving the CI/coverage/release pipeline goes green on a no-op change spend 3–5x longer debugging pipeline issues *entangled with* business-logic issues later. Prove the skeleton first.

---

## Table of Contents

### 🎯 WHY ADOPT THIS TEMPLATE
- [B1 · The Buy-In — What You Get for Free](#b1--the-buy-in--what-you-get-for-free)
  - **[What "Free" Actually Means Here](#what-free-actually-means-here)**
  - **[The Concrete Deliverables You Inherit](#the-concrete-deliverables-you-inherit)**
  - **[Tech Stack](#tech-stack)**
- [B2 · What This Template Is Not](#b2--what-this-template-is-not)
- [B3 · Fit Test — Should This Be Your Starting Point?](#b3--fit-test--should-this-be-your-starting-point)

### 🚀 BOOTSTRAP
- [S1 · Cloning, Renaming & First Green Build](#s1--cloning-renaming--first-green-build)
- [S2 · Repository & Secrets Configuration](#s2--repository--secrets-configuration)
- [S3 · Local Dev Loop Across All Three Languages](#s3--local-dev-loop-across-all-three-languages)

### 🧩 REPLACING THE SAMPLE DOMAIN LOGIC
- [R1 · Anatomy of the Three-Layer Contract](#r1--anatomy-of-the-three-layer-contract)
- [R2 · Swapping the C++26 Core for Your Algorithm](#r2--swapping-the-c26-core-for-your-algorithm)
- [R3 · Extending the Python Service & CLI](#r3--extending-the-python-service--cli)
- [R4 · Extending the TypeScript Dashboard](#r4--extending-the-typescript-dashboard)

### 🏛️ INSTITUTIONAL HARDENING — WHAT TO ADD YOURSELF
- [H1 · Coverage Discipline Beyond 100% Line Coverage](#h1--coverage-discipline-beyond-100-line-coverage)
- [H2 · From Single-Node to Production Topology](#h2--from-single-node-to-production-topology)
- [H3 · Observability, Secrets & Compliance Gaps to Close](#h3--observability-secrets--compliance-gaps-to-close)

- **[Quick-Reference Checklist](#quick-reference-checklist)**

[🔝 Back to Top](#table-of-contents)

---
---

# 🎯 WHY ADOPT THIS TEMPLATE

---

## B1 · The Buy-In — What You Get for Free

**Open with the intuition (15 seconds):**
> "Every quant shop reinvents the same monorepo scaffolding — a fast native core, a thin service layer, a typed UI, tests that actually gate merges, and a release process that doesn't lie about what shipped. This template exists so your team's first commit is business logic, not build-system archaeology."

---

### What "Free" Actually Means Here

```
CAPABILITY                          WITHOUT THIS TEMPLATE              WITH THIS TEMPLATE
──────────────────────────────────  ──────────────────────────────    ──────────────────────────────
Zero-copy C++ ↔ Python FFI           2–5 days evaluating pybind11      Pre-wired nanobind + std::span
                                     vs nanobind vs Cython vs ctypes;   boundary; GIL-release pattern
                                     GIL-release bugs discovered in    already correct; CMake option
                                     production                        switches cleanly between native
                                                                       GoogleTest build and Python
                                                                       wheel build

100%-coverage-gated CI              "We'll add tests later" becomes   pytest --cov-fail-under=100 and
                                     technical debt that compounds     Vitest coverage.thresholds are
                                     silently for quarters             already wired to FAIL the build,
                                                                       not just report a number

3-OS CI matrix (Win11/              Discovered on someone's laptop    Windows 11, Ubuntu LTS, and
Ubuntu/RHEL9)                       three weeks before a regulated    RHEL 9 (via UBI9 container) all
                                     RHEL production deploy that the   build and test on every push,
                                     compiler flags don't match        before day one of your project

Reproducible, auditable             "What exactly is in v2.3.1?"      git archive-based tarball +
releases                            answered by scrolling Slack       SHA-256 checksum + source-
                                     history                           controlled CHANGELOG.md, all
                                                                       generated from the commit that
                                                                       is actually tagged

Style-guide conformance             Style drift starts week one,      Google C++/Python/TypeScript
baked into project structure        bikeshedding starts week two      style conventions are the
                                                                       default, not a wiki page nobody
                                                                       reads

A known-good "one true          New hires ask "where does the         apps/ = deployables,
example" of layering            business logic go?" for their         libs/ = shared, reusable,
                                 first two weeks                       independently-versioned code.
                                                                       Every new slice follows the
                                                                       same shape.
```

**Say it out loud:** *"The template's value isn't the VWAP-bucketing sample — you'll delete that. The value is that the plumbing between a compiled hot path, an async service, and a typed frontend is already correct, already tested, and already enforced by CI, so your team's calendar time goes to alpha, not scaffolding."*

---

### The Concrete Deliverables You Inherit

```
LAYER               YOU INHERIT                                    YOU STILL OWN
──────────────────  ─────────────────────────────────────────────  ─────────────────────────────
libs/*-engine/      CMake + nanobind wiring, GoogleTest harness,    Your actual algorithm(s);
(C++26)              -Wall -Wextra -Werror baseline, C++26           domain-specific numerical
                     standard config, Release/-O3 defaults           stability & precision analysis

apps/*-service/     FastAPI app skeleton, Typer CLI skeleton,       Your domain schemas, business
(Python 3.13)        Pydantic validation pattern, pytest +           rules, auth/authz, rate
                     coverage gate, service-layer/API-layer          limiting, persistence layer
                     separation

apps/dashboard/     Vite + React 18 + TypeScript 7 scaffold,        Your actual UI/UX, charting,
(TypeScript 7)       typed API client pattern, Vitest + Testing      state management at scale,
                     Library harness, strict tsconfig                design system

.github/workflows/  3-OS test matrix, Codecov wiring,               Branch protection rules,
                     changelog-generating release job,               required reviewers, deploy
                     concurrency guards, script-injection-safe        targets beyond docker compose
                     changelog handling

docker/             Multi-stage backend build, nginx-fronted         TLS termination, WAF, secrets
                     frontend build, healthchecks, non-root           manager integration, autoscaling
                     runtime user
```

### Tech Stack

This polyglot monorepo template is engineered using a high-performance, zero-copy architecture leveraging the following modern language standards:

[![C++26](https://img.shields.io/badge/C%2B%2B-26-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/26)
[![Python 3.13](https://img.shields.io/badge/Python-3.13-3776AB?style=for-the-badge&logo=python&logoColor=white)](https://docs.python.org/3.13/)
[![TypeScript 7.0.2](https://img.shields.io/badge/TypeScript-7.0.2-3178C6?style=for-the-badge&logo=typescript&logoColor=white)](https://www.typescriptlang.org/)

* **[C++26](https://en.cppreference.com/w/cpp/26)**: Serves as the ultra-low latency execution hot-path engine.
* **[Python 3.13](https://docs.python.org/3.13/)**: Drives the quantitative backend strategies and seamless zero-copy FFI via `nanobind`.
* **[TypeScript 7.0.2](https://www.typescriptlang.org/)**: Powers the strictly-typed, real-time React web dashboard.


[🔝 Back to Top](#-table-of-contents)

---
---

## B2 · What This Template Is Not

**Open with the intuition (15 seconds):**
> "A template that claims to solve deployment topology, market data ingestion, and risk limits for every asset class simultaneously is lying to you. This one solves the *plumbing* problem generically and leaves the *domain* problem to you, on purpose."

```
THIS TEMPLATE DOES NOT SHIP...              BECAUSE...
───────────────────────────────────────    ──────────────────────────────────────────────
A market data adapter / feed handler        Every venue/vendor integration is bespoke;
                                             shipping a fake one would be actively misleading

An order/execution management system        OMS/EMS integration is regulated, stateful, and
                                             firm-specific — not something to templatize

Cloud infrastructure-as-code                docker-compose.yml models a single-box production
(Terraform/Pulumi/CDK)                      topology deliberately; multi-region/HA topology
                                             is an org-specific decision, not a template default

A secrets manager integration               Vault/AWS Secrets Manager/Azure Key Vault choice
                                             is an infra-team decision; the template surfaces
                                             the seam (env vars, no hardcoded secrets) instead

Authentication/authorization                Every firm's SSO/entitlements model differs; adding
                                             a fake one would need to be ripped out immediately
```

**Say it out loud:** *"If a template tries to be everything, it becomes something nobody trusts enough to build on. This one's job is to make the unglamorous 20% — build system, test gating, release integrity — a solved problem, and to get out of the way for the 80% that's actually your firm's edge."*

[🔝 Back to Top](#table-of-contents)

---
---

## B3 · Fit Test — Should This Be Your Starting Point?

```
YOUR SITUATION                                          VERDICT
───────────────────────────────────────────────────────  ──────────────────────────────────────
New service needs a fast numerical core callable          STRONG FIT — this is exactly the
from Python, with a UI on top                             three-layer shape this template models

Pure-Python microservice, no performance-critical          PARTIAL FIT — delete libs/core-engine
native path needed                                         entirely; keep the FastAPI/Typer/pytest/
                                                            CI pattern, it still saves real time

Pure C++ trading system with no Python/web surface          WEAK FIT — take libs/core-engine's
                                                            CMake + GoogleTest + C++26 conventions;
                                                            skip the rest

Need multi-region, multi-cloud, HA-from-day-one            USE AS A STARTING POINT, NOT AS-IS —
infrastructure                                              the docker-compose.yml topology is a
                                                             single-box reference; your infra team
                                                             replaces it with Terraform/K8s using the
                                                             same container images this repo builds

Regulated environment requiring FIPS-validated              REVIEW CAREFULLY — verify your toolchain
cryptography or specific approved-package lists              (GCC 14, Python 3.13 wheel deps, npm
                                                              deps) against your firm's approved list
                                                              before adopting wholesale
```

[🔝 Back to Top](#table-of-contents)

---
---

# 🚀 BOOTSTRAP

---

## S1 · Cloning, Renaming & First Green Build

**Open with the intuition (15 seconds):**
> "The single highest-leverage thing you can do on day one is get a trivial, no-op change through the full pipeline — build, test, coverage gate, release — before writing a line of domain logic. Everything after that is a known-good baseline you're diffing against."

---

### Step-by-Step Bootstrap

```
STEP  ACTION                                              VERIFY
────  ─────────────────────────────────────────────────  ─────────────────────────────────────
 1    Use GitHub's "Use this template" (or `git clone`     New repo exists, is NOT a fork
      + push to a fresh remote if template mode isn't      (forks inherit PR-target settings
      available)                                           you don't want)

 2    Global rename: `core_engine` → `<your_lib>_engine`,   `grep -ril "core_engine\|trading-      `
      `trading-strategy` / `trading_strategy` → your        strategy\|trading_strategy" .`
      service name, `dashboard` → your app name              returns zero matches

 3    Update `libs/<lib>/pyproject.toml` `name`,             `uv pip install -e .` from libs/
      `apps/<svc>/pyproject.toml` `[tool.uv.sources]`        succeeds with the new names
      path + name, `apps/dashboard/package.json` `name`

 4    Push to `main`, confirm all three CI jobs             GitHub Actions tab: cpp-tests,
      (`cpp-tests`, `python-tests`, `frontend-tests`)        python-tests, frontend-tests all
      go green on the unmodified sample code                green on all OS matrix legs

 5    Confirm the `release` job produces a tag,              A `vX.Y.Z` GitHub Release exists
      a `CHANGELOG.md` commit, and a downloadable            with a `.tar.gz` + `.sha256` asset;
      tarball with checksum                                  `CHANGELOG.md` is updated on `main`
```

**Say it out loud:** *"If step 5 doesn't work on the unmodified template, it will not magically start working once you've added three weeks of domain logic on top. Debug the pipeline in isolation, first."*

---

### Common Bootstrap Failures & Fixes

```
SYMPTOM                                    ROOT CAUSE                          FIX
──────────────────────────────────────    ──────────────────────────────     ──────────────────────
release job fails to push to main          Branch protection blocks the       Add `RELEASE_PAT` repo
                                            default GITHUB_TOKEN               secret (a PAT/App token
                                                                               with contents:write that
                                                                               satisfies required checks)

RHEL9 cpp-tests job fails to find           gcc-toolset-14 not available in    Pin to a specific UBI9
gcc-toolset-14                              the UBI9 mirror snapshot in use    tag or vendor an internal
                                                                               mirror; verify against
                                                                               Red Hat's current repos

Codecov upload silently no-ops              CODECOV_TOKEN secret not set       Fine for public repos on
                                                                               Codecov's tokenless
                                                                               uploader; required for
                                                                               private repos — add the
                                                                               secret in repo settings

Windows uv install step fails               PowerShell execution policy        Use the `irm | iex`
                                             blocks remote script execution     invocation exactly as
                                             on self-hosted Windows runners     shipped; for self-hosted
                                                                               runners, pre-install uv
                                                                               in the runner image instead
```

[🔝 Back to Top](#table-of-contents)

---
---

## S2 · Repository & Secrets Configuration

```
SETTING                          VALUE                                 WHY
───────────────────────────────  ────────────────────────────────────  ──────────────────────────
Branch protection on `main`      Require status checks: cpp-tests,     Prevents an unreviewed or
                                  python-tests, frontend-tests          untested change from ever
                                  (all matrix legs)                     reaching a releasable state

Secret: RELEASE_PAT              Fine-grained PAT, contents:write,     Lets the release job push a
                                  scoped to this repo only              commit + tag to protected main

Secret: CODECOV_TOKEN             From codecov.io repo settings         Required for private-repo
                                                                        coverage uploads

Required reviewers                ≥ 1 for apps/, ≥ 1 for libs/ (use     Native/FFI boundary changes
                                  CODEOWNERS to route by path)          warrant a second set of eyes
                                                                        on memory-safety implications
```

[🔝 Back to Top](#table-of-contents)

---
---

## S3 · Local Dev Loop Across All Three Languages

```
LANGUAGE       COMMAND                                          WHEN TO RUN
─────────────  ───────────────────────────────────────────────  ──────────────────────────────
C++26          cmake -S . -B build -G Ninja                      Before every commit touching
               -DCORE_ENGINE_BUILD_TESTS=ON && cmake --build      libs/*/src or libs/*/include
               build && ctest --test-dir build

Python 3.13    uv run pytest                                     Before every commit touching
               (from apps/<service>/)                            apps/*/src or apps/*/tests

TypeScript 7   npm run test                                      Before every commit touching
               (from apps/dashboard/)                             apps/dashboard/src
```

**Say it out loud:** *"None of these commands are optional pre-push rituals — they're literally what CI runs. If it's red locally, it will be red in the matrix, on all three operating systems, and the release job will never trigger."*

[🔝 Back to Top](#table-of-contents)

---
---

# 🧩 REPLACING THE SAMPLE DOMAIN LOGIC

---

## R1 · Anatomy of the Three-Layer Contract

**Open with the intuition (15 seconds):**
> "Every request flows through exactly three boundaries: HTTP/CLI → validated schema → native compute. Each boundary has exactly one job. Violate that separation — say, by putting business logic in the FastAPI route handler — and you lose the ability to unit test it without spinning up HTTP."

```
                     ┌──────────────────────┐
   HTTP request  ──▶ │  api.py / cli.py      │  Translation only: parse input,
                     │  (thin boundary)      │  call service, shape output.
                     └──────────┬───────────┘  No business logic lives here.
                                │
                     ┌──────────▼───────────┐
                     │  service.py           │  The ONLY module that touches
                     │  (orchestration)       │  the native extension. Owns
                     └──────────┬───────────┘  array marshalling, GIL release.
                                │
                     ┌──────────▼───────────┐
                     │  core_engine (C++26)   │  Pure computation. No I/O,
                     │  (hot path)            │  no Python objects, no
                     └───────────────────────┘  allocation inside the loop.
```

**Say it out loud:** *"When you're deciding where a new piece of logic goes, ask: does it touch HTTP/CLI concerns only? api.py/cli.py. Does it orchestrate one or more native calls, or apply business rules? service.py. Is it pure, allocation-conscious numerical computation? The engine. Getting this placement right is what keeps 100% coverage achievable — each layer is trivially testable in isolation."*

[🔝 Back to Top](#table-of-contents)

---
---

## R2 · Swapping the C++26 Core for Your Algorithm

```
STEP  ACTION
────  ──────────────────────────────────────────────────────────────────────────────
 1    Replace the function body in libs/<lib>/include/<lib>/<algo>.h — keep the
      header-only pattern if your algorithm is simple enough; split into .h/.cc if
      the implementation grows past ~150 lines, per the Google C++ style guide

 2    Preserve the precondition-validation-at-the-boundary pattern: validate once,
      throw std::invalid_argument with a specific message, never let malformed
      input reach the hot loop

 3    Preserve the std::span<const T> parameter pattern for zero-copy ingestion;
      resist the temptation to accept std::vector by value

 4    Update libs/<lib>/tests/*_test.cc: one test per logical branch, including
      every precondition-violation path — this is what makes --cov-fail-under=100
      achievable rather than aspirational

 5    Update libs/<lib>/src/engine.cpp bindings: keep nb::gil_scoped_release,
      keep .noconvert() on ndarray args to force callers to pass correctly-typed
      contiguous arrays rather than silently paying a conversion-copy cost
```

**Say it out loud:** *"The engine's contract with Python is: you hand me validated, contiguous, correctly-typed memory, and I hand you back a result with zero hidden copies. Breaking that contract — accepting arbitrary Python objects, allocating inside the loop, doing I/O — is how a 'zero-copy C++ core' quietly becomes a liability."*

[🔝 Back to Top](#table-of-contents)

---
---

## R3 · Extending the Python Service & CLI

```
TO ADD...                        TOUCH THESE FILES                              DON'T FORGET
────────────────────────────    ──────────────────────────────────────────    ──────────────────────
A new request/response field     apps/<svc>/src/<pkg>/models.py                 field_validator for
                                                                                 cross-field invariants

A new REST endpoint               apps/<svc>/src/<pkg>/api.py                    A matching pytest in
                                                                                 tests/test_api.py; the
                                                                                 coverage gate will fail
                                                                                 the build otherwise

A new CLI command                 apps/<svc>/src/<pkg>/cli.py                    Route it through the
                                                                                 SAME service.py function
                                                                                 the API endpoint uses —
                                                                                 never duplicate logic
                                                                                 between CLI and API

A new dependency                  apps/<svc>/pyproject.toml [project]            Pin a minimum version;
                                   dependencies, or [project.optional-           re-run uv pip install
                                   dependencies] dev for test-only deps          -e .[dev] and confirm
                                                                                 pytest still passes
```

[🔝 Back to Top](#table-of-contents)

---
---

## R4 · Extending the TypeScript Dashboard

```
TO ADD...                        TOUCH THESE FILES                              DON'T FORGET
────────────────────────────    ──────────────────────────────────────────    ──────────────────────
A new backend call                apps/dashboard/src/api.ts                     Accept an injectable
                                                                                 fetch parameter (see
                                                                                 runAnalysis) so the
                                                                                 success AND failure
                                                                                 paths are unit-testable
                                                                                 without a real network

A new UI surface                  apps/dashboard/src/<Component>.tsx +          A matching *.test.tsx
                                   apps/dashboard/src/<Component>.test.tsx       using Testing Library;
                                                                                 assert on rendered
                                                                                 output, not implementation

A new dependency                  apps/dashboard/package.json                    Confirm it has first-
                                                                                 class TypeScript 7 types
                                                                                 or a maintained @types
                                                                                 package before adding
```

[🔝 Back to Top](#table-of-contents)

---
---

# 🏛️ INSTITUTIONAL HARDENING — WHAT TO ADD YOURSELF

---

## H1 · Coverage Discipline Beyond 100% Line Coverage

**Open with the intuition (15 seconds):**
> "100% line coverage tells you every line executed at least once. It does not tell you every *numerically dangerous input* was exercised. For a quant codebase, the second question matters more."

```
GAP THE TEMPLATE LEAVES OPEN                    WHAT TO ADD
───────────────────────────────────────────    ──────────────────────────────────────
No property-based / fuzz testing on the         Add RapidCheck or a hand-rolled
numerical core                                  randomized-input harness asserting
                                                 invariants (e.g., VWAP always lies
                                                 within [min price, max price])

No floating-point edge-case suite                Add explicit tests for NaN/Inf inputs,
(denormals, catastrophic cancellation)           extreme magnitude ratios, and confirm
                                                  the engine fails loudly rather than
                                                  producing silently wrong numbers

No mutation testing                              Consider mutmut (Python) / a C++
                                                  mutation tool to verify the test suite
                                                  actually kills injected bugs, not just
                                                  achieves line coverage
```

[🔝 Back to Top](#table-of-contents)

---
---

## H2 · From Single-Node to Production Topology

```
TEMPLATE GIVES YOU                              YOU ADD FOR REAL PRODUCTION
───────────────────────────────────────────    ──────────────────────────────────────
Single-box docker-compose.yml (backend +        Kubernetes manifests or Terraform-
nginx-fronted frontend on one Docker network)   provisioned ECS/EKS with horizontal pod
                                                 autoscaling on the backend service

nginx reverse proxy, no TLS termination          A TLS-terminating load balancer or
                                                 ingress controller in front of nginx;
                                                 nginx.conf as shipped assumes TLS is
                                                 handled upstream

No persistence layer                             A database/cache layer if your domain
                                                 logic needs state beyond a single
                                                 request/response cycle

No rate limiting / backpressure                  API gateway-level rate limiting;
                                                 consider a bounded request queue in
                                                 front of the native engine if request
                                                 volume can spike faster than the C++
                                                 core can drain it
```

[🔝 Back to Top](#table-of-contents)

---
---

## H3 · Observability, Secrets & Compliance Gaps to Close

```
TEMPLATE GIVES YOU                              YOU ADD FOR REAL PRODUCTION
───────────────────────────────────────────    ──────────────────────────────────────
Docker HEALTHCHECK on both containers            Structured logging (JSON) + a log
                                                 aggregator (ELK/Loki/CloudWatch);
                                                 the template intentionally ships
                                                 unstructured stdout logging only

No metrics/tracing instrumentation               OpenTelemetry spans around the
                                                 service.py → core_engine boundary,
                                                 since that's where P99 latency
                                                 actually lives

Secrets read from plain environment              A secrets manager (Vault/AWS Secrets
variables in Docker Compose                      Manager) for any real credential;
                                                 docker-compose.yml as shipped has
                                                 zero secrets by design — keep it that
                                                 way and inject at the orchestration
                                                 layer, never bake into the image

No audit trail on release provenance              If your firm requires it, extend the
beyond the git history + release tarball          release job to sign the tarball
checksum                                          (Sigstore/cosign) and record the
                                                   signing identity for auditability
```

**Say it out loud:** *"None of these gaps are oversights — they're seams left intentionally open because the correct answer is firm-specific. The template's job was to make sure you never have to ask 'wait, does our C++ engine even release the GIL correctly?' three months in. It was never going to be able to answer 'which secrets manager does our firm standardize on?' for you."*

[🔝 Back to Top](#table-of-contents)

---
---

## Quick-Reference Checklist

```
□  Repository created from template (not a fork of it)
□  All occurrences of core_engine / trading-strategy / dashboard renamed
□  cpp-tests, python-tests, frontend-tests all green on all OS matrix legs
□  release job produces a tag + CHANGELOG.md commit + tarball + checksum
□  RELEASE_PAT and CODECOV_TOKEN secrets configured
□  Branch protection requires all CI jobs before merge to main
□  Sample VWAP algorithm replaced; every new branch has a corresponding test
□  api.py / cli.py route through the same service.py function — no duplicated logic
□  api.ts calls use the injectable-fetch pattern for testability
□  Production topology (TLS, autoscaling, secrets manager) planned before go-live
```

[🔝 Back to Top](#table-of-contents)


---
---