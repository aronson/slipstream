FROM debian:bookworm-slim AS builder

WORKDIR /usr/src/app

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y \
    meson \
    cmake \
    git \
    pkg-config \
    libssl-dev \
    ninja-build \
    clang

COPY . .

RUN --mount=type=cache,target=/usr/src/app/meson-build-release \
    CC=clang CXX=clang++ meson setup --wipe \
    -Db_lto=true \
    --buildtype=release \
    --warnlevel=0 \
    -Ddefault_library=static \
    meson-build-release && \
    meson compile -C meson-build-release && \
    cp meson-build-release/slipstream-client . && \
    cp meson-build-release/slipstream-server .

FROM gcr.io/distroless/base-debian12 AS runtime

WORKDIR /usr/src/app

COPY ./certs/ ./certs/

ENV PATH=/usr/src/app/:$PATH

LABEL org.opencontainers.image.source=https://github.com/EndPositive/slipstream

FROM runtime AS client

COPY --from=builder --chmod=755 /usr/src/app/slipstream-client .

ENTRYPOINT ["/usr/src/app/slipstream-client"]

FROM runtime AS server

COPY --from=builder --chmod=755 /usr/src/app/slipstream-server .

ENTRYPOINT ["/usr/src/app/slipstream-server"]
