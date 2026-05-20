# syntax=docker/dockerfile:1.7
#
# pac-man build container (#8).
#
# Self-contained toolchain image so CI (and devcontainers / off-CI
# consumers) can build the Agon Light 2 binary without bespoke host
# setup. Single platform — no cc65 / sjasmplus / neo packager / etc.
#
#   ez80asm             Agon Light 2 assembler (AgonPlatform GitHub release)
#   python3 + Pillow    asset pipeline (sprite-sheet extractors)
#   make                build driver
#
# Two-stage build: stage 1 fetches the ez80asm release tarball; stage 2
# is the runtime image with python + the staged binary. Keeps the final
# image free of build-essential / curl / jq.
#
# Build:   docker build -t ghcr.io/andymccall/pac-man-builder:dev .
# Run:     docker run --rm -v "$PWD:/work" ghcr.io/andymccall/pac-man-builder:dev make

# ---------------------------------------------------------------------------
# Stage 1 — fetch ez80asm release.
# ---------------------------------------------------------------------------
FROM debian:bookworm-slim AS builder

ARG DEBIAN_FRONTEND=noninteractive
ARG TARGETARCH

# Pin to "latest" by default; bump intentionally when upstream ships a
# relevant fix. EZ80ASM_RELEASE can be set to a specific tag.
ARG EZ80ASM_RELEASE=latest

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        curl \
        file \
        jq \
        tar \
    && rm -rf /var/lib/apt/lists/*

# AgonPlatform/agon-ez80asm publishes pre-built Linux binaries on its
# GitHub releases page. The Linux asset name has shifted across
# releases; we filter for one that mentions "linux" + the host arch,
# and reject Windows / macOS / Agon-hardware builds. file(1) sniffs
# the actual format because the asset filename has lied about it
# before (e.g. ".gz" actually being .tar.gz).
RUN set -eux; \
    case "${TARGETARCH:-amd64}" in \
        amd64) ASSET_ARCH="x86_64" ;; \
        arm64) ASSET_ARCH="aarch64" ;; \
        *) echo "Unsupported TARGETARCH: ${TARGETARCH}" >&2; exit 1 ;; \
    esac; \
    if [ "${EZ80ASM_RELEASE}" = "latest" ]; then \
        REL_API="https://api.github.com/repos/AgonPlatform/agon-ez80asm/releases/latest"; \
    else \
        REL_API="https://api.github.com/repos/AgonPlatform/agon-ez80asm/releases/tags/${EZ80ASM_RELEASE}"; \
    fi; \
    ASSET_URL=$(curl -fsSL "${REL_API}" \
        | jq -r ".assets[].browser_download_url \
                 | select(test(\"linux\"; \"i\")) \
                 | select(test(\"${ASSET_ARCH}\"; \"i\")) \
                 | select(test(\"\\\\.exe$|\\\\.bin$|darwin|macos|win\"; \"i\") | not)" \
        | head -1); \
    [ -n "${ASSET_URL}" ] || { echo "No suitable ez80asm asset found"; exit 1; }; \
    echo "Fetching ${ASSET_URL}"; \
    curl -fsSL -o /tmp/ez80asm-asset "${ASSET_URL}"; \
    mkdir -p /tmp/ez80asm; \
    cd /tmp/ez80asm; \
    case "$(file -b /tmp/ez80asm-asset)" in \
        *gzip*) \
            if file -b --uncompress /tmp/ez80asm-asset | grep -qi 'tar archive'; then \
                tar -xzf /tmp/ez80asm-asset; \
            else \
                gunzip -c /tmp/ez80asm-asset > ez80asm; chmod +x ez80asm; \
            fi ;; \
        *tar*) tar -xf /tmp/ez80asm-asset ;; \
        *executable*|*ELF*) cp /tmp/ez80asm-asset ez80asm; chmod +x ez80asm ;; \
        *) echo "Unknown asset format"; file -b /tmp/ez80asm-asset; exit 1 ;; \
    esac; \
    EZ80_BIN=$(find /tmp/ez80asm -maxdepth 3 -type f -name 'ez80asm' | head -1); \
    [ -n "${EZ80_BIN}" ] || { echo "ez80asm binary not in extracted asset"; ls -laR /tmp/ez80asm; exit 1; }; \
    install -m 0755 "${EZ80_BIN}" /usr/local/bin/ez80asm; \
    rm -rf /tmp/ez80asm /tmp/ez80asm-asset; \
    /usr/local/bin/ez80asm --version || /usr/local/bin/ez80asm -v || true

# ---------------------------------------------------------------------------
# Stage 2 — runtime image. Keeps curl / jq / file out of the final
# layer so `docker pull` is small and CI runs are fast.
# ---------------------------------------------------------------------------
FROM debian:bookworm-slim

ARG DEBIAN_FRONTEND=noninteractive

# python3 + Pillow drive the asset pipeline (sprite-sheet extractors
# in scripts/, font / banner / score-popup bitmap generators). make is
# the build driver.
RUN apt-get update && apt-get install -y --no-install-recommends \
        make \
        python3 \
        python3-pil \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/local/bin/ez80asm /usr/local/bin/ez80asm

# OCI image metadata (GHCR uses these for the package page).
LABEL org.opencontainers.image.source="https://github.com/andymccall/pac-man"
LABEL org.opencontainers.image.description="Agon Light 2 build toolchain for pac-man (ez80asm + Python/Pillow)."
LABEL org.opencontainers.image.licenses="MIT"

WORKDIR /work
CMD ["bash"]
