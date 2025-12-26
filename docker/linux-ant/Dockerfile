# =============================================================================
# QDomyos-Zwift: Multi-Stage Docker Build (x86-64 / Desktop Linux)
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
#
# Cross-compiles QDomyos-Zwift for linux/amd64 platform (Ubuntu 20.04+, Debian 11+).
# Uses Debian Bookworm with pinned Python 3.11.2 for stable ANT+ integration.
#
# Build stages:
#   1. Builder: Compiles Qt5/C++ application with embedded Python ANT+ support
#   2. Artifact: Minimal busybox container for file extraction only
#
# Output: /qdomyos-zwift-x86-64-ant/ directory with binary, scripts, and configs
#
# Usage:
#   docker buildx build --platform linux/amd64 -f docker/linux-ant/Dockerfile .
#   docker buildx build --build-arg ENABLE_DEBUG_LOGS=1 ...  # Debug mode
# =============================================================================

# --- Stage 1: The Builder ---
FROM debian:bookworm AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install all necessary build dependencies
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
    python3-pip \
    python3=3.11.2-1+b1 \
    python3-dev=3.11.2-1+b1 \
    python3-venv=3.11.2-1+b1 \
    libusb-1.0-0-dev \
    libudev-dev

# Set up the Python virtual environment for ANT+ support
RUN python3.11 -m venv /opt/ant_venv && \
    /opt/ant_venv/bin/pip install --upgrade pip --quiet && \
    /opt/ant_venv/bin/pip install openant pyusb pybind11 --quiet

# Copy the entire project source into the container
WORKDIR /app
COPY . .

# Generate devices.ini and validate immediately (fail-fast)
RUN set -e; \
    echo "Generating devices.ini from settings.qml..."; \
    cd /app; \
    /opt/ant_venv/bin/python3 src/devices/antlinux/generate_devices.py src/settings.qml; \
    test -f src/devices.ini || { echo "ERROR: devices.ini not generated"; exit 1; }; \
    test -f src/devices/antlinux/devices_optimized.json || { echo "ERROR: devices_optimized.json not generated"; exit 1; }; \
    test -d src/devices/antlinux/.menu_cache || { echo "ERROR: .menu_cache not created"; exit 1; }; \
    echo "Python artifacts validated"

# Initialize only the needed submodules (avoid full recursive fetch)
RUN git submodule update --init src/smtpclient/ && \
    git submodule update --init src/qmdnsengine/ && \
    git submodule update --init src/qthttpserver/ && \
    git submodule update --init tst/googletest/

# Configure the project for an ANT+ build
RUN echo 'include(devices/antlinux/antlinux.pri)' >> src/qdomyos-zwift.pro && \
    echo '/opt/ant_venv/bin/python3' > src/.ant_venv_path && \
    sed -i '/QtHttpServer/d' src/qdomyos-zwift.pro && \
    find src -type f \( -name '*.cpp' -o -name '*.h' \) -exec sed -i 's/#include <QtHttpServer/\/\/#include <QtHttpServer/' {} + && \
    find src -type f \( -name '*.cpp' -o -name '*.h' \) -exec sed -i 's/QHttpServer/\/\/QHttpServer/' {} +

# Build and validate binary immediately (fail-fast)
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
    echo "Build complete"

# --- Stage 2: Minimal Artifact Container ---
FROM busybox:latest

# Create the application directory structure
RUN mkdir -p /qdomyos-zwift-x86-64-ant

# Copy artifacts - now using unified wrapper from src/devices/antlinux/
COPY --from=builder /app/src/qdomyos-zwift-bin /qdomyos-zwift-x86-64-ant/
COPY src/devices/antlinux/qdomyos-zwift-wrapper.sh /qdomyos-zwift-x86-64-ant/qdomyos-zwift
COPY src/devices/antlinux/setup-dashboard.sh /qdomyos-zwift-x86-64-ant/
COPY src/devices/antlinux/bt_provider.py /qdomyos-zwift-x86-64-ant/
COPY src/devices/antlinux/test_ant.py /qdomyos-zwift-x86-64-ant/
COPY src/devices/antlinux/ant_broadcaster.py /qdomyos-zwift-x86-64-ant/
COPY src/devices/antlinux/README.md /qdomyos-zwift-x86-64-ant/
COPY --from=builder /app/src/devices.ini /qdomyos-zwift-x86-64-ant/devices.ini
COPY --from=builder /app/src/devices/antlinux/devices_optimized.json /qdomyos-zwift-x86-64-ant/devices_optimized.json
COPY --from=builder /app/src/devices/antlinux/.menu_cache /qdomyos-zwift-x86-64-ant/.menu_cache

RUN chmod +x /qdomyos-zwift-x86-64-ant/qdomyos-zwift && \
    chmod +x /qdomyos-zwift-x86-64-ant/setup-dashboard.sh || true

CMD ["/bin/sh"]