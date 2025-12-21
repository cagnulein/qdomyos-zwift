#!/bin/bash
# QZ TUI STABILITY DIAGNOSTIC

# 1. Setup environment to match your dashboard
SET_U_VAL=$(set -o | grep nounset | awk '{print $2}')
if [ "$SET_U_VAL" == "off" ]; then set -u; fi

# Colors
BLUE='\033[1;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Test Path (This often causes the sed error)
TEST_PATH="/home/adam/.config/Roberto Viola/qDomyos-Zwift.conf"
TEST_STR="${BLUE}║${NC} Config: ${CYAN}${TEST_PATH}${NC}"

echo "--- TUI LOGIC DIAGNOSTIC ---"
echo "BASH_VERSION: $BASH_VERSION"
echo "Path to test: $TEST_PATH"

# ---------------------------------------------------------
# TEST 1: Width Calculation Methods
# ---------------------------------------------------------
echo -e "\n[1] TESTING WIDTH METHODS (Check for 'sed' or 'command not found' errors)"

# Method A: Standard Sed with / (Likely to fail)
echo -n "Method A (Sed /): "
(echo -e "$TEST_STR" | sed 's/\x1b\[[0-9;]*[a-zA-Z]//g' | wc -m) 2>&1 || echo "CRASHED"

# Method B: Sed with @ delimiter
echo -n "Method B (Sed @): "
(echo -e "$TEST_STR" | sed 's@\x1b\[[0-9;]*[a-zA-Z]@@g' | wc -m) 2>&1 || echo "CRASHED"

# Method C: Bash Pattern Replacement (Check for globbing error)
echo -n "Method C (Bash Pattern): "
ANSI_PATTERN=$'\E'\[[0-9;]*[a-zA-Z]
(PLAIN="${TEST_STR//$ANSI_PATTERN/}"; echo "${#PLAIN}") 2>&1 || echo "CRASHED"

# Method D: Pipe-based cleanup (The "Safe" way)
echo -n "Method D (Pipe-Cleanup): "
(printf '%b' "$TEST_STR" | sed 's/\x1b\[[0-9;]*[a-zA-Z]//g' | tr -d '\n\r' | wc -m) 2>&1 || echo "CRASHED"

# ---------------------------------------------------------
# TEST 2: Interactive Input Methods
# ---------------------------------------------------------
echo -e "\n[2] TESTING INPUT METHODS"
echo "Instructions: For each test, type '5' then hit ENTER."

# Input A: The Research DD Method
echo -n "Method A (Raw stty + dd): "
old_stty=$(stty -g)
stty -icanon -echo min 1 time 0
result_a=$(dd bs=1 count=1 2>/dev/null </dev/tty)
stty "$old_stty"
echo "Received: [$result_a]"

# Input B: The Read -rsn1 Method
echo -n "Method B (read -rsn1): "
IFS= read -rsn1 result_b
echo "Received: [$result_b]"

echo -e "\n--- DIAGNOSTIC COMPLETE ---"
echo "Please copy and paste the text above back into the chat."
