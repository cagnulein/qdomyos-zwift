# =============================================================================
# QDomyos-Zwift: Simplified Docker Build v3.0 (ARM64 / Raspberry Pi)
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
#
# Compiles QDomyos-Zwift for linux/arm64 using system Qt5 (no bundled libraries).
# Target: Raspberry Pi OS Bookworm (Debian 12) with Qt 5.15 and Python 3.11
#
# Build stages:
#   1. Builder: Compiles Qt5/C++ application with embedded Python ANT+ support
#   2. Artifact: Minimal busybox container for file extraction only
#
# Output: /qdomyos-zwift-arm64-ant/ directory with binary and scripts
#         Users must install system Qt5 packages via apt-get
#
# Breaking changes from v2.x:
#   - No bundled Qt5 libraries (removed ~100MB)
#   - No RPATH patching
#   - Requires Raspberry Pi OS Bookworm (Debian 12+) on target system
#   - Wrapper checks OS version and Qt5 availability
#
# Usage:
#   docker buildx build --platform linux/arm64 -f Dockerfile-arm64 .
#   docker buildx build --build-arg ENABLE_DEBUG_LOGS=1 ...  # Debug mode
# =============================================================================

# --- Stage 1: The Builder ---
FROM debian:bookworm AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies (Debian Bookworm Qt 5.15.8)
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    git \
    cmake \
    qtbase5-dev \
    qtbase5-private-dev \
    qtchooser \
    qt5-qmake \
    qtbase5-dev-tools \
    qttools5-dev-tools \
    libqt5svg5-dev \
    qtmultimedia5-dev \
    libqt5charts5-dev \
    qtpositioning5-dev \
    qtconnectivity5-dev \
    libqt5websockets5-dev \
    libqt5texttospeech5-dev \
    libqt5bluetooth5 \
    libqt5networkauth5-dev \
    qml-module-qtlocation \
    qml-module-qtpositioning \
    qtlocation5-dev \
    libqt5quickcontrols2-5 \
    qtquickcontrols2-5-dev \
    qml-module-qtquick-controls2 \
    libqt5sql5-sqlite \
    libqt5sql5-mysql \
    libqt5sql5-psql \
    libsqlite3-dev \
    python3 \
    python3-dev \
    python3-venv \
    python3-pip \
    libusb-1.0-0-dev \
    libudev-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Set up Python virtual environment for ANT+ support (Debian 12 has Python 3.11)
RUN python3 -m venv /opt/ant_venv && \
    /opt/ant_venv/bin/pip install --upgrade pip --quiet && \
    /opt/ant_venv/bin/pip install openant pyusb pybind11 --quiet

# Copy project source
WORKDIR /app
COPY . .

# Generate devices.ini and validate (fail-fast)
RUN set -e; \
    echo "Generating devices.ini from settings.qml..."; \
    /opt/ant_venv/bin/python3 src/devices/antlinux/generate_devices.py src/settings.qml; \
    test -f src/devices.ini || { echo "ERROR: devices.ini not generated"; exit 1; }; \
    test -f src/devices/antlinux/devices_optimized.json || { echo "ERROR: devices_optimized.json not generated"; exit 1; }; \
    echo "Python artifacts validated"

# Initialize needed submodules
RUN git submodule update --init src/smtpclient/ && \
    git submodule update --init src/qmdnsengine/ && \
    git submodule update --init src/qthttpserver/ && \
    git submodule update --init tst/googletest/

# Configure for ANT+ build
RUN echo 'include(devices/antlinux/antlinux.pri)' >> src/qdomyos-zwift.pro && \
    echo '/opt/ant_venv/bin/python3' > src/.ant_venv_path && \
    sed -i '/QtHttpServer/d' src/qdomyos-zwift.pro && \
    find src -type f \( -name '*.cpp' -o -name '*.h' \) -exec sed -i 's/#include <QtHttpServer/\/\/#include <QtHttpServer/' {} + && \
    find src -type f \( -name '*.cpp' -o -name '*.h' \) -exec sed -i 's/QHttpServer/\/\/QHttpServer/' {} +

# Build binary (no bundling, no RPATH modification)
ARG ENABLE_DEBUG_LOGS=0

RUN set -e; \
    cd src; \
    if [ "$ENABLE_DEBUG_LOGS" = "1" ]; then \
        echo "Building with debug logs enabled"; \
        qmake "CONFIG+=debug" "CONFIG+=optimize_full" \
              "DEFINES-=QT_NO_DEBUG" "DEFINES-=QT_NO_DEBUG_OUTPUT" \
              "QMAKE_CXXFLAGS+=-ffile-prefix-map=/app=." qdomyos-zwift.pro; \
        make -j$(nproc); \
    else \
        echo "Building release mode"; \
        qmake "CONFIG+=release" "QMAKE_CXXFLAGS+=-ffile-prefix-map=/app=." qdomyos-zwift.pro; \
        make -j$(nproc); \
        strip qdomyos-zwift; \
    fi; \
    mv qdomyos-zwift qdomyos-zwift-bin; \
    test -f qdomyos-zwift-bin || { echo "ERROR: Binary not created"; exit 1; }; \
    echo "Build complete - binary uses system Qt5"

# --- Stage 2: Minimal Artifact Container ---
FROM busybox:latest

# Create application directory
RUN mkdir -p /qdomyos-zwift-arm64-ant

# Copy binary and scripts only (no bundled libraries/plugins)
COPY --from=builder /app/src/qdomyos-zwift-bin /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/qdomyos-zwift-wrapper.sh /qdomyos-zwift-arm64-ant/qdomyos-zwift
COPY src/devices/antlinux/setup-dashboard.sh /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/bt_provider.py /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/test_ant.py /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/ant_broadcaster.py /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/README.md /qdomyos-zwift-arm64-ant/
COPY --from=builder /app/src/devices.ini /qdomyos-zwift-arm64-ant/devices.ini
COPY --from=builder /app/src/devices/antlinux/devices_optimized.json /qdomyos-zwift-arm64-ant/devices_optimized.json
COPY --from=builder /app/src/devices/antlinux/.menu_cache /qdomyos-zwift-arm64-ant/.menu_cache

RUN chmod +x /qdomyos-zwift-arm64-ant/qdomyos-zwift && \
    chmod +x /qdomyos-zwift-arm64-ant/setup-dashboard.sh || true

CMD ["/bin/sh"]