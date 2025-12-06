#!/usr/bin/bash
# =============================================================================
# QDomyos-Zwift: Universal Docker Build Script (v1.0)
#
# This script builds the QDomyos-Zwift application for different target
# architectures using a containerized Docker environment. It requires Docker
# and the buildx plugin to be installed.
#
# This script replaces the previous cross-compiler script and provides a single,
# unified interface for all Docker-based builds.
#
# USAGE:
#   From the 'src' directory, run:
#   ./docker-build.sh --arch <ARCH>
#
#   ARCH can be:
#     - arm64   (for Raspberry Pi)
#     - x86_64  (for Desktop Linux)
#
# Contributor(s): bassai-sho
# Development assisted by AI analysis tools
# =============================================================================

set -euo pipefail

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
    echo "Usage: $0 --arch <ARCHITECTURE> (arm64 or x86_64)"
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
    if [[ "$1" == "arm64" ]] && [[ "$(uname -m)" != "aarch64" ]]; then
        if ! ls /proc/sys/fs/binfmt_misc/ | grep -q 'qemu-aarch64'; then
            info "Registering QEMU..."
            docker run --rm --privileged multiarch/qemu-user-static --reset -p yes >/dev/null 2>&1 || true
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
        "x86_64")
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
    local CONTAINER_NAME="qdomyos-zwift-legacy-extractor"
    
    info "Building Docker image '$IMAGE_TAG'..."

    # Enable cache only when the buildx driver supports cache export.
    local CACHE_ARGS=()
    local DRIVER
    DRIVER="$(current_buildx_driver)"
    if [[ "$DRIVER" != "docker" ]]; then
        local CACHE_DIR="$PROJECT_ROOT/.buildx-cache"
        mkdir -p "$CACHE_DIR"
        CACHE_ARGS+=(--cache-from=type=local,src="$CACHE_DIR")
        CACHE_ARGS+=(--cache-to=type=local,dest="$CACHE_DIR",mode=max)
    else
        info "Buildx driver is 'docker'; skipping cache export/import (not supported)."
    fi

    # Add --no-cache after buildx to force a clean build if ever needed.
    if ! docker buildx build --progress=plain $PLATFORM_FLAG \
         "${CACHE_ARGS[@]}" \
         -t "$IMAGE_TAG" -f "$DOCKERFILE_PATH" --load .; then
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
    docker exec "$TEMP_CONTAINER" chown -R $(id -u):$(id -g) "$INTERNAL_APPDIR_PATH"
    
    # Now copy files with correct ownership
    docker cp "$TEMP_CONTAINER:$INTERNAL_APPDIR_PATH/." "$OUTPUT_DIR/"
    
    # Clean up temp container
    docker rm -f "$TEMP_CONTAINER" >/dev/null
    
    # Make binaries executable
    chmod +x "$OUTPUT_DIR/qdomyos-zwift" "$OUTPUT_DIR/qdomyos-zwift-bin"
    
    info "Creating zip artifact (matching GitHub runner behavior)..."
    rm -f "$ARTIFACT_NAME"
    zip -r "$ARTIFACT_NAME" "$OUTPUT_DIR"
    
    # Clean up the extracted directory
    rm -rf "$OUTPUT_DIR"
    
    echo ""
    ls -lh "$ARTIFACT_NAME"
    success "Build complete! Output artifact: $PROJECT_ROOT/$ARTIFACT_NAME"
    
    echo -e "\n${C_YELLOW}To use the application:${C_RESET}"
    echo -e "  1. Extract: ${C_YELLOW}unzip $PROJECT_ROOT/$ARTIFACT_NAME -d $PROJECT_ROOT/${C_RESET}"
    echo -e "  2. Execute: ${C_YELLOW}$PROJECT_ROOT/$OUTPUT_DIR/qdomyos-zwift${C_RESET}"
    echo -e "\n${C_BLUE}Note:${C_RESET} The wrapper script automatically finds libpython3.11 in:"
    echo -e "  - ~/.pyenv/versions/3.11.*/lib"
    echo -e "  - System library paths"
}

# Main execution starts here
main "$@"