# PICkeeby Makefile

-include config.mk

ifeq ($(OS),Windows_NT)
	RUNTIME_OS := windows
else
	RUNTIME_OS := unix
endif

BUILD_DIR := build
SRC_DIR := src
PLD_DIR := hardware/pld

GALETTE ?= galette
XC8 ?= xc8-cc
DFP ?= $(error Run ./configure first)
IPE ?= $(error Run ./configure first)
PROGRAMMER ?= PK5
MCU ?= 16F18344
CFLAGS := -mcpu=$(MCU) -O2 -std=c99
LDFLAGS := -mcpu=$(MCU) -mwarn=-3

FW_SRC := $(wildcard $(SRC_DIR)/*.c)
FW_HEX := $(BUILD_DIR)/PICkeeby.hex
PLD_SRC := $(PLD_DIR)/PICkeeby.pld
PLD_OUT := $(BUILD_DIR)/PICkeeby.jed

.PHONY: all firmware pld flash flash-pld clean gitclean help

all: firmware pld

firmware: $(FW_HEX)

$(FW_HEX): $(FW_SRC) | $(BUILD_DIR)
	"$(XC8)" $(CFLAGS) "-mdfp=$(DFP)" $(LDFLAGS) -o $@ $(FW_SRC)

pld: $(PLD_OUT)

$(PLD_OUT): $(PLD_SRC) | $(BUILD_DIR)
ifeq ($(RUNTIME_OS),windows)
	copy "$(subst /,\,$(PLD_SRC))" "$(subst /,\,$(BUILD_DIR))"
else
	cp $(PLD_SRC) $(BUILD_DIR)/
endif
	cd $(BUILD_DIR) && $(GALETTE) $(notdir $(PLD_SRC))

$(BUILD_DIR):
ifeq ($(RUNTIME_OS),windows)
	if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
else
	mkdir -p $(BUILD_DIR)
endif

flash: $(FW_HEX)
	"$(IPE)" -TP$(PROGRAMMER) -P$(MCU) -W -M -F"$(FW_HEX)"

flash-pld: $(PLD_OUT)
	minipro -p ATF22V10C -w $(PLD_OUT)

clean:
ifeq ($(RUNTIME_OS),windows)
	if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
else
	rm -rf $(BUILD_DIR)
endif

gitclean:
	git clean -Xfd

define HELPTEXT
PICkeeby

Targets:
  all       - Build everything
  firmware  - Build PIC firmware
  pld       - Build PLD/GAL using galette
  flash     - Flash firmware using MPLAB IPE
  flash-pld - Flash PLD using minipro
  clean     - Remove build outputs
  gitclean  - Remove all files matched by .gitignore
  help      - Show this help

Configuration (override with environment variables):
  MCU        = $(MCU)
  PROGRAMMER = $(PROGRAMMER)

Run ./configure first.
endef

help:
	$(info $(HELPTEXT))
