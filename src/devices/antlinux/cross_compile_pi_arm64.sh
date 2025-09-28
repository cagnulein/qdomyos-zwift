#!/usr/bin/env bash
# =============================================================================
# Hardened Cross-Compilation Script v21 (Enhanced Multi-Platform Support)
#
# A robust, multi-distribution cross-compilation script with comprehensive
# error handling, performance optimization, and recovery mechanisms.
# Supports Ubuntu 20.04+, Debian 11+, WSL, and derivative distributions.
# =============================================================================

set -euo pipefail

# ------------------ Global Configuration ------------------
readonly SCRIPT_VERSION="21"
readonly PROJECT_FILE="qdomyos-zwift.pro"
readonly BUILD_DIR="build-aarch64"
readonly CROSS_QMAKE="/usr/bin/aarch64-linux-gnu-qmake"
readonly BACKUP_PREFIX=".qz-backup"
readonly VENV_NAME="ant_venv"
readonly MIN_DISK_SPACE_MB=2048
readonly MIN_MEMORY_MB=1024

# Global variables (populated dynamically)
PYTHON_VERSION_STR=""
PYTHON_PKG_SUFFIX=""
DIST_ID=""
DIST_VERSION=""
DIST_CODENAME=""
MAKEFLAGS=""
ESSENTIAL_TARGET_PACKAGES=()
IS_WSL=0
IS_CONTAINER=0
APT_UPDATE_DONE=0

# Global timing variables
BUILD_START_TIME=""
COMPILE_START_TIME=""
COMPILE_END_TIME=""
SCRIPT_START_TIME=""

# Package lists
readonly REQUIRED_HOST_PACKAGES=(
    "build-essential" "git" "gcc-aarch64-linux-gnu" "g++-aarch64-linux-gnu"
    "qemu-user-static" "qt5-qmake" "qtbase5-dev-tools" "python3" "python3-pip"
    "python3-venv" "python3-dev" "ccache" "curl" "wget" "ca-certificates"
    "apt-utils" "lsb-release" "file" "pkg-config"
)

# ------------------ Helper Functions ------------------
err() {
    echo -e "\n❌ ERROR: $*" >&2
    cleanup_on_failure
    exit 1
}

info() { echo ">>> $*"; }
warn() { echo "⚠️  WARNING: $*" >&2; }
success() { echo "✅ $*"; }

cleanup_on_failure() {
    local exit_code=$?
    if [[ $exit_code -ne 0 && -d "$BUILD_DIR" ]]; then
        warn "Build failed with exit code $exit_code"
        warn "Diagnostic information:"
        echo "  - Build directory: $BUILD_DIR"
        echo "  - Available disk space:"
        df -h "$PWD" 2>/dev/null | tail -1 || echo "    (unavailable)"
        echo "  - Available memory:"
        free -h 2>/dev/null | head -2 || echo "    (unavailable)"

        if [[ -f "$BUILD_DIR/Makefile" ]]; then
            echo "  - Build was configured successfully"
        else
            echo "  - Build configuration failed"
        fi
    fi
}

# Convert seconds to HH:MM:SS format
format_duration() {
    local total_seconds=$1
    local hours=$((total_seconds / 3600))
    local minutes=$(((total_seconds % 3600) / 60))
    local seconds=$((total_seconds % 60))
    
    if [[ $hours -gt 0 ]]; then
        printf "%d:%02d:%02d" $hours $minutes $seconds
    elif [[ $minutes -gt 0 ]]; then
        printf "%d:%02d" $minutes $seconds
    else
        printf "0:%02d" $seconds
    fi
}

# Set up cleanup trap
trap cleanup_on_failure ERR

# ------------------ System Detection & Validation ------------------
detect_environment() {
    info "Detecting system environment..."

    # Detect WSL
    if [[ -n "${WSL_DISTRO_NAME:-}" ]] || [[ -n "${WSLENV:-}" ]] || grep -qi microsoft /proc/version 2>/dev/null; then
        IS_WSL=1
        info "  > Running in WSL environment: ${WSL_DISTRO_NAME:-Unknown}"
    fi

    # Detect container environment
    if [[ -f /.dockerenv ]] || [[ -n "${container:-}" ]] || grep -q 'docker\|lxc' /proc/1/cgroup 2>/dev/null; then
        IS_CONTAINER=1
        info "  > Running in container environment"
    fi

    # Check for systemd in WSL2 (affects some package installations)
    if [[ $IS_WSL -eq 1 ]]; then
        if command -v systemctl >/dev/null 2>&1 && systemctl is-system-running >/dev/null 2>&1; then
            info "  > WSL with systemd enabled"
        else
            info "  > WSL without systemd"
        fi
    fi

    # Validate basic requirements
    validate_system_requirements
}

validate_system_requirements() {
    info "Validating system requirements..."

    # Check disk space
    local available_mb
    available_mb=$(df -BM "$PWD" 2>/dev/null | tail -1 | awk '{gsub(/M/, "", $4); print $4}' || echo "0")
    if [[ $available_mb -lt $MIN_DISK_SPACE_MB ]]; then
        warn "Low disk space: ${available_mb}MB available, ${MIN_DISK_SPACE_MB}MB recommended"
        read -p "Continue anyway? (y/N) " -n 1 -r; echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            err "Insufficient disk space for build"
        fi
    fi

    # Check memory
    local available_mem_mb
    available_mem_mb=$(free -m 2>/dev/null | awk '/^Mem:/{print $2}' || echo "0")
    if [[ $available_mem_mb -lt $MIN_MEMORY_MB ]]; then
        warn "Low memory: ${available_mem_mb}MB available, ${MIN_MEMORY_MB}MB recommended"
        warn "Build may be slower or fail on complex files"
    fi

    # Check for required directories (especially important in containers)
    local required_dirs=("/usr/share/keyrings" "/etc/apt" "/var/cache/apt")
    for dir in "${required_dirs[@]}"; do
        if [[ ! -d "$dir" ]]; then
            warn "Required directory missing: $dir"
            if [[ $IS_CONTAINER -eq 1 ]]; then
                warn "This may indicate an incomplete container image"
            fi
        fi
    done

    success "System requirements validation completed"
}

detect_distribution() {
    info "Detecting system distribution..."

    if [[ -f /etc/os-release ]]; then
        # shellcheck source=/dev/null
        source /etc/os-release
        DIST_ID="${ID:-unknown}"
        DIST_VERSION="${VERSION_ID:-unknown}"
        DIST_CODENAME="${VERSION_CODENAME:-unknown}"
    elif command -v lsb_release >/dev/null 2>&1; then
        DIST_ID=$(lsb_release -si | tr '[:upper:]' '[:lower:]')
        DIST_VERSION=$(lsb_release -sr)
        DIST_CODENAME=$(lsb_release -sc)
    else
        err "Cannot detect distribution. Please install lsb-release package."
    fi

    # Handle special cases for derivative distributions
    case "$DIST_ID" in
        "pop")
            DIST_ID="ubuntu"  # Pop!_OS uses Ubuntu repos
            ;;
        "linuxmint"|"elementary")
            DIST_ID="ubuntu"  # These use Ubuntu repos
            ;;
        "kali")
            DIST_ID="debian"  # Kali uses Debian-style repos
            ;;
    esac

    # Enhanced codename detection for Debian
    if [[ "$DIST_ID" == "debian" && "$DIST_CODENAME" == "unknown" ]]; then
        if [[ -f /etc/debian_version ]]; then
            local version
            version=$(cat /etc/debian_version)
            case "$version" in
                13*|"trixie"*) DIST_CODENAME="trixie" ;;
                12*|"bookworm"*) DIST_CODENAME="bookworm" ;;
                11*|"bullseye"*) DIST_CODENAME="bullseye" ;;
                10*|"buster"*) DIST_CODENAME="buster" ;;
                *) DIST_CODENAME="bookworm" ;;  # Default to current stable
            esac
            warn "Detected Debian codename via fallback: $DIST_CODENAME"
        fi
    fi

    # WSL-specific adjustments
    if [[ $IS_WSL -eq 1 ]]; then
        info "  > WSL-specific distribution detection"
        
        # Fix incorrect version reporting in WSL
        if [[ "$DIST_ID" == "debian" && "$DIST_VERSION" != "12" && -f /etc/debian_version ]]; then
            local actual_version
            actual_version=$(cat /etc/debian_version | grep -o '^[0-9]\+' || echo "unknown")
            if [[ "$actual_version" == "12" ]]; then
                DIST_VERSION="12"
                info "  > Corrected Debian version: $DIST_VERSION"
            fi
        fi
        
        # WSL Ubuntu may report different version numbers  
        if [[ "$DIST_ID" == "ubuntu" ]]; then
            case "$DIST_VERSION" in
                "20.04"|"22.04"|"24.04") ;;  # Known good versions
                *) warn "WSL Ubuntu version $DIST_VERSION may have different package availability" ;;
            esac
        elif [[ "$DIST_ID" == "debian" ]]; then
            case "$DIST_VERSION" in
                "11"|"12") ;;  # Known good versions
                *) warn "WSL Debian version $DIST_VERSION may have different package availability" ;;
            esac
        fi
    fi

    info "  > Distribution: $DIST_ID $DIST_VERSION ($DIST_CODENAME)"
    if [[ $IS_WSL -eq 1 ]]; then
        info "  > Environment: WSL"
    elif [[ $IS_CONTAINER -eq 1 ]]; then
        info "  > Environment: Container"
    fi

    # Validate supported distributions
    case "$DIST_ID" in
        ubuntu|debian) ;;
        *) warn "Unsupported distribution '$DIST_ID'. Proceeding with best-effort configuration." ;;
    esac
}

detect_python_environment() {
    info "Detecting Python environment..."

    if ! command -v python3 >/dev/null 2>&1; then
        err "Python 3 is required but not found. Please install python3 package."
    fi

    # Enhanced Python version detection with fallbacks
    local python_cmd="python3"
    if ! PYTHON_VERSION_STR=$(python3 -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")' 2>/dev/null); then
        # Fallback method
        if command -v python3 >/dev/null 2>&1; then
            local py_version
            py_version=$(python3 --version 2>&1 | grep -o '[0-9]\+\.[0-9]\+' | head -1 || echo "")
            if [[ -n "$py_version" ]]; then
                PYTHON_VERSION_STR="$py_version"
            else
                err "Failed to detect Python version using multiple methods"
            fi
        else
            err "Python 3 installation appears corrupted"
        fi
    fi

    info "  > Python version: $PYTHON_VERSION_STR"

    # Enhanced Python package suffix detection
    local test_pkg_base="libpython${PYTHON_VERSION_STR}"
    local output

    # First try t64 variant (for newer distributions)
    output=$(apt-cache policy "${test_pkg_base}t64:arm64" 2>/dev/null || true)
    if [[ "$output" == *"Candidate:"* && "$output" != *"Candidate: (none)"* ]]; then
        PYTHON_PKG_SUFFIX="t64"
        info "  > Python package suffix: t64 (modern distribution)"
        return 0
    fi

    # Try standard naming
    output=$(apt-cache policy "${test_pkg_base}:arm64" 2>/dev/null || true)
    if [[ "$output" == *"Candidate:"* && "$output" != *"Candidate: (none)"* ]]; then
        PYTHON_PKG_SUFFIX=""
        info "  > Python package suffix: none (standard naming)"
        return 0
    fi

    # Enhanced error reporting for Python package detection
    warn "Cannot find Python ${PYTHON_VERSION_STR} packages for ARM64"
    info "Available Python packages:"
    apt-cache search "libpython.*-dev" | grep -E "(arm64|:arm64)" | head -5 || echo "  (none found)"
    
    if [[ $APT_UPDATE_DONE -eq 0 ]]; then
        warn "This might be resolved by updating package cache or configuring sources"
        err "Cannot proceed without Python ARM64 packages. Try: --configure-sources --full"
    else
        err "Python ARM64 packages not available. Check APT sources configuration."
    fi
}

populate_target_packages() {
    ESSENTIAL_TARGET_PACKAGES=(
        "qtbase5-dev:arm64"
        "qtbase5-private-dev:arm64"
        "qtconnectivity5-dev:arm64"
        "libqt5charts5-dev:arm64"
        "libqt5sensors5-dev:arm64"
        "qtpositioning5-dev:arm64"
        "qtdeclarative5-dev:arm64"
        "qtquickcontrols2-5-dev:arm64"
        "libqt5websockets5-dev:arm64"
        "qtmultimedia5-dev:arm64"
        "libqt5texttospeech5-dev:arm64"
        "libqt5networkauth5-dev:arm64"
        "libqt5sql5-sqlite:arm64"
        "qtlocation5-dev:arm64"
        "libusb-1.0-0-dev:arm64"
        "libudev-dev:arm64"
        "libpython${PYTHON_VERSION_STR}-dev:arm64"
        "libpython${PYTHON_VERSION_STR}${PYTHON_PKG_SUFFIX}:arm64"
    )
}

# ------------------ Performance Optimization ------------------
optimize_build_performance() {
    info "Optimizing build performance..."

    local cpu_count
    cpu_count=$(nproc 2>/dev/null || echo "1")
    local mem_gb
    mem_gb=$(free -g 2>/dev/null | awk '/^Mem:/{print $2}' || echo "2")

    # More conservative memory estimation for different environments
    local mem_per_job=2  # GB per compilation job
    if [[ $IS_WSL -eq 1 ]]; then
        mem_per_job=1  # WSL tends to use more memory
    elif [[ $IS_CONTAINER -eq 1 ]]; then
        mem_per_job=1  # Containers may have memory limits
    fi

    local mem_limited_jobs=$((mem_gb / mem_per_job))
    [[ $mem_limited_jobs -lt 1 ]] && mem_limited_jobs=1

    local optimal_jobs
    if [[ $mem_limited_jobs -lt $cpu_count ]]; then
        optimal_jobs=$mem_limited_jobs
        warn "Limiting build jobs to $optimal_jobs due to memory constraints (${mem_gb}GB available)"
    else
        optimal_jobs=$cpu_count
    fi

    # Cap maximum jobs to prevent system overload
    local max_jobs=8
    if [[ $optimal_jobs -gt $max_jobs ]]; then
        optimal_jobs=$max_jobs
        info "Capping build jobs to $max_jobs to prevent system overload"
    fi

    MAKEFLAGS="-j${optimal_jobs}"
    info "  > Build jobs: $optimal_jobs"

    # Enhanced ccache configuration
    if command -v ccache >/dev/null 2>&1; then
        export CC="ccache aarch64-linux-gnu-gcc"
        export CXX="ccache aarch64-linux-gnu-g++"
        export CCACHE_DIR="${HOME}/.ccache"

        # Environment-specific ccache configuration
        local cache_size="2G"
        local cache_files=10000
        
        if [[ $IS_WSL -eq 1 ]]; then
            # WSL may have limited disk space
            cache_size="1G"
            cache_files=5000
        elif [[ $IS_CONTAINER -eq 1 ]]; then
            # Containers may have ephemeral storage
            cache_size="1G"
            cache_files=5000
        fi

        ccache --max-size="$cache_size" >/dev/null 2>&1 || true
        ccache --max-files="$cache_files" >/dev/null 2>&1 || true

        info "  > Enabled ccache (${cache_size} limit)"
        
        # Show cache stats if available
        if ccache --show-stats >/dev/null 2>&1; then
            local cache_stats
            cache_stats=$(ccache --show-stats 2>/dev/null | grep -E "(hit rate|files|size)" | tr '\n' '; ' || echo "stats unavailable")
            info "  > ccache status: $cache_stats"
        fi
    else
        info "  > ccache not available - install for faster rebuilds"
    fi
}

# ------------------ APT Configuration & Backup ------------------
safe_apt_update() {
    if [[ $APT_UPDATE_DONE -eq 1 ]]; then
        return 0
    fi

    info "Updating package cache..."
    
    # Retry mechanism for APT update (network issues are common)
    local max_attempts=3
    local attempt=1
    
    while [[ $attempt -le $max_attempts ]]; do
        if sudo apt-get update -qq 2>/dev/null; then
            APT_UPDATE_DONE=1
            return 0
        else
            warn "APT update attempt $attempt failed"
            if [[ $attempt -lt $max_attempts ]]; then
                info "Retrying in 5 seconds..."
                sleep 5
            fi
            ((attempt++))
        fi
    done
    
    err "Failed to update package cache after $max_attempts attempts"
}

create_apt_backup() {
    local timestamp
    timestamp=$(date +%s)
    local backup_dir="${BACKUP_PREFIX}-apt-${timestamp}"

    info "Creating APT configuration backup..."
    mkdir -p "$backup_dir"

    # Backup main sources
    if [[ -f /etc/apt/sources.list ]]; then
        cp /etc/apt/sources.list "$backup_dir/"
    fi

    # Backup sources.list.d directory
    if [[ -d /etc/apt/sources.list.d ]]; then
        cp -r /etc/apt/sources.list.d "$backup_dir/"
    fi

    # Store current foreign architectures
    dpkg --print-foreign-architectures > "$backup_dir/foreign_architectures" 2>/dev/null || true

    # Store backup location and environment info
    echo "$backup_dir" > "${BACKUP_PREFIX}-apt-path"
    echo "Distribution: $DIST_ID $DIST_VERSION ($DIST_CODENAME)" >> "$backup_dir/restore_info"
    echo "Environment: WSL=$IS_WSL Container=$IS_CONTAINER" >> "$backup_dir/restore_info"
    echo "Created: $(date)" >> "$backup_dir/restore_info"
    
    info "  > Backup created: $backup_dir"
}

restore_apt_backup() {
    if [[ ! -f "${BACKUP_PREFIX}-apt-path" ]]; then
        err "No APT backup found. Cannot restore configuration."
    fi

    local backup_dir
    backup_dir=$(cat "${BACKUP_PREFIX}-apt-path")

    if [[ ! -d "$backup_dir" ]]; then
        err "Backup directory not found: $backup_dir"
    fi

    warn "This will restore your APT configuration from: $backup_dir"
    if [[ -f "$backup_dir/restore_info" ]]; then
        info "Backup details:"
        cat "$backup_dir/restore_info" | sed 's/^/  /'
    fi
    
    read -p "Do you want to continue? (y/N) " -n 1 -r; echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        err "Restore aborted by user."
    fi

    info "Restoring APT configuration..."

    # Restore main sources
    if [[ -f "$backup_dir/sources.list" ]]; then
        sudo cp "$backup_dir/sources.list" /etc/apt/
        info "  > Restored main sources list"
    fi

    # Restore sources.list.d
    if [[ -d "$backup_dir/sources.list.d" ]]; then
        sudo rm -rf /etc/apt/sources.list.d/*
        sudo cp -r "$backup_dir/sources.list.d"/* /etc/apt/sources.list.d/ 2>/dev/null || true
        info "  > Restored sources.list.d directory"
    fi

    # Reset APT update flag to force refresh
    APT_UPDATE_DONE=0
    safe_apt_update

    success "APT configuration restored successfully"
    rm -rf "$backup_dir" "${BACKUP_PREFIX}-apt-path"
}

configure_apt_sources_ubuntu() {
    local sources_file="/etc/apt/sources.list.d/99-qdomyos-zwift-cross.sources"

    warn "--- CONFIGURING UBUNTU APT SOURCES ---"
    warn "This will add a dedicated source file for cross-compilation:"
    warn "  $sources_file"
    warn "Your existing configuration will not be modified."
    
    if [[ $IS_WSL -eq 1 ]]; then
        warn "WSL detected - using ports.ubuntu.com for ARM64 packages"
    fi
    
    read -p "Do you want to continue? (y/N) " -n 1 -r; echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        err "Configuration aborted by user."
    fi

    info "Writing ARM64 source configuration..."
    
    # Use appropriate mirror for different environments
    local ubuntu_mirror="http://ports.ubuntu.com/ubuntu-ports/"
    local keyring="/usr/share/keyrings/ubuntu-archive-keyring.gpg"
    
    # Check if keyring exists
    if [[ ! -f "$keyring" ]]; then
        warn "Ubuntu keyring not found, trying alternative configuration"
        keyring="/usr/share/keyrings/archive.key"
    fi
    
    sudo tee "$sources_file" > /dev/null <<EOF
# Source configuration for QDomyos-Zwift cross-compilation
# This file can be safely removed to disable ARM64 package sources
# Generated by cross_compile.sh v${SCRIPT_VERSION} on $(date)
# Environment: WSL=$IS_WSL Container=$IS_CONTAINER

Types: deb
URIs: ${ubuntu_mirror}
Suites: ${DIST_CODENAME} ${DIST_CODENAME}-updates ${DIST_CODENAME}-backports ${DIST_CODENAME}-security
Components: main restricted universe multiverse
Architectures: arm64
Signed-By: ${keyring}
EOF

    APT_UPDATE_DONE=0
    safe_apt_update
    success "Ubuntu APT sources configured successfully"
}

configure_apt_sources_debian() {
    local sources_file="/etc/apt/sources.list"

    warn "--- CONFIGURING DEBIAN APT SOURCES ---"
    warn "This will REPLACE your main sources file to enable multi-arch:"
    warn "  $sources_file"
    warn "Detected Debian version: $DIST_CODENAME"
    warn "A backup will be created automatically."
    
    if [[ $IS_WSL -eq 1 ]]; then
        warn "WSL detected - optimized mirror configuration will be used"
    fi
    
    read -p "Do you want to continue? (y/N) " -n 1 -r; echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        err "Configuration aborted by user."
    fi

    info "Writing multi-architecture source configuration..."
    
    # Use appropriate mirrors
    local debian_mirror="http://deb.debian.org/debian/"
    local security_mirror="http://security.debian.org/debian-security/"
    
    sudo tee "$sources_file" > /dev/null <<EOF
# /etc/apt/sources.list configured by QDomyos-Zwift build script for multi-arch
# Generated by cross_compile.sh v${SCRIPT_VERSION} on $(date)
# Environment: WSL=$IS_WSL Container=$IS_CONTAINER
# Original configuration backed up

# Main repositories with multi-arch support
deb [arch=amd64,arm64] ${debian_mirror} ${DIST_CODENAME} main contrib non-free non-free-firmware
deb [arch=amd64,arm64] ${debian_mirror} ${DIST_CODENAME}-updates main contrib non-free non-free-firmware
deb [arch=amd64,arm64] ${security_mirror} ${DIST_CODENAME}-security main contrib non-free non-free-firmware

# Backports (optional, uncomment if needed)
# deb [arch=amd64,arm64] ${debian_mirror} ${DIST_CODENAME}-backports main contrib non-free non-free-firmware
EOF

    APT_UPDATE_DONE=0
    safe_apt_update
    success "Debian APT sources configured successfully"
}

# ------------------ Package Management ------------------
validate_target_packages() {
    info "Validating ARM64 package availability..."
    local missing_packages=()
    local suggestions=()
    local total_packages=${#ESSENTIAL_TARGET_PACKAGES[@]}

    for pkg in "${ESSENTIAL_TARGET_PACKAGES[@]}"; do
        local output
        output=$(apt-cache policy "$pkg" 2>/dev/null || true)
        if [[ "$output" != *"Candidate:"* ]] || [[ "$output" == *"Candidate: (none)"* ]]; then
            missing_packages+=("$pkg")

            # Generate suggestions for common issues
            case "$pkg" in
                *qt5*) suggestions+=("• Check Qt5 packages: apt search qt5.*dev | grep arm64") ;;
                *python*) suggestions+=("• Check Python packages: apt search libpython.*dev | grep arm64") ;;
                *usb*) suggestions+=("• Check USB packages: apt search libusb.*dev | grep arm64") ;;
            esac
        fi
    done

    if [[ ${#missing_packages[@]} -gt 0 ]]; then
        warn "\n📦 Missing ARM64 packages (${#missing_packages[@]}/${total_packages} total):"
        printf '   %s\n' "${missing_packages[@]}"

        if [[ ${#suggestions[@]} -gt 0 ]]; then
            warn "\n🔍 Troubleshooting suggestions:"
            printf '   %s\n' "${suggestions[@]}"
        fi

        warn "\n💡 Common solutions:"
        echo "   • Ensure ARM64 architecture is enabled: sudo dpkg --add-architecture arm64"
        echo "   • Update package cache: sudo apt-get update"

        # Environment-specific suggestions
        if [[ $IS_WSL -eq 1 ]]; then
            warn "\n🐧 WSL-specific suggestions:"
            echo "   • Ensure WSL has network connectivity"
            echo "   • Try: wsl --update (from Windows)"
        fi

        case "$DIST_ID" in
            ubuntu|debian)
                err "Run with '--configure-sources' to automatically configure APT sources."
                ;;
            *)
                err "Please ensure your APT sources support multi-architecture package downloads."
                ;;
        esac
    fi

    success "All required ARM64 packages are available ($total_packages packages)"
}

setup_system_dependencies() {
    info "Setting up system dependencies..."

    # Enable ARM64 architecture if not already enabled
    if ! dpkg --print-foreign-architectures 2>/dev/null | grep -q "arm64"; then
        info "  > Enabling ARM64 architecture..."
        sudo dpkg --add-architecture arm64
        APT_UPDATE_DONE=0  # Force update after architecture change
        safe_apt_update
    else
        info "  > ARM64 architecture already enabled"
    fi

    # Update package cache if needed
    safe_apt_update

    # Validate packages before attempting installation
    validate_target_packages

    info "  > Installing host packages..."
    
    # Enhanced package availability check for host packages
    local filtered_host_packages=()
    local already_installed=()
    
    for pkg in "${REQUIRED_HOST_PACKAGES[@]}"; do
        # Check if already installed first
        if dpkg -l "$pkg" 2>/dev/null | grep -q '^ii'; then
            already_installed+=("$pkg")
            continue
        fi
        
        # Check if available for installation
        local policy_output
        policy_output=$(apt-cache policy "$pkg" 2>/dev/null || echo "")
        if [[ "$policy_output" =~ Candidate:.*[0-9] ]] && [[ ! "$policy_output" =~ "Candidate: (none)" ]]; then
            filtered_host_packages+=("$pkg")
        fi
    done
    
    if [[ ${#already_installed[@]} -gt 0 ]]; then
        info "    > Already installed: ${#already_installed[@]} packages"
    fi
    
    if [[ ${#filtered_host_packages[@]} -gt 0 ]]; then
        info "    > Installing ${#filtered_host_packages[@]} new packages..."
        sudo apt-get install -y "${filtered_host_packages[@]}"
    else
        info "    > All required host packages are already installed"
    fi

    info "  > Installing ARM64 target packages..."
    # Install in smaller batches to handle potential issues better
    local batch_size=5
    local installed_count=0
    
    for ((i=0; i<${#ESSENTIAL_TARGET_PACKAGES[@]}; i+=batch_size)); do
        local batch=("${ESSENTIAL_TARGET_PACKAGES[@]:i:batch_size}")
        if sudo apt-get install -y --no-install-recommends "${batch[@]}"; then
            installed_count=$((installed_count + ${#batch[@]}))
        else
            warn "Failed to install package batch, trying individually..."
            for pkg in "${batch[@]}"; do
                if sudo apt-get install -y --no-install-recommends "$pkg"; then
                    installed_count=$((installed_count + 1))
                else
                    warn "Failed to install: $pkg"
                fi
            done
        fi
    done

    if [[ $installed_count -lt ${#ESSENTIAL_TARGET_PACKAGES[@]} ]]; then
        warn "Some packages failed to install ($installed_count/${#ESSENTIAL_TARGET_PACKAGES[@]} successful)"
        warn "Build may fail due to missing dependencies"
    fi

    success "System dependencies installed successfully ($installed_count packages)"
}

# ------------------ Python Environment ------------------
extract_python_headers_and_libs() {
    info "Extracting Python ARM64 components..."
    local headers_dir="$BUILD_DIR/python-arm64-headers"
    local libs_dir="$BUILD_DIR/python-arm64-libs"

    # Check if already extracted and valid
    if [[ -d "$headers_dir" && -d "$libs_dir" ]]; then
        local lib_count
        lib_count=$(find "$libs_dir" -name "libpython${PYTHON_VERSION_STR}*" 2>/dev/null | wc -l)
        if [[ $lib_count -gt 0 ]]; then
            info "  > Python ARM64 components already extracted ($lib_count libraries found)"
            return 0
        fi
    fi

    local temp_dir
    temp_dir=$(mktemp -d -t python-cross-extract-XXXXXX)
    rm -rf "$headers_dir" "$libs_dir"
    mkdir -p "$headers_dir" "$libs_dir"

    # Ensure cleanup of temp directory
    trap "rm -rf '$temp_dir'" EXIT

    info "  > Downloading Python packages to APT cache..."
    local dev_pkg="libpython${PYTHON_VERSION_STR}-dev:arm64"
    local runtime_pkg="libpython${PYTHON_VERSION_STR}${PYTHON_PKG_SUFFIX}:arm64"

    # Enhanced error handling for package download
    if ! sudo apt-get install --download-only --reinstall -y "$dev_pkg" "$runtime_pkg"; then
        err "Failed to download Python packages. Check package availability and network connection."
    fi

    info "  > Locating .deb files in APT cache..."
    
    # Enhanced .deb file location with better error handling
    local dev_deb_file runtime_deb_file
    local cache_dir="/var/cache/apt/archives"
    
    # Find development package
    dev_deb_file=$(find "$cache_dir" -maxdepth 1 -name "libpython${PYTHON_VERSION_STR}-dev_*_arm64.deb" -type f 2>/dev/null | sort -V | tail -1)
    if [[ -z "$dev_deb_file" || ! -f "$dev_deb_file" ]]; then
        err "Development package not found in APT cache: libpython${PYTHON_VERSION_STR}-dev:arm64"
    fi
    cp "$dev_deb_file" "$temp_dir/"
    info "    > Found dev package: $(basename "$dev_deb_file")"

    # Find runtime package
    local runtime_pattern="libpython${PYTHON_VERSION_STR}${PYTHON_PKG_SUFFIX}_*_arm64.deb"
    runtime_deb_file=$(find "$cache_dir" -maxdepth 1 -name "$runtime_pattern" -type f 2>/dev/null | sort -V | tail -1)
    
    if [[ -z "$runtime_deb_file" || ! -f "$runtime_deb_file" ]]; then
        err "Runtime package not found in APT cache: libpython${PYTHON_VERSION_STR}${PYTHON_PKG_SUFFIX}:arm64"
    fi
    cp "$runtime_deb_file" "$temp_dir/"
    info "    > Found runtime package: $(basename "$runtime_deb_file")"

    info "  > Extracting packages..."
    local dev_deb_name runtime_deb_name
    dev_deb_name=$(basename "$dev_deb_file")
    runtime_deb_name=$(basename "$runtime_deb_file")

    if ! dpkg -x "$temp_dir/$dev_deb_name" "$temp_dir/dev"; then
        err "Failed to extract development package"
    fi
    
    if ! dpkg -x "$temp_dir/$runtime_deb_name" "$temp_dir/lib"; then
        err "Failed to extract runtime package"
    fi

    info "  > Copying headers and libraries..."
    local python_include_path="$temp_dir/dev/usr/include/python${PYTHON_VERSION_STR}"
    if [[ -d "$python_include_path" ]]; then
        cp -r "$python_include_path" "$headers_dir/"
    else
        # Try alternative paths
        local alt_include_path
        alt_include_path=$(find "$temp_dir/dev" -type d -name "python${PYTHON_VERSION_STR}*" | head -1)
        if [[ -d "$alt_include_path" ]]; then
            cp -r "$alt_include_path" "$headers_dir/"
            warn "Used alternative include path: $alt_include_path"
        else
            err "Python headers not found in development package"
        fi
    fi

    local lib_search_path="$temp_dir/lib/usr/lib/aarch64-linux-gnu"
    if [[ -d "$lib_search_path" ]]; then
        find "$lib_search_path" -name "libpython${PYTHON_VERSION_STR}*.so*" -exec cp -d {} "$libs_dir/" \;
    else
        # Try alternative lib paths
        local alt_lib_path
        alt_lib_path=$(find "$temp_dir/lib" -type d -name "aarch64-linux-gnu" | head -1)
        if [[ -d "$alt_lib_path" ]]; then
            find "$alt_lib_path" -name "libpython${PYTHON_VERSION_STR}*.so*" -exec cp -d {} "$libs_dir/" \;
            warn "Used alternative lib path: $alt_lib_path"
        else
            err "Python library directory not found in runtime package"
        fi
    fi

    # Verify extraction
    local extracted_libs
    extracted_libs=$(find "$libs_dir" -name "libpython${PYTHON_VERSION_STR}*" 2>/dev/null | wc -l)
    if [[ $extracted_libs -eq 0 ]]; then
        err "Failed to extract Python libraries"
    fi

    success "Python components extracted ($extracted_libs libraries)"
}

setup_python_venv() {
    info "Setting up Python virtual environment..."
    local venv_path="${HOME}/${VENV_NAME}"

    # Check if venv exists and has required packages
    if [[ -d "$venv_path" ]] && "$venv_path/bin/python3" -c "import pybind11, usb, openant" 2>/dev/null; then
        info "  > Virtual environment already configured"
        echo "$venv_path/bin/python3" > .ant_venv_path
        return 0
    fi

    info "  > Creating/updating virtual environment..."
    rm -rf "$venv_path"
    
    # Enhanced venv creation with better error handling
    if ! python3 -m venv "$venv_path"; then
        err "Failed to create Python virtual environment. Check Python3 venv module availability."
    fi

    # Verify venv was created successfully
    if [[ ! -f "$venv_path/bin/python3" ]]; then
        err "Virtual environment creation failed - Python interpreter not found"
    fi

    info "  > Installing Python packages..."
    
    # Upgrade pip first with timeout and retry mechanism
    local max_attempts=3
    local attempt=1
    
    while [[ $attempt -le $max_attempts ]]; do
        if timeout 60 "$venv_path/bin/python3" -m pip install --upgrade pip >/dev/null 2>&1; then
            break
        else
            warn "Pip upgrade attempt $attempt failed"
            if [[ $attempt -lt $max_attempts ]]; then
                info "Retrying pip upgrade..."
                sleep 2
            else
                warn "Pip upgrade failed after $max_attempts attempts, continuing anyway"
            fi
        fi
        ((attempt++))
    done
    
    # Install required packages with better error handling
    local required_packages=("pybind11" "pyusb" "openant")
    local failed_packages=()
    
    for pkg in "${required_packages[@]}"; do
        info "    > Installing $pkg..."
        if ! timeout 60 "$venv_path/bin/python3" -m pip install "$pkg" >/dev/null 2>&1; then
            failed_packages+=("$pkg")
            warn "Failed to install Python package: $pkg"
        fi
    done
    
    if [[ ${#failed_packages[@]} -gt 0 ]]; then
        warn "Some Python packages failed to install: ${failed_packages[*]}"
        warn "ANT+ functionality may be limited"
    fi

    # Test the venv
    if "$venv_path/bin/python3" -c "import sys; print(f'Python {sys.version}')" >/dev/null 2>&1; then
        echo "$venv_path/bin/python3" > .ant_venv_path
        success "Python virtual environment ready"
    else
        err "Virtual environment verification failed"
    fi
}

pregenerate_ant_header() {
    info "Pre-generating ANT+ header..."
    local output_header="$BUILD_DIR/ant_footpod_script.h"
    local venv_python

    if [[ ! -f .ant_venv_path ]]; then
        err "Python virtual environment not set up. Run setup first."
    fi

    venv_python=$(cat .ant_venv_path)

    if [[ ! -x "$venv_python" ]]; then
        err "Python virtual environment binary not found: $venv_python"
    fi

    local py_script="devices/antlinux/py_to_header.py"
    local ant_script="devices/antlinux/ant_broadcaster.py"

    if [[ ! -f "$py_script" ]]; then
        err "Python-to-header script not found: $py_script"
    fi

    if [[ ! -f "$ant_script" ]]; then
        err "ANT footpod script not found: $ant_script"
    fi

    # Enhanced header generation with better error handling and output capture
    info "  > Running: $venv_python $py_script $ant_script $output_header"
    if ! "$venv_python" "$py_script" "$ant_script" "$output_header" 2>&1; then
        err "Failed to generate ANT+ header using Python script"
    fi

    if [[ ! -f "$output_header" ]]; then
        err "ANT+ header was not created: $output_header"
    fi

    # Verify the generated header
    if [[ ! -s "$output_header" ]]; then
        err "Generated ANT+ header is empty"
    fi

    success "ANT+ header generated successfully ($(wc -l < "$output_header") lines)"
}

# ------------------ Build Process ------------------
check_submodules() {
    info "Checking Git submodules..."

    # More flexible Git repository detection
    local git_root
    git_root=$(git rev-parse --show-toplevel 2>/dev/null || echo "")
    
    if [[ -z "$git_root" ]]; then
        warn "Not in a Git repository. Skipping submodule check."
        warn "If this project uses submodules, initialize them manually:"
        warn "  git submodule update --init --recursive"
        return 0
    fi

    info "  > Git repository root: $git_root"

    # Check if submodules are defined
    if [[ ! -f "$git_root/.gitmodules" ]]; then
        info "  > No submodules defined in this repository"
        return 0
    fi

    # Check submodule status
    local submodule_status
    submodule_status=$(cd "$git_root" && git submodule status --recursive 2>/dev/null || echo "ERROR")
    
    if [[ "$submodule_status" == "ERROR" ]]; then
        warn "Could not check submodule status. You may need to run:"
        warn "  git submodule update --init --recursive"
        return 0
    fi

    if echo "$submodule_status" | grep -q '^-'; then
        err "Git submodules not initialized. Please run:\n\n    git submodule update --init --recursive\n"
    fi

    local submodule_count
    submodule_count=$(echo "$submodule_status" | wc -l)
    success "Git submodules are properly initialized ($submodule_count submodules)"
}

force_clean_all() {
    info "Performing deep clean..."

    # Clean ccache
    if command -v ccache >/dev/null 2>&1; then
        ccache -C >/dev/null 2>&1 || true
        ccache -z >/dev/null 2>&1 || true
        info "  > Cleared ccache"
    fi

    # Clean build directory
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    info "  > Cleaned build directory"

    # CRITICAL FIX: Remove stale generated headers
    local generated_headers=(
        "devices/antfootpod/linux/ant_footpod_script.h"
        "devices/antlinux/ant_footpod_script.h"
    )
    
    for header in "${generated_headers[@]}"; do
        if [[ -f "$header" ]]; then
            rm -f "$header"
            info "  > Removed stale generated header: $header"
        fi
    done

    # Clean any existing build artifacts
    if make distclean >/dev/null 2>&1; then
        info "  > Cleaned source directory with 'make distclean'"
    else
        # Alternative cleanup methods
        local cleanup_files=("Makefile" "*.o" "moc_*" "qrc_*" "ui_*")
        for pattern in "${cleanup_files[@]}"; do
            if compgen -G "$pattern" > /dev/null; then
                rm -f $pattern
            fi
        done
        info "  > Cleaned source directory (manual cleanup)"
    fi

    success "Deep clean completed"
}

verify_cross_toolchain() {
    info "Verifying cross-compilation toolchain..."

    local required_tools=(
        "aarch64-linux-gnu-gcc"
        "aarch64-linux-gnu-g++"
        "aarch64-linux-gnu-pkg-config"
        "$CROSS_QMAKE"
    )

    local missing_tools=()
    for tool in "${required_tools[@]}"; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            missing_tools+=("$tool")
        else
            info "  > Found: $tool"
            # Show version for verification
            local version
            version=$("$tool" --version 2>/dev/null | head -1 || echo "version unknown")
            info "    Version: $version"
        fi
    done

    if [[ ${#missing_tools[@]} -gt 0 ]]; then
        err "Missing cross-compilation tools: ${missing_tools[*]}\nInstall with: sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu"
    fi

 # Enhanced pkg-config test for cross-compilation
    local pkg_config_paths=(
        "/usr/lib/aarch64-linux-gnu/pkgconfig"
        "/usr/lib/pkgconfig"
        "/usr/share/pkgconfig"
    )
    
    local qt_found=0
    for path in "${pkg_config_paths[@]}"; do
        if PKG_CONFIG_PATH="$path" PKG_CONFIG_LIBDIR="$path" aarch64-linux-gnu-pkg-config --exists Qt5Core 2>/dev/null; then
            info "  > pkg-config test: Qt5 Core found in $path"
            qt_found=1
            break
        fi
    done
    
    if [[ $qt_found -eq 0 ]]; then
        # Try alternative Qt5 detection methods
        if find /usr/lib/aarch64-linux-gnu -name "libQt5Core.so*" 2>/dev/null | head -1 | grep -q .; then
            info "  > pkg-config test: Qt5 libraries found via filesystem search"
            qt_found=1
        fi
    fi
    
    if [[ $qt_found -eq 0 ]]; then
        warn "pkg-config test: Qt5 Core not found for ARM64 (may cause build issues)"
        warn "This is usually not critical if Qt5 ARM64 packages are installed"
    fi

    # Enhanced cross-compiler test
    info "  > Testing cross-compiler..."
    local test_file="/tmp/test_cross_$"
    local test_source='#include <iostream>\nint main(){std::cout<<"OK"<<std::endl;return 0;}'
    
    if echo -e "$test_source" | aarch64-linux-gnu-g++ -x c++ - -o "$test_file" 2>/dev/null; then
        # Verify the binary is actually ARM64
        local file_output
        file_output=$(file "$test_file" 2>/dev/null || echo "unknown")
        if [[ "$file_output" == *"aarch64"* ]] || [[ "$file_output" == *"ARM64"* ]]; then
            success "Cross-compiler test: C++ compilation successful (ARM64)"
        else
            warn "Cross-compiler test: Binary not ARM64: $file_output"
        fi
    else
        err "Cross-compiler test compilation failed"
    fi
    rm -f "$test_file"

    # Test pkg-config for cross-compilation
    if PKG_CONFIG_PATH="/usr/lib/aarch64-linux-gnu/pkgconfig" aarch64-linux-gnu-pkg-config --exists qtcore 2>/dev/null; then
        info "  > pkg-config test: Qt5 Core found for ARM64"
    else
        warn "pkg-config test: Qt5 Core not found for ARM64 (may cause build issues)"
    fi

    success "Cross-compilation toolchain verified"
}

configure_build() {
    info "Configuring project with cross-qmake..."

    # Verify project file exists in current directory
    if [[ ! -f "$PROJECT_FILE" ]]; then
        info "  > Current directory: $(pwd)"
        info "  > Looking for: $PROJECT_FILE"
        info "  > Directory contents:"
        ls -la *.pro 2>/dev/null || echo "    No .pro files found"
        err "Project file '$PROJECT_FILE' not found in current directory"
    fi

    info "  > Found project file: $PROJECT_FILE"

    # Get full paths to cross-compilation tools
    local gcc_path cxx_path ar_path strip_path
    gcc_path=$(command -v aarch64-linux-gnu-gcc) || err "aarch64-linux-gnu-gcc not found in PATH"
    cxx_path=$(command -v aarch64-linux-gnu-g++) || err "aarch64-linux-gnu-g++ not found in PATH"
    ar_path=$(command -v aarch64-linux-gnu-ar) || err "aarch64-linux-gnu-ar not found in PATH"
    strip_path=$(command -v aarch64-linux-gnu-strip) || err "aarch64-linux-gnu-strip not found in PATH"

    info "  > Located cross-compilation tools:"
    info "    GCC: $gcc_path"
    info "    G++: $cxx_path"

    # Enhanced environment setup for cross-compilation
    export PKG_CONFIG_PATH="/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig"
    export PKG_CONFIG_LIBDIR="/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig"
    export PKG_CONFIG_SYSROOT_DIR="/"
    export CROSS_COMPILE="aarch64-linux-gnu-"

    # Configure compiler paths (with or without ccache)
    if command -v ccache >/dev/null 2>&1; then
        export CC="ccache $gcc_path"
        export CXX="ccache $cxx_path"
        info "  > Using ccache with cross-compilation tools"
    else
        export CC="$gcc_path"
        export CXX="$cxx_path"
    fi

    export AR="$ar_path"
    export STRIP="$strip_path"

    # Ensure ANT+ virtual environment path is available
    local ant_venv_python=""
    if [[ -f ".ant_venv_path" ]]; then
        ant_venv_python=$(cat .ant_venv_path)
        if [[ -x "$ant_venv_python" ]]; then
            info "  > ANT+ Python environment: $ant_venv_python"
            export ANT_VENV_PYTHON="$ant_venv_python"
        else
            warn "ANT+ Python environment not executable: $ant_venv_python"
        fi
    else
        warn "ANT+ virtual environment path not found - ANT+ support may be disabled"
    fi

    # Prepare paths
    local project_path python_headers_path python_libs_path
    project_path="$(pwd)/$PROJECT_FILE"
    python_headers_path="$(pwd)/$BUILD_DIR/python-arm64-headers"
    python_libs_path="$(pwd)/$BUILD_DIR/python-arm64-libs"

    info "  > Python ARM64 paths:"
    info "    Headers: $python_headers_path"
    info "    Libraries: $python_libs_path"

    # Build qmake arguments
    local qmake_args=(
        "$project_path"
        "CONFIG+=release"
        "CONFIG+=private_headers"
        "CONFIG+=cross_compile"
        "DEFINES+=CROSS_COMPILE=1"
        "QMAKE_CC=$gcc_path"
        "QMAKE_CXX=$cxx_path"
        "QMAKE_LINK=$cxx_path"
        "QMAKE_AR=$ar_path"
        "QMAKE_STRIP=$strip_path"
        "QMAKE_CXXFLAGS+=-std=c++17"
        "QMAKE_CXXFLAGS+=-ffile-prefix-map=${PWD}=."
        "QMAKE_INCDIR+=/usr/include/aarch64-linux-gnu"
        "QMAKE_LIBDIR+=/usr/lib/aarch64-linux-gnu"
        "QMAKE_INCDIR+=$python_headers_path"
        "QMAKE_LIBDIR+=$python_libs_path"
        "PYTHON_INCLUDE_PATH=$python_headers_path"
        "PYTHON_LIB_PATH=$python_libs_path"
        "PYTHON_VERSION=$PYTHON_VERSION_STR"
    )

    # Add ANT+ configuration if available
    if [[ -n "$ant_venv_python" ]]; then
        qmake_args+=(
            "CONFIG+=enable_ant"
            "DEFINES+=ANT_ENABLED=1"
            "ANT_VENV_PYTHON=$ant_venv_python"
            "DEFINES+=ANT_VENV_AVAILABLE=1"
        )
    else
        info "  > ANT+ support disabled (no Python environment)"
    fi

    # Environment-specific adjustments
    if [[ $IS_WSL -eq 1 ]]; then
        # WSL may need additional flags
        qmake_args+=("QMAKE_CXXFLAGS+=-D_WSL_BUILD=1")
        info "  > Added WSL-specific build flags"
    fi

    info "  > qmake configuration summary:"
    info "    Target architecture: ARM64 (aarch64)"
    info "    Build type: Release"
    info "    Cross-compile: enabled"
    info "    ANT+ support: ${ant_venv_python:+enabled}${ant_venv_python:-disabled}"
    info "    Python version: $PYTHON_VERSION_STR"

    info "  > Running qmake with cross-compilation settings..."
    
    # Run qmake with enhanced error reporting
    local qmake_output qmake_exit_code
    if qmake_output=$(cd "$BUILD_DIR" && "$CROSS_QMAKE" "${qmake_args[@]}" 2>&1); then
        qmake_exit_code=0
    else
        qmake_exit_code=$?
    fi

    if [[ $qmake_exit_code -ne 0 ]]; then
        err "qmake configuration failed with exit code $qmake_exit_code:\n$qmake_output"
    fi

    if [[ ! -f "$BUILD_DIR/Makefile" ]]; then
        err "qmake configuration failed - no Makefile generated"
    fi

    # Enhanced Makefile verification
    info "  > Verifying Makefile configuration..."
    local makefile_issues=()
    
    if ! grep -q "aarch64-linux-gnu" "$BUILD_DIR/Makefile"; then
        makefile_issues+=("Cross-compilation settings not detected")
    fi
    
    if [[ -n "$ant_venv_python" ]] && ! grep -q -i "ant" "$BUILD_DIR/Makefile"; then
        makefile_issues+=("ANT+ configuration missing")
    fi
    
    if [[ ${#makefile_issues[@]} -gt 0 ]]; then
        warn "Makefile verification issues:"
        printf '  - %s\n' "${makefile_issues[@]}"
        warn "Build may not work as expected"
    else
        success "Makefile verification passed"
    fi

    # Display build configuration summary
    info "  > Build configuration summary:"
    if [[ -f "$BUILD_DIR/Makefile" ]]; then
        local compiler_line ant_objects_count
        compiler_line=$(grep '^CXX' "$BUILD_DIR/Makefile" | head -1 || echo "Compiler line not found")
        ant_objects_count=$(grep -c "AntWrapper\|ant_footpod" "$BUILD_DIR/Makefile" 2>/dev/null || echo "0")
        
        echo "    $compiler_line"
        echo "    ANT+ objects referenced: $ant_objects_count"
        echo "    Build directory: $BUILD_DIR"
    fi

    success "Project configured successfully for cross-compilation"
}

build_project() {
    info "Building project with cross-compiler..."
    info "  > Using build flags: $MAKEFLAGS"

    COMPILE_START_TIME=$(date +%s)
    local compile_start_display build_log
    compile_start_display=$(date)
    build_log="$BUILD_DIR/build.log"

    info "  > Compilation started at: $compile_start_display"
    info "  > Build log: $build_log"
    
    # Run make with output redirection for logging
    if make -C "$BUILD_DIR" $MAKEFLAGS 2>&1 | tee "$build_log"; then
        COMPILE_END_TIME=$(date +%s)
        local compile_time compile_end_display
        compile_time=$((COMPILE_END_TIME - COMPILE_START_TIME))
        compile_end_display=$(date)
        
        success "Build completed successfully in $(format_duration $compile_time)"
        info "  > Compilation ended at: $compile_end_display"
        
        # Show build statistics
        local object_count executable_size
        object_count=$(find "$BUILD_DIR" -name "*.o" | wc -l)
        if [[ -f "$BUILD_DIR/qdomyos-zwift" ]]; then
            executable_size=$(du -h "$BUILD_DIR/qdomyos-zwift" | cut -f1)
            info "  > Build statistics:"
            info "    Object files: $object_count"
            info "    Executable size: $executable_size"
        fi
    else
        local build_exit_code=$?
        COMPILE_END_TIME=$(date +%s)
        local compile_time
        compile_time=$((COMPILE_END_TIME - COMPILE_START_TIME))
        
        warn "Build failed after $(format_duration $compile_time) with exit code $build_exit_code"
        
        # Analyze build log for common issues
        if [[ -f "$build_log" ]]; then
            warn "Build error analysis:"
            
            local error_patterns=(
                "No such file or directory"
                "undefined reference"
                "fatal error"
                "cannot find -l"
                "Permission denied"
            )
            
            for pattern in "${error_patterns[@]}"; do
                local count
                count=$(grep -c "$pattern" "$build_log" 2>/dev/null || echo "0")
                if [[ $count -gt 0 ]]; then
                    echo "  - $pattern: $count occurrence(s)"
                fi
            done
            
            warn "Last 10 lines of build log:"
            tail -10 "$build_log" | sed 's/^/  /'
        fi
        
        err "Build failed. Check build log: $build_log"
    fi

    # Show ccache statistics if available
    if command -v ccache >/dev/null 2>&1; then
        info "Final ccache statistics:"
        ccache --show-stats 2>/dev/null | grep -E "(hit rate|files|size)" | sed 's/^/  /' || true
    fi
}

verify_build_result() {
    local binary_path="$BUILD_DIR/qdomyos-zwift"

    info "Verifying build result..."

    if [[ ! -f "$binary_path" ]]; then
        err "Build completed but binary not found: $binary_path"
    fi

    # Enhanced binary verification
    info "  > Binary verification:"
    
    # Check ELF architecture
    local file_output
    file_output=$(file "$binary_path" 2>/dev/null || echo "unknown")
    if [[ "$file_output" == *"aarch64"* ]] || [[ "$file_output" == *"ARM64"* ]]; then
        success "    Architecture: ARM64 ✓"
    else
        err "    Architecture verification failed: $file_output"
    fi

    # Check binary size (sanity check)
    local size_kb size_mb
    size_kb=$(du -k "$binary_path" 2>/dev/null | cut -f1 || echo "0")
    size_mb=$((size_kb / 1024))
    
    if [[ $size_kb -lt 1000 ]]; then
        warn "    Binary size unusually small: ${size_kb}KB"
    elif [[ $size_mb -gt 100 ]]; then
        warn "    Binary size unusually large: ${size_mb}MB"
    else
        success "    Binary size: ${size_mb}MB ✓"
    fi

    # Check for executable permissions
    if [[ -x "$binary_path" ]]; then
        success "    Executable permissions: ✓"
    else
        warn "    Binary is not executable"
        chmod +x "$binary_path"
        info "    Fixed executable permissions"
    fi

    # Enhanced dynamic library dependency check
    if command -v qemu-aarch64-static >/dev/null 2>&1; then
        info "  > Testing dynamic library dependencies..."
        
        local qemu_test_output qemu_exit_code
        if command -v timeout >/dev/null 2>&1; then
            qemu_test_output=$(timeout 10s qemu-aarch64-static -L /usr/aarch64-linux-gnu "$binary_path" --version 2>&1 || true)
            qemu_exit_code=$?
        else
            info "    Timeout command unavailable, skipping qemu test"
            qemu_exit_code=124
        fi

        case $qemu_exit_code in
            0)
                success "    Dynamic library test: ✓ (executable runs)"
                if [[ "$qemu_test_output" == *"QDomyos"* ]]; then
                    info "    Application identification confirmed"
                fi
                ;;
            124)
                info "    Dynamic library test: Skipped (timeout/unavailable)"
                ;;
            *)
                local missing_count
                missing_count=$(echo "$qemu_test_output" | grep -c "not found" 2>/dev/null || echo "0")
                if [[ $missing_count -eq 0 ]]; then
                    info "    Dynamic library test: ✓ (no missing libraries)"
                else
                    warn "    Dynamic library test: $missing_count missing libraries detected"
                    echo "$qemu_test_output" | grep "not found" | head -3 | sed 's/^/      /'
                fi
                ;;
        esac
    fi

    # Additional verification using readelf
    if command -v aarch64-linux-gnu-readelf >/dev/null 2>&1; then
        local elf_machine interpreter
        elf_machine=$(aarch64-linux-gnu-readelf -h "$binary_path" 2>/dev/null | grep "Machine:" | awk '{print $2}' || echo "unknown")
        interpreter=$(aarch64-linux-gnu-readelf -l "$binary_path" 2>/dev/null | grep "program interpreter" | sed 's/.*: \[//' | sed 's/\]//' || echo "unknown")
        
        if [[ "$elf_machine" == "AArch64" ]]; then
            success "    ELF machine type: $elf_machine ✓"
        else
            warn "    ELF machine type: $elf_machine (expected AArch64)"
        fi
        
        if [[ "$interpreter" == *"aarch64"* ]]; then
            success "    Program interpreter: ARM64 ✓"
        else
            info "    Program interpreter: $interpreter"
        fi
    fi

    # Copy binary to convenient location with enhanced naming
    local output_binary="./qdomyos-zwift-arm64"
    local timestamp
    timestamp=$(date +%Y%m%d-%H%M%S)
    
    info "  > Installing binary..."
    cp "$binary_path" "$output_binary"
    
    # Create timestamped backup
    cp "$binary_path" "./qdomyos-zwift-arm64-$timestamp"
    
    # Final verification of copied binary
    if [[ -f "$output_binary" ]] && [[ -x "$output_binary" ]]; then
        success "Binary installed: $(realpath "$output_binary")"
        info "Timestamped copy: $(realpath "./qdomyos-zwift-arm64-$timestamp")"
        
        # Display final binary information
        info "Final binary information:"
        file "$output_binary" | sed 's/^/  /'
        ls -lh "$output_binary" | sed 's/^/  /'
    else
        err "Failed to create final binary: $output_binary"
    fi
}

# ------------------ Pre-Build Summary & Confirmation ------------------
show_build_summary() {
    echo ""
    echo "🔍 BUILD CONFIGURATION SUMMARY"
    echo "============================================"
    echo "📋 Environment Details:"
    echo "   • Distribution: $DIST_ID $DIST_VERSION ($DIST_CODENAME)"
    echo "   • Environment: $([ $IS_WSL -eq 1 ] && echo "WSL" || ([ $IS_CONTAINER -eq 1 ] && echo "Container" || echo "Native"))"
    echo "   • Python version: $PYTHON_VERSION_STR"
    echo "   • Build jobs: $(echo $MAKEFLAGS | sed 's/-j//')"
    echo ""
    echo "🎯 Build Configuration:"
    echo "   • Target architecture: ARM64 (aarch64)"
    echo "   • Build type: Release with cross-compilation"
    echo "   • ANT+ support: $([ -f .ant_venv_path ] && echo "✅ Enabled" || echo "❌ Disabled")"
    echo "   • ccache: $(command -v ccache >/dev/null 2>&1 && echo "✅ Available" || echo "❌ Not available")"
    echo ""
    echo "📁 Paths:"
    echo "   • Project file: $PROJECT_FILE"
    echo "   • Build directory: $BUILD_DIR"
    echo "   • Python headers: $BUILD_DIR/python-arm64-headers"
    echo "   • Output binary: ./qdomyos-zwift-arm64"
    echo ""
    echo "⏱️  Starting compilation in 10 seconds... (Ctrl+C to abort)"
    
    # Countdown with progress indication
    for i in {10..1}; do
        printf "\r⏳ Starting in %2d seconds... " "$i"
        sleep 1
    done
    printf "\r✅ Starting compilation now!    \n\n"
}

# ------------------ Help and Usage ------------------
show_usage() {
    cat << 'EOF'
🏃 QDomyos-Zwift Cross-Compilation Script v21

USAGE:
    ./cross_compile.sh [OPTIONS]

OPTIONS:
    --full                  Install all system dependencies and build
    --configure-sources     Configure APT sources for cross-compilation
    --restore-apt          Restore original APT configuration
    --clean                 Clean build artifacts and exit
    --help                  Show this help message

EXAMPLES:
    # First-time setup on Ubuntu/Debian
    ./cross_compile.sh --configure-sources --full

    # Regular build (after initial setup)
    ./cross_compile.sh

    # Clean build
    ./cross_compile.sh --clean
    ./cross_compile.sh --full

    # Restore original APT configuration
    ./cross_compile.sh --restore-apt

SYSTEM REQUIREMENTS:
    • Ubuntu 20.04+ or Debian 11+ (or compatible derivatives)
    • WSL2 with Ubuntu/Debian (WSL1 not recommended)
    • 4GB+ RAM recommended for parallel compilation
    • 2GB+ free disk space for build dependencies
    • Internet connection for package downloads

SUPPORTED ENVIRONMENTS:
    ✅ Ubuntu 20.04, 22.04, 24.04+ (native)
    ✅ Debian 11 (Bullseye), 12 (Bookworm) (native)
    ✅ WSL2 with Ubuntu/Debian distributions
    ✅ Docker/Podman containers (Ubuntu/Debian based)
    ✅ Pop!_OS, Elementary OS, Linux Mint
    ⚠️  Other Debian-based distributions (may work)
    ❌ WSL1 (limited functionality)

WSL-SPECIFIC NOTES:
    • Ensure WSL2 is being used (not WSL1)
    • Network connectivity required for package downloads
    • May need 'wsl --update' from Windows if issues occur
    • systemd support recommended for full functionality

CONTAINER NOTES:
    • Use --privileged flag if needed for qemu-user-static
    • Ensure sufficient memory allocation
    • Network access required for package downloads

For more information: https://github.com/cagnulein/qdomyos-zwift
EOF
}

# ------------------ Main Function ------------------
main() {
    local do_full_setup=0
    local do_configure_sources=0
    local do_restore_apt=0
    local do_clean_only=0

    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --full)
                do_full_setup=1
                shift
                ;;
            --configure-sources)
                do_configure_sources=1
                shift
                ;;
            --restore-apt)
                do_restore_apt=1
                shift
                ;;
            --clean)
                do_clean_only=1
                shift
                ;;
            --help)
                show_usage
                exit 0
                ;;
            *)
                err "Unknown option: $1\n\nUse --help for usage information."
                ;;
        esac
    done

    # Initialize timing
    SCRIPT_START_TIME=$(date +%s)
    BUILD_START_TIME=$(date)

    echo "🏃 QDomyos-Zwift Cross-Compilation Script v${SCRIPT_VERSION}"
    echo "=================================================="
    echo "Started at: $BUILD_START_TIME"
    echo ""

    # Verify we're in the right directory
    if [[ ! -f "$PROJECT_FILE" ]]; then
        err "Project file '$PROJECT_FILE' not found.\nPlease run this script from the project 'src' directory."
    fi

    # Handle special operations first
    if [[ $do_restore_apt -eq 1 ]]; then
        restore_apt_backup
        exit 0
    fi

    if [[ $do_clean_only -eq 1 ]]; then
        force_clean_all
        success "Clean operation completed"
        exit 0
    fi

    # Initialize system detection and validation
    detect_environment
    detect_distribution
    detect_python_environment
    populate_target_packages
    optimize_build_performance

    # Configure APT sources if requested
    if [[ $do_configure_sources -eq 1 ]]; then
        create_apt_backup

        case "$DIST_ID" in
            ubuntu)
                configure_apt_sources_ubuntu
                ;;
            debian)
                configure_apt_sources_debian
                ;;
            *)
                warn "APT source configuration not implemented for '$DIST_ID'"
                warn "You may need to manually configure multi-arch repositories"
                ;;
        esac
    fi

    # Install system dependencies if requested
    if [[ $do_full_setup -eq 1 ]]; then
        setup_system_dependencies
    fi

    # Verify prerequisites
    check_submodules
    verify_cross_toolchain

    # Call the summary function
    show_build_summary

    # Prepare build environment
    setup_python_venv
    force_clean_all
    extract_python_headers_and_libs
    pregenerate_ant_header

    # Configure and build
    configure_build
    build_project
    verify_build_result

    # Calculate total execution time
    local script_end_time total_time compile_time setup_time
    script_end_time=$(date +%s)
    total_time=$((script_end_time - SCRIPT_START_TIME))
    
    if [[ -n "$COMPILE_START_TIME" && -n "$COMPILE_END_TIME" ]]; then
        compile_time=$((COMPILE_END_TIME - COMPILE_START_TIME))
        setup_time=$((COMPILE_START_TIME - SCRIPT_START_TIME))
    else
        compile_time=0
        setup_time=$total_time
    fi

    # Enhanced success message with comprehensive timing
    echo ""
    echo "🎉 SUCCESS! Cross-compilation completed successfully!"
    echo "=================================================="
    echo "⏱️  TIMING SUMMARY"
    echo "   • Script started:     $BUILD_START_TIME"
    echo "   • Compilation started: $([ $COMPILE_START_TIME -gt 0 ] && date -d "@$COMPILE_START_TIME" || echo "Not started")"
    echo "   • Compilation ended:   $([ $COMPILE_END_TIME -gt 0 ] && date -d "@$COMPILE_END_TIME" || echo "Not completed")"
    echo "   • Script ended:       $(date)"
    echo ""
    echo "   • Setup time:         $(format_duration $setup_time)"
    echo "   • Compilation time:   $(format_duration $compile_time)" 
    echo "   • Total time:         $(format_duration $total_time)"
    echo ""
    echo "📁 ARM64 binary location: $(pwd)/qdomyos-zwift-arm64"
    echo "🔍 Binary details:"
    file "./qdomyos-zwift-arm64" 2>/dev/null | sed 's/^/   /' || echo "   File information unavailable"
    echo ""
    
    # Environment-specific deployment notes
    if [[ $IS_WSL -eq 1 ]]; then
        echo "📝 WSL deployment notes:"
        echo "   1. Binary is ready for ARM64 Linux systems"
        echo "   2. Cannot run directly in WSL (different architecture)"
        echo "   3. Transfer to ARM64 device (Raspberry Pi, ARM servers, etc.)"
    elif [[ $IS_CONTAINER -eq 1 ]]; then
        echo "📝 Container deployment notes:"
        echo "   1. Copy binary from container to host or target device"
        echo "   2. Ensure target device has required runtime libraries"
        echo "   3. Binary is statically linked where possible"
    else
        echo "📝 Deployment notes:"
        echo "   1. Transfer binary to ARM64 target device"
        echo "   2. Install runtime dependencies on target device"
    fi
    
    echo ""
    echo "🚀 Next steps:"
    echo "   1. Transfer qdomyos-zwift-arm64 to your ARM64 target device"
    echo "   2. Make it executable: chmod +x qdomyos-zwift-arm64"
    echo "   3. Install Qt5 runtime libraries on target:"
    echo "      sudo apt-get install qtbase5-dev qtmultimedia5-dev [other qt5 packages]"
    echo "   4. Test execution: ./qdomyos-zwift-arm64 --help"
    echo ""
    echo "🔧 For future builds:"
    echo "   ./cross_compile.sh                    # Quick rebuild"
    echo "   ./cross_compile.sh --clean --full    # Full clean rebuild"
    echo ""
    echo "📊 Build environment summary:"
    echo "   Distribution: $DIST_ID $DIST_VERSION ($DIST_CODENAME)"
    echo "   Environment: $([ $IS_WSL -eq 1 ] && echo "WSL" || ([ $IS_CONTAINER -eq 1 ] && echo "Container" || echo "Native"))"
    echo "   Python version: $PYTHON_VERSION_STR"
    echo "   Build jobs: $(echo $MAKEFLAGS | sed 's/-j//')"
    echo "   ANT+ support: $([ -f .ant_venv_path ] && echo "enabled" || echo "disabled")"
    echo ""
}

# ------------------ Script Execution ------------------
# Ensure we exit cleanly on interruption
trap 'echo -e "\n\n⚠️  Build interrupted by user"; exit 130' INT TERM

# Check for root execution (not recommended but may be needed in containers)
if [[ $EUID -eq 0 ]]; then
    if [[ $IS_CONTAINER -eq 1 ]]; then
        warn "Running as root in container environment"
        warn "This may be necessary for container builds"
    else
        warn "Running as root is not recommended for security reasons"
        warn "This script will use sudo when necessary"
        read -p "Continue anyway? (y/N) " -n 1 -r; echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
fi

# Verify bash version (some advanced features require bash 4+)
if [[ ${BASH_VERSION%%.*} -lt 4 ]]; then
    warn "Bash version ${BASH_VERSION} detected. Bash 4+ recommended for full functionality."
fi

# Run main function with all arguments
main "$@"
