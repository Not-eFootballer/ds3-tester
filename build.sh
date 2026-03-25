#!/bin/bash
# ============================================
# DS3 Button Tester - Build Script
# Requires: VitaSDK installed
# ============================================

set -e

if [ -z "$VITASDK" ]; then
    echo "ERROR: VITASDK environment variable not set!"
    echo ""
    echo "Install VitaSDK first:"
    echo "  https://vitasdk.org/"
    echo ""
    echo "Then set:"
    echo "  export VITASDK=/usr/local/vitasdk"
    echo "  export PATH=\$VITASDK/bin:\$PATH"
    exit 1
fi

echo "=== DS3 Button Tester - Building ==="
echo "VitaSDK: $VITASDK"

mkdir -p build
cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake"
make -j$(nproc)

echo ""
echo "=== Build Complete ==="
if [ -f ds3_tester.vpk ]; then
    echo "VPK ready: build/ds3_tester.vpk"
    echo ""
    echo "Transfer to your PS Vita/PS TV and install with VitaShell."
else
    echo "ERROR: VPK not found. Check build errors above."
    exit 1
fi
