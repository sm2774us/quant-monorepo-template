# syntax=docker/dockerfile:1
FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build g++-14 python3.13 python3.13-venv \
    curl ca-certificates && \
    rm -rf /var/lib/apt/lists/*

ENV CC=gcc-14 CXX=g++-14
RUN curl -LsSf https://astral.sh/uv/install.sh | sh
ENV PATH="/root/.local/bin:${PATH}"

WORKDIR /src
COPY libs/core-engine/ libs/core-engine/
COPY apps/trading-strategy/ apps/trading-strategy/

WORKDIR /src/apps/trading-strategy
RUN uv venv /opt/venv && \
    . /opt/venv/bin/activate && \
    uv pip install -e ../../libs/core-engine && \
    uv pip install -e .

FROM ubuntu:24.04 AS runtime
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3.13 && rm -rf /var/lib/apt/lists/* && \
    useradd --create-home --shell /usr/sbin/nologin appuser

COPY --from=build /opt/venv /opt/venv
ENV PATH="/opt/venv/bin:${PATH}"

WORKDIR /app
USER appuser
EXPOSE 8000
HEALTHCHECK --interval=10s --timeout=3s --retries=5 \
    CMD python3 -c "import urllib.request; urllib.request.urlopen('http://127.0.0.1:8000/api/v1/health', timeout=2)" || exit 1

ENTRYPOINT ["trading-strategy", "serve", "--host", "0.0.0.0", "--port", "8000"]
