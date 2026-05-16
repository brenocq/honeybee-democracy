#!/bin/bash
set -o pipefail

# ── Colors ────────────────────────────────────────────────────────────────────
BOLD='\033[1m'
DIM='\033[2m'
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

# ── Defaults ──────────────────────────────────────────────────────────────────
BUILD_TYPE="Release"
EXECUTE=false
TEST=false
CORES=8

# ── Help ──────────────────────────────────────────────────────────────────────
usage() {
    echo -e ""
    echo -e "  ${BOLD}Honeybee Democracy build script${NC}"
    echo -e ""
    echo -e "  ${BOLD}Usage:${NC} ./build.sh [options]"
    echo -e ""
    echo -e "  ${BOLD}Options:${NC}"
    echo -e "    ${CYAN}-d, --debug${NC}      Build in Debug mode   ${DIM}(default: Release)${NC}"
    echo -e "    ${CYAN}-j, --jobs${NC}  N    Number of parallel jobs ${DIM}(default: 8)${NC}"
    echo -e "    ${CYAN}-t, --test${NC}       Run tests after a successful build"
    echo -e "    ${CYAN}-x, --execute${NC}    Run the binary after a successful build"
    echo -e "    ${CYAN}-h, --help${NC}       Show this help message"
    echo -e ""
}

# ── Argument parsing ──────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)   BUILD_TYPE="Debug";  shift ;;
        -j|--jobs)    CORES="$2";          shift 2 ;;
        -t|--test)    TEST=true;           shift ;;
        -x|--execute) EXECUTE=true;        shift ;;
        -h|--help)    usage;               exit 0 ;;
        *)
            echo -e "${RED}Unknown option:${NC} $1"
            usage
            exit 1
            ;;
    esac
done

BUILD_DIR="build/$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')"
BINARY="$BUILD_DIR/honeybee_democracy"

# ── Header ────────────────────────────────────────────────────────────────────
echo -e ""
echo -e "  ${BOLD}Honeybee Democracy${NC} ${DIM}·${NC} ${CYAN}${BUILD_TYPE}${NC}"
echo -e "  ${DIM}────────────────────────────${NC}"

# ── Configure ─────────────────────────────────────────────────────────────────
echo -e ""
echo -e "  ${BOLD}Configuring...${NC}"
if ! cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON 2>&1 \
    | sed 's/^/  /'; then
    echo -e ""
    echo -e "  ${RED}${BOLD}✗ Configure failed${NC}"
    echo -e ""
    exit 1
fi

# ── Build ─────────────────────────────────────────────────────────────────────
echo -e ""
echo -e "  ${BOLD}Building...${NC}"
if ! cmake --build "$BUILD_DIR" --parallel "$CORES" 2>&1 | sed 's/^/  /'; then
    echo -e ""
    echo -e "  ${RED}${BOLD}✗ Build failed${NC}"
    echo -e ""
    exit 1
fi

echo -e ""
echo -e "  ${GREEN}${BOLD}✓ Build succeeded${NC}  ${DIM}→ $BINARY${NC}"
echo -e ""

# ── Test ──────────────────────────────────────────────────────────────────────
if $TEST; then
    echo -e "  ${BOLD}Running tests...${NC}"
    if ! ctest --test-dir "$BUILD_DIR" --output-on-failure --no-tests=error 2>&1 | sed 's/^/  /'; then
        echo -e ""
        echo -e "  ${RED}${BOLD}✗ Tests failed${NC}"
        echo -e ""
        exit 1
    fi

    echo -e ""
    echo -e "  ${GREEN}${BOLD}✓ Tests passed${NC}"
    echo -e ""
fi

# ── Execute ───────────────────────────────────────────────────────────────────
if $EXECUTE; then
    echo -e "  ${BOLD}Running ${CYAN}honeybee_democracy${NC}${BOLD}...${NC}"
    echo -e "  ${DIM}────────────────────────────${NC}"
    echo -e ""
    "$BINARY"
fi
