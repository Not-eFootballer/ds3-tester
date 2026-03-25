# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

DS3 Button Tester — PS Vita / PS TV homebrew app that displays all DualShock 3 controller inputs in real-time. Written in C, targets VitaSDK (ARM Cortex-A9). Uses direct framebuffer rendering (no GPU/GXM).

## Build

Requires VitaSDK installed with `$VITASDK` env var set:

```bash
export VITASDK=/usr/local/vitasdk
export PATH=$VITASDK/bin:$PATH
./build.sh
```

Output: `build/ds3_tester.vpk` — install on Vita/PS TV via VitaShell.

Manual build:
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake"
make -j$(nproc)
```

## Architecture

Single-file app (`src/main.c`, ~400 lines) with no external dependencies beyond VitaSDK stubs.

**Rendering pipeline:** Software framebuffer in CDRAM (960×544 RGBA8888) → `sceDisplaySetFrameBuf` each frame. All drawing is CPU-based using primitive functions (fill_rect, fill_circle, draw_text). Includes an embedded 5×7 bitmap font (ASCII 32-127).

**Input handling:** Reads two controller ports via `sceCtrlPeekBufferPositiveExt2` — port 0 (Vita built-in) and port 1 (external DS3 on PS TV) — then merges them with OR for buttons and override for analog axes.

**Color format:** A8B8G8R8 (`0xAABBGGRR`) — note this is NOT standard ARGB. When adding colors, blue and red channels are swapped vs typical hex.

## Key Constants

- `VITA_TITLEID`: `BTNT00001`
- `VITA_APP_NAME`: `DS3 Button Tester`
- Exit combo: START + SELECT pressed simultaneously

## Linked Stubs

SceDisplay, SceCtrl, SceProcessmgr, SceSysmem (see CMakeLists.txt). Adding new Vita API calls requires adding the corresponding `_stub` library.
