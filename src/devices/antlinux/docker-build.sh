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
HAS_BUILDX=false  # Will be set by setup_and_verify_docker()

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

warn() {
    echo -e "${C_YELLOW}⚠ WARNING:${C_RESET} $*"
}

show_usage() {
    echo "Usage: $0 --arch <ARCHITECTURE> (arm64 or x86-64)"
    echo ""
    echo "Options:"
    echo "  --arch <arch>          Architecture to build (arm64 or x86-64)"
    echo "  --enable-debug-logs    Enable debug logging in the binary"
    echo "  --install-buildx       Install Docker buildx plugin (if missing)"
    echo ""
    echo "Examples:"
    echo "  $0 --arch x86-64"
    echo "  $0 --arch arm64 --enable-debug-logs"
    echo "  $0 --install-buildx    # Install buildx and exit"
}

# Installation functions
install_docker_from_official_repo() {
    echo ""
    info "Removing old Docker packages..."
    sudo apt-get remove -y docker docker-engine docker.io containerd runc 2>/dev/null || true
    
    info "Downloading Docker installation script..."
    curl -fsSL https://get.docker.com -o /tmp/get-docker.sh
    
    info "Installing Docker (this may take a few minutes)..."
    sudo sh /tmp/get-docker.sh
    rm -f /tmp/get-docker.sh
    
    info "Adding user '$USER' to docker group..."
    sudo usermod -aG docker "$USER"
    
    echo ""
    success "Docker installed successfully!"
    echo ""
    echo -e "${C_YELLOW}⚠ IMPORTANT: You must log out and back in for group changes to take effect.${C_RESET}"
    echo ""
    echo "After re-login, verify installation with:"
    echo -e "  ${C_BLUE}docker version${C_RESET}"
    echo -e "  ${C_BLUE}docker buildx version${C_RESET}"
    echo ""
    echo "Then re-run this script to build your project."
    
    exit 0
}

install_buildx_plugin() {
    local BUILDX_VERSION="0.12.1"
    local ARCH_SUFFIX=""
    
    case "$(uname -m)" in
        x86_64) ARCH_SUFFIX="linux-amd64" ;;
        aarch64) ARCH_SUFFIX="linux-arm64" ;;
        *) err "Unsupported architecture for buildx installation: $(uname -m)" ;;
    esac
    
    echo ""
    info "Creating plugin directory..."
    mkdir -p ~/.docker/cli-plugins
    
    info "Downloading buildx v${BUILDX_VERSION}..."
    local DOWNLOAD_URL="https://github.com/docker/buildx/releases/download/v${BUILDX_VERSION}/buildx-v${BUILDX_VERSION}.${ARCH_SUFFIX}"
    
    if ! curl -L "$DOWNLOAD_URL" -o ~/.docker/cli-plugins/docker-buildx 2>/dev/null; then
        err "Failed to download buildx. Please check your internet connection."
    fi
    
    info "Making buildx executable..."
    chmod +x ~/.docker/cli-plugins/docker-buildx
    
    echo ""
    success "Buildx plugin installed successfully!"
    
    # Test buildx
    if docker buildx version >/dev/null 2>&1; then
        success "Buildx is working: $(docker buildx version | head -1)"
        echo ""
        info "Creating buildx builder instance..."
        docker buildx create --use --name multiarch-builder --driver docker-container 2>/dev/null || true
        echo ""
        success "Setup complete!"
        return 0
    else
        err "Buildx installation completed but verification failed. Try restarting your terminal."
    fi
}

check_docker_and_offer_installation() {
    # Check Docker availability
    if ! command -v docker >/dev/null 2>&1; then
        # Docker not installed at all
        echo ""
        echo -e "${C_RED}✗ Docker is not installed on this system.${C_RESET}"
        echo ""
        echo "Docker is required to build QDomyos-Zwift."
        echo "This will install Docker CE (includes buildx) from get.docker.com"
        echo ""
        echo -e "${C_YELLOW}⚠ Requires sudo and logout/login after installation${C_RESET}"
        echo ""
        read -p "Install Docker now? [Y/n]: " -r
        
        # Default to Yes if empty or Y/y
        if [[ -z "$REPLY" ]] || [[ $REPLY =~ ^[Yy]$ ]]; then
            install_docker_from_official_repo
            # Function exits script after installation
        else
            echo ""
            echo "Please install Docker manually and re-run this script."
            echo "Installation instructions: https://docs.docker.com/engine/install/"
            exit 1
        fi
    fi
    
    # Docker exists, check if it's running
    if ! docker info >/dev/null 2>&1; then
        echo -e "${C_RED}✗ Docker is installed but not running.${C_RESET}"
        echo ""
        echo "Please start Docker with:"
        echo -e "  ${C_BLUE}sudo systemctl start docker${C_RESET}"
        echo ""
        echo "Or add your user to the docker group:"
        echo -e "  ${C_BLUE}sudo usermod -aG docker \$USER${C_RESET}"
        echo -e "  ${C_BLUE}newgrp docker${C_RESET}"
        exit 1
    fi
    
    # Docker is running, check buildx
    if ! docker buildx version >/dev/null 2>&1; then
        # Buildx not installed
        echo ""
        warn "Docker is installed but buildx plugin is missing."
        echo ""
        echo "Buildx provides:"
        echo "  • Cross-platform builds (x86-64 and ARM64)"
        echo "  • Better build caching (faster rebuilds)"
        echo "  • Advanced BuildKit features"
        echo ""
        echo "This will download and install buildx to ~/.docker/cli-plugins/"
        echo ""
        read -p "Install buildx now? [Y/n]: " -r
        
        # Default to Yes if empty or Y/y
        if [[ -z "$REPLY" ]] || [[ $REPLY =~ ^[Yy]$ ]]; then
            if install_buildx_plugin; then
                echo ""
                info "Buildx installed successfully. Continuing with build..."
                echo ""
                return 0
            else
                return 1
            fi
        else
            echo ""
            warn "Continuing without buildx (x86-64 builds only, no caching)..."
            echo ""
            return 0
        fi
    fi
    
    # All good - Docker and buildx available
    return 0
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
    # Call the interactive installation helper
    check_docker_and_offer_installation
    
    info "Docker version: $(docker version --format '{{.Server.Version}}' 2>/dev/null || echo 'unknown')"
    
    # Check if buildx is available
    if docker buildx version >/dev/null 2>&1; then
        HAS_BUILDX=true
        info "Buildx version: $(docker buildx version 2>/dev/null | head -1 || echo 'unknown')"
        
        # Ensure a buildx builder instance exists
        if ! docker buildx ls 2>/dev/null | grep -q "multiarch-builder"; then
            info "Creating buildx builder instance..."
            docker buildx create --use --name multiarch-builder --driver docker-container >/dev/null 2>&1 || true
        else
            # Use existing builder
            docker buildx use multiarch-builder >/dev/null 2>&1 || true
        fi
    else
        HAS_BUILDX=false
        warn "Docker buildx not available - will use regular docker build (x86-64 only)"
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
    if [[ "$HAS_BUILDX" != "true" ]]; then
        echo "none"
        return
    fi
    
    local driver
    driver=$(docker buildx ls 2>/dev/null | awk 'NR==1 {next} /^\*/ {print $3; exit}')
    echo "${driver:-docker}"
}

main() {
    local ARCH=""
    local INSTALL_BUILDX_ONLY=false
    
    if [[ $# -eq 0 ]]; then
        show_usage
        exit 0
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
            --install-buildx)
                INSTALL_BUILDX_ONLY=true
                shift
                ;;
            --help)
                show_usage
                exit 0
                ;;
            *)
                echo "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done
    
    # Handle --install-buildx flag (install and exit)
    if [[ "$INSTALL_BUILDX_ONLY" == "true" ]]; then
        echo -e "${C_BLUE}═══════════════════════════════════════════════════════════${C_RESET}"
        echo -e "${C_BLUE}  QDomyos-Zwift: Buildx Installation Helper${C_RESET}"
        echo -e "${C_BLUE}═══════════════════════════════════════════════════════════${C_RESET}"
        
        if ! command -v docker >/dev/null 2>&1; then
            # Docker not installed - offer full installation
            echo ""
            echo -e "${C_RED}✗ Docker is not installed.${C_RESET}"
            echo ""
            read -p "Would you like to install Docker (includes buildx)? [Y/n]: " -r
            
            # Default to Yes if empty or Y/y
            if [[ -z "$REPLY" ]] || [[ $REPLY =~ ^[Yy]$ ]]; then
                install_docker_from_official_repo
            else
                echo "Installation cancelled."
                exit 1
            fi
        elif ! docker buildx version >/dev/null 2>&1; then
            # Docker exists but buildx missing - install plugin only
            echo ""
            echo "Docker is installed but buildx plugin is missing."
            echo "This will download and install buildx to ~/.docker/cli-plugins/"
            echo ""
            read -p "Install buildx now? [Y/n]: " -r
            
            # Default to Yes if empty or Y/y
            if [[ -z "$REPLY" ]] || [[ $REPLY =~ ^[Yy]$ ]]; then
                install_buildx_plugin
            else
                echo "Installation cancelled."
                exit 1
            fi
        else
            # Buildx already installed
            echo ""
            success "Buildx is already installed: $(docker buildx version | head -1)"
            echo ""
            echo "Nothing to do!"
            exit 0
        fi
        exit 0
    fi
    
    # Architecture is required for build operations
    if [[ -z "$ARCH" ]]; then
        show_usage
        err "No architecture specified. Use: --arch x86-64 or --arch arm64"
    fi
    
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

    # Determine build method based on buildx availability
    if [[ "$HAS_BUILDX" == "true" ]]; then
        # Use buildx
        info "Using docker buildx for build..."
        
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
        BUILD_ARGS=()
        
        # Check if --progress flag is supported (buildx 0.6.0+)
        # Older buildx versions don't support --progress
        if docker buildx build --help 2>&1 | grep -q -- '--progress'; then
            BUILD_ARGS+=(--progress=plain)
        else
            info "Buildx version doesn't support --progress flag, skipping..."
        fi
        
        if [[ -n "$PLATFORM_FLAG" ]]; then
            BUILD_ARGS+=($PLATFORM_FLAG)
        fi
        if [[ ${#CACHE_ARGS[@]} -gt 0 ]]; then
            BUILD_ARGS+=("${CACHE_ARGS[@]}")
        fi
        if [[ -n "$DEBUG_ARG" ]]; then
            BUILD_ARGS+=($DEBUG_ARG)
        fi
        BUILD_ARGS+=(-t "$IMAGE_TAG" -f "$DOCKERFILE_PATH" --load)

        if ! docker buildx build "${BUILD_ARGS[@]}" .; then
            err "Docker buildx build failed."
        fi
    else
        # Use regular docker build (no buildx)
        info "Using regular docker build (buildx not available)..."
        
        # Note: ARM64 cross-compilation requires buildx
        if [[ "$ARCH" == "arm64" ]] && [[ "$(uname -m)" != "aarch64" ]]; then
            err "ARM64 cross-compilation requires docker buildx. Please install buildx or build on an ARM64 machine."
        fi
        
        # Build with regular docker
        local DEBUG_ARG=""
        if [[ "$ENABLE_DEBUG_LOGS" == "true" ]]; then
            DEBUG_ARG="--build-arg ENABLE_DEBUG_LOGS=1"
        fi
        
        BUILD_ARGS=()
        if [[ -n "$DEBUG_ARG" ]]; then
            BUILD_ARGS+=($DEBUG_ARG)
        fi
        BUILD_ARGS+=(-t "$IMAGE_TAG" -f "$DOCKERFILE_PATH")
        
        if ! docker build "${BUILD_ARGS[@]}" .; then
            err "Docker build failed."
        fi
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