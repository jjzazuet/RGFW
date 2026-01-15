#!/bin/bash
# Test script for HiDPI scale detection

echo "=========================================="
echo "RGFW HiDPI Scale Detection Test"
echo "=========================================="
echo ""
echo "This test will create 3 windows sequentially."
echo "Watch for these messages:"
echo "  - 'Initial scale: X.XXx x X.XXx' when window is created"
echo "  - '🔍 HiDPI Scale Update' if scale changes"
echo "  - 'Final scale: X.XXx x X.XXx' when window closes"
echo ""
echo "Expected behavior on HiDPI displays (e.g., 3x scaling):"
echo "  - Initial scale should show 3.00x x 3.00x"
echo "  - No scale updates during normal operation"
echo "  - If you move window between displays with different DPI,"
echo "    you should see scale update callbacks"
echo ""
echo "Press Enter to start test..."
read

./libdecor_multi_test

echo ""
echo "=========================================="
echo "Test completed!"
echo "=========================================="
