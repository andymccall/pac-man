# ---------------------------------------------------------------------------
# Pac-Man - Agon Light Makefile
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# Tools
# ---------------------------------------------------------------------------
EZ80ASM            = ez80asm
EZ80ASM_FLAGS      = -x
FAB_AGON_EMU       = fab-agon-emulator
FAB_AGON_EMU_FLAGS = --vdp ~/development/tools/fab-agon/firmware/vdp_console8.so \
                     --mos ~/development/tools/fab-agon/firmware/mos_console8.bin \
                     --sdcard bin/

# ---------------------------------------------------------------------------
# Directories & Project Info
# ---------------------------------------------------------------------------
NAME               = pac-man
SRCDIR             = src
BUILDDIR           = bin
RELEASEDIR         = release

GIT_INFO           := $(shell git describe --always --tags)

# ---------------------------------------------------------------------------
# Hardware: Agon Light (eZ80)
# ---------------------------------------------------------------------------
# Every file listed here is a dependency. If one changes, $(AGON_OUT) rebuilds.
AGON_SRCS = $(wildcard $(SRCDIR)/*.asm) \
            $(wildcard $(SRCDIR)/**/*.asm) \
            $(wildcard $(SRCDIR)/**/*.inc)

AGON_MAIN = $(SRCDIR)/$(NAME).asm
AGON_OUT  = $(BUILDDIR)/$(NAME).bin

# ---------------------------------------------------------------------------
# Phony Targets
# ---------------------------------------------------------------------------
.PHONY: all build run release clean help

all: build

# ---------------------------------------------------------------------------
# Build Logic
# ---------------------------------------------------------------------------

# 'build' just points to the output file. 
# If $(AGON_OUT) is up to date, nothing happens.
build: $(AGON_OUT)

$(AGON_OUT): $(AGON_SRCS)
	@echo "Detected changes in source files. Assembling..."
	@mkdir -p $(BUILDDIR)
	$(EZ80ASM) $(EZ80ASM_FLAGS) $(AGON_MAIN)
	@if [ -f $(SRCDIR)/$(NAME).bin ]; then \
		mv $(SRCDIR)/$(NAME).bin $(AGON_OUT); \
	fi
	@echo "Build complete: $(AGON_OUT)"

run: build
	@echo "Launching Emulator..."
	$(FAB_AGON_EMU) $(FAB_AGON_EMU_FLAGS)

# ---------------------------------------------------------------------------
# Housekeeping & Release
# ---------------------------------------------------------------------------

help:
	@echo "Agon Build System:"
	@echo "  build    (default) Rebuilds only if .asm or .inc files changed"
	@echo "  run      Builds (if needed) and launches emulator"
	@echo "  clean    Force remove all build artifacts"

clean:
	@echo "Cleaning project..."
	rm -rf $(BUILDDIR)
	rm -rf $(RELEASEDIR)
	rm -f $(SRCDIR)/*.bin

release: build
	@echo "Packaging release $(GIT_INFO)..."
	@mkdir -p $(RELEASEDIR)/$(NAME)
	cp $(AGON_OUT) $(RELEASEDIR)/$(NAME)/
	@test -f README.md && cp README.md $(RELEASEDIR)/$(NAME)/ || true
	cd $(RELEASEDIR) && zip -r $(NAME)-$(GIT_INFO).zip $(NAME)/
	@rm -rf $(RELEASEDIR)/$(NAME)