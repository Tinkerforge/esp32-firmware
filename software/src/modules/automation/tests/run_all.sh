#!/bin/bash
# Run all automation integration tests against a device.
#
# Usage:
#   ./run_all.sh <device-ip>
#   ./run_all.sh <device-ip> --verbose

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ $# -lt 1 ]; then
    echo "Usage: $0 <device-ip> [--verbose]"
    exit 1
fi

HOST="$1"
shift
EXTRA_ARGS=("$@")

TESTS=(
    test_automation_relay.py
    test_automation_http.py
    test_automation_sg_ready.py
    test_automation_dap.py
    test_automation_delay.py
    test_automation_delayed_cancel.py
    test_automation_combined.py
    test_automation_cron.py
)

PASSED=0
FAILED=0
FAILED_NAMES=()

for test in "${TESTS[@]}"; do
    echo ""
    echo "================================================================"
    echo "  Running: $test"
    echo "================================================================"
    if python3 "$SCRIPT_DIR/$test" "$HOST" "${EXTRA_ARGS[@]}"; then
        PASSED=$((PASSED + 1))
    else
        FAILED=$((FAILED + 1))
        FAILED_NAMES+=("$test")
    fi
done

echo ""
echo "================================================================"
echo "  All tests done: $PASSED passed, $FAILED failed, ${#TESTS[@]} total"
echo "================================================================"

if [ $FAILED -gt 0 ]; then
    echo ""
    echo "Failed:"
    for name in "${FAILED_NAMES[@]}"; do
        echo "  - $name"
    done
    exit 1
fi
