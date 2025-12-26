#!/usr/bin/bash
# =============================================================================
# QDomyos-Zwift: Universal Docker Build Script
#
# Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
# Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
#
# Unified Docker build helper for multi-architecture images (x86-64, arm64).
# Usage (from src/): ./docker-build.sh --arch <arch> [--enable-debug-logs]
# =============================================================================

set -euo pipefail

# Build configuration flags
ENABLE_DEBUG_LOGS=false

# Color codes
C_GREEN="\033[0;32m"
C_RED="\033[0;31m"
C_YELLOW="\033[0;33m"
C_BLUE="\033[0;34m"
C_RESET="\033[0m"

# Helper functions
err() {
    echo -e "\n${C_RED}✗ ERROR: $*${C_RESET}" >&2
    exit 1
}

info() {
    echo -e "${C_BLUE}>>> $*${C_RESET}"
}

success() {
    echo -e "${C_GREEN}✓ SUCCESS:${C_RESET} $*"
}

show_usage() {
    echo "Usage: $0 --arch <ARCHITECTURE> (arm64 or x86-64)"
}

# Find project root directory
find_project_root() {
    local current_dir="$PWD"
    local max_depth=10
    local depth=0
    
    while [[ $depth -lt $max_depth ]]; do
        if [[ -d "$current_dir/docker" ]] && [[ -d "$current_dir/src" ]]; then
            echo "$current_dir"
            return 0
        fi
        
        if [[ "$current_dir" == "/" ]]; then
            break
        fi
        
        current_dir="$(dirname "$current_dir")"
        ((depth++))
    done
    
    return 1
}

setup_and_verify_docker() {
    if ! command -v docker >/dev/null 2>&1; then
        err "Docker is not installed."
    fi
    
    if ! docker info >/dev/null 2>&1; then
        err "Docker is not running or user is not in docker group. Try: sudo usermod -aG docker \$USER && newgrp docker"
    fi
}

setup_qemu_if_needed() {
    # Only run if building for arm64 on a non-arm64 machine
    if [[ "$1" == "arm64" ]] && [[ "$(uname -m)" != "aarch64" ]]; then
        
        # Check if the STABLE emulator is already registered.
        # We specifically look for the tonistiigi behavior or just ensure aarch64 exists.
        if ! grep -q 'aarch64' /proc/sys/fs/binfmt_misc/* 2>/dev/null; then
            info "Configuring QEMU environment..."
            
            # 1. Clean up any potential old/unstable QEMU handlers
            docker run --privileged --rm tonistiigi/binfmt --uninstall qemu-* >/dev/null 2>&1 || true
            
            # 2. Install the stable handlers (matches GitHub Actions behavior)
            info "Registering stable QEMU handlers (tonistiigi/binfmt)..."
            docker run --privileged --rm tonistiigi/binfmt --install all >/dev/null 2>&1 || true
            
            success "QEMU registered successfully."
        else
            info "QEMU is already registered."
        fi
    fi
}

# Detect the current buildx driver. The "docker" driver cannot export cache.
current_buildx_driver() {
    local driver
    driver=$(docker buildx ls 2>/dev/null | awk 'NR==1 {next} /^\*/ {print $3; exit}')
    echo "${driver:-docker}"
}

main() {
    local ARCH=""
    
    if [[ $# -eq 0 ]]; then
        show_usage
        err "No architecture specified."
    fi
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --arch)
                ARCH="$2"
                shift 2
                ;;
            --enable-debug-logs)
                ENABLE_DEBUG_LOGS=true
                info "Debug logging ENABLED - qDebug() output will be visible"
                shift
                ;;
            *)
                show_usage
                exit 0
                ;;
        esac
    done
    
    # Find and change to project root
    info "Locating project root directory..."
    local PROJECT_ROOT
    if ! PROJECT_ROOT="$(find_project_root)"; then
        err "Could not find project root. Ensure you're running from within the qdomyos-zwift project."
    fi
    
    info "Project root found at: $PROJECT_ROOT"
    cd "$PROJECT_ROOT" || err "Failed to change to project root directory."
    
    setup_and_verify_docker
    setup_qemu_if_needed "$ARCH"
    
    local DOCKERFILE_PATH=""
    local PLATFORM_FLAG=""
    local INTERNAL_APPDIR_PATH=""
    local OUTPUT_DIR=""
    local ARTIFACT_NAME=""
    
    case "$ARCH" in
        "x86-64")
            DOCKERFILE_PATH="docker/linux-ant/Dockerfile"
            PLATFORM_FLAG="--platform linux/amd64"
            INTERNAL_APPDIR_PATH="/qdomyos-zwift-x86-64-ant"
            OUTPUT_DIR="qdomyos-zwift-x86-64-ant"
            ARTIFACT_NAME="linux-binary-x86-64-ant.zip"
            ;;
        "arm64")
            DOCKERFILE_PATH="docker/rpi-ant/Dockerfile"
            PLATFORM_FLAG="--platform linux/arm64"
            INTERNAL_APPDIR_PATH="/qdomyos-zwift-arm64-ant"
            OUTPUT_DIR="qdomyos-zwift-arm64-ant"
            ARTIFACT_NAME="linux-binary-arm64-ant.zip"
            ;;
        *)
            err "Unsupported architecture: '$ARCH'."
            ;;
    esac
    
    if [[ ! -f "$DOCKERFILE_PATH" ]]; then
        err "Dockerfile not found at: $DOCKERFILE_PATH."
    fi
    
    local IMAGE_TAG="qdomyos-zwift-legacy-builder:$ARCH"
    
    info "Building Docker image '$IMAGE_TAG'..."

    # Enable cache only when the buildx driver supports cache export.
    local CACHE_ARGS=()
    local DRIVER
    DRIVER="$(current_buildx_driver)"
    if [[ "$DRIVER" != "docker" ]]; then
        local CACHE_DIR="$PROJECT_ROOT/.buildx-cache"
        mkdir -p "$CACHE_DIR"
        CACHE_ARGS+=("--cache-from=type=local,src=$CACHE_DIR")
        CACHE_ARGS+=("--cache-to=type=local,dest=$CACHE_DIR,mode=max")
    else
        info "Buildx driver is 'docker'; skipping cache export/import (not supported)."
    fi

    # The 'docker' buildx driver (local docker) does not accept --platform
    # in some Docker/Buildx versions. Avoid passing the platform flag when
    # the driver is 'docker' to prevent "unknown flag: --platform" errors.
    if [[ "$DRIVER" == "docker" ]]; then
        PLATFORM_FLAG=""
    fi

    # Pass debug flag to Docker build as a build argument
    local DEBUG_ARG=""
    if [[ "$ENABLE_DEBUG_LOGS" == "true" ]]; then
        DEBUG_ARG="--build-arg ENABLE_DEBUG_LOGS=1"
    fi

    # Assemble buildx arguments into an array to ensure the final context
    # (.) is always passed and flags are only included when non-empty.
    BUILD_ARGS=(--progress=plain)
    if [[ -n "$PLATFORM_FLAG" ]]; then
        BUILD_ARGS+=("$PLATFORM_FLAG")
    fi
    if [[ ${#CACHE_ARGS[@]} -gt 0 ]]; then
        BUILD_ARGS+=("${CACHE_ARGS[@]}")
    fi
    if [[ -n "$DEBUG_ARG" ]]; then
        BUILD_ARGS+=("$DEBUG_ARG")
    fi
    BUILD_ARGS+=(-t "$IMAGE_TAG" -f "$DOCKERFILE_PATH" --load)

    if ! docker buildx build "${BUILD_ARGS[@]}" .; then
        err "Docker build failed."
    fi
    success "Docker image built successfully."
    
    info "Extracting binaries from container..."
    
    # Clean up old output directory using Docker to avoid permission issues
    if [ -d "$OUTPUT_DIR" ]; then
        info "Cleaning up previous build artifacts..."
        docker run --rm -v "$PROJECT_ROOT:/workspace" busybox rm -rf "/workspace/$(basename "$OUTPUT_DIR")"
    fi
    
    mkdir -p "$OUTPUT_DIR"
    
    # Create temp container and run it to change ownership before extracting
    # This avoids needing sudo for chown on the host
    TEMP_CONTAINER="qz-extract-$$"
    docker rm -f "$TEMP_CONTAINER" >/dev/null 2>&1 || true
    
    # Run container with a command that changes ownership and keeps it running briefly
    docker run -d --name "$TEMP_CONTAINER" "$IMAGE_TAG" sleep 10 >/dev/null
    
    # Change ownership inside container to match host user (works because we're in the container context)
    docker exec "$TEMP_CONTAINER" chown -R "$(id -u):$(id -g)" "$INTERNAL_APPDIR_PATH"
    
    # Now copy files with correct ownership
    docker cp "$TEMP_CONTAINER:$INTERNAL_APPDIR_PATH/." "$OUTPUT_DIR/"
    
    # Clean up temp container
    docker rm -f "$TEMP_CONTAINER" >/dev/null
    
    # Make binaries executable
    chmod +x "$OUTPUT_DIR/qdomyos-zwift" "$OUTPUT_DIR/qdomyos-zwift-bin"
    
    info "Creating zip artifact (matching GitHub runner behavior)..."
    rm -f "$ARTIFACT_NAME"
    # Change to output directory parent and zip with directory structure
    (cd "$(dirname "$OUTPUT_DIR")" && zip -r "$PROJECT_ROOT/$ARTIFACT_NAME" "$(basename "$OUTPUT_DIR")")
    
    # Clean up the extracted directory
    rm -rf "$OUTPUT_DIR"
    
    echo ""
    ls -lh "$ARTIFACT_NAME"
    success "Build complete! Output artifact: $PROJECT_ROOT/$ARTIFACT_NAME"
    
    echo -e "\n${C_YELLOW}To use the application:${C_RESET}"
    echo -e "  1. Extract: ${C_YELLOW}unzip $ARTIFACT_NAME${C_RESET}"
    echo -e "  2. Navigate: ${C_YELLOW}cd $OUTPUT_DIR${C_RESET}"
    echo -e "  3. Execute: ${C_YELLOW}./qdomyos-zwift${C_RESET}"
    echo -e "\n${C_BLUE}Note:${C_RESET} The wrapper script automatically finds libpython3.11 in:"
    echo -e "  - ~/.pyenv/versions/3.11.*/lib"
    echo -e "  - System library paths"
}

# Main execution starts here
main "$@"