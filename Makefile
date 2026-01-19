# PICkeeby Makefile
# PS/2 Keyboard Controller for Homebrew Computers

# Include generated configuration (run ./configure first)
-include config.mk

# Directories
BUILD_DIR := build
SRC_DIR := src
PLD_DIR := hardware/pld

# Tool defaults (can be overridden by config.mk or environment)
GALETTE ?= galette
JAVA ?= java
XC8 ?= xc8-cc
DFP ?= $(error Run ./configure first, or set DFP manually)
IPE ?= $(error Run ./configure first, or set IPE manually)

# Target device
MCU := 16F18344

# Compiler flags
# Note: DFP path must point to the xc8 subfolder within the Device Family Pack
# Paths with spaces are handled via quoting in the recipe
CFLAGS := -mcpu=$(MCU) -O2 -std=c99
LDFLAGS := -mcpu=$(MCU) -mwarn=-3

# Source files
FW_SRC := $(wildcard $(SRC_DIR)/*.c)
FW_OBJ := $(FW_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.p1)
FW_HEX := $(BUILD_DIR)/PICkeeby.hex

# PLD sources
PLD_SRC := $(PLD_DIR)/PICkeeby.pld
PLD_OUT := $(BUILD_DIR)/PICkeeby.jed

# Default target
.PHONY: all
all: firmware pld

# Build firmware
.PHONY: firmware
firmware: $(FW_HEX)

$(FW_HEX): $(FW_SRC) | $(BUILD_DIR)
	"$(XC8)" $(CFLAGS) "-mdfp=$(DFP)" $(LDFLAGS) -o $@ $(FW_SRC)

# Build PLD using galette
# galette outputs to same directory as input, so we copy input first
.PHONY: pld
pld: $(PLD_OUT)

$(PLD_OUT): $(PLD_SRC) | $(BUILD_DIR)
ifeq ($(HOST_OS),windows)
	copy "$(subst /,\,$(PLD_SRC))" "$(subst /,\,$(BUILD_DIR))"
	cd $(BUILD_DIR) && $(GALETTE) $(notdir $(PLD_SRC))
else
	cp $(PLD_SRC) $(BUILD_DIR)/
	cd $(BUILD_DIR) && $(GALETTE) $(notdir $(PLD_SRC))
endif

$(BUILD_DIR):
ifeq ($(HOST_OS),windows)
	if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
else
	mkdir -p $(BUILD_DIR)
endif

# Flash firmware using MPLAB IPE
# Requires PICkit or other Microchip programmer
# PROGRAMMER can be set to: PK3, PK4, PK5, SNAP, ICD4, ICD5, etc.
PROGRAMMER ?= PK5

.PHONY: flash
flash: $(FW_HEX)
	@echo "Flashing firmware using MPLAB IPE..."
	@echo "Note: Ensure your programmer is connected"
	"$(IPE)" -TP$(PROGRAMMER) -P$(MCU) -W -M -F"$(FW_HEX)"

# Flash PLD using minipro
.PHONY: flash-pld
flash-pld: $(PLD_OUT)
	minipro -p ATF22V10C -w $(PLD_OUT)

# Clean build outputs
.PHONY: clean
clean:
ifeq ($(HOST_OS),windows)
	if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
else
	rm -rf $(BUILD_DIR)
endif

# Remove all files matched by .gitignore
.PHONY: gitclean
gitclean:
	git clean -Xfd

# Help target
.PHONY: help
help:
	@echo "PICkeeby Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build everything (default)"
	@echo "  firmware  - Build PIC firmware (.hex)"
	@echo "  pld       - Build PLD/GAL using galette"
	@echo "  flash     - Flash firmware using MPLAB IPE"
	@echo "  flash-pld - Flash PLD using minipro"
	@echo "  clean     - Remove build outputs"
	@echo "  gitclean  - Remove all files matched by .gitignore"
	@echo "  help      - Show this help"
	@echo ""
	@echo "Configuration:"
	@echo "  MCU      = $(MCU)"
	@echo "  XC8      = $(XC8)"
	@echo ""
	@echo "Run ./configure (or .\\configure.ps1 on Windows) first."

