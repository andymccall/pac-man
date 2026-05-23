# ---------------------------------------------------------------------------
# Pac-Man — Agon Light 2 Makefile
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# Tools
# ---------------------------------------------------------------------------
EZ80ASM            = ez80asm
EZ80ASM_FLAGS      = -x
FAB_AGON_EMU       = fab-agon-emulator
FAB_AGON_EMU_FLAGS = --vdp $(HOME)/development/tools/fab-agon/firmware/vdp_console8.so \
                     --mos $(HOME)/development/tools/fab-agon/firmware/mos_console8.bin \
                     --sdcard bin/

# ---------------------------------------------------------------------------
# Project info
# ---------------------------------------------------------------------------
NAME               = pac-man
# Output binary name follows the Agon `dir` convention (uppercase 8.3
# basename + .BIN extension) so it sits naturally alongside the other
# MOS-shipped binaries on the SD card. Source files / release zip /
# repo / branch names stay lowercase (Unix convention).
BIN_NAME           = PAC-MAN.BIN
SRCDIR             = src
BUILDDIR           = bin
RELEASEDIR         = release

GIT_INFO           := $(shell git describe --always --tags 2>/dev/null || echo dev)

# ---------------------------------------------------------------------------
# Sources & dependencies
# ---------------------------------------------------------------------------
# Recursive find — GNU Make's "**" wildcard is NOT a globstar (it's the same
# as "*"), so $(SRCDIR)/**/*.inc only catches one-level subdirs. Files like
# src/includes/game/states/dying.inc are 3+ levels deep and would silently
# fall out of the dependency list, causing edits to those files to be
# ignored by `make` ("Nothing to be done for 'all'" despite real changes).
# See #75.
AGON_SRCS := $(shell find $(SRCDIR) -type f \( -name '*.asm' -o -name '*.inc' \))

# Asset files (.rgba2 binaries that are .incbin'd from the .inc tree).
# Editing one of these should rebuild too.
AGON_ASSETS := $(shell find $(SRCDIR)/assets -type f -name '*.rgba2' 2>/dev/null)

AGON_MAIN = $(SRCDIR)/$(NAME).asm
AGON_OUT  = $(BUILDDIR)/$(BIN_NAME)

# ---------------------------------------------------------------------------
# ANSI colour + emoji output
# ---------------------------------------------------------------------------
# Pure-shell ANSI escapes via printf. Pass NO_COLOR=1 to make for plain
# output (CI logs, dumb terminals).
ifndef NO_COLOR
BOLD    := \033[1m
RESET   := \033[0m
CYAN    := \033[0;36m
GREEN   := \033[0;32m
YELLOW  := \033[1;33m
RED     := \033[0;31m
MAGENTA := \033[0;35m
DIM     := \033[2m
endif

RULE := ──────────────────────────────────────────────────────────────

# ---------------------------------------------------------------------------
# Docker (containerised build via Dockerfile, #8)
# ---------------------------------------------------------------------------
# Image tag is overridable so CI can target a registry-qualified name and
# devs can pin to a specific build. The volume mount + --user keeps any
# build artifacts owned by the host user, not root.
DOCKER_IMAGE      ?= pac-man-builder:dev
DOCKER_RUN_FLAGS  ?= --rm -v "$(CURDIR):/work" -w /work --user "$$(id -u):$$(id -g)"
# Targets that make sense to run inside the container. `run` is excluded —
# it needs the host emulator + a display, can't go in a sealed container.
DOCKER_INNER_TARGETS = build clean release all

# ---------------------------------------------------------------------------
# Phony targets
# ---------------------------------------------------------------------------
.PHONY: all build run release clean help \
        docker-image docker-shell $(addprefix docker-, $(DOCKER_INNER_TARGETS))

all: build

# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------
build: $(AGON_OUT)

$(AGON_OUT): $(AGON_SRCS) $(AGON_ASSETS)
	@printf "\n$(BOLD)$(CYAN)🟡 Assembling $(NAME) for Agon Light 2$(RESET)\n"
	@printf "$(CYAN)$(RULE)$(RESET)\n"
	@printf "$(DIM)sources: $(words $(AGON_SRCS))   assets: $(words $(AGON_ASSETS))$(RESET)\n"
	@mkdir -p $(BUILDDIR)
	$(EZ80ASM) $(EZ80ASM_FLAGS) $(AGON_MAIN)
	@if [ -f $(SRCDIR)/$(NAME).bin ]; then \
		mv $(SRCDIR)/$(NAME).bin $(AGON_OUT); \
	fi
	@SIZE=$$(stat -c %s $(AGON_OUT) 2>/dev/null || stat -f %z $(AGON_OUT)); \
	printf "$(BOLD)$(GREEN)✅ Build complete: $(AGON_OUT)$(RESET) $(DIM)($$SIZE bytes)$(RESET)\n"

# ---------------------------------------------------------------------------
# Run
# ---------------------------------------------------------------------------
run: build
	@printf "\n$(BOLD)$(CYAN)🚀 Launching $(NAME) in fab-agon-emulator$(RESET)\n"
	@printf "$(CYAN)$(RULE)$(RESET)\n"
	$(FAB_AGON_EMU) $(FAB_AGON_EMU_FLAGS)

# ---------------------------------------------------------------------------
# Housekeeping & release
# ---------------------------------------------------------------------------
clean:
	@printf "\n$(BOLD)$(YELLOW)🧹 Cleaning build artifacts$(RESET)\n"
	@printf "$(YELLOW)$(RULE)$(RESET)\n"
	rm -rf $(BUILDDIR)
	rm -rf $(RELEASEDIR)
	rm -f $(SRCDIR)/*.bin
	@printf "$(BOLD)$(GREEN)✅ Clean$(RESET)\n"

release: build
	@printf "\n$(BOLD)$(MAGENTA)📦 Packaging release $(GIT_INFO)$(RESET)\n"
	@printf "$(MAGENTA)$(RULE)$(RESET)\n"
	@mkdir -p $(RELEASEDIR)/$(NAME)
	cp $(AGON_OUT) $(RELEASEDIR)/$(NAME)/
	@test -f README.md && cp README.md $(RELEASEDIR)/$(NAME)/ || true
	cd $(RELEASEDIR) && zip -r $(NAME)-$(GIT_INFO).zip $(NAME)/
	@rm -rf $(RELEASEDIR)/$(NAME)
	@printf "$(BOLD)$(GREEN)✅ Release: $(RELEASEDIR)/$(NAME)-$(GIT_INFO).zip$(RESET)\n"

# ---------------------------------------------------------------------------
# Docker targets
# ---------------------------------------------------------------------------
docker-image:
	@printf "\n$(BOLD)$(CYAN)🐳 Building toolchain image $(DOCKER_IMAGE)$(RESET)\n"
	@printf "$(CYAN)$(RULE)$(RESET)\n"
	docker build -t $(DOCKER_IMAGE) .
	@printf "$(BOLD)$(GREEN)✅ Image built: $(DOCKER_IMAGE)$(RESET)\n"

docker-shell: docker-image
	@printf "\n$(BOLD)$(CYAN)🐳 Dropping into $(DOCKER_IMAGE)$(RESET)\n"
	@printf "$(CYAN)$(RULE)$(RESET)\n"
	docker run -it $(DOCKER_RUN_FLAGS) $(DOCKER_IMAGE) bash

# Pattern rule: `make docker-<inner>` builds the image (no-op if present)
# then runs `make <inner>` inside it. The plain `docker-build` runs the
# host's `make build` inside the container — useful for verifying a
# fresh clone assembles cleanly in the locked toolchain.
$(addprefix docker-, $(DOCKER_INNER_TARGETS)): docker-% : docker-image
	@printf "\n$(BOLD)$(CYAN)🐳 make $* (in $(DOCKER_IMAGE))$(RESET)\n"
	@printf "$(CYAN)$(RULE)$(RESET)\n"
	docker run $(DOCKER_RUN_FLAGS) $(DOCKER_IMAGE) make $*
	@printf "$(BOLD)$(GREEN)✅ docker-$* complete$(RESET)\n"

# ---------------------------------------------------------------------------
# Help
# ---------------------------------------------------------------------------
help:
	@printf "\n$(BOLD)$(CYAN)🟡 Pac-Man — Agon Light 2 build$(RESET)\n"
	@printf "$(CYAN)$(RULE)$(RESET)\n"
	@printf "  $(BOLD)build$(RESET)    rebuild $(AGON_OUT) only if a source/asset changed (default)\n"
	@printf "  $(BOLD)run$(RESET)      build (if needed) and launch fab-agon-emulator\n"
	@printf "  $(BOLD)release$(RESET)  build + package as $(RELEASEDIR)/$(NAME)-<git>.zip\n"
	@printf "  $(BOLD)clean$(RESET)    remove $(BUILDDIR)/, $(RELEASEDIR)/, and any stray *.bin in $(SRCDIR)/\n"
	@printf "\n$(BOLD)$(CYAN)🐳 Containerised$(RESET)\n"
	@printf "  $(BOLD)docker-image$(RESET)    build the toolchain image (#8) → $(DOCKER_IMAGE)\n"
	@printf "  $(BOLD)docker-build$(RESET)    run \`make build\` inside the container\n"
	@printf "  $(BOLD)docker-clean$(RESET)    run \`make clean\` inside the container\n"
	@printf "  $(BOLD)docker-release$(RESET)  run \`make release\` inside the container\n"
	@printf "  $(BOLD)docker-shell$(RESET)    drop into a bash shell inside the container\n"
	@printf "\n$(DIM)Set NO_COLOR=1 to suppress ANSI escapes (CI / dumb terminals).$(RESET)\n"
	@printf "$(DIM)Override DOCKER_IMAGE=… to use a different image tag (default: $(DOCKER_IMAGE)).$(RESET)\n"
