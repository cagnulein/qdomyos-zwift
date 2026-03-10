# =============================================================================
# QDomyos-Zwift: Multi-Stage Docker Build (ARM64 / Raspberry Pi)
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
#
# Cross-compiles QDomyos-Zwift for linux/arm64 platform (Raspberry Pi 3+).
# Uses Debian Bookworm with pinned Python 3.11.2 for stable ANT+ integration.
#
# Build stages:
#   1. Builder: Compiles Qt5/C++ application with embedded Python ANT+ support
#   2. Artifact: Minimal busybox container for file extraction only
#
# Output: /qdomyos-zwift-arm64-ant/ directory with binary, scripts, configs, and
#         bundled Qt5 libraries for cross-Raspberry-Pi-OS-version compatibility
#
# Usage:
#   docker buildx build --platform linux/arm64 -f docker/rpi-ant/Dockerfile .
#   docker buildx build --build-arg ENABLE_DEBUG_LOGS=1 ...  # Debug mode
# =============================================================================

# --- Stage 1: The Builder ---
FROM debian:bookworm AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies (Debian 12 Bookworm)
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
    python3=3.11.2-1+b1 \
    python3-dev=3.11.2-1+b1 \
    python3-venv=3.11.2-1+b1 \
    python3-pip \
    libusb-1.0-0-dev \
    libudev-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Set up Python virtual environment (Debian 12 uses Python 3.11)
RUN python3 -m venv /opt/ant_venv && \
    /opt/ant_venv/bin/pip install --upgrade pip --quiet && \
    /opt/ant_venv/bin/pip install openant pyusb pybind11 --quiet

# Copy project source
WORKDIR /app
COPY . .

# Generate devices.ini
RUN set -e; \
    echo "Generating devices.ini from settings.qml..."; \
    /opt/ant_venv/bin/python3 src/devices/antlinux/generate_devices.py src/settings.qml; \
    test -f src/devices.ini || { echo "ERROR: devices.ini not generated"; exit 1; }; \
    test -f src/devices/antlinux/devices_optimized.json || { echo "ERROR: devices_optimized.json not generated"; exit 1; }

# Initialize submodules
RUN git submodule update --init src/smtpclient/ && \
    git submodule update --init src/qmdnsengine/ && \
    git submodule update --init src/qthttpserver/ && \
    git submodule update --init tst/googletest/

# Configure for ANT+ build
# Configure for ANT+ build
RUN echo "\ninclude(devices/antlinux/antlinux.pri)\n" >> src/qdomyos-zwift.pro && \
    echo "=========================" && \
    cat src/qdomyos-zwift.pro && \
    echo "=========================" && \
    echo '/opt/ant_venv/bin/python3' > src/.ant_venv_path && \
    sed -i '/QtHttpServer/d' src/qdomyos-zwift.pro && \
    find src -type f \( -name '*.cpp' -o -name '*.h' \) -exec sed -i 's/#include <QtHttpServer/\/\/#include <QtHttpServer/' {} + && \
    find src -type f \( -name '*.cpp' -o -name '*.h' \) -exec sed -i 's/QHttpServer/\/\/QHttpServer/' {} +

# Build binary
ARG ENABLE_DEBUG_LOGS=0

RUN set -e; \
    cd src; \
    if [ "$ENABLE_DEBUG_LOGS" = "1" ]; then \
        qmake "CONFIG+=debug" "CONFIG+=optimize_full" \
              "DEFINES-=QT_NO_DEBUG" "DEFINES-=QT_NO_DEBUG_OUTPUT" \
              "QMAKE_CXXFLAGS+=-ffile-prefix-map=/app=." qdomyos-zwift.pro; \
        make -j$(nproc); \
    else \
        qmake "CONFIG+=release" "QMAKE_CXXFLAGS+=-ffile-prefix-map=/app=." qdomyos-zwift.pro; \
        make -j$(nproc); \
        strip qdomyos-zwift; \
    fi; \
    mv qdomyos-zwift qdomyos-zwift-bin; \
    test -f qdomyos-zwift-bin || { echo "ERROR: Binary not created"; exit 1; }

# --- Stage 2: Artifact Container ---
FROM busybox:latest

# Create application directory structure
RUN mkdir -p /qdomyos-zwift-arm64-ant/qml

# Copy binary
COPY --from=builder /app/src/qdomyos-zwift-bin /qdomyos-zwift-arm64-ant/

# Copy QML files to subdirectory (v3.1.1: cleaner structure)
COPY --from=builder /app/src/*.qml /qdomyos-zwift-arm64-ant/qml/

# Copy scripts
COPY src/devices/antlinux/qdomyos-zwift-wrapper.sh /qdomyos-zwift-arm64-ant/qdomyos-zwift
COPY src/devices/antlinux/setup-dashboard.sh /qdomyos-zwift-arm64-ant/

# Copy Python scripts
COPY src/devices/antlinux/bt_provider.py /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/test_ant.py /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/ant_broadcaster.py /qdomyos-zwift-arm64-ant/

# Copy documentation and config
COPY src/devices/antlinux/README.md /qdomyos-zwift-arm64-ant/
COPY --from=builder /app/src/devices.ini /qdomyos-zwift-arm64-ant/devices.ini
COPY --from=builder /app/src/devices/antlinux/devices_optimized.json /qdomyos-zwift-arm64-ant/devices_optimized.json
COPY --from=builder /app/src/devices/antlinux/.menu_cache /qdomyos-zwift-arm64-ant/.menu_cache

# Set permissions
RUN chmod +x /qdomyos-zwift-arm64-ant/qdomyos-zwift && \
    chmod +x /qdomyos-zwift-arm64-ant/setup-dashboard.sh || true

CMD ["/bin/sh"]